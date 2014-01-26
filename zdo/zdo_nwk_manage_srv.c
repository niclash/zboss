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
#include "zb_secur.h"

#include "zb_bank_8.h"

#ifndef ZB_LIMITED_FEATURES
/*! \addtogroup ZB_ZDO */
/*! @{ */
void zb_zdo_new_channel_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_nwk_do_leave_local(zb_uint8_t param) ZB_CALLBACK;

/* Handle nwk_update_req, 2.4.3.3.9 Mgmt_NWK_Update_req */
void zb_zdo_mgmt_nwk_update_handler(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_apsde_data_indication_t *ind;
  zb_zdo_mgmt_nwk_update_req_hdr_t *req_hdr;
  zb_uint8_t *aps_body;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
//  zb_uint8_t update_id;
  zb_uint16_t manager_addr;
  zb_uint8_t send_resp = 0;
  zb_uint8_t status = ZB_ZDP_STATUS_SUCCESS;
  zb_uint32_t scan_channels;
  zb_uint8_t tsn;

  TRACE_MSG(TRACE_ZDO3, ">>mgmt_nwk_update_handler %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(buf);
  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  /*
    2.4.2.8 Transmission of ZDP Commands
   | Transaction sequence number (1byte) | Transaction data (variable) |
  */
  tsn = *aps_body;
  aps_body++;
  req_hdr = (zb_zdo_mgmt_nwk_update_req_hdr_t*)aps_body;
  aps_body += sizeof(zb_zdo_mgmt_nwk_update_req_hdr_t);
  ZB_LETOH32(&scan_channels, &req_hdr->scan_channels);

  TRACE_MSG(TRACE_ZDO2, "scan_duration %hx, scan_channels %d %d",
    (FMT__H_D_D, req_hdr->scan_duration, *((zb_uint16_t*)&req_hdr->scan_channels), *(((zb_uint16_t*)&req_hdr->scan_channels) + 1)));
  if (req_hdr->scan_duration == ZB_ZDO_NEW_ACTIVE_CHANNEL)
  {
    TRACE_MSG(TRACE_ZDO2, "new act ch %d", (FMT__D, (zb_uint16_t)req_hdr->scan_channels));
    if (req_hdr->scan_channels >= (1l<<ZB_MAC_START_CHANNEL_NUMBER) &&
        req_hdr->scan_channels <= (1l<<ZB_MAC_MAX_CHANNEL_NUMBER))
    {
      zb_uint8_t i = 11;
      /*
         start nwkNetworkBroadcastDeliveryTime timer On timer
         expiration, change channel to the new value, increment
         NIB.UpdateId and reset counters
      */
      while (!(req_hdr->scan_channels & (1l << i)))
      {
        i++;
      }
      ZB_SCHEDULE_ALARM(zb_zdo_new_channel_cb, i,
                        ZB_NWK_BROADCAST_DELIVERY_TIME());
    }
  }
  else if (req_hdr->scan_duration == ZB_ZDO_NEW_CHANNEL_MASK)
  {
    TRACE_MSG(TRACE_ZDO2, "new ch mask", (FMT__0));
    /* save channel mask in APS IB */
    ZB_AIB().aps_channel_mask = req_hdr->scan_channels;
    /* TODO: check, what should i do with update_id? */
//    update_id = *aps_body;
    aps_body++;
    ZB_LETOH16(&manager_addr, aps_body);

    /* store nwkManagerAddr in the NIB */
    ZB_NIB_NWK_MANAGER_ADDR() = manager_addr;
  }
  else if (req_hdr->scan_duration <= ZB_ZDO_MAX_SCAN_DURATION)
  {
    zb_uint8_t scan_count = *aps_body;

    /* Table 2.87 Fields of the Mgmt_NWK_Update_req Command
       ScanCount valid range: 0x00 - 0x05
       This field represents the number of energy scans to be
       conducted and reported.
    */
    TRACE_MSG(TRACE_ZDO2, "ed scan", (FMT__0));
    if (scan_count > 5)
    {
      send_resp = 1;
      status = ZB_ZDP_STATUS_INV_REQUESTTYPE;
      TRACE_MSG(TRACE_ZDO1, "mgmt_nwk_update_handler, error incorrect scan_count %hd", (FMT__H, scan_count));
    }
    else if (scan_count > 0)
    {
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_channels = scan_channels;
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_duration = req_hdr->scan_duration;
      ZG->zdo.zdo_ctx.nwk_upd_req.scan_count = scan_count;
      ZG->zdo.zdo_ctx.nwk_upd_req.dst_addr = ind->src_addr;
      ZG->zdo.zdo_ctx.nwk_upd_req.tsn = tsn;
      ZB_APS_SET_ZDO_ED_SCAN_FLAG();
      zb_start_ed_scan(param);
    }
    else
    {
      send_resp = 1;
    }
  }
  else
  {
    TRACE_MSG(TRACE_ZDO1, "mgmt_nwk_update_handler, error incorrect scan_duration %hd", (FMT__H, req_hdr->scan_duration));
    /* response with error if addr mode is not unicast */
    if (ZB_APS_FC_GET_DELIVERY_MODE(ind->fc) == ZB_APS_DELIVERY_UNICAST)
    {
      send_resp = 1;
      status = ZB_NWK_STATUS_INVALID_REQUEST;
    }
  }

  /* check if transmission was not broadcast */
  if (send_resp && ind->src_addr < ZB_NWK_BROADCAST_LOW_POWER_ROUTER)
  {
    zb_zdo_mgmt_nwk_update_notify_param_t *update_notify_param;
    zb_uint16_t dst_addr = ind->src_addr;

    update_notify_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_notify_param_t);
    ZB_BZERO(update_notify_param, sizeof(zb_zdo_mgmt_nwk_update_notify_param_t));
    update_notify_param->hdr.status = status;
    update_notify_param->tsn = tsn;
    update_notify_param->dst_addr = dst_addr;

    zb_zdo_nwk_upd_notify(param);
  }

  TRACE_MSG(TRACE_ZDO3, "<<mgmt_nwk_update_handler", (FMT__0));
}

