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
PURPOSE: Roitines specific to mlme scan
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"

#include "zb_bank_2.h"

/*! \addtogroup ZB_MAC */
/*! @{ */



void zb_handle_scan_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  zb_uint8_t scan_type;

  TRACE_MSG(TRACE_MAC2, ">> zb_handle_scan_request", (FMT__0));

  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
  scan_type = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t)->scan_type;

  switch (scan_type)
  {
    case ACTIVE_SCAN:
      ret = zb_mlme_active_scan();
      break;
#ifndef ZB_LIMITED_FEATURES
    case ED_SCAN:
      ret = zb_mlme_ed_scan();
      MAC_CTX().ed_scan_step_passed = 0;
      break;
    case ORPHAN_SCAN:
      ret = zb_mlme_orphan_scan();
      break;
#endif
    default:
      ret = RET_NOT_IMPLEMENTED;
      TRACE_MSG(TRACE_MAC3, "bad scan type %hd", (FMT__H, scan_type));
      MAC_CTX().mac_status = MAC_INVALID_PARAMETER;
      break;
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_handle_scan_request i", (FMT__D, ret));
}


/* 7.1.11.1 MLME-SCAN.request */
void zb_mlme_scan_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_mlme_scan_params_t *params;
  zb_uint8_t scan_type;
  zb_uint8_t handle_scan_called = 0;

  TRACE_MSG(TRACE_MAC2, ">> zb_mlme_scan_request %hd", (FMT__H, param));

  params = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_scan_params_t);
  ZB_ASSERT(params);
  MAC_CTX().mac_status = MAC_SUCCESS;
  scan_type = params->scan_type;

  if (params->scan_duration > ZB_MAX_SCAN_DURATION_VALUE && scan_type != ORPHAN_SCAN)
  {
    ret = RET_ERROR;
    MAC_CTX().mac_status = MAC_INVALID_PARAMETER;
  }

  if ((ret == RET_OK)&&(!MAC_CTX().mlme_scan_in_progress))
  {
    /* process request immediately*/
    MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);

    ZB_SCHEDULE_CALLBACK(zb_handle_scan_request, param);
    handle_scan_called = 1;
  }
  if (!handle_scan_called)
  {
    zb_mac_scan_confirm_t *scan_confirm;

    scan_confirm = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_scan_confirm_t);
    scan_confirm->status = (ret == RET_OK) ? MAC_SUCCESS :
      MAC_CTX().mac_status != MAC_SUCCESS ? MAC_CTX().mac_status : MAC_INVALID_PARAMETER;
    scan_confirm->scan_type = scan_type;
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }

  TRACE_MSG(TRACE_MAC2, "<< zb_mlme_scan_request", (FMT__0));
}


#ifndef ZB_LIMITED_FEATURES
zb_ret_t zb_mlme_ed_scan()
{
  zb_ret_t ret = RET_OK;
  zb_mac_scan_confirm_t *scan_confirm;

/*
  mac spec 7.5.2.1.1 ED channel scan
  - discard all frames received over the PHY data service (UBEC stack accepts only beacon frames)
  - check one-by-one all logical channels, if it is specified in the requested
  channel mask, switch to this channel, set phyCurrentChannel = new_channel_number;
  phyCurrentPage = 0 alwayes for ZB
  - perform ED measurement for current channel during [(aBaseSuperframeDuration * (2^n + 1)) symbols] time.
  - save maximum ED value to confirmation buffer
  - perform scan confirm on procedure finish
*/
  TRACE_MSG(TRACE_MAC1, ">> zb_mlme_ed_scan", (FMT__0));
  {
    zb_mlme_scan_params_t *scan_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t);
    MAC_CTX().unscanned_channels = scan_params->channels;
    /* timeout is calculated in beacon intervals */
    MAC_CTX().rt_ctx.ed_scan.scan_timeout = (1l << scan_params->scan_duration) + 1;
  }
  ZB_BUF_REUSE(MAC_CTX().pending_buf);
  scan_confirm = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_scan_confirm_t);
  ZB_ASSERT(scan_confirm);
  ZB_BZERO(scan_confirm, sizeof(zb_mac_scan_confirm_t));
  scan_confirm->unscanned_channels = MAC_CTX().unscanned_channels;

