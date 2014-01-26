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
PURPOSE: Network confirm routine
*/

#include <string.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

/*! \addtogroup ZB_APS */
/*! @{ */

#include "zb_bank_6.h"

/* Note: all that callbacks are temporary here. It must be somewhere in */

#ifndef ZB_LIMITED_FEATURES
void zb_nlme_route_discovery_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_route_discovery_confirm %p", (FMT__P, buf));

  if ( buf )
  {
    zb_nlme_route_discovery_confirm_t *confirm = (zb_nlme_route_discovery_confirm_t *)ZB_BUF_BEGIN(buf);
    ZVUNUSED(confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_route_discovery_confirm", (FMT__0));
}
#endif


#if 0
void zb_nlme_sync_loss_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_sync_loss_indication %hd", (FMT__H, param));

  ZVUNUSED(buf);
  /* TODO: implement */

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_sync_loss_indication %d", (FMT__D, ret));
}
#endif
/*! @} */