/* sends 2.4.4.3.9 Mgmt_NWK_Update_notify */
void zb_zdo_nwk_upd_notify(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_nwk_update_notify_param_t *notify_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_notify_param_t);
  zb_zdo_mgmt_nwk_update_notify_hdr_t *notify_resp;
  zb_uint8_t *ed_scan_values;

  TRACE_MSG(TRACE_ZDO3, ">>nwk_upd_notify %hd", (FMT__H, param));

  ZB_APS_CLEAR_ZDO_ED_SCAN_FLAG();
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_nwk_update_notify_hdr_t), notify_resp);

  ZB_MEMCPY(notify_resp, &notify_param->hdr, sizeof(zb_zdo_mgmt_nwk_update_notify_hdr_t));
  ZB_HTOLE32(&notify_resp->scanned_channels, &notify_param->hdr.scanned_channels);

  if (notify_resp->scanned_channels_list_count)
  {
    /* Assert just to be sure that we have enough space for response */
    ZB_ASSERT((ZB_APS_PAYLOAD_MAX_LEN - sizeof(zb_zdo_mgmt_nwk_update_notify_hdr_t)) > notify_resp->scanned_channels_list_count);

    ZB_BUF_ALLOC_RIGHT(buf, notify_resp->scanned_channels_list_count, ed_scan_values);
    ZB_MEMCPY(ed_scan_values, notify_param->energy_values, notify_resp->scanned_channels_list_count);
    TRACE_MSG(TRACE_ZDO3, "ch count %hd, buf len %hd",
              (FMT__H_H, notify_resp->scanned_channels_list_count, ZB_BUF_LEN(buf)));
  }

  TRACE_MSG(TRACE_ZDO3, "total tr %hd, tr fail %hd, ack %hd",
    (FMT__H_H_H, notify_resp->total_transmissions, notify_resp->transmission_failures, ZB_ZDO_GET_SEND_WITH_ACK()));
  if (ZB_ZDO_GET_SEND_WITH_ACK())
  {
    /* send update notify as request to set ack needed option */
    zdo_send_req_by_short(ZDO_MGMT_NWK_UPDATE_NOTIFY_CLID, param, zb_zdo_channel_check_finish_cb,
                          notify_param->dst_addr, 1);

    ZB_ZDO_CLEAR_SEND_WITH_ACK();
  }
  else
  {
    zdo_send_resp_by_short(ZDO_MGMT_NWK_UPDATE_NOTIFY_CLID, param, notify_param->tsn, notify_param->dst_addr);
  }

  if (notify_resp->status == ZB_ZDP_STATUS_SUCCESS && ZG->zdo.zdo_ctx.nwk_upd_req.scan_count)
  {
    ZB_APS_SET_ZDO_ED_SCAN_FLAG();
    ZB_GET_OUT_BUF_DELAYED(zb_start_ed_scan);
  }

  TRACE_MSG(TRACE_ZDO3, "<<nwk_upd_notify", (FMT__0));
}

