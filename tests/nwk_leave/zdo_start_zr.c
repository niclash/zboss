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

#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_ieee_addr_d   = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_ieee_addr = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_ieee_addr_d   = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_ieee_addr = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
#endif

void send_data(zb_uint8_t param) ZB_CALLBACK;
void data_indication(zb_uint8_t param) ZB_CALLBACK;

void zb_leave_req(zb_uint8_t param) ZB_CALLBACK;
void zb_nwk_leave_req(zb_uint8_t param) ZB_CALLBACK;
void zb_leave_callback(zb_uint8_t param) ZB_CALLBACK;

void zb_start_join(zb_uint8_t param) ZB_CALLBACK;
zb_ret_t initiate_rejoin1(zb_buf_t *buf);

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
  ZB_INIT("zdo_start_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_start_zr", "1", "1");
#endif

  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

  /* join as a router */
  ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;

#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
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

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(data_indication);
    if (flag)
    {
      zb_leave_req(param);
      ZB_SCHEDULE_ALARM(send_data, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000));
      ZB_SCHEDULE_ALARM(zb_start_join, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(10000));

      /* rejoin after leave from ZC */
      ZB_SCHEDULE_ALARM(zb_start_join, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100000));
      ZB_SCHEDULE_ALARM(send_data, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(150000));
      flag = ZB_FALSE;
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
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

  ZB_64BIT_ADDR_COPY(req->device_address, g_ieee_addr);
  /* for local leave mgmt
  ZB_64BIT_ADDR_ZERO(req->device_address);
  */
  req->remove_children = ZB_FALSE;
  req->rejoin = ZB_FALSE;
  req->dst_addr = 0x0; /*for local leave mgmt 0x1*/
  zdo_mgmt_leave_req(param, zb_leave_callback);
}

zb_ret_t initiate_rejoin1(zb_buf_t *buf)
{
  zb_nlme_join_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_join_request_t);

  ZB_MEMSET(req, 0, sizeof(*req));

  ZB_EXTPANID_COPY(req->extended_pan_id, ZB_AIB().aps_use_extended_pan_id);
#ifdef ZB_ROUTER_ROLE
  if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE)
  {
    ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
    ZB_MAC_CAP_SET_ROUTER_CAPS(req->capability_information);
    TRACE_MSG(TRACE_APS1, "Rejoin to pan " TRACE_FORMAT_64 " as ZR", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
  }
  else
#endif
  {
    TRACE_MSG(TRACE_APS1, "Rejoin to pan " TRACE_FORMAT_64 " as ZE", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
    if (MAC_PIB().mac_rx_on_when_idle)
    {
      ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(req->capability_information, 1);
    }
  }
  ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(req->capability_information, 1);
  req->rejoin_network = ZB_NLME_REJOIN_METHOD_REJOIN;
  req->scan_channels = ZB_AIB().aps_channel_mask;
  req->scan_duration = ZB_DEFAULT_SCAN_DURATION;
  ZG->zdo.handle.rejoin = 1;
  ZG->nwk.handle.joined = 0;

  return ZB_SCHEDULE_CALLBACK(zb_nlme_join_request, ZB_REF_FROM_BUF(buf));
}

void zb_start_join(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;

  ZVUNUSED(param);
  TRACE_MSG(TRACE_ERROR, "rejoin", (FMT__0));

  ZDO_CTX().zdo_ctx.discovery_ctx.disc_count = ZDO_CTX().conf_attr.nwk_scan_attempts;;
  ZG->zdo.handle.started = 0;
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

  ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;

#ifdef ZB_SECURITY
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif
  if (!ZB_EXTPANID_IS_ZERO(ZB_AIB().aps_use_extended_pan_id))
  {
    zb_buf_t *buf = zb_get_out_buf();

    ret = initiate_rejoin1(buf);
  }
  else
  {
    zb_buf_t *buf = zb_get_out_buf();
    zb_nlme_network_discovery_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_network_discovery_request_t);
#ifdef ZB_ROUTER_ROLE
    if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE)
    {
      ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
    }
#endif
    req->scan_channels = ZB_AIB().aps_channel_mask;
    req->scan_duration = ZB_DEFAULT_SCAN_DURATION;
    TRACE_MSG(TRACE_APS1, "disc, then join by assoc", (FMT__0));
    ret = ZB_SCHEDULE_CALLBACK(zb_nlme_network_discovery_request, ZB_REF_FROM_BUF(buf));
  }
}

void send_data(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, 30, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0; /* send to ZC */
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = 0;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 2;
  req->dst_endpoint = 1;

  buf->u.hdr.handle = 0x11;

  for (i = 0 ; i < 30 ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }
  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
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
}

/*! @} */

