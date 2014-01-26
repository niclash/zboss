/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: ZDO network management functions, client side
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_hash.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"

#include "zb_bank_a.h"

/*! \addtogroup ZB_ZDO */
/*! @{ */

#ifndef ZB_LIMITED_FEATURES

void zb_zdo_channel_change_timer_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_long_timer_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_start_long_timer(zb_callback_t func, zb_uint8_t param, zb_uint8_t long_timer);

void zb_zdo_check_fails(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
#ifndef ZB_ED_ROLE
  TRACE_MSG(TRACE_MAC2, ">> zb_zdo_check_fails tx total %d fail %d", (FMT__D_D, ZB_NIB_NWK_TX_TOTAL(), ZB_NIB_NWK_TX_FAIL()));

  if (ZB_NIB_NWK_TX_TOTAL() > ZB_TX_TOTAL_THRESHOLD && !ZB_ZDO_GET_CHECK_FAILS())
  {
    if (ZB_NIB_NWK_TX_FAIL() > (ZB_NIB_NWK_TX_TOTAL() / ZB_FAILS_PERCENTAGE))
    {
      ZB_ZDO_SET_CHECK_FAILS();
      ZB_SCHEDULE_ALARM(zb_zdo_channel_check_timer_cb, 0, ZB_ZDO_CHANNEL_CHECK_TIMEOUT);

      /* TODO: KLUDGE: this zb_start_ed_scan call can conflict with call in zdo_nwk_manage_srv.c */
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_count = 1;
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_channels = ZB_DEFAULT_APS_CHANNEL_MASK;
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_duration = ZB_DEFAULT_SCAN_DURATION;
      ZG->zdo.zdo_ctx.nwk_upd_req.tsn = 0;
      ZG->zdo.zdo_ctx.nwk_upd_req.dst_addr = 0;
      ZB_APS_SET_CHANNEL_MANAGER_ED_SCAN_FLAG();

#ifndef ZB_LIMITED_FEATURES
      ZB_GET_OUT_BUF_DELAYED(zb_start_ed_scan);
#endif
    }
  }
#endif
  TRACE_MSG(TRACE_MAC2, "<< zb_zdo_check_fails", (FMT__0));
}

void zb_zdo_channel_check_timer_cb(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC2, "channel_check_timer_cb", (FMT__0));
  ZB_ZDO_CLEAR_CHECK_FAILS(); /* clear flag to allow next channel interference resolution */
}

#ifndef ZB_LIMITED_FEATURES
void zb_nlme_ed_scan_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_ed_scan_confirm %p", (FMT__P, buf));

  if ( buf )
  {
    if (ZB_APS_GET_ZDO_ED_SCAN_FLAG())
    {
      zb_mac_scan_confirm_t confirm;
      zb_mac_scan_confirm_t *confirm_param = ZB_GET_BUF_PARAM(buf, zb_mac_scan_confirm_t);
      zb_zdo_mgmt_nwk_update_notify_param_t *notify_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_notify_param_t);
      zb_uint32_t channel_mask;
      zb_uint32_t channel_mask_mul = 0x800; /* channel mul start from first channel in mask */
	  zb_uint8_t i;

      ZB_MEMCPY(&confirm, confirm_param, sizeof(zb_mac_scan_confirm_t));
      notify_param->hdr.status = confirm.status;
      /* scan_channels - bit mask of channels to scan;
         unscanned_channels - bit mask of unscanned channels */
      notify_param->hdr.scanned_channels = ZG->zdo.zdo_ctx.nwk_upd_req.scan_channels & ~confirm.unscanned_channels;

      notify_param->hdr.total_transmissions = ZB_NIB_NWK_TX_TOTAL();
      notify_param->hdr.transmission_failures = ZB_NIB_NWK_TX_FAIL();

      channel_mask = notify_param->hdr.scanned_channels;
      notify_param->hdr.scanned_channels_list_count = 0;
      notify_param->tsn = ZG->zdo.zdo_ctx.nwk_upd_req.tsn; /* value was save as context data */
      notify_param->dst_addr = ZG->zdo.zdo_ctx.nwk_upd_req.dst_addr; /* value was save as context data */

      /* for upd notify ED values are stored consecutivly in the array
       * only for channels that were really scanned; scanned channels list is
       * stored channel bit mask in scanned_chennels field */
      for(i = ZB_MAC_START_CHANNEL_NUMBER; i <= ZB_MAC_MAX_CHANNEL_NUMBER; i++)
      {        
		if (channel_mask & (channel_mask_mul))
        {
		  channel_mask_mul*=2;
          TRACE_MSG(TRACE_NWK1, "chan num %hd, item %hd, energy %hd",
                    (FMT__H_H_H, i, notify_param->hdr.scanned_channels_list_count,
                     confirm.list.energy_detect[i - ZB_MAC_START_CHANNEL_NUMBER]));
          notify_param->energy_values[notify_param->hdr.scanned_channels_list_count++] =
            confirm.list.energy_detect[i - ZB_MAC_START_CHANNEL_NUMBER];
          ZB_ASSERT(notify_param->hdr.scanned_channels_list_count <= ZB_MAC_SUPPORTED_CHANNELS);
        }
      }

      ZB_SCHEDULE_CALLBACK(zb_zdo_nwk_upd_notify, param);
    }
    else if (ZB_APS_GET_CHANNEL_MANAGER_ED_SCAN_FLAG())
    {
      ZB_SCHEDULE_CALLBACK(zb_zdo_channel_check_scan_result, param);
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_ed_scan_confirm", (FMT__0));
}


