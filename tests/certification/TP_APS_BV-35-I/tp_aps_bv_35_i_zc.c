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
PURPOSE:
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

zb_ieee_addr_t g_ieee_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_zr1_ieee_addr = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
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


void unbind_req_cb(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_COMMON1, "bind_req_cb", (FMT__0));
  zb_free_buf(ZB_BUF_FROM_REF(param));
}

void unbind_req(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = zb_get_out_buf();
  zb_zdo_bind_req_param_t *bind_param = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);

  TRACE_MSG(TRACE_COMMON1, "bind_req", (FMT__0));

  ZB_MEMCPY(bind_param->src_address, g_ieee_addr, sizeof(zb_ieee_addr_t));
  bind_param->src_endp = 1;
  bind_param->cluster_id = TP_BUFFER_TEST_REQUEST_CLID;
  bind_param->dst_addr_mode = ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT;
  bind_param->dst_address.addr_short = param;
  bind_param->req_dst_addr = zb_address_short_by_ieee(g_zr1_ieee_addr);
  TRACE_MSG(TRACE_COMMON1, "dst addr %d", (FMT__D, bind_param->req_dst_addr));

  zb_zdo_unbind_req(ZB_REF_FROM_BUF(buf), unbind_req_cb);
}

void bind_req_cb(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_COMMON1, "bind_req_cb", (FMT__0));
  zb_free_buf(ZB_BUF_FROM_REF(param));
}

void bind_req(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = zb_get_out_buf();
  zb_zdo_bind_req_param_t *bind_param = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);

  TRACE_MSG(TRACE_COMMON1, "bind_req", (FMT__0));

  ZB_MEMCPY(bind_param->src_address, g_ieee_addr, sizeof(zb_ieee_addr_t));
  bind_param->src_endp = 1;
  bind_param->cluster_id = TP_BUFFER_TEST_REQUEST_CLID;
  bind_param->dst_addr_mode = ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT;
  bind_param->dst_address.addr_short = param;
  bind_param->req_dst_addr = zb_address_short_by_ieee(g_zr1_ieee_addr);
  TRACE_MSG(TRACE_COMMON1, "dst addr %d", (FMT__D, bind_param->req_dst_addr));

  zb_zdo_bind_req(ZB_REF_FROM_BUF(buf), bind_req_cb);
}


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ERROR, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));

  if (buf->u.hdr.status == RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    ZB_SCHEDULE_ALARM(bind_req, 0x03, 30*ZB_TIME_ONE_SECOND);
    ZB_SCHEDULE_ALARM(bind_req, 0x04, 40*ZB_TIME_ONE_SECOND);

    ZB_SCHEDULE_ALARM(unbind_req, 0x03, 60*ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device START FAILED", (FMT__0));
  }

  zb_free_buf(buf);
}
