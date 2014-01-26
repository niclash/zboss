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
PURPOSE: ZDO Discovery services - client side.
Mandatory calls onnly. Other calls will be implemented in some other project scope.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_hash.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"

#include "zb_bank_9.h"
/*! \addtogroup ZB_ZDO */
/*! @{ */

void zb_zdo_nwk_addr_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_req_param_t *req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_nwk_addr_req_param_t);
  zb_zdo_nwk_addr_req_t *req;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_nwk_addr_req param %hd", (FMT__H, param));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_nwk_addr_req_t), req);
  ZB_HTOLE64(req->ieee_addr, req_param->ieee_addr);
  req->request_type = req_param->request_type;
  req->start_index = req_param->start_index;

  zdo_send_req_by_short(ZDO_NWK_ADDR_REQ_CLID, param, cb, req_param->dst_addr, 1);
}


void zb_zdo_ieee_addr_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_ieee_addr_req_t *req = (zb_zdo_ieee_addr_req_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_uint16_t addr = req->nwk_addr;
  ZB_HTOLE16(&req->nwk_addr, &addr);
  zdo_send_req_by_short(ZDO_IEEE_ADDR_REQ_CLID, param, cb, addr, 1);
}


void zb_zdo_node_desc_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_node_desc_req_t *req = (zb_zdo_node_desc_req_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_uint16_t addr = req->nwk_addr;
  ZB_HTOLE16(&req->nwk_addr, &addr);
  zdo_send_req_by_short(ZDO_NODE_DESC_REQ_CLID, param, cb, addr, 1);
}


void zb_zdo_power_desc_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_power_desc_req_t *req = (zb_zdo_power_desc_req_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_uint16_t addr = req->nwk_addr;
  ZB_HTOLE16(&req->nwk_addr, &addr);
  zdo_send_req_by_short(ZDO_POWER_DESC_REQ_CLID, param, cb, addr, 1);
}


void zb_zdo_simple_desc_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_simple_desc_req_t *req = (zb_zdo_simple_desc_req_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_uint16_t addr = req->nwk_addr;
  ZB_HTOLE16(&req->nwk_addr, &addr);
  zdo_send_req_by_short(ZDO_SIMPLE_DESC_REQ_CLID, param, cb, addr, 1);
}


void zb_zdo_active_ep_req(zb_uint8_t param, zb_callback_t cb)
{
  zb_zdo_active_ep_req_t *req = (zb_zdo_active_ep_req_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_uint16_t addr = req->nwk_addr;
  ZB_HTOLE16(&req->nwk_addr, &addr);
  zdo_send_req_by_short(ZDO_ACTIVE_EP_REQ_CLID, param, cb, addr, 1);
}


void zb_zdo_match_desc_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_zdo_match_desc_req_head_t *req_head;
  zb_zdo_match_desc_req_tail_t *req_tail;
  zb_zdo_match_desc_param_t *match_param;
  zb_zdo_match_desc_param_t *match_param_init;
  zb_uint8_t *req_cluster_id;
  zb_uint16_t *cluster_id;
  zb_uint_t i;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ZDO3, ">>zb_zdo_match_desc_req param %hd", (FMT__D, param));

  match_param_init = (zb_zdo_match_desc_param_t*)ZB_BUF_BEGIN(buf);
  match_param = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_match_desc_param_t) +
    (match_param_init->num_in_clusters + match_param_init->num_out_clusters) * sizeof(zb_uint16_t));
  ZB_MEMCPY(match_param, match_param_init, sizeof(zb_zdo_match_desc_param_t) +
    (match_param_init->num_in_clusters + match_param_init->num_out_clusters) * sizeof(zb_uint16_t));

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_match_desc_req_head_t) + match_param->num_in_clusters * sizeof(zb_uint16_t) +
                       sizeof(zb_zdo_match_desc_req_tail_t) + match_param->num_out_clusters * sizeof(zb_uint16_t), req_head);

  ZB_HTOLE16(&req_head->nwk_addr, &match_param->nwk_addr);
  ZB_HTOLE16(&req_head->profile_id, &match_param->profile_id);
  req_head->num_in_clusters = match_param->num_in_clusters;

  cluster_id = match_param->cluster_list;
  /* req_cluster_id was pointer to 2-bytes integer, but need ta have it char*
   * to shutup ARM compiler. Really, there is no unaligned access. */
  req_cluster_id = (zb_uint8_t*)(req_head + 1);
  for(i = 0; i < match_param->num_in_clusters; i++)
  {
    ZB_HTOLE16(req_cluster_id, cluster_id);
    req_cluster_id += sizeof(zb_uint16_t);
    cluster_id++;
  }

  req_tail = (zb_zdo_match_desc_req_tail_t*)req_cluster_id;
  
  //ZB_HTOLE16(&req_tail->num_out_clusters, &match_param->num_out_clusters);
  req_tail->num_out_clusters = match_param->num_out_clusters;
  req_cluster_id = (zb_uint8_t*)(req_tail + 1);
  for(i = 0; i < match_param->num_out_clusters; i++)
  {
    ZB_HTOLE16(req_cluster_id, cluster_id);
    req_cluster_id += sizeof(zb_uint16_t);
    cluster_id++;
  }

  TRACE_MSG(TRACE_ZDO3, "addr %d, profile id %d, num in cl %hd, num out cl %hd",
    (FMT__D_D_H_H, req_head->nwk_addr, req_head->profile_id, req_head->num_in_clusters, req_tail->num_out_clusters));

  zdo_send_req_by_short(ZDO_MATCH_DESC_REQ_CLID, param, cb, match_param->nwk_addr, 1);
  TRACE_MSG(TRACE_ZDO3, "<<zb_zdo_match_desc_req", (FMT__0));
}




/*! @} */