void zb_zdo_channel_check_scan_result(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_mac_scan_confirm_t *scan_result = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_scan_confirm_t);
  zb_bool_t free_buf = ZB_TRUE;

  TRACE_MSG(TRACE_MAC2, ">> zb_zdo_channel_check_scan_result %d", (FMT__D, param));

  ZB_APS_CLEAR_CHANNEL_MANAGER_ED_SCAN_FLAG();

  TRACE_MSG(TRACE_MAC3, "status %hd, unscanned %x %x", (FMT__H_D_D, scan_result->status,
    *((zb_uint16_t*)&scan_result->unscanned_channels), *((zb_uint16_t*)&scan_result->unscanned_channels + 1)));
  if ( scan_result->status == MAC_SUCCESS &&
       !(scan_result->unscanned_channels & (1l << ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL())) )
  {
    TRACE_MSG(TRACE_MAC3, "cur chan %hd, cur energy %hd",
      (FMT__H_H, ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL(),
       scan_result->list.energy_detect[ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL() - ZB_MAC_START_CHANNEL_NUMBER]));

    if (scan_result->list.energy_detect[ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL() - ZB_MAC_START_CHANNEL_NUMBER] > ZB_CHANNEL_BUSY_ED_VALUE)
    {
      /* send Mgmt_NWK_Update_notify to channel manager */
      ZB_APS_SET_ZDO_ED_SCAN_FLAG();
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_count = 0;
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_channels = ZB_TRANSCEIVER_ALL_CHANNELS_MASK;
      ZG->zdo.zdo_ctx.nwk_upd_req.tsn = 0; /* not used */
      ZG->zdo.zdo_ctx.nwk_upd_req.dst_addr = ZB_NIB_NWK_MANAGER_ADDR();
      ZB_ZDO_SET_SEND_WITH_ACK();

      /* call zb_nlme_ed_scan_confirm(), it will send zb_zdo_nwk_upd_notify() */
      zb_nlme_ed_scan_confirm(param);
      free_buf = ZB_FALSE;
    }
  }
  if (free_buf)
  {
    zb_free_buf(buf);
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_zdo_channel_check_scan_result", (FMT__0));
}
#endif
void zb_zdo_channel_check_finish_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  if (buf->u.hdr.status == RET_OK)
  {
    //finish channel check FFD side
    ZB_NIB_NWK_TX_TOTAL() = 0;
    ZB_NIB_NWK_TX_FAIL() = 0;
  }
}