#ifndef ZB_NS_BUILD

  MAC_CTX().rt_ctx.ed_scan.channel_number = ZB_MAC_START_CHANNEL_NUMBER;
  MAC_CTX().rt_ctx.ed_scan.save_channel = MAC_CTX().current_channel;
  MAC_CTX().rt_ctx.ed_scan.max_rssi_value = 0;
  ret = ZB_SCHEDULE_ALARM(zb_mlme_scan_step, 0, MAC_CTX().rt_ctx.ed_scan.scan_timeout);

#else  /* ZB_NS_BUILD */
  ZB_BZERO(&scan_confirm->list.energy_detect[0], sizeof(scan_confirm->list.energy_detect));
  scan_confirm->result_list_size = ZB_MAC_SUPPORTED_CHANNELS;
  ret = ZB_SCHEDULE_CALLBACK(zb_mlme_scan_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
#ifdef ZB_CHANNEL_ERROR_TEST
  /* channel interference test, show energy on current channel */
  TRACE_MSG(TRACE_MAC3, "ch_err_test %hd, logical_channel %hd, ch index %hd", (FMT__H_H_H,
                                                                               ZB_MAC_GET_CHANNEL_ERROR_TEST(), ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL(),
                                                                               ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL() - ZB_MAC_START_CHANNEL_NUMBER));

  if (ZB_MAC_GET_CHANNEL_ERROR_TEST())
  {
    scan_confirm->list.energy_detect[ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL() - ZB_MAC_START_CHANNEL_NUMBER] =
      ZB_CHANNEL_BUSY_ED_VALUE + 1;
  }
#endif
#endif /* ZB_NS_BUILD */

  TRACE_MSG(TRACE_MAC1, "<< zb_mlme_ed_scan, ret %i", (FMT__D, ret));
  return ret;
}
#endif

