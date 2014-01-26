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

#include "zb_bank_8.h"

#ifndef ZB_LIMITED_FEATURES
/*! \addtogroup ZB_ZDO */
/*! @{ */
void zdo_mgmt_leave_cli(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;

void zb_zdo_new_channel_cb(zb_uint8_t param) ZB_CALLBACK;

void zb_zdo_mgmt_nwk_update_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_zdo_mgmt_nwk_update_req_t *req_param;
  zb_zdo_mgmt_nwk_update_req_hdr_t *req;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr_8;
  zb_uint16_t *ptr_16;
  zb_uint8_t resp_counter = 1;

  TRACE_MSG(TRACE_ZDO2, ">> zb_zdo_mgmt_nwk_update_req param %hd", (FMT__D, param));
  req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_req_t);

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_nwk_update_req_hdr_t) + sizeof(zb_uint8_t), req);

  ZB_HTOLE32(&req->scan_channels, &req_param->hdr.scan_channels);
  req->scan_duration = req_param->hdr.scan_duration;
  ptr_8 = (zb_uint8_t*)(req + 1);

  *ptr_8 = req_param->scan_count;
  if (req->scan_duration <= ZB_ZDO_MAX_SCAN_DURATION)
  {
    TRACE_MSG(TRACE_ZDO3, "ed scan req", (FMT__0));
    resp_counter = req_param->scan_count;
  }
  else if (req->scan_duration == ZB_ZDO_NEW_ACTIVE_CHANNEL ||
           req->scan_duration == ZB_ZDO_NEW_CHANNEL_MASK)
  {
    TRACE_MSG(TRACE_ZDO2, "new act channel/mask", (FMT__0));
    *ptr_8 = req_param->update_id;
    buf->u.hdr.zdo_cmd_no_resp = 1;
    if (req->scan_duration == ZB_ZDO_NEW_CHANNEL_MASK)
    {
      TRACE_MSG(TRACE_ZDO2, "new mask", (FMT__0));
      ZB_BUF_ALLOC_RIGHT(buf, sizeof(zb_uint16_t), ptr_16);
      ZB_HTOLE16(ptr_16, &req_param->manager_addr);
    }
    else
    {
      zb_uint8_t i = 11;
      while (!(req_param->hdr.scan_channels & (1l << i)))
      {
        i++;
      }
      TRACE_MSG(TRACE_ZDO2, "set new channel %hd after %d", (FMT__H_D, i, ZB_NWK_BROADCAST_DELIVERY_TIME()));
      ZB_SCHEDULE_ALARM(zb_zdo_new_channel_cb, i,
                        ZB_NWK_BROADCAST_DELIVERY_TIME());
    }
  }
  zdo_send_req_by_short(ZDO_MGMT_NWK_UPDATE_REQ_CLID, param, cb, req_param->dst_addr, resp_counter);

  TRACE_MSG(TRACE_ZDO2, "<< zb_zdo_mgmt_nwk_update_req", (FMT__0));
}

void zb_zdo_system_server_discovery_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_zdo_system_server_discovery_req_t *req;
  zb_zdo_system_server_discovery_param_t *req_param;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_system_server_discovery_param_t);
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_system_server_discovery_param_t), req);

  ZB_HTOLE16(&req->server_mask, &req_param->server_mask);

  TRACE_MSG(TRACE_ZDO3, "zb_zdo_system_server_discovery_req server_mask %x", (FMT__D, req->server_mask));
  ZDO_CTX().system_server_discovery_cb = cb;
  zdo_send_req_by_short(ZDO_SYSTEM_SERVER_DISCOVERY_REQ_CLID, param, NULL, ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE, 1);
}



void zb_zdo_mgmt_lqi_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_zdo_mgmt_lqi_req_t *req;
  zb_zdo_mgmt_lqi_param_t *req_param;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ZDO3, "zb_zdo_mgmt_lqi_req param %hd", (FMT__D, param));
  req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_lqi_param_t);

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_nwk_update_req_hdr_t), req);
  req->start_index = req_param->start_index;

  zdo_send_req_by_short(ZDO_MGMT_LQI_REQ_CLID, param, cb, req_param->dst_addr, 1);
}


