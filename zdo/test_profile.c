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
PURPOSE: AF layer
*/

#include "zb_common.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_af_globals.h"
#include "zb_zdo.h"
#include "zb_test_profile.h"

#include "zb_bank_9.h"
/*! \addtogroup ZB_AF */
/*! @{ */

#ifdef ZB_TEST_PROFILE

static void tp_buffer_test_request_handler(zb_uint8_t param) ZB_SDCC_REENTRANT;
static void tp_buffer_test_response_handler(zb_uint8_t param) ZB_SDCC_REENTRANT;
static void tp_buffer_test_response(zb_uint8_t param) ZB_SDCC_REENTRANT;

void zb_test_profile_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = (zb_apsde_data_indication_t *)ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);
  zb_uint8_t skip_free_buf = 1;

  TRACE_MSG(TRACE_ZDO1, "zb_test_profile_indication %hd clu 0x%hx", (FMT__H_H, param, ind->clusterid));

  switch (ind->clusterid)
  {
    case TP_BUFFER_TEST_REQUEST_CLID:
    case TP_BUFFER_TEST_REQUEST_CLID2:
      TRACE_MSG(TRACE_ZDO1, "BUFFER_TEST_REQUEST", (FMT__0));
      tp_buffer_test_request_handler(param);
      break;

    case TP_BUFFER_TEST_RESPONSE_CLID:
      TRACE_MSG(TRACE_ZDO1, "BUFFER_TEST_RESPONSE", (FMT__0));
      tp_buffer_test_response_handler(param);
      break;

    default:
      TRACE_MSG(TRACE_ZDO1, "unhandl clu %hd - drop", (FMT__H, ind->clusterid));
      skip_free_buf = 0;
      break;
  }

  if (!skip_free_buf)
  {
    zb_free_buf(asdu);
  }
}

void tp_start_send_counted_packet(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  ZB_GET_OUT_BUF_DELAYED(tp_send_counted_packet);
}

void tp_send_counted_packet(zb_uint8_t param) ZB_CALLBACK
{
  zb_tp_transmit_counted_packets_req_t *req;

  TRACE_MSG(TRACE_ZDO3, "send_counted_packet pack num %d, counter %d",
    (FMT__D_D, ZG->zdo.test_prof_ctx.tp_counted_packets.params.packets_number, ZG->zdo.test_prof_ctx.tp_counted_packets.counter));

  if (ZG->zdo.test_prof_ctx.tp_counted_packets.params.packets_number > ZG->zdo.test_prof_ctx.tp_counted_packets.counter)
  {
    ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(zb_tp_transmit_counted_packets_req_t) +
                         ZG->zdo.test_prof_ctx.tp_counted_packets.params.len, req);

    req->len = ZG->zdo.test_prof_ctx.tp_counted_packets.params.len + sizeof(zb_uint16_t); /* data length + sequence counter length */
    req->counter = ZG->zdo.test_prof_ctx.tp_counted_packets.counter++;
    ZB_MEMSET((zb_uint8_t*)(req + 1), 0xAA, ZG->zdo.test_prof_ctx.tp_counted_packets.params.len); /* put some test data */
    tp_send_req_by_short(TP_TRANSMIT_COUNTED_PACKETS_CLID, param, ZG->zdo.test_prof_ctx.tp_counted_packets.params.dst_addr,
                         ZB_TEST_PROFILE_EP, ZB_TEST_PROFILE_EP,              /* ZB_APSDE_TX_OPT_ACK_TX */ 0);

    ZB_SCHEDULE_ALARM(tp_start_send_counted_packet, 0,
      ZB_MILLISECONDS_TO_BEACON_INTERVAL(ZG->zdo.test_prof_ctx.tp_counted_packets.params.idle_time));
  }
  else
  {
    ZB_SCHEDULE_CALLBACK(ZG->zdo.test_prof_ctx.tp_counted_packets.user_cb, param);
  }
}

void zb_tp_transmit_counted_packets_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_tp_transmit_counted_packets_param_t *params = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_tp_transmit_counted_packets_param_t);

  TRACE_MSG(TRACE_ZDO3, "transmit_counted_packets_req param %hd", (FMT__H, param));

  ZB_MEMCPY(&ZG->zdo.test_prof_ctx.tp_counted_packets.params, params, sizeof(zb_tp_transmit_counted_packets_param_t));
  ZG->zdo.test_prof_ctx.tp_counted_packets.user_cb = cb;
  ZG->zdo.test_prof_ctx.tp_counted_packets.counter = 0;

  tp_send_counted_packet(param);
}


void tp_send_req_by_short(zb_uint16_t command_id, zb_uint8_t param,zb_uint16_t addr, zb_uint8_t src_ep, zb_uint8_t dst_ep, zb_uint8_t tx_options) ZB_SDCC_REENTRANT
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

  TRACE_MSG(TRACE_ZDO2, "tp_send_req_by_short param %hd", (FMT__H, param));
  ZB_BZERO(dreq, sizeof(*dreq));

  dreq->profileid = ZB_TEST_PROFILE_ID;
  dreq->clusterid = command_id;
  dreq->dst_endpoint = dst_ep;
  dreq->src_endpoint = src_ep;
  dreq->dst_addr.addr_short = addr;
  dreq->tx_options = tx_options;
  dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}