void zb_start_ed_scan(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_ed_scan_request_t *rq;

  TRACE_MSG(TRACE_ZDO3, "zb_start_ed_scan, param %d", (FMT__D, param));
  rq = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_ed_scan_request_t);

  ZG->zdo.zdo_ctx.nwk_upd_req.scan_count--;
  rq->scan_channels = ZG->zdo.zdo_ctx.nwk_upd_req.scan_channels;
  rq->scan_duration = ZG->zdo.zdo_ctx.nwk_upd_req.scan_duration;

  ZB_SCHEDULE_CALLBACK(zb_nlme_ed_scan_request, param);
}

void zb_zdo_new_channel_cb(zb_uint8_t param) ZB_CALLBACK
{
  /* Upon receipt of a Mgmt_NWK_Update_req with a change of channels,
   * change channel to the new value, increment NIB.UpdateId and reset
   * counters */
  TRACE_MSG(TRACE_ZDO2, "new_channel_cb ch %hd", (FMT__H, param));

  ZB_TRANSCEIVER_SET_CHANNEL(param); /* ignore retcode */
  ZB_NIB_NWK_TX_TOTAL() = 0;
  ZB_NIB_NWK_TX_FAIL() = 0;
  ZB_NIB_UPDATE_ID()++;
}

void zdo_system_server_discovery_res(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body;
  zb_uint8_t tsn;
  zb_uint16_t server_mask;
  zb_zdo_system_server_discovery_resp_t *resp;
  zb_apsde_data_indication_t *ind;

  TRACE_MSG(TRACE_ZDO3, ">>zdo_system_server_discovery_res %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  tsn = *aps_body;
  aps_body++;

  ZB_LETOH16(&server_mask, aps_body);

  TRACE_MSG(TRACE_ZDO3, "param server_mask %x, desc server_mask %x",
            (FMT__D_D, server_mask, ZB_ZDO_NODE_DESC()->server_mask));
  server_mask &= ZB_ZDO_NODE_DESC()->server_mask;
  if (server_mask)
  {
    TRACE_MSG(TRACE_ZDO3, "send response mask %x", (FMT__D, server_mask));
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_system_server_discovery_resp_t), resp);
    resp->status = ZB_ZDP_STATUS_SUCCESS;
    ZB_HTOLE16(&resp->server_mask, &server_mask);
    TRACE_MSG(TRACE_ZDO3, "send response addr %x", (FMT__D, ind->src_addr));
    zdo_send_resp_by_short(ZDO_SYSTEM_SERVER_DISCOVERY_RESP_CLID, param, tsn, ind->src_addr);
  }
  else
  {
    zb_free_buf(buf);
  }
  TRACE_MSG(TRACE_ZDO3, "<<zdo_system_server_discovery_res", (FMT__0));
}

void zb_zdo_mgmt_nwk_leave_res(zb_uint8_t param, zb_callback_t cb)
{
  TRACE_MSG(TRACE_ZDO3, ">>zb_zdo_mgmt_nwk_leave_req param %hd", (FMT__D, param));
  if (cb != NULL)
  {
    ZB_SCHEDULE_CALLBACK(cb, param);
  }

}

