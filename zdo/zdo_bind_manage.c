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
PURPOSE: ZDO Bind management
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
/* todo: this define used only to fit in 64k */
#ifndef ZB_LIMITED_FEATURES

zb_uint8_t* copy_cluster_id(zb_uint8_t *cluster_dst, zb_uint8_t *cluster_src, zb_uint8_t cluster_num) ZB_SDCC_REENTRANT;
static void zb_zdo_bind_unbind_req(zb_uint8_t param, zb_callback_t cb, zb_bool_t bind) ZB_SDCC_REENTRANT;
static void send_bind_unbind_req(zb_uint8_t param, zb_uint8_t target_dev_num, zb_uint16_t cluster_id, zb_callback_t cb, zb_bool_t bind) ZB_SDCC_REENTRANT;

void zb_zdo_send_check_bind_unbind(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_bind_unbind_check_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_end_device_bind_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_end_device_unbind_cb(zb_uint8_t param) ZB_CALLBACK;

void zb_zdo_bind_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_bind_unbind_req(param, cb, ZB_TRUE);
}

void zb_zdo_unbind_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_bind_unbind_req(param, cb, ZB_FALSE);
}

static void zb_zdo_bind_unbind_req(zb_uint8_t param, zb_callback_t cb, zb_bool_t bind) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_req_param_t *bind_param;
  zb_zdo_bind_req_head_t *bind_req;

  TRACE_MSG(TRACE_ZDO2, ">> zb_zdo_bind_unbind_req param %hd, bind %hd", (FMT__D_H, param, bind));
  bind_param = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_bind_req_head_t), bind_req);

  ZB_HTOLE64(bind_req->src_address, bind_param->src_address);
  bind_req->src_endp = bind_param->src_endp;
  ZB_HTOLE16(&bind_req->cluster_id, &bind_param->cluster_id);
  bind_req->dst_addr_mode = bind_param->dst_addr_mode;

  if (bind_param->dst_addr_mode == ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT)
  {
    zb_zdo_bind_req_tail_1_t *bind_req_tail_1;
    ZB_BUF_ALLOC_RIGHT(buf, sizeof(zb_zdo_bind_req_tail_1_t), bind_req_tail_1);
    ZB_HTOLE16(&bind_req_tail_1->dst_addr, &bind_param->dst_address.addr_short);
    TRACE_MSG(TRACE_ZDO3, "dst addr %d", (FMT__D, bind_req_tail_1->dst_addr));
  }
  else if (bind_param->dst_addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT)
  {
    zb_zdo_bind_req_tail_2_t *bind_req_tail_2;
    ZB_BUF_ALLOC_RIGHT(buf, sizeof(zb_zdo_bind_req_tail_2_t), bind_req_tail_2);
    ZB_HTOLE64(bind_req_tail_2->dst_addr, bind_param->dst_address.addr_long);
    bind_req_tail_2->dst_endp = bind_param->dst_endp;

    ZB_DUMP_IEEE_ADDR(bind_req_tail_2->dst_addr);
    TRACE_MSG(TRACE_ZDO3, "dst endpoint %hd", (FMT__H, bind_req_tail_2->dst_endp));
  }
  else
  {
    ZB_ASSERT(0);
  }

  ZB_DUMP_IEEE_ADDR(bind_req->src_address);
  TRACE_MSG(TRACE_ZDO3, "src_endpoint %hd, clusterid %d, addr_mode %hd",
    (FMT__H_D_H, bind_req->src_endp, bind_req->cluster_id, bind_req->dst_addr_mode));

  zdo_send_req_by_short(bind == ZB_TRUE ? ZDO_BIND_REQ_CLID : ZDO_UNBIND_REQ_CLID, param, cb, bind_param->req_dst_addr, 1);

  TRACE_MSG(TRACE_ZDO2, "<< zb_zdo_bind_unbind_req param", (FMT__0));
}

void zb_zdo_bind_unbind_res(zb_uint8_t param, zb_bool_t bind) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_resp_t *bind_resp;
  zb_uint8_t tsn;
  zb_uint8_t *aps_body;
  zb_apsde_data_indication_t *ind;
  zb_uint16_t dst_addr;
