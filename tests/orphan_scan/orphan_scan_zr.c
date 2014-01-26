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

#ifndef ZB_ROUTER_ROLE
#error Router role is not compiled!
#endif

/*! \addtogroup ZB_TESTS */
/*! @{ */

/*
  ZR joins to ZC, then rejoins using orphan procedure
 */


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
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
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



void do_rejoin(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_join_request_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_join_request_t);

  ZB_MEMSET(req, 0, sizeof(*req)); /* all defaults to 0 */
  ZB_EXTPANID_COPY(req->extended_pan_id, ZB_PIB_EXTENDED_ADDRESS() );
  req->rejoin_network = ZB_NLME_REJOIN_METHOD_DIRECT;
  req->scan_channels = ZB_DEFAULT_APS_CHANNEL_MASK;

  TRACE_MSG(TRACE_ERROR, "Device started OK first time - initiate orphan rejoin", (FMT__0));
  ZB_SCHEDULE_CALLBACK(zb_nlme_join_request, param);
}



void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  static zb_ushort_t cnt = 0;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    if (cnt == 0)
    {
      ZB_SCHEDULE_ALARM(do_rejoin, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(10000));
      cnt++;
    }
    else
    {
      TRACE_MSG(TRACE_ERROR, "Device started second time - test pass SUCCESS", (FMT__0));
      zb_free_buf(buf);
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}


/*! @} */
