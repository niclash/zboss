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
PURPOSE: Network address assign
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_3.h"

/* all routines called from nwk (bank1) - not need to be banker */

#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE

/* see 3.6.1.6 for Distributed address assign mechanism */
/* and doc/html/nwk/Zigbee-Tree-Routing-How-It-Works-and-Why-It-Sucks.html */
zb_uint16_t zb_nwk_daa_calc_cskip(zb_uint8_t depth ) ZB_SDCC_REENTRANT
{
  zb_uint16_t ret = 0;

  TRACE_MSG(TRACE_NWK1, ">>zb_nwk_daa_calc_cskip depth %d", (FMT__D, depth));

  if ( ZG->nwk.nib.max_routers == 1 )
  {
    ret = 1 + ZG->nwk.nib.max_children*(ZG->nwk.nib.max_depth - depth - 1);
  }
  else
  {
    zb_uint16_t tmp = 1;
    int i, num, den;

    for (i = 0; i < ZG->nwk.nib.max_depth - depth - 1; i++)
    {
      tmp *= ZG->nwk.nib.max_routers;
    }

    num = (int)(1 + ZG->nwk.nib.max_children - ZG->nwk.nib.max_routers - ZG->nwk.nib.max_children*tmp);
    den = (int)(1 - ZG->nwk.nib.max_routers);
    ret = (zb_uint16_t)(num/den);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nwk_daa_calc_cskip %d", (FMT__D, ret));
  return ret;
}

#endif

/*! @} */