void zdo_lqi_resp(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body;
  zb_uint8_t tsn;
  zb_zdo_mgmt_lqi_resp_t *resp;
  zb_apsde_data_indication_t *ind;
#ifdef ZB_ROUTER_ROLE
  zb_zdo_mgmt_lqi_req_t *req;
  zb_ushort_t i;
  zb_zdo_neighbor_table_record_t *record;
  zb_uint8_t max_records_num;
  zb_uint8_t records_num;
#endif

  TRACE_MSG(TRACE_ZDO3, ">>zdo_lqi_resp %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(buf);
  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  tsn = *aps_body;
  aps_body++;

#ifdef ZB_ROUTER_ROLE
  req = (zb_zdo_mgmt_lqi_req_t*)aps_body;
  /* calculate total header size */
  i = ZB_APS_HDR_SIZE(ZB_APS_FRAME_DATA)
#ifdef ZB_SECURITY
    + sizeof(zb_aps_nwk_aux_frame_hdr_t)
#endif
    + ZB_NWK_FULL_HDR_SIZE(1) + ZB_MAC_MAX_HEADER_SIZE(1, 1)
    + sizeof(zb_zdo_mgmt_lqi_resp_t) + ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME;
  max_records_num = ZB_IO_BUF_SIZE / i;

  records_num = (ZG->nwk.neighbor.base_neighbor_used > req->start_index) ?
    ZG->nwk.neighbor.base_neighbor_used - req->start_index : 0;
  TRACE_MSG(TRACE_ZDO3, "max rec %hd, used %hd, start indx %hd",
            (FMT__H_H_H, max_records_num, ZG->nwk.neighbor.base_neighbor_used, req->start_index));

  records_num = (records_num < max_records_num) ? records_num : max_records_num;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_lqi_resp_t) + records_num * sizeof(zb_zdo_neighbor_table_record_t), resp);

  resp->status = ZB_ZDP_STATUS_SUCCESS;
  resp->neighbor_table_entries = ZG->nwk.neighbor.base_neighbor_used;
  resp->start_index = req->start_index;
  resp->neighbor_table_list_count = records_num;
  record = (zb_zdo_neighbor_table_record_t*)(resp + 1);

  TRACE_MSG(TRACE_ZDO3, "will add records %hd", (FMT__H, records_num));
  for (i = 0; i < ZG->nwk.neighbor.base_neighbor_size && records_num; ++i)
  {
    if (ZG->nwk.neighbor.base_neighbor[i].used)
    {
      ZB_MEMCPY(record->ext_pan_id, ZB_NIB_EXT_PAN_ID(), sizeof(zb_ext_pan_id_t));
      zb_address_by_ref(record->ext_addr, &record->network_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);

      ZB_ZDO_RECORD_SET_DEVICE_TYPE(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].device_type);
      ZB_ZDO_RECORD_SET_RX_ON_WHEN_IDLE(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle);
      ZB_ZDO_RECORD_SET_RELATIONSHIP(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].relationship);
      record->permit_join = ZG->nwk.neighbor.base_neighbor[i].permit_joining;
      record->depth = ZG->nwk.neighbor.base_neighbor[i].depth;
      record->lqi = ZG->nwk.neighbor.base_neighbor[i].lqi;
      records_num--;
      record++;
    }
  }

#else
  /* end device case */
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_lqi_resp_t), resp);
  ZB_BZERO(resp, sizeof(zb_zdo_mgmt_lqi_resp_t));
  resp->status = ZB_ZDP_STATUS_NOT_SUPPORTED;
#endif

  zdo_send_resp_by_short(ZDO_MGMT_LQI_RESP_CLID, param, tsn, ind->src_addr);

  TRACE_MSG(TRACE_ZDO3, "<< zdo_lqi_resp", (FMT__0));
}