/* Performs channel change procedure, server side */
void zdo_change_channel(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *body = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_nwk_update_notify_hdr_t *upd_notify = (zb_zdo_mgmt_nwk_update_notify_hdr_t*)body;
  zb_uint8_t *scan_result;
  zb_uint8_t min_energy = 0xff;
  zb_uint8_t current_energy = 0;
  zb_uint8_t new_channel_num = 0;
  zb_uint8_t i;
  zb_bool_t free_buffer = ZB_TRUE;

  TRACE_MSG(TRACE_MAC2, ">> zdo_change_channel %hd", (FMT__H, param));

  TRACE_MSG(TRACE_MAC3, "ch chaged %hd, status %hd, count %hd",
            (FMT__H_H_H, ZB_ZDO_GET_CHANNEL_CHANGED(), upd_notify->status, upd_notify->scanned_channels_list_count));
  if (!ZB_ZDO_GET_CHANNEL_CHANGED())
  {
    body += sizeof(zb_zdo_mgmt_nwk_update_notify_hdr_t);
    if (upd_notify->status == ZB_ZDP_STATUS_SUCCESS && upd_notify->scanned_channels_list_count)
    {
      scan_result = body;
      /* parse scan results */
      for (i = ZB_MAC_START_CHANNEL_NUMBER; i <= ZB_MAC_MAX_CHANNEL_NUMBER; i++)
      {
        TRACE_MSG(TRACE_MAC3, "i %hd", (FMT__H, i));
        if (upd_notify->scanned_channels & (1l << i))
        {
          TRACE_MSG(TRACE_MAC3, "scan_result %hd", (FMT__H, *scan_result));
          if (*scan_result < min_energy)
          {
            new_channel_num = i;
            min_energy = *scan_result;
          }
          if (i == ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL())
          {
            current_energy = *scan_result;
          }
          scan_result++;
        }
      }
      TRACE_MSG(TRACE_MAC3, "current_energy %hd, min_energy %hd, new_channel %hd",
                (FMT__H_H_H, current_energy, min_energy, new_channel_num));
      if (current_energy > ZB_CHANNEL_BUSY_ED_VALUE)
      {
        if (new_channel_num && min_energy < ZB_CHANNEL_FREE_ED_VALUE)
        {
          zb_zdo_mgmt_nwk_update_req_t *req;

          //will change channel

          free_buffer = ZB_FALSE;
          req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_req_t);

          ZB_NIB_UPDATE_ID()++;
          req->hdr.scan_channels = new_channel_num;
          req->hdr.scan_duration = ZB_ZDO_NEW_ACTIVE_CHANNEL;
          req->update_id = ZB_NIB_UPDATE_ID();
          req->dst_addr = ZB_NWK_BROADCAST_ROUTER_COORDINATOR;

#ifndef ZB_LIMITED_FEATURES
          zb_zdo_mgmt_nwk_update_req(param, NULL);
#endif
          ZB_ZDO_SET_CHANNEL_CHANGED();
          zb_zdo_start_long_timer(zb_zdo_channel_change_timer_cb, 0, ZB_ZDO_APS_CHANEL_TIMER);
        }
      }
    }
  }
  if (free_buffer)
  {
    zb_free_buf(buf);
  }

  TRACE_MSG(TRACE_MAC2, "<< zdo_change_channel", (FMT__0));
}


void zb_zdo_start_long_timer(zb_callback_t func, zb_uint8_t param, zb_uint8_t long_timer)
{
  ZB_ASSERT(ZG->zdo.long_timer_cb == NULL); /* long timer should be free */
  ZG->zdo.long_timer = long_timer;
  ZG->zdo.long_timer_cb = func;
  ZG->zdo.long_timer_param = param;
  ZB_SCHEDULE_ALARM(zb_zdo_long_timer_cb, 0, ZB_ZDO_1_MIN_TIMEOUT);
}


void zb_zdo_long_timer_cb(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  ZG->zdo.long_timer--;
  TRACE_MSG(TRACE_MAC3, "long_timer_cb timer val %hd", (FMT__H, ZG->zdo.long_timer));
  if (ZG->zdo.long_timer)
  {
    ZB_SCHEDULE_ALARM(zb_zdo_long_timer_cb, 0, ZB_ZDO_1_MIN_TIMEOUT);
  }
  else
  {
    ZB_SCHEDULE_CALLBACK(ZG->zdo.long_timer_cb, ZG->zdo.long_timer_param);
    ZG->zdo.long_timer_cb = NULL;
  }
}

/* clear channel_changed flag */
void zb_zdo_channel_change_timer_cb(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC1, "zb_zdo_channel_change_timer_cb", (FMT__0));
  ZB_ZDO_CLEAR_CHANNEL_CHANGED();
}

#endif  /* ZB_LIMITED_FEATURES */

/*! @} */