#ifndef ZB_DISABLE_BIND_REQ
  zb_zdo_bind_req_head_t *bind_req;
  zb_apsme_binding_req_t *aps_bind_req;
  zb_uint8_t status;
#endif
  TRACE_MSG(TRACE_ZDO2, ">> zb_zdo_bind_res param %hd, bind %hd", (FMT__D_H, param, bind));

  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  dst_addr = ind->src_addr;

  aps_body = ZB_BUF_BEGIN(buf);
  tsn = *aps_body;
  aps_body++;
#ifndef ZB_DISABLE_BIND_REQ
  bind_req = (zb_zdo_bind_req_head_t*)aps_body;
  aps_body += sizeof(zb_zdo_bind_req_head_t);
  aps_bind_req = ZB_GET_BUF_PARAM(buf, zb_apsme_binding_req_t);

  ZB_HTOLE64(aps_bind_req->src_addr, bind_req->src_address);
  aps_bind_req->src_endpoint = bind_req->src_endp;
  ZB_HTOLE16(&aps_bind_req->clusterid, &bind_req->cluster_id);
  aps_bind_req->addr_mode = bind_req->dst_addr_mode;

  if (aps_bind_req->addr_mode == ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT)
  {
    ZB_HTOLE16(&aps_bind_req->dst_addr.addr_short, aps_body);
    TRACE_MSG(TRACE_ZDO3, "dst addr %d", (FMT__D, aps_bind_req->dst_addr.addr_short));
  }
  else if (aps_bind_req->addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT)
  {
    ZB_HTOLE64(&aps_bind_req->dst_addr.addr_long, aps_body);
    aps_body += sizeof(zb_ieee_addr_t);
    aps_bind_req->dst_endpoint = *aps_body;
    ZB_DUMP_IEEE_ADDR(aps_bind_req->dst_addr.addr_long);
    TRACE_MSG(TRACE_ZDO3, "dst_endpoint %hd", (FMT__H, aps_bind_req->dst_endpoint));
  }

  ZB_DUMP_IEEE_ADDR(aps_bind_req->src_addr);
  TRACE_MSG(TRACE_ZDO3, "src_endpoint %hd, clusterid %d, addr_mode %hd, dst_endpoint %hd",
    (FMT__H_D_H_H, aps_bind_req->src_endpoint, aps_bind_req->clusterid, aps_bind_req->addr_mode, aps_bind_req->dst_endpoint));

  if (bind == ZB_TRUE)
  {
    zb_apsme_bind_request(param);
  }
  else
  {
    zb_apsme_unbind_request(param);
  }

  status = buf->u.hdr.status == RET_INVALID_BINDING ? ZB_ZDP_STATUS_NO_ENTRY : buf->u.hdr.status;
#endif
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_bind_resp_t), bind_resp);
#ifdef ZB_DISABLE_BIND_REQ
  bind_resp->status = ZB_ZDP_STATUS_NOT_SUPPORTED;
#else
  bind_resp->status = status;
  TRACE_MSG(TRACE_ZDO3, "send resp, dst addr %d status %d", (FMT__D_D, dst_addr, status));
#endif
  zdo_send_resp_by_short(bind == ZB_TRUE ? ZDO_BIND_RESP_CLID : ZDO_UNBIND_RESP_CLID, param, tsn, dst_addr);
  TRACE_MSG(TRACE_ZDO2, "<< zb_zdo_bind_res", (FMT__0));
}


void zb_zdo_add_group_req(zb_uint8_t param, zb_callback_t cb)
{
  /* TODO: implement using register_zdo_cb? */
  zb_apsme_add_group_request(param);
  zb_schedule_callback(cb, param);
}