void zdo_mgmt_leave_srv(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_ushort_t i;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_leave_req_t req;
  zb_uint8_t *aps_body;
  zb_apsde_data_indication_t *ind;

  TRACE_MSG(TRACE_ZDO3, ">>zdo_leave_srv %hd", (FMT__H, param));

  /*
    We are here because we got mgmt_leave_req from the remote (or locally, but
    thru aps & nwk).
  */

  /**
     \par Notes about LEAVE

     - when got mgmt_leave_req, fill pending list and call nlme.leave.request

     - nlme.leave.request either :
     - uincasts to its child, call leave.confirm at packet send complete
     - if its leave for us, broadcasts LEAVE command, call leave.confirm at packet send complete

     - when got LEAVE command from the net:
     - from child with 'request' = 0 and we are not TC, send UPDATE-DEVICE
     to TC, then forget this child
     - from parent with 'request' or 'remove child' == 1, send broadcast LEAVE
     with same 'remove child' and 'request' = 0
     call leave.confirm at packet send complete
     - else - from any device with request = 0 - forget this device

     - leave.confirm called when LEAVE command has sent, from mcps-data.confirm
     (not when LEAVE procedure complete - see later)

     - if LEAVE was caused by mgmt_leave_req receive, we must send send mgmt_leave_rsp.
     We can do it only if we did not leave network yet.
     Means, we must remember somehow address of device which issued mgmt_leave_req and,
     if it not empty, send resp to it.
     Need a list of pending mgmt_leave_req. It holds address ref (1b) and buffer id (1b).
     Do not clear entry in this list now - we still need it.

     If no entry in this list, there was no mgmt_leave_req, so can call "leave finish" now.

     - when mgmt_leave_rsp successfuly sent (means - from aps-data.confirm), we must check:
     do we need to leave network and rejoin after it?
     We use here, again, same buffer, so can use same list of pending mgmt_leave_req.
     If it was leave rsp, call call "leave finish" now.
  */

  /* add entry to the leave req table */
  for (i = 0 ;
       i < ZB_ZDO_PENDING_LEAVE_SIZE
         && ZG->nwk.leave_context.pending_list[i].used ;
       ++i)
  {
  }

  aps_body = ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);

  if (i == ZB_ZDO_PENDING_LEAVE_SIZE)
  {
    zb_uint8_t *status_p;

    TRACE_MSG(TRACE_ERROR, "out of pending leave list send resp now.!", (FMT__0));
    /* send resp just now. */

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_uint8_t), status_p);
    *status_p = ZB_ZDP_STATUS_INSUFFICIENT_SPACE;
    zdo_send_resp_by_short(ZDO_MGMT_LEAVE_RESP_CLID, param, *aps_body, ind->src_addr);
  }
  else
  {
    if (i + 1 > ZG->nwk.leave_context.pending_list_size)
    {
      ZG->nwk.leave_context.pending_list_size = i + 1;
    }

    ZG->nwk.leave_context.pending_list[i].tsn = *aps_body;
    ZB_MEMCPY(&req, (aps_body + 1), sizeof(zb_zdo_mgmt_leave_req_t));
    ZG->nwk.leave_context.pending_list[i].src_addr = ind->src_addr;
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
  TRACE_MSG(TRACE_ZDO3, "<<zdo_mgmt_leave_srv", (FMT__0));
}

#ifndef ZB_LIMITED_FEATURES
zb_bool_t zdo_try_send_mgmt_leave_rsp(zb_uint8_t param, zb_uint8_t status, zb_uint8_t will_leave) ZB_SDCC_REENTRANT
{
  zb_ushort_t i;
  for (i = 0 ; i < ZG->nwk.leave_context.pending_list_size ; ++i)
  {
    if (ZG->nwk.leave_context.pending_list[i].used
        && ZG->nwk.leave_context.pending_list[i].buf_ref == param)
    {
      zb_uint8_t *status_p;
      TRACE_MSG(TRACE_ZDO3, "sending mgmt_leave_rsp i %hd, tsn %hd, addr %d, buf_ref %hd",
                (FMT__H_H_D_H, i, ZG->nwk.leave_context.pending_list[i].tsn,
                 ZG->nwk.leave_context.pending_list[i].src_addr,
                 ZG->nwk.leave_context.pending_list[i].buf_ref));

      ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(zb_uint8_t), status_p);
      *status_p = status;
      if (ZG->nwk.leave_context.pending_list[i].src_addr == ZB_PIB_SHORT_ADDRESS())
      {
        zb_ret_t ret = ZB_ZDP_STATUS_SUCCESS;
        zb_uint8_t *tsn_p;

        ZB_BUF_ALLOC_LEFT(ZB_BUF_FROM_REF(param), 1, tsn_p);
        *tsn_p = ZG->nwk.leave_context.pending_list[i].tsn;
        ret = zdo_af_resp(param);
        if (ret == ZB_ZDP_STATUS_SUCCESS)
        {
          ret = zdo_try_mgmt_leave_complete(param);
        }
        TRACE_MSG(TRACE_ZDO3, "ret %hd", (FMT__H, ret));
      }
      else
      {
        zdo_send_resp_by_short(ZDO_MGMT_LEAVE_RESP_CLID, param,
                               ZG->nwk.leave_context.pending_list[i].tsn,
                               ZG->nwk.leave_context.pending_list[i].src_addr);
      }

      if (status != 0 || !will_leave)
      {
        ZG->nwk.leave_context.pending_list[i].used = 0;
        if (ZG->nwk.leave_context.pending_list_size == i + 1)
        {
          ZG->nwk.leave_context.pending_list_size = i;
        }
      }
      return ZB_TRUE;
    }
  }
  return ZB_FALSE;
}
#endif