void tp_send_req_by_EP(zb_uint16_t command_id, zb_uint8_t param, zb_uint8_t src_ep, zb_uint8_t dst_ep, zb_uint8_t tx_options) ZB_SDCC_REENTRANT
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

  TRACE_MSG(TRACE_ZDO3, "tp_send_req_by_bind_addr param %hd, src_ep %hd, dst_ep %hd", (FMT__H_H_H, param, src_ep, dst_ep));
  ZB_BZERO(dreq, sizeof(*dreq));

  dreq->profileid = ZB_TEST_PROFILE_ID;
  dreq->clusterid = command_id;
  dreq->dst_endpoint = dst_ep;
  dreq->src_endpoint = src_ep;
  dreq->tx_options = tx_options;
  dreq->addr_mode = ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}

#if 0                           /* not used */
void tp_send_req_by_EP_brdcast(zb_uint16_t command_id, zb_uint8_t param, zb_uint8_t src_ep, zb_uint8_t dst_ep, zb_uint8_t tx_options) ZB_SDCC_REENTRANT
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

  TRACE_MSG(TRACE_ZDO3, "tp_send_req_by_bind_addr param %hd, src_ep %hd, dst_ep %hd", (FMT__H_H_H, param, src_ep, dst_ep));
  ZB_MEMSET(dreq, 0, sizeof(*dreq));

  dreq->profileid = ZB_TEST_PROFILE_ID;
  dreq->clusterid = command_id;
  dreq->dst_endpoint = dst_ep;
  dreq->src_endpoint = src_ep;
  dreq->tx_options = tx_options;
  dreq->dst_addr.addr_short = ZB_NWK_BROADCAST_ALL_DEVICES;
  dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}
#endif

void buffer_test_req_timeout(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_ZDO2, "buffer_test_req_timeout", (FMT__0));
  if (ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb)
  {
    ZB_SCHEDULE_CALLBACK(ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb, ZB_TP_BUFFER_TEST_FAIL);
    ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb = NULL;
  }
}

/* 6.7	Buffer test request, CID=0x1c */
void zb_tp_buffer_test_request(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_buffer_test_req_param_t *req_param = ZB_GET_BUF_PARAM(buf, zb_buffer_test_req_param_t);
  zb_buffer_test_req_t *req;

  TRACE_MSG(TRACE_ZDO2, "zb_tp_buffer_test_request param %hd, len %hd", (FMT__H_H, param, req_param->len));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_buffer_test_req_t), req);
  req->len = req_param->len;
  req_param->src_ep = ZB_TEST_PROFILE_EP;
  req_param->dst_ep = ZB_TEST_PROFILE_EP;

  ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb = cb;
  TRACE_MSG(TRACE_ZDO3, "sched buffer_test_req_timeout, timeout %d", (FMT__D, 10 * ZB_TIME_ONE_SECOND));
  /* use callback to determine that response is not received */
  ZB_SCHEDULE_ALARM(buffer_test_req_timeout, 0,  30 * ZB_TIME_ONE_SECOND);

  tp_send_req_by_short(TP_BUFFER_TEST_REQUEST_CLID, param, req_param->dst_addr, req_param->src_ep, req_param->dst_ep, 0);
}

/* 6.7	Buffer test request, CID=0x1c */
void zb_tp_buffer_test_request_EP(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_buffer_test_req_param_EP_t *req_param = ZB_GET_BUF_PARAM(buf, zb_buffer_test_req_param_EP_t);
  zb_buffer_test_req_t *req;

  TRACE_MSG(TRACE_ZDO3, "zb_tp_buffer_test_request_EP param %hd, len %hd", (FMT__H_H, param, req_param->len));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_buffer_test_req_t), req);
  req->len = req_param->len;

  ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb = cb;
  TRACE_MSG(TRACE_ZDO3, "zb_tp_buffer_test_request_EP, timeout %d", (FMT__D, 10 * ZB_TIME_ONE_SECOND));
  /* use callback to determine that response is not received */
  ZB_SCHEDULE_ALARM(buffer_test_req_timeout, 0, 30 * ZB_TIME_ONE_SECOND);

  TRACE_MSG(TRACE_ZDO3, "request_EP src ep %hd, dst ep %hd", (FMT__H_H, req_param->src_ep, req_param->dst_ep));
  tp_send_req_by_EP(TP_BUFFER_TEST_REQUEST_CLID, param, req_param->src_ep, req_param->dst_ep, 0);
}