void zb_get_peer_short_addr_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  zb_uint16_t nwk_addr;
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));

  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2, "resp status %hd, nwk addr %d", (FMT__H_D, resp->status, resp->nwk_addr));
  ZB_DUMP_IEEE_ADDR(resp->ieee_addr);
  if (resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);
    zb_address_update(ieee_addr, nwk_addr, ZB_TRUE, &addr_ref);
  }
  TRACE_MSG(TRACE_ZDO2, "schedule cb %p param %hd",
            (FMT__P_H, ZDO_CTX().zdo_ctx.get_short_addr_ctx.cb, ZDO_CTX().zdo_ctx.get_short_addr_ctx.param));
  if (ZDO_CTX().zdo_ctx.get_short_addr_ctx.cb)
  {
    ZB_SCHEDULE_CALLBACK(ZDO_CTX().zdo_ctx.get_short_addr_ctx.cb, ZDO_CTX().zdo_ctx.get_short_addr_ctx.param);
  }

  zb_free_buf(buf);
}

/*
   Calls zb_zdo_nwk_addr_req to get peer network address
   param param - buffer reference to use for i/o
 */

void zb_get_peer_short_addr(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_req_param_t *req_param;

  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr param %hd", (FMT__H, param));

  req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_nwk_addr_req_param_t);

  req_param->dst_addr = 0; /* send command to coordinator */
  zb_address_ieee_by_ref(req_param->ieee_addr, ZDO_CTX().zdo_ctx.get_short_addr_ctx.dst_addr_ref);
  req_param->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
  req_param->start_index = 0;
  zb_zdo_nwk_addr_req(param, zb_get_peer_short_addr_cb);
}

void zb_start_get_peer_short_addr(zb_address_ieee_ref_t dst_addr_ref, zb_callback_t cb, zb_uint8_t param)
{
  TRACE_MSG(TRACE_ZDO2, "zb_start_get_peer_short_addr dst_addr_ref %hd", (FMT__H, dst_addr_ref));
  ZDO_CTX().zdo_ctx.get_short_addr_ctx.dst_addr_ref = dst_addr_ref;
  ZDO_CTX().zdo_ctx.get_short_addr_ctx.cb = cb;
  ZDO_CTX().zdo_ctx.get_short_addr_ctx.param = param;
  ZB_GET_OUT_BUF_DELAYED(zb_get_peer_short_addr);
}

static void copy_end_device_bind_req_head(zb_zdo_end_device_bind_req_head_t *dst_head, zb_zdo_end_device_bind_req_head_t *src_head) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO2, "copy_end_device_bind_req_head", (FMT__0));
  ZB_HTOLE16(&dst_head->binding_target, &src_head->binding_target);
  ZB_HTOLE64(dst_head->src_ieee_addr, src_head->src_ieee_addr);
  dst_head->src_endp = src_head->src_endp;
  ZB_HTOLE16(&dst_head->profile_id, &src_head->profile_id);
  dst_head->num_in_cluster = src_head->num_in_cluster;
}

void zb_end_device_bind_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_end_device_bind_req_param_t *req_param_init;
  zb_end_device_bind_req_param_t *req_param;
  zb_zdo_end_device_bind_req_head_t *req_head;
  zb_zdo_end_device_bind_req_tail_t *req_tail;

  TRACE_MSG(TRACE_ZDO2, "zb_end_device_bind_req, param %hd", (FMT__H, param));

  req_param_init = (zb_end_device_bind_req_param_t*)ZB_BUF_BEGIN(buf);
  req_param = ZB_GET_BUF_TAIL(buf, sizeof(zb_end_device_bind_req_param_t) +
    (req_param_init->head_param.num_in_cluster + req_param_init->tail_param.num_out_cluster) * sizeof(zb_uint16_t));

  ZB_MEMCPY(req_param, req_param_init, sizeof(zb_end_device_bind_req_param_t) +
    (req_param_init->head_param.num_in_cluster + req_param_init->tail_param.num_out_cluster) * sizeof(zb_uint16_t));

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_end_device_bind_req_head_t) + sizeof(zb_zdo_end_device_bind_req_tail_t) +
                       (req_param->head_param.num_in_cluster + req_param->tail_param.num_out_cluster) * sizeof(zb_uint16_t),
                       req_head);
  copy_end_device_bind_req_head(req_head, &req_param->head_param);
  req_tail = (zb_zdo_end_device_bind_req_tail_t*)copy_cluster_id(
    (zb_uint8_t*)(req_head + 1), (zb_uint8_t *)req_param->cluster_list, req_param->head_param.num_in_cluster);

  req_tail->num_out_cluster = req_param->tail_param.num_out_cluster;
  copy_cluster_id((zb_uint8_t*)(req_tail + 1), (zb_uint8_t *)(req_param->cluster_list + req_param->head_param.num_in_cluster),
                  req_param->tail_param.num_out_cluster);
  TRACE_MSG(TRACE_ZDO3, "binding_target %d, src_endp %hd, profile_id %d, num_in_cluster %hd, num_out_cluster %hd",
            (FMT__D_H_D_H_H, req_head->binding_target, req_head->src_endp, req_head->profile_id, req_head->num_in_cluster,
             req_tail->num_out_cluster));
  ZB_DUMP_IEEE_ADDR(req_head->src_ieee_addr);

  zdo_send_req_by_short(ZDO_END_DEVICE_BIND_REQ_CLID, param, cb, req_param->dst_addr, 1);
}

