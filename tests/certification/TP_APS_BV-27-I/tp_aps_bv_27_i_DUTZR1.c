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
PURPOSE: TP/ÔÇÛ/BV-27-I Router – Joins the network
Verify that the router device is capable to join a network.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

void zb_test(zb_uint8_t param);
void send_data(zb_buf_t *buf);
void data_indication(zb_uint8_t param);

#ifdef ZB_NS_BUILD
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000)
#else
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000)
#endif

zb_ieee_addr_t g_ieee_addr   = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#ifdef ZB_EMBER_GOLDEN_UNIT
zb_ieee_addr_t g_ieee_addr_d = {0xF0, 0x23, 0x07, 0x00, 0x00, 0xED, 0x21, 0x00};
#else
zb_ieee_addr_t g_ieee_addr_d = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("tp_aps_bv_27_i_DUTZR1", argv[1], argv[2]);
#else
  ZB_INIT("tp_aps_bv_27_i_DUTZR1", "1", "1");
#endif

  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

  /* join as a router */
  ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
  ZB_NWK().max_children = 0;

#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_PAN_ID();
#endif

#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif
/* TODO: it is temporary, need remove after fix device annonce */
#ifndef ZB_EMBER_GOLDEN_UNIT
  {
    zb_address_ieee_ref_t ref;
    zb_address_update(g_ieee_addr, 0x1, ZB_FALSE, &ref);
  }
  {
    zb_address_ieee_ref_t ref;
    zb_address_update(g_ieee_addr_d, 0xa2, ZB_FALSE, &ref);
  }
#endif
  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}


void get_ieee_addr(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = zb_get_out_buf();
  zb_zdo_ieee_addr_req_t *req_param;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req_param);

  req_param->nwk_addr = zb_address_short_by_ieee(g_ieee_addr_d);
  req_param->start_index = 0;
  req_param->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
  if (param == 0)
  {
    zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), zb_test);
  }
}



void zb_test(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_binding_req_t *req;

  req = ZB_GET_BUF_PARAM(buf, zb_apsme_binding_req_t);
  ZB_MEMCPY(&req->src_addr, &g_ieee_addr, sizeof(zb_ieee_addr_t));
  req->src_endpoint = 1;
  req->clusterid = 1;
  req->addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
  ZB_MEMCPY(&req->dst_addr.addr_long, &g_ieee_addr_d, sizeof(zb_ieee_addr_t));
  req->dst_endpoint = 0xF0;
  zb_apsme_bind_request(ZB_REF_FROM_BUF(buf));
  send_data((zb_buf_t *)ZB_BUF_FROM_REF(param));
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    ZB_SCHEDULE_ALARM(get_ieee_addr, 0, SEND_TMOUT);
    zb_af_set_data_indication(data_indication);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void send_data(zb_buf_t *buf)
{
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, 70, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->addr_mode = ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->clusterid = 1;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 1;

  buf->u.hdr.handle = 0x11;

  for (i = 0 ; i < 70 ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }
  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS3, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS3, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
    if (ptr[i] != i % 32 + '0')
    {
      TRACE_MSG(TRACE_ERROR, "Bad data %hx %c wants %x %c", (FMT__H_C_D_C, ptr[i], ptr[i],
                              (int)(i % 32 + '0'), (char)i % 32 + '0'));
    }
  }
}
