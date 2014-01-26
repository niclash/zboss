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
#include "zb_secur.h"
#include "zb_secur_api.h"

zb_ieee_addr_t g_zc_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

zb_uint8_t g_key[16] = { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};

#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_r1_addr = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_r1_addr = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#endif

#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_r2_addr = {0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_r2_addr = {0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
#endif




/*! \addtogroup ZB_TESTS */
/*! @{ */

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif


void remove_r2(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_remove_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_remove_device_req_t);

  TRACE_MSG(TRACE_SECUR1, "force r2 remove", (FMT__0));

  ZB_IEEE_ADDR_COPY(req->parent_address, g_r1_addr);
  ZB_IEEE_ADDR_COPY(req->child_address, g_r2_addr);
  ZB_SCHEDULE_CALLBACK(zb_secur_apsme_remove_device, param);
}


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
  zb_secur_setup_preconfigured_key(g_key, 0);
  /* assign our address */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;
  /* allow only one join, then let's ZR1 utilize joins */
  ZG->nwk.nib.max_children = 1;
#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_PAN_ID();
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
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    zb_schedule_alarm(remove_r2, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100000));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

/*! @} */
