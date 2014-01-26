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
PURPOSE: ZDO common functions, both client and server side
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
/*! \addtogroup ZB_ZDO */
/*! @{ */

#ifndef ZB_LIMITED_FEATURES2
static void zdo_send_req(zb_uint8_t param, zb_callback_t cb, zb_uint8_t resp_counter) ZB_SDCC_REENTRANT;

void zdo_send_req_by_short(zb_uint16_t command_id, zb_uint8_t param, zb_callback_t cb, zb_uint16_t addr,
                           zb_uint8_t resp_counter)
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

  TRACE_MSG(TRACE_ZDO2, ">> zdo_send_req_by_short ", (FMT__0));
  ZB_BZERO(dreq, sizeof(*dreq));
  dreq->dst_addr.addr_short = addr;
  if (!ZB_NWK_IS_ADDRESS_BROADCAST(addr))
  {
    dreq->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  }
  dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;

  dreq->clusterid = command_id;
  zdo_send_req(param, cb, resp_counter);
  TRACE_MSG(TRACE_ZDO2, "<< zdo_send_req_by_short ", (FMT__0));
}


void zdo_send_req_by_long(zb_uint8_t command_id, zb_uint8_t param, zb_callback_t cb, zb_ieee_addr_t addr)
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

  ZB_BZERO(dreq, sizeof(*dreq));
  ZB_IEEE_ADDR_COPY(dreq->dst_addr.addr_long, addr);
  dreq->addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
  dreq->clusterid = command_id;
  dreq->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  zdo_send_req(param, cb, 1);
}


static void zdo_send_req(zb_uint8_t param, zb_callback_t cb, zb_uint8_t resp_counter) ZB_SDCC_REENTRANT
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));
  zb_uint8_t *tsn_p;

  TRACE_MSG(TRACE_ZDO2, ">> zdo_send_req ", (FMT__0));
  ZB_BUF_ALLOC_LEFT(ZB_BUF_FROM_REF(param), 1, tsn_p);
  ZDO_CTX().tsn++;
  *tsn_p = ZDO_CTX().tsn;
  if (dreq->clusterid == ZDO_SYSTEM_SERVER_DISCOVERY_REQ_CLID)
  {
    ZDO_CTX().system_server_discovery_tsn = ZDO_CTX().tsn;
  }
  register_zdo_cb(ZDO_CTX().tsn, cb, resp_counter);

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
  TRACE_MSG(TRACE_ZDO2, "<< zdo_send_req ", (FMT__0));
}


void zdo_send_resp_by_short(zb_uint16_t command_id, zb_uint8_t param, zb_uint8_t tsn, zb_uint16_t addr) ZB_SDCC_REENTRANT
{
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));
  zb_uint8_t *tsn_p;

  TRACE_MSG(TRACE_ZDO3, "zdo_send_resp_by_short, command_id %d, param %hd, tsn %hd, addr %d", (FMT__D_H_H_D, command_id, param, tsn, addr));
  ZB_BZERO(dreq, sizeof(*dreq));
  dreq->dst_addr.addr_short = addr;
  if (!ZB_NWK_IS_ADDRESS_BROADCAST(addr))
  {
    dreq->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  }
  dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  dreq->clusterid = command_id;
  ZB_BUF_ALLOC_LEFT(ZB_BUF_FROM_REF(param), 1, tsn_p);
  *tsn_p = tsn;
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}


void register_zdo_cb(zb_uint8_t tsn, zb_callback_t cb, zb_uint8_t resp_counter) ZB_SDCC_REENTRANT
{
  zb_ushort_t h_i = ZB_TSN_HASH(tsn);
  zb_ushort_t i = h_i;
  /* TODO: add timeout for callbacks - we need to clean it if
   * response didn't come */

  if (cb)
  {
    do
    {
      if (ZDO_CTX().zdo_cb[i].func == NULL)
      {
        /* found free slot */
        ZDO_CTX().zdo_cb[i].tsn = tsn;
        ZDO_CTX().zdo_cb[i].func = cb;
        ZDO_CTX().zdo_cb[i].resp_counter = resp_counter;
        return;
      }
      i = (i + 1) % ZDO_TRAN_TABLE_SIZE;
    }
    while ( i != h_i );
    TRACE_MSG(TRACE_ERROR, "No sp for ZDO cb, tsn 0x%hx", (FMT__H, tsn));
  }
}


/**
   Call user's routine on ZDO response

   This routine called when AF send packet to ZDO, ZDO parse it and detect rsp
   (cluster id starts from 0x8000).
 */
zb_ret_t zdo_af_resp(zb_uint8_t param)
{
  zb_uint8_t tsn = *(zb_uint8_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_ushort_t h_i = ZB_TSN_HASH(tsn);
  zb_ushort_t i = h_i;
  
  zb_buf_cut_left(ZB_BUF_FROM_REF(param), sizeof(tsn));
  TRACE_MSG(TRACE_ZDO2, ">> zdo_af_resp %hd", (FMT__H, tsn));
  do
  {
    if (ZDO_CTX().zdo_cb[i].tsn == tsn
        && ZDO_CTX().zdo_cb[i].func != NULL)
    {
      /* found - schedule it to execution */
      /* Really use zb_schedule_callback(), not ZB_SCHEDULE_CALLBACK(): no
       * effect in Keil but for sdcc ZB_SCHEDULE_CALLBACK constructs stub name. */
      zb_schedule_callback(ZDO_CTX().zdo_cb[i].func, param);

      ZDO_CTX().zdo_cb[i].resp_counter--;
      if (!ZDO_CTX().zdo_cb[i].resp_counter)
      {
        /* mark as free */
        ZDO_CTX().zdo_cb[i].func = NULL;
      }
      TRACE_MSG(TRACE_ZDO2, "<< zdo_af_resp RET_OK", (FMT__0));
      return RET_OK;
    }
    i = (i + 1) % ZDO_TRAN_TABLE_SIZE;
  }
  while ( i != h_i );
  TRACE_MSG(TRACE_ERROR, "tsn 0x%hx not found!", (FMT__H, tsn));
  /* zb_free_buf(ZB_BUF_FROM_REF(param)); free buffer on caller level */
  TRACE_MSG(TRACE_ZDO2, ">> zdo_af_resp NOT FOUND", (FMT__H, tsn));
  return RET_NOT_FOUND;
}

#endif  /* ZB_LIMITED_FEATURES22 */

/*! @} */