void zb_zdo_end_device_bind_timer(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_ZDO2, "zb_zdo_end_device_bind_timer param %hd", (FMT__H, param));
  ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param = ZB_UNDEFINED_BUFFER;
  ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param = ZB_UNDEFINED_BUFFER;
  zb_zdo_end_device_bind_resp(param, ZB_ZDP_STATUS_TIMEOUT);
}
#ifndef ZB_DISABLE_ED_BIND_REQ
static void check_cluster_list(zb_uint8_t device_num,
                               zb_uint8_t *cluster_list1, zb_uint8_t num_cluster1,
                               zb_uint8_t *cluster_list2, zb_uint8_t num_cluster2) ZB_SDCC_REENTRANT
{
  zb_uint8_t i;
  zb_uint8_t j;
  zb_uint16_t cluster_id;

  TRACE_MSG(TRACE_ZDO3, "check cluster device num %hd, list1 %p, num1 %hd, list2 %p, num2 %hd",
            (FMT__H_P_H_P_H, device_num, (zb_uint8_t *)cluster_list1, num_cluster1, (zb_uint8_t *)cluster_list2, num_cluster2));

  ZDO_CTX().end_device_bind_ctx.bind_device_info[device_num].cluster_num = 0;
  for(i = 0; i < num_cluster1; i++)
  {
    for (j =0; j < num_cluster2; j++)
    {
      if (!ZB_MEMCMP(cluster_list1 + i * sizeof(zb_uint16_t), cluster_list2 + j * sizeof(zb_uint16_t), sizeof(zb_uint16_t)))
      {
        /* convert endian - it was not converted on receive */
        ZB_LETOH16(&cluster_id, cluster_list1 + i * sizeof(zb_uint16_t));
        ZDO_CTX().end_device_bind_ctx.bind_device_info[device_num].cluster_list[
          ZDO_CTX().end_device_bind_ctx.bind_device_info[device_num].cluster_num++] = cluster_id;

        if (ZDO_CTX().end_device_bind_ctx.bind_device_info[device_num].cluster_num >= ZB_ZDO_MAX_CLUSTER_LIST)
        {
          /* TODO: make cluster_list memory dynamic */
          TRACE_MSG(TRACE_ZDO2, "max cluster num exceeded!", (FMT__0));
          break;
        }
        TRACE_MSG(TRACE_ZDO2, "add cluster id %d, num %hd",
                  (FMT__D_H, cluster_id, ZDO_CTX().end_device_bind_ctx.bind_device_info[device_num].cluster_num));
      }
    }
  }
}
#endif
void zb_zdo_end_device_bind_handler(zb_uint8_t param) ZB_SDCC_REENTRANT
{
#ifdef ZB_DISABLE_ED_BIND_REQ
  zb_zdo_end_device_bind_resp(param, ZB_ZDP_STATUS_NOT_SUPPORTED);
#else
  zb_buf_t *buf1 = ZB_BUF_FROM_REF(param);
  zb_zdo_end_device_bind_req_head_t *req_head1;
  zb_zdo_end_device_bind_req_head_t *req_head2;
  zb_zdo_end_device_bind_req_tail_t *req_tail1;
  zb_zdo_end_device_bind_req_tail_t *req_tail2;
  zb_uint8_t *aps_body;
  zb_zdp_status_t status = ZB_ZDP_STATUS_SUCCESS;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_end_device_bind_handler, param %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(buf1);
  req_head2 = (zb_zdo_end_device_bind_req_head_t*)(aps_body + 1);

  TRACE_MSG(TRACE_ZDO3, "endp %hd, end_device_bind_param[0] %hd end_device_bind_param[1] %hd",
            (FMT__H_H_H, req_head2->src_endp, ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param,
             ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param));
  if (req_head2->src_endp < ZB_MIN_ENDPOINT_NUMBER || req_head2->src_endp > ZB_MAX_ENDPOINT_NUMBER)
  {
    /* send resp with fail status */
    zb_zdo_end_device_bind_resp(param, ZB_ZDP_STATUS_INVALID_EP);
  }
  else
  {
    if (ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param == ZB_UNDEFINED_BUFFER)
    {
      /* Schedule time out and wait for the second end device bund request */
      ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param = param;
      ZB_SCHEDULE_ALARM(zb_zdo_end_device_bind_timer, param, /*ZDO_CTX().conf_attr.enddev_bind_timeout*/60 * ZB_TIME_ONE_SECOND);
    }
    else if (ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param != ZB_UNDEFINED_BUFFER)
    {
      /* bind end device process is in progress, so return error to
       * this new request. Spec doesn't have BUSY error code, so use TIMEOUT */
      TRACE_MSG(TRACE_ZDO3, "return error TIMEOUT status", (FMT__0));
      zb_zdo_end_device_bind_resp(param, ZB_ZDP_STATUS_TIMEOUT);
    }
    else
    {
      ZB_SCHEDULE_ALARM_CANCEL(zb_zdo_end_device_bind_timer, ZB_ALARM_ALL_CB);
      buf1 = ZB_BUF_FROM_REF(ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param);
      aps_body = ZB_BUF_BEGIN(buf1);
      req_head1 = (zb_zdo_end_device_bind_req_head_t*)(aps_body + 1);

      /* both requests req_head1 and req_head2 has not converted
       * LETOH parameters - compare it without conversion */
      TRACE_MSG(TRACE_ZDO3, "req1 binding_target %d, src_endp %hd, profile_id %d, num_in_cluster %hd",
                (FMT__D_H_D_H, req_head1->binding_target, req_head1->src_endp, req_head1->profile_id, req_head1->num_in_cluster));

      TRACE_MSG(TRACE_ZDO3, "req2 binding_target %d, src_endp %hd, profile_id %d, num_in_cluster %hd",
                (FMT__D_H_D_H, req_head2->binding_target, req_head2->src_endp, req_head2->profile_id, req_head2->num_in_cluster));
      if (req_head1->profile_id != req_head2->profile_id)
      {
        status = ZB_ZDP_STATUS_NO_MATCH;
      }
      else
      {
        req_tail1 = (zb_zdo_end_device_bind_req_tail_t*)((zb_uint8_t*)req_head1 + sizeof(zb_zdo_end_device_bind_req_head_t) +
                                                         req_head1->num_in_cluster * sizeof(zb_uint16_t));
        req_tail2 = (zb_zdo_end_device_bind_req_tail_t*)((zb_uint8_t*)req_head2 + sizeof(zb_zdo_end_device_bind_req_head_t) +
                                                         req_head2->num_in_cluster * sizeof(zb_uint16_t));
        TRACE_MSG(TRACE_ZDO3, "req1 num_out_cluster %hd, req2 num_out_cluster %hd",
                  (FMT__H_H, req_tail1->num_out_cluster, req_tail2->num_out_cluster));

        check_cluster_list(ZB_ZDO_BIND_DEV_1,
                           (zb_uint8_t*)(req_tail1 + 1) /* req1 out cluster list */, req_tail1->num_out_cluster,
                           (zb_uint8_t*)(req_head2 + 1) /* req2 in cluster list */, req_head2->num_in_cluster);
        check_cluster_list(ZB_ZDO_BIND_DEV_2,
                           (zb_uint8_t*)(req_head1 + 1) /* req1 in cluster list */, req_head1->num_in_cluster,
                           (zb_uint8_t*)(req_tail2 + 1) /* req2 out cluster list */, req_tail2->num_out_cluster);

        if (!ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].cluster_num &&
            !ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].cluster_num)
        {
          status = ZB_ZDP_STATUS_NO_MATCH;
        }
      } /* else, profile_id */

      if (status == ZB_ZDP_STATUS_SUCCESS)
      {
        /* Will send bind/unbind requests to devices binding_target1,
         * binding_target2. We have only local binding tables, so
         * binding_target address equal to end_device_bind_req src
         * address and src_ieee_addr is ieee address of the
         * binding_target - do not save it. If binding table is on the
         * primary binding cahce device, we'll need to save
         * src_ieee_addr also  */

        ZB_LETOH16(&ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].binding_target, &req_head1->binding_target);
        ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].src_endp = req_head1->src_endp;

        ZB_LETOH16(&ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].binding_target, &req_head2->binding_target);
        ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].src_endp = req_head2->src_endp;
        ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param = param;

        TRACE_MSG(TRACE_ZDO3, "save ctx dev1 %d, endp %hd, param %hd, cluster_num %hd",
                  (FMT__D_H_H_H,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].binding_target,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].src_endp,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].cluster_num));
        TRACE_MSG(TRACE_ZDO3, "save ctx dev2 %d, endp %hd, param %hd, cluster_num %hd",
                  (FMT__D_H_H_H,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].binding_target,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].src_endp,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param,
                   ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].cluster_num));

        ZB_GET_OUT_BUF_DELAYED(zb_zdo_send_check_bind_unbind);
      }
      /* send response to both devices */
      zb_zdo_end_device_bind_resp(param, status);
      zb_zdo_end_device_bind_resp(ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param, status);
      if (status != ZB_ZDP_STATUS_SUCCESS)
      {
        TRACE_MSG(TRACE_ZDO3, "no match found", (FMT__0));
        ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param = ZB_UNDEFINED_BUFFER;
        ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param = ZB_UNDEFINED_BUFFER;
      }
    } /* else, end_device_bind_param[ZB_ZDO_BIND_DEV_1] */
  }
