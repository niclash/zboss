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
PURPOSE: Test for ZC application written using ZDO.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

/*! \addtogroup ZB_TESTS */
/*! @{ */

/* For NS build first ieee addr byte should be unique */
#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_ieee_addr   = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_ieee_addr_d = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_ieee_addr   = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_ieee_addr_d = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
#endif


void data_indication(zb_uint8_t param) ZB_CALLBACK;
void send_data(zb_uint8_t param) ZB_CALLBACK;

void zb_leave_req(zb_uint8_t param) ZB_CALLBACK;
void zb_nwk_leave_req(zb_uint8_t param) ZB_CALLBACK;
void zb_leave_callback(zb_uint8_t param) ZB_CALLBACK;

zb_bool_t flag;

MAIN()
{
  ARGV_UNUSED;

  flag = ZB_TRUE;
#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_start_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_start_zc", "1", "1");
#endif

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;

  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

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

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_ERROR, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(data_indication);
    if (flag)
    {
      ZB_SCHEDULE_ALARM(zb_leave_req, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(120000));
      flag = ZB_FALSE;
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device START FAILED", (FMT__0));
    zb_free_buf(buf);
  }
}

void zb_leave_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t *ret = (zb_uint8_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));

  TRACE_MSG(TRACE_ERROR, "LEAVE MGMT CALLBACK %hd", (FMT__H, *ret));
}

void zb_leave_req(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  ZB_GET_OUT_BUF_DELAYED(zb_nwk_leave_req);
}

void zb_nwk_leave_req(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_leave_param_t *req = NULL;

  TRACE_MSG(TRACE_ERROR, "zb_leave_req", (FMT__0));

  req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_leave_param_t);

  ZB_64BIT_ADDR_ZERO(req->device_address);
  req->remove_children = ZB_FALSE;
  req->rejoin = ZB_FALSE;
  req->dst_addr = 0x0aa2;
  zdo_mgmt_leave_req(param, zb_leave_callback);
}

void data_indication(zb_uint8_t param) ZB_CALLBACK
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
  send_data(param);
}

void send_data(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t ind = *ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, 30, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = ind.src_addr;
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = 0;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 1;
  req->dst_endpoint = 2;
  buf->u.hdr.handle = 0x11;
  for (i = 0 ; i < 30 ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }
  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

/*! @} */