/* this is a universal routine for ed/active/orphan scans */
void zb_mlme_scan_step(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t channel_number;
  zb_mlme_scan_params_t *scan_params;
  zb_ret_t ret = RET_OK;
  zb_uint16_t timeout;
  zb_uint32_t *unscanned_channels;

  TRACE_MSG(TRACE_MAC1, ">> zb_mlme_scan_step", (FMT__0));
  ZVUNUSED(param);
  channel_number = ZB_MAC_START_CHANNEL_NUMBER;
  scan_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t);
  /* Table 2.80   Fields of the Mgmt_NWK_Disc_req Command (the other scans
     requests are using the same parameters
     A value used to calculate the length
     of time to spend scanning each
     channel. The time spent scanning
     each channel is
     (aBaseSuperframeDuration * (2^n +
     1)) symbols, where n is the value of
     the ScanDuration parameter. */
  timeout = (1l << scan_params->scan_duration) + 1;
  MAC_CTX().mlme_scan_in_progress = 1;
  unscanned_channels = &MAC_CTX().unscanned_channels;
  for (;channel_number < ZB_MAC_START_CHANNEL_NUMBER + ZB_MAC_MAX_CHANNEL_NUMBER;channel_number++)
  {
    if (*unscanned_channels & 1l<<channel_number)
    {
      if (scan_params->scan_type == ED_SCAN)
      {
        if (MAC_CTX().ed_scan_step_passed)
        {
#ifndef ZB_NS_BUILD
          zb_uint8_t rssi_value;
#endif
          /* 11/23/11 CR:1855:MAJOR not sure: is it ok to get
             rssi only once per channel? */
          /* 11/23/11 CR:1855:MAJOR:DISCUSS Seems like it's ok.
             We're using RSSI mode 1. Check zb_transceiver_get_rssi()
             implementation for details. But I've moved it, and now
             rssi is got right before next channel switching.
          */
#ifndef ZB_NS_BUILD
          zb_transceiver_get_rssi(&rssi_value);
          if (rssi_value > MAC_CTX().rt_ctx.ed_scan.max_rssi_value)
          {
            MAC_CTX().rt_ctx.ed_scan.max_rssi_value = rssi_value;
          }
#endif
        }
        else
        {
          MAC_CTX().ed_scan_step_passed = 1;
        }

        ret = ZB_SCHEDULE_ALARM(zb_mlme_scan_step, 0, timeout);
      }
      TRACE_MSG(TRACE_MAC2, "set channel %hd", (FMT__H, channel_number));
      ZB_TRANSCEIVER_SET_CHANNEL(channel_number);
      *unscanned_channels &=~(1l<<channel_number);
      if (scan_params->scan_type == ACTIVE_SCAN)
      {
        ret = zb_beacon_request_command();
        if (ret == RET_OK)
        {
          /* check beacon request TX status */
          /* There's nothing to do during active scan, so, synchronous */
          ret = zb_check_cmd_tx_status();
        }
      }
      else if (scan_params->scan_type == ORPHAN_SCAN)
      {
        ret = zb_orphan_notification_command();
        ZB_WAIT_FOR_TX();
      }
      ZB_SCHEDULE_ALARM_CANCEL(zb_mlme_scan_step, 0);
      ret = ZB_SCHEDULE_ALARM(zb_mlme_scan_step, 0, timeout);
      break;
    }
  }
  if (channel_number == (ZB_MAC_START_CHANNEL_NUMBER + ZB_MAC_MAX_CHANNEL_NUMBER))
  {
    zb_mac_scan_confirm_t *scan_confirm;
#ifdef ZB_MAC_TESTING_MODE
    zb_buf_t *desc_list_buf;
    zb_uint8_t desc_count = 0;
    if (scan_type == ACTIVE_SCAN)
    {
      /* copy list of pan descriptors if it was formed. macAutoRequest case */
      if (MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param != ZB_UNDEFINED_BUFFER)
      {
        desc_list_buf = ZB_BUF_FROM_REF(MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param);
        ZB_BUF_COPY(MAC_CTX().pending_buf, desc_list_buf);
        desc_count = ZB_BUF_LEN(desc_list_buf) / sizeof(zb_pan_descriptor_t);
        TRACE_MSG(TRACE_MAC3, "copied %hd pan desc", (FMT__H, desc_count));
        zb_free_buf(desc_list_buf);
        MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param = ZB_UNDEFINED_BUFFER;
      }
    }
#endif /* ZB_MAC_TESTING_MODE */
    if (scan_params->scan_type == ED_SCAN)
    {
      TRACE_MSG(TRACE_MAC1, "restoring original channel after ed_scan", (FMT__0));
      ZB_TRANSCEIVER_SET_CHANNEL(MAC_CTX().rt_ctx.ed_scan.save_channel);
    }
    /* There's no need to restore channel
       after active or orphan scan, because we will choose
       new channel, according to scan results.
    */
    /* ZB_TRANSCEIVER_SET_CHANNEL(MAC_CTX().rt_ctx.ed_scan.save_channel);*/
    scan_confirm = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_scan_confirm_t);
    TRACE_MSG(TRACE_MAC3, "beacon found %hd", (FMT__H, MAC_CTX().rt_ctx.active_scan.beacon_found));
    if (scan_params->scan_type == ED_SCAN || MAC_CTX().rt_ctx.active_scan.beacon_found
        || MAC_CTX().rt_ctx.orphan_scan.got_realignment)
    {
      scan_confirm->status = MAC_SUCCESS;
      /* Q: do we really need to zero here?  What about got_realignment?
       *
       * A: I think yes, because it is the only indication
       * for NO_BEACON status, that will not affect ED or ORPHAN scans. ED just
       * doesn't need any packets, and ORPHAN needs a realignment command that
       * is processed in appropriate function */
      MAC_CTX().rt_ctx.active_scan.beacon_found = 0;
    }
    else
    {
      scan_confirm->status = MAC_NO_BEACON;
    }
#ifdef ZB_MAC_TESTING_MODE
    {
      scan_confirm->result_list_size = desc_count;
    }
#endif
    scan_confirm->scan_type = scan_params->scan_type;
    MAC_CTX().mlme_scan_in_progress = 0;
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }
  TRACE_MSG(TRACE_MAC1, "<< zb_mlme_scan_step", (FMT__0));
}

zb_ret_t zb_mlme_active_scan() ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mlme_scan_params_t *scan_params;
  zb_uint8_t channel_number;