#endif
}

void zb_zdo_send_check_bind_unbind(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_ZDO3, "zb_zdo_send_check_bind_unbind param %hd", (FMT__H, param));

  if (ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].cluster_num)
  {
    ZDO_CTX().end_device_bind_ctx.current_device = ZB_ZDO_BIND_DEV_1;
  }
  else
  {
    ZDO_CTX().end_device_bind_ctx.current_device = ZB_ZDO_BIND_DEV_2;
  }
  send_bind_unbind_req(param, ZDO_CTX().end_device_bind_ctx.current_device,
                       ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_list[0],
                       zb_zdo_bind_unbind_check_cb, ZB_FALSE);

}

static void send_bind_unbind_req(zb_uint8_t param, zb_uint8_t target_dev_num, zb_uint16_t cluster_id, zb_callback_t cb, zb_bool_t bind) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_req_param_t *bind_param;
  zb_uint8_t peer_dev_num = ZB_ZDO_PEER_DEVICE_NUM(target_dev_num);

  TRACE_MSG(TRACE_ZDO3, "send_bind_unbind_req param %hd, dev num %hd, peer %hd, cluster_id %d, bind %hd, cb %p",
            (FMT__H_H_H_D_H_P, param, target_dev_num, peer_dev_num, cluster_id, (zb_uint8_t)bind, cb));

  zb_buf_initial_alloc(buf, 0);
  bind_param = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);
  zb_address_ieee_by_short(ZDO_CTX().end_device_bind_ctx.bind_device_info[target_dev_num].binding_target, bind_param->src_address);
  bind_param->src_endp = ZDO_CTX().end_device_bind_ctx.bind_device_info[target_dev_num].src_endp;

  bind_param->cluster_id = cluster_id;
  bind_param->dst_addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;

  zb_address_ieee_by_short(ZDO_CTX().end_device_bind_ctx.bind_device_info[peer_dev_num].binding_target,
                           bind_param->dst_address.addr_long);

  bind_param->dst_endp = ZDO_CTX().end_device_bind_ctx.bind_device_info[peer_dev_num].src_endp;

  bind_param->req_dst_addr = ZDO_CTX().end_device_bind_ctx.bind_device_info[target_dev_num].binding_target;
  TRACE_MSG(TRACE_COMMON1, "dst addr %d", (FMT__D, bind_param->req_dst_addr));

  if (bind)
  {
    zb_zdo_bind_req(param, cb);
  }
  else
  {
    zb_zdo_unbind_req(param, cb);
  }
}

