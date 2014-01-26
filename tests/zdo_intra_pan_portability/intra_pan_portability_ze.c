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

/*! \addtogroup ZB_TESTS */
/*! @{ */

#if !defined ZB_PLATFORM_LINUX_ARM_2400 && !defined ZB_PLATFORM_LINUX_PC32
#ifndef ZB_CHANNEL_ERROR_TEST
#error define ZB_CHANNEL_ERROR_TEST in zb_config.h
#endif
#endif

void send_data(zb_uint8_t param) ZB_CALLBACK;

zb_ieee_addr_t g_ieee_addr = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

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
  ZB_INIT("zdo_zed", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zed", "2", "2");
#endif

  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
#endif
#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif

  /* become an ED */
  ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ED;
  ZB_PIB_RX_ON_WHEN_IDLE() = ZB_TRUE;

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
    /* send smth to the parent periodicaly */
    ZB_SCHEDULE_ALARM(send_data, param, ZB_TIME_ONE_SECOND);
    ZB_MAC_SET_CHANNEL_ERROR_TEST();
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void send_data(zb_uint8_t param)
{
  zb_uint8_t *ptr = NULL;
  zb_short_t i;
  zb_buf_t *buf = NULL;

  if ( !ZG->nwk.handle.joined )
  {
    ZB_SCHEDULE_ALARM(send_data, param, ZB_TIME_ONE_SECOND);
    return;
  }

  if ( !param )
  {
    ZB_GET_OUT_BUF_DELAYED(send_data);
    return;
  }
  buf = ZB_BUF_FROM_REF(param);
  ZB_BUF_INITIAL_ALLOC(buf, 70, ptr);
  for (i = 0 ; i < 70 ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }

  {
    zb_apsde_data_req_t *req;

    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    /* send to parent */
    zb_address_short_by_ref(&req->dst_addr.addr_short, ZG->nwk.handle.parent);
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;

    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;

    buf->u.hdr.handle = 0x11;
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
  }
  ZB_SCHEDULE_ALARM(send_data, 0, ZB_TIME_ONE_SECOND);
}

/*! @} */