/*
  mac spec 7.5.2.1.2 Active channel scan
  - set macPANId to 0xffff in order to accept all incoming beacons
  - switch to next channel
  - send beacon request, mac spec 7.3.7 Beacon request command
  - enable receiver for [aBaseSuperframeDuration * (2^n + 1)] symbols
  == (2^n + 1)Beacon_Intervals, n == request.ScanDuration; accept only beacon frames
  - use mode macAutoRequest == FALSE: send each beacon to the higher
  layer using MLME-BEACON-NOTIFY indication. Beacon frame can contain payload
  - if frame_control.Security Enabled == 1, unsecure the beacon frame (mac spec 7.5.8.2.3)      --- not supported now
  - if at least 1 beacon request was successfully sent but no beacons were found, set status NO_BEACON
*/

  TRACE_MSG(TRACE_MAC1, ">> zb_mlme_active_scan", (FMT__0));

  scan_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t);
  ZB_ASSERT(scan_params);

  TRACE_MSG(TRACE_MAC1, "idle state, set beacon found == 0", (FMT__0));
  MAC_CTX().rt_ctx.active_scan.beacon_found = 0;
#ifdef ZB_MAC_TESTING_MODE
  MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param = ZB_UNDEFINED_BUFFER;
#endif
  channel_number = ZB_MAC_START_CHANNEL_NUMBER;

  TRACE_MSG(TRACE_MAC3, "set beacon mode ret %d, param channels 0x%x", (FMT__D_D, ret, scan_params->channels));
  MAC_CTX().unscanned_channels = scan_params->channels;

#ifdef ZB_MAC_TESTING_MODE
  if (MAC_CTX().rt_ctx.active_scan.stop_scan)
  {
    ZB_SCHEDULE_ALARM_CANCEL(zb_mac_scan_timeout, 0);
    ret = RET_OK;
    break;
  }
#endif
  TRACE_MSG(TRACE_MAC2, "chan mask %x %x , chan %hd",
            (FMT__D_D_H, ((zb_uint16_t*)&scan_params->channels)[0], ((zb_uint16_t*)&scan_params->channels)[1],
             channel_number));
  ZB_SCHEDULE_CALLBACK(zb_mlme_scan_step,0);
  TRACE_MSG(TRACE_MAC1, "<< zb_mlme_active_scan, ret %i", (FMT__D, ret));
  return ret;
}


#ifndef ZB_LIMITED_FEATURES
zb_ret_t zb_mlme_orphan_scan() ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mlme_scan_params_t *scan_params;

  /* 7.5.2.1.4
     - discard all frames except realignment command frames
     - switch to the channel
     - send an orphan notification command
     - enable receiver for at most macResponseWaitTime symbols and waits for
     coordinator realignment command
     - if realignment command received - end, otherwise set next channel
  */

  TRACE_MSG(TRACE_MAC1, ">> mlme_orphan_scan", (FMT__0));
  /* clear answer flag */
  scan_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t);
  MAC_CTX().unscanned_channels = scan_params->channels;

  MAC_CTX().rt_ctx.orphan_scan.got_realignment = 0;

  /* TODO: Configiure transiver to accept only realignment commands */

  ret = ZB_SCHEDULE_ALARM(zb_mlme_scan_step, 0, ZB_MAC_PIB_RESPONSE_WAIT_TIME);

  TRACE_MSG(TRACE_MAC1, "<< mlme_orphan_scan %i", (FMT__D, ret));
  return ret;
}