static void send_bind_unbind_with_check(zb_uint8_t param, zb_bool_t bind, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO2, "send_bind_unbind_with_check param %hd, bind %hd, cur device %hd, cluster_num %hd",
            (FMT__H_H_H_H, param, bind, ZDO_CTX().end_device_bind_ctx.current_device,
             ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_num));

  if (!ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_num)
  {
    ZDO_CTX().end_device_bind_ctx.current_device++;
    if (ZDO_CTX().end_device_bind_ctx.current_device == ZB_ZDO_BIND_DEV_2)
    {
      ZB_SCHEDULE_CALLBACK(cb, param);
    }
    else
    {
      TRACE_MSG(TRACE_ZDO2, "All bind req are sent, free buf", (FMT__0));
      ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param = ZB_UNDEFINED_BUFFER;
      ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param = ZB_UNDEFINED_BUFFER;
      zb_free_buf(ZB_BUF_FROM_REF(param));
    }
  }
  else
  {
    send_bind_unbind_req(param, ZDO_CTX().end_device_bind_ctx.current_device,
                         ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_list[
                           ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_num - 1],
                         cb, bind);
    ZDO_CTX().end_device_bind_ctx.bind_device_info[ZDO_CTX().end_device_bind_ctx.current_device].cluster_num--;
    }
}

void zb_zdo_bind_unbind_check_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_resp_t *resp = (zb_zdo_bind_resp_t*)ZB_BUF_BEGIN(buf);
  zb_bool_t bind_req;
  zb_callback_t cb;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_bind_unbind_check_cb resp param %hd, status %hd", (FMT__H_H, param, resp->status));

  if (resp->status == ZB_ZDP_STATUS_NO_ENTRY)
  {
    TRACE_MSG(TRACE_COMMON1, "will do bind", (FMT__0));
    bind_req = ZB_TRUE;
    cb = zb_zdo_end_device_bind_cb;
  }
  else
  {
    TRACE_MSG(TRACE_COMMON1, "will do unbind", (FMT__0));
    bind_req = ZB_FALSE;
    cb = zb_zdo_end_device_unbind_cb;

  }
  send_bind_unbind_with_check(param, bind_req, cb);
}

