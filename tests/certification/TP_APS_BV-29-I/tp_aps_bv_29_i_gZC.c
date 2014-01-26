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
PURPOSE: TP/APS/BV-29-I Coordinator
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#ifdef ZB_NS_BUILD
#define SEND_TMOUT 1000
#else
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000)
#endif


void zb_bind_callback(zb_uint8_t param);

zb_ieee_addr_t g_ieee_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_ieee_addr_d = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_ieee_addr_d = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#endif
zb_uint8_t i = 0;

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
  ZB_INIT("tp_aps_bv_29_i_gZC", argv[1], argv[2]);
#else
  ZB_INIT("tp_aps_bv_29_i_gZC", "1", "1");
#endif

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  MAC_PIB().mac_pan_id = 0x1aaa;
  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_PAN_ID();
#endif

#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif

  i = 2;
  if ( zdo_dev_start() != RET_OK )
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

void zb_bind_callback(zb_uint8_t param)
{
  zb_uint8_t status = RET_OK;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body = NULL;

  aps_body = ZB_BUF_BEGIN(buf);
  ZB_MEMCPY(&status, aps_body, sizeof(status));

  TRACE_MSG(TRACE_INFO1, "zb_bind_callback %hd", (FMT__H, status));
  if (status == RET_OK)
  {
    zb_zdo_bind_req_param_t *req;

    req = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);
    ZB_MEMCPY(&req->src_address, &g_ieee_addr_d, sizeof(zb_ieee_addr_t));
    req->src_endp = i;
    req->cluster_id = 1;
    req->dst_addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
    ZB_MEMCPY(&req->dst_address.addr_long, &g_ieee_addr, sizeof(zb_ieee_addr_t));
    req->dst_endp = 240;
    req->req_dst_addr = 1;

    zb_zdo_bind_req(ZB_REF_FROM_BUF(buf), zb_bind_callback);
    i++;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "TABLE FULL %hd", (FMT__H, status));
    zb_free_buf(buf);
    return;
  }
}

void zb_test(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_ret_t ret = 0;
  zb_zdo_bind_req_param_t *req;

  req = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);
  ZB_MEMCPY(&req->src_address, &g_ieee_addr_d, sizeof(zb_ieee_addr_t));
  req->src_endp = 1;
  req->cluster_id = 1;
  req->dst_addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
  ZB_MEMCPY(&req->dst_address.addr_long, &g_ieee_addr, sizeof(zb_ieee_addr_t));
  req->dst_endp = 240;
  req->req_dst_addr = 1;
  zb_zdo_bind_req(ZB_REF_FROM_BUF(buf), zb_bind_callback);
  ret = buf->u.hdr.status;
  if (ret == RET_TABLE_FULL)
  {
    TRACE_MSG(TRACE_ERROR, "TABLE FULL %d", (FMT__D, ret));
  }
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ERROR, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    ZB_SCHEDULE_ALARM(zb_test, param, SEND_TMOUT);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device START FAILED", (FMT__0));
  }
}