/*
  7.3.6 sends orphan notification command
  return RET_OK, RET_ERROR
*/
zb_ret_t zb_orphan_notification_command()
{

  zb_ret_t ret;
  zb_uint8_t mhr_len;
  zb_uint8_t *ptr = NULL;
  zb_mac_mhr_t mhr;

/*
  Orphan notification command
  1. Fill MHR fields
  - set dst pan id = 0xffff
  - set dst addr = 0xffff
  2. Fill FCF
  - set frame pending = 0, ack req = 0, security enabled = 0
  - set dst addr mode to ZB_ADDR_16BIT_DEV_OR_BROADCAST
  - set src addr mode to ZB_ADDR_64BIT_DEV
  3. Set command frame id = 0x07 (Beacon request)
*/

  TRACE_MSG(TRACE_MAC2, ">>orphan_notif_cmd", (FMT__0));

  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_64BIT_DEV, ZB_ADDR_16BIT_DEV_OR_BROADCAST, 1);
  {
    zb_uint8_t packet_length = mhr_len + 1;
    ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
    ZB_ASSERT(ptr);
    ZB_BZERO(ptr, packet_length);
  }

  /* TODO: optimize FC fill */
  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);

  /* 7.2.1 General MAC frame format */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();

  mhr.dst_pan_id = ZB_BROADCAST_PAN_ID;
  mhr.dst_addr.addr_short = ZB_MAC_SHORT_ADDR_NO_VALUE;
  ZB_IEEE_ADDR_COPY(mhr.src_addr.addr_long, ZB_PIB_EXTENDED_ADDRESS());

  zb_mac_fill_mhr(ptr, &mhr);
  *(ptr + mhr_len) = MAC_CMD_ORPHAN_NOTIFICATION;
  MAC_ADD_FCS(MAC_CTX().operation_buf);

  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  TRACE_MSG(TRACE_MAC2, "<<orphan_notif_cmd %hd", (FMT__H, ret));
  return ret;
}
#endif  /* ZB_LIMITED_FEATURES */

#ifdef ZB_MAC_TESTING_MODE
void zb_mac_store_pan_desc(zb_buf_t *beacon_buf)
{
  zb_uint8_t *mac_hdr = ZB_MAC_GET_FCF_PTR(ZB_BUF_BEGIN(beacon_buf));
  zb_mac_mhr_t mhr;
  zb_uint8_t mhr_len;
  zb_pan_descriptor_t pan_desc;
  zb_pan_descriptor_t *pan_desc_buf;
  zb_buf_t *desc_list_buf;
  zb_uint8_t desc_count;

  TRACE_MSG(TRACE_NWK1, ">>store_pan_desc %p", (FMT__P, beacon_buf));

  mhr_len = zb_parse_mhr(&mhr, mac_hdr);

  TRACE_MSG(TRACE_NWK3, "add pan desc", (FMT__0));
  pan_desc.coord_addr_mode = ZB_FCF_GET_SRC_ADDRESSING_MODE(&mhr.frame_control);
  pan_desc.coord_pan_id = mhr.src_pan_id;
  ZB_MEMCPY(&pan_desc.coord_address, &mhr.src_addr, sizeof(union zb_addr_u));
  pan_desc.logical_channel = ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL();


  ZB_GET_SUPERFRAME(mac_hdr, mhr_len, &pan_desc.super_frame_spec);

  pan_desc.gts_permit = 0; /* use ZB_MAC_GET_GTS_FIELDS() to get exact gts value.
                              Zigbee uses beaconless mode, so gts is not used always */

  pan_desc.link_quality = ZB_MAC_GET_LQI(beacon_buf);

  if (MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param == ZB_UNDEFINED_BUFFER)
  {
    desc_list_buf = beacon_buf;
    ZB_BUF_REUSE(desc_list_buf);
    MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param = ZB_REF_FROM_BUF(beacon_buf);
  }
  else
  {
    desc_list_buf = ZB_BUF_FROM_REF(MAC_CTX().rt_ctx.active_scan.pan_desc_buf_param);
  }

  /* do not calculate pan descriptors number - it can be calculated using buffer length  */
  /* in this check take into account size of scan confirm structure - descriptors will follow it */
  desc_count = ZB_BUF_LEN(desc_list_buf) / sizeof(zb_pan_descriptor_t);

  if ( (ZB_BUF_GET_FREE_SIZE(desc_list_buf) >= (sizeof(zb_pan_descriptor_t) + sizeof(zb_mac_scan_confirm_t))) &&
       desc_count < ZB_ACTIVE_SCAN_MAX_PAN_DESC_COUNT)
  {
    ZB_BUF_ALLOC_RIGHT(desc_list_buf, sizeof(zb_pan_descriptor_t), pan_desc_buf);
    ZB_MEMCPY(pan_desc_buf, &pan_desc, sizeof(zb_pan_descriptor_t));
  }
  else
  {
    TRACE_MSG(TRACE_NWK3, "stop scan, no free space", (FMT__0));
    MAC_CTX().rt_ctx.active_scan.stop_scan = 1;
  }

  TRACE_MSG(TRACE_NWK1, "<<store_pan_desc", (FMT__0));
}
#endif /* ZB_MAC_TESTING_MODE */


/*! @} */