void zb_zdo_end_device_bind_cb(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_ZDO2, "zb_zdo_end_device_bind_cb param %hd", (FMT__H, param));
  send_bind_unbind_with_check(param, ZB_TRUE, zb_zdo_end_device_bind_cb);
}

void zb_zdo_end_device_unbind_cb(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_ZDO2, "zb_zdo_end_device_unbind_cb param %hd", (FMT__H, param));
  send_bind_unbind_with_check(param, ZB_FALSE, zb_zdo_end_device_unbind_cb);
}

void zb_zdo_end_device_bind_resp(zb_uint8_t param, zb_zdp_status_t status) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_end_device_bind_resp_t *resp;
  zb_apsde_data_indication_t *ind;
  zb_uint8_t tsn;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_end_device_bind_resp param %hd, status %hd", (FMT__H_H, param, status));

  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  tsn = *ZB_BUF_BEGIN(buf);

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_end_device_bind_resp_t), resp);
  resp->status = status;

  TRACE_MSG(TRACE_ZDO3, "send resp, dst addr %d, tsn %hd", (FMT__D_H, ind->src_addr, tsn));
  zdo_send_resp_by_short(ZDO_END_DEVICE_BIND_RESP_CLID, param, tsn, ind->src_addr);
}
#endif /* ZB_LIMITED_FEATURES */
/*! @} */