zb_bool_t zdo_try_mgmt_leave_complete(zb_uint8_t param)
{
  zb_ushort_t i;
  for (i = 0 ; i < ZG->nwk.leave_context.pending_list_size ; ++i)
  {
    if (ZG->nwk.leave_context.pending_list[i].used
        && ZG->nwk.leave_context.pending_list[i].buf_ref == param)
    {
      ZG->nwk.leave_context.pending_list[i].used = 0;
      if (ZG->nwk.leave_context.pending_list_size == i + 1)
      {
        ZG->nwk.leave_context.pending_list_size = i;
      }
      TRACE_MSG(TRACE_ZDO1, "complete LEAVE after msmt_leave_rsp confirm", (FMT__0));
      if (ZG->nwk.leave_context.pending_list[i].src_addr == ZB_PIB_SHORT_ADDRESS())
      {
        /* local leave mgmt request */
        TRACE_MSG(TRACE_ZDO3, "local leave mgmt leave complete", (FMT__0));
        ZB_GET_OUT_BUF_DELAYED(zb_nwk_do_leave_local);
      }
      else
      {
        zb_nwk_do_leave(param, ZG->nwk.leave_context.rejoin_after_leave);
      }
      return ZB_TRUE;
    }
  }
  return ZB_FALSE;
}

void zb_nwk_do_leave_local(zb_uint8_t param) ZB_CALLBACK
{
  zb_nwk_do_leave(param, ZG->nwk.leave_context.rejoin_after_leave);
}

#ifdef ZB_ROUTER_ROLE
void zb_zdo_mgmt_permit_joining_handle(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_permit_joining_req_t *req;
  zb_nlme_permit_joining_request_t *req_param;

  TRACE_MSG(TRACE_ZDO3, ">>mgmt_nwk_update_handler %hd", (FMT__H, param));

  aps_body++;
  req = (zb_zdo_mgmt_permit_joining_req_t *)aps_body;

  TRACE_MSG(TRACE_ZDO3, "permit_duration %hd tc_significance %hd", (FMT__H_H, req->permit_duration, req->tc_significance));
  req_param = (zb_nlme_permit_joining_request_t *)ZB_GET_BUF_PARAM(buf, zb_nlme_permit_joining_request_t);
  req_param->permit_duration = req->permit_duration;
  ZB_SCHEDULE_CALLBACK(zb_nlme_permit_joining_request, param);

#if defined ZB_SECURITY && defined ZB_COORDINATOR_ROLE
  if ( ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR
       && req->tc_significance )
  {
    if ( req->permit_duration )
    {
      ZDO_CTX().handle.allow_auth = 1;
    }
    else
    {
      ZDO_CTX().handle.allow_auth = 0;
    }
  }
#endif

  TRACE_MSG(TRACE_ZDO3, "<<mgmt_nwk_update_handler", (FMT__0));
}
#endif /* ZB_ROUTER_ROLE */

#endif  /* ZB_LIMITED_FEATURES */
/*! @} */