void zdo_mgmt_leave_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_zdo_mgmt_leave_req_t *req;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_leave_param_t req_param;

  ZB_MEMCPY(&req_param, ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_leave_param_t), sizeof(req_param));
  TRACE_MSG(TRACE_ZDO3, "zb_zdo_mgmt_leave_req param %hd", (FMT__D, param));

  /*
   * Is is possible to do zdo_mgmt_leave_req locally. In such case dst_addr ==
   * our address. Let's not handle it here but let's APS&NWK pass it up for us.
   * There is a problem (or not a problem?) in such case: we will got
   * response (callback call) really before our local LEAVE complete.
   * It may or may not be a problem.
   */
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_leave_req_t), req);
  ZB_IEEE_ADDR_COPY(req->device_address, req_param.device_address);
  req->remove_children = req_param.remove_children;
  req->rejoin = req_param.rejoin;
  if (req_param.dst_addr == ZB_PIB_SHORT_ADDRESS())
  {
    /* local leave mgmt request */
    TRACE_MSG(TRACE_ZDO3, "local leave mgmt req", (FMT__0));
    zdo_mgmt_leave_cli(param, cb);
  }
  else
  {
    zdo_send_req_by_short(ZDO_MGMT_LEAVE_REQ_CLID, param, cb, req_param.dst_addr, 1);
  }
}

void zdo_mgmt_leave_cli(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_ushort_t i = 0;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_leave_req_t req;

  TRACE_MSG(TRACE_ZDO3, ">>zdo_leave_cli %hd", (FMT__H, param));

  /* add entry to the leave req table */
  for (i = 0 ;
       i < ZB_ZDO_PENDING_LEAVE_SIZE
         && ZG->nwk.leave_context.pending_list[i].used ;
       ++i)
  {
  }

  if (i == ZB_ZDO_PENDING_LEAVE_SIZE)
  {
    zb_uint8_t *status_p;

    TRACE_MSG(TRACE_ERROR, "out of pending leave list send resp now.!", (FMT__0));
    /* send resp just now. */

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_uint8_t), status_p);
    *status_p = ZB_ZDP_STATUS_INSUFFICIENT_SPACE;
    zb_schedule_callback(cb, param);
  }
  else
  {
    zb_uint8_t tsn = 0;

    ZDO_CTX().tsn++;
    tsn = ZDO_CTX().tsn;
    register_zdo_cb(tsn, cb, 1);

    if (i + 1 > ZG->nwk.leave_context.pending_list_size)
    {
      ZG->nwk.leave_context.pending_list_size = i + 1;
    }

    ZG->nwk.leave_context.pending_list[i].tsn = tsn;
    ZB_MEMCPY(&req, ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param)), sizeof(zb_zdo_mgmt_leave_req_t));
    ZG->nwk.leave_context.pending_list[i].src_addr = ZB_PIB_SHORT_ADDRESS();
    ZG->nwk.leave_context.pending_list[i].buf_ref = param;
    ZG->nwk.leave_context.pending_list[i].used = 1;
    TRACE_MSG(TRACE_ZDO3, "remember mgmt_leave at i %hd, tsn %hd, addr %d, buf_ref %hd",
              (FMT__H_H_D_H, i, ZG->nwk.leave_context.pending_list[i].tsn,
               ZG->nwk.leave_context.pending_list[i].src_addr,
               ZG->nwk.leave_context.pending_list[i].buf_ref));

    /* Now locally call LEAVE.request */
    {
      zb_nlme_leave_request_t *lr = NULL;

      lr = ZB_GET_BUF_PARAM(buf, zb_nlme_leave_request_t);
      ZB_IEEE_ADDR_COPY(lr->device_address, req.device_address);
      lr->remove_children = req.remove_children;
      lr->rejoin = req.rejoin;
      ZB_SCHEDULE_CALLBACK(zb_nlme_leave_request, param);
    }
  }
  TRACE_MSG(TRACE_ZDO3, "<<zdo_mgmt_leave_cli", (FMT__0));
}

#ifdef ZB_ROUTER_ROLE
void zb_zdo_mgmt_permit_joining_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_permit_joining_req_t *req;
  zb_zdo_mgmt_permit_joining_req_param_t req_param;

  TRACE_MSG(TRACE_ZDO3, ">> zb_zdo_mgmt_permit_joining_req param %hd cd %p", (FMT__H_P, param, cb));

  ZB_MEMCPY(&req_param, ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_permit_joining_req_param_t), sizeof(req_param));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_permit_joining_req_t), req);
  req->permit_duration = req_param.permit_duration;
  req->tc_significance = req_param.tc_significance;

  zdo_send_req_by_short(ZDO_MGMT_PERMIT_JOINING_CLID, param, cb, req_param.dest_addr, 1);

  TRACE_MSG(TRACE_ZDO3, "<< zb_zdo_mgmt_permit_joining_req", (FMT__0));
}
#endif /* ZB_ROUTER_ROLE */

#endif
/*! @} */