static void tp_buffer_test_request_handler(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_buffer_test_req_t *test_req;
  zb_buffer_test_response_param_t *test_param;
  zb_apsde_data_indication_t *ind = (zb_apsde_data_indication_t *)ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);

  TRACE_MSG(TRACE_ZDO3, "tp_buffer_test_request_handler %x", (FMT__D, param));
  TRACE_MSG(TRACE_ZDO3, "src ept %hd, dst ept %hd", (FMT__H_H, ind->src_endpoint, ind->dst_endpoint));
  ZB_APS_HDR_CUT(asdu);
  test_req = (zb_buffer_test_req_t*)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  test_param = ZB_GET_BUF_PARAM(asdu, zb_buffer_test_response_param_t);
  /*test_param must be filled with values from ind first of all, because we use
    the same buffer*/
  test_param->src_ep = ind->dst_endpoint;
  test_param->dst_ep = ind->src_endpoint;
  test_param->dst_addr = ind->src_addr;
  TRACE_MSG(TRACE_ZDO3, "src ept %hd, dst ept %hd", (FMT__H_H, ind->src_endpoint, ind->dst_endpoint));
  test_param->len = test_req->len;
  test_param->status = ZB_TP_BUFFER_TEST_OK;
  tp_buffer_test_response(param);
}


void tp_buffer_test_response(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buffer_test_response_t *test_resp;
  zb_buffer_test_response_param_t *test_param;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body;
  zb_uint8_t i;
  zb_uint8_t send_len;

  TRACE_MSG(TRACE_ZDO3, "tp_buffer_test_response %x", (FMT__D, param));
  test_param = ZB_GET_BUF_PARAM(buf, zb_buffer_test_response_param_t);
  send_len = (test_param->status == ZB_TP_BUFFER_TEST_OK) ? test_param->len : 0;
  TRACE_MSG(TRACE_ZDO3, "len %hd, status %hd, send_len %hd", (FMT__H_H_H, test_param->len, test_param->status, send_len));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_buffer_test_response_t) + send_len, test_resp);
  TRACE_MSG(TRACE_ZDO3, "src ept resp %hd, dst ept resp%hd", (FMT__H_H, test_param->src_ep, test_param->dst_ep));
  test_resp->len = test_param->len;
  test_resp->status = test_param->status;
  if (test_param->status == ZB_TP_BUFFER_TEST_OK)
  {
    aps_body = (zb_uint8_t*)(test_resp + 1);
    for (i = 0; i < test_param->len; i++)
    {
      *aps_body = i;
      aps_body++;
    }
  }
  tp_send_req_by_short(TP_BUFFER_TEST_RESPONSE_CLID, param, test_param->dst_addr, test_param->src_ep, test_param->dst_ep,
ZB_APSDE_TX_OPT_ACK_TX);
}

static void tp_buffer_test_response_handler(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_buffer_test_response_t *test_resp;

  TRACE_MSG(TRACE_ZDO2, "tp_buffer_test_response_handler %x", (FMT__D, param));
  ZB_APS_HDR_CUT(asdu);

  test_resp = (zb_buffer_test_response_t*)ZB_BUF_BEGIN(asdu);
  TRACE_MSG(TRACE_ZDO2, "status %hd, len %hd", (FMT__H_H, test_resp->status, test_resp->len));
  ZB_SCHEDULE_ALARM_CANCEL(buffer_test_req_timeout, 0);
  if (ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb)
  {
    ZB_SCHEDULE_CALLBACK(ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb, test_resp->status);
  }
  zb_free_buf(asdu);
  ZG->zdo.test_prof_ctx.zb_tp_buffer_test_request.user_cb = NULL;
}


void tp_packet_ack(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_aps_hdr_t aps_hdr;
  zb_buffer_test_response_param_t *test_param;
  zb_buffer_test_response_t *test_resp;

  TRACE_MSG(TRACE_APS3, ">>tp_packet_ack: param %hd", (FMT__H, param));

  zb_aps_hdr_parse(asdu, &aps_hdr, ZB_FALSE);
  ZB_APS_HDR_CUT(asdu);

  if (aps_hdr.clusterid == TP_BUFFER_TEST_RESPONSE_CLID)
  {
    if (asdu->u.hdr.status == 0)
    {
      TRACE_MSG(TRACE_APS3, "buffer test ack ok", (FMT__0));
      zb_free_buf(asdu);
    }
    else
    {
      TRACE_MSG(TRACE_APS3, "buffer test ack failed", (FMT__0));

      test_resp = (zb_buffer_test_response_t*)ZB_BUF_BEGIN(asdu);
      test_param = ZB_GET_BUF_PARAM(asdu, zb_buffer_test_response_param_t);
      test_param->len = test_resp->len;
      test_param->status = ZB_TP_BUFFER_TEST_FAIL;
      test_param->dst_addr = aps_hdr.src_addr;
      tp_buffer_test_response(param);
    }
  }
  else
  {
    TRACE_MSG(TRACE_APS3, "unknown cluster id %x", (FMT__D, aps_hdr.clusterid));
    zb_free_buf(asdu);
  }
  TRACE_MSG(TRACE_APS3, "<< tp_packet_ack", (FMT__0));
}

#endif /* ZB_TEST_PROFILE */

/*! @} */
