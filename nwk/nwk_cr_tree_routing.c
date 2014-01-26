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
PURPOSE: Network tree routing
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"

#include "zb_bank_4.h"
#if defined ZB_NWK_TREE_ROUTING && defined ZB_ROUTER_ROLE

/*! \addtogroup ZB_NWK */
/*! @{ */

void zb_nwk_tree_routing_init() ZB_SDCC_REENTRANT
{
  ZG->nwk.nib.cskip = 0xFFFF;
}

/**
   Find child router to forward packet to

   @return pointer to neighbor table entry, NULL otherwise
 */
static zb_neighbor_tbl_ent_t *zb_nwk_tree_routing_find_router(zb_uint16_t dest_address) ZB_SDCC_REENTRANT
{
  zb_neighbor_tbl_ent_t *ret = NULL;
  zb_address_ieee_ref_t ieee_ref;
  zb_uint16_t lower_address, upper_address;
  int i;

  TRACE_MSG(TRACE_NWK1, ">>zb_nwk_tree_routing_find_router dest_address %d", (FMT__D, dest_address));

  /* try to find exact address first */
  if ( zb_address_by_short(dest_address, ZB_FALSE, ZB_FALSE, &ieee_ref) == RET_OK
       && ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] != (zb_uint8_t)-1 )
  {
    ret = &ZG->nwk.neighbor.base_neighbor[ ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] ];
  }

  if ( !ret )
  {
    /* We need find a child router whose address allocation contains the
     * destination address. Means router address space contains destination
     * address */
    for ( i = 0; i < ZG->nwk.nib.max_routers; i++ )
    {
      lower_address = (zb_uint16_t)(ZG->nwk.nib.cskip * i + 1);
      upper_address = lower_address + ZG->nwk.nib.cskip;

      if ( dest_address > lower_address
           && dest_address < upper_address )
      {
        if ( zb_address_by_short(lower_address, ZB_FALSE, ZB_FALSE, &ieee_ref) == RET_OK
             && ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] != (zb_uint8_t)-1 )
        {
          ret = &ZG->nwk.neighbor.base_neighbor[ ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] ];
        }

        /* stop address space is found */
        break;
      }
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nwk_tree_routing_find_router %p", (FMT__P, ret));
  return ret;
}


zb_neighbor_tbl_ent_t *zb_nwk_tree_routing_route(zb_uint16_t dest_address) ZB_SDCC_REENTRANT
{
  zb_neighbor_tbl_ent_t *ret = NULL;
  zb_uint16_t cskip_parent;
  zb_bool_t route_down;

  TRACE_MSG(TRACE_NWK1, ">>zb_nwk_tree_routing_route dest_address %d", (FMT__D, dest_address));

  /* To route packet up we should calculate parent Cskip value, but if we are
   * the coordinator it's useless cause we will route only down */
  cskip_parent = ( ZG->nwk.nib.depth ) ? zb_nwk_daa_calc_cskip(ZG->nwk.nib.depth - 1) : 0;

  /* Route direction */
  route_down = ( (dest_address > ZG->mac.pib.mac_short_address) && (dest_address < ZG->mac.pib.mac_short_address + cskip_parent) );

  if ( (ZG->mac.pib.mac_short_address == 0) || route_down )
  {
    ret = zb_nwk_tree_routing_find_router(dest_address);
  }
  else
  {
    zb_nwk_neighbor_get(ZG->nwk.handle.parent, ZB_FALSE, &ret);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nwk_tree_routing_route %p", (FMT__P, ret));
  return ret;
}

/*! @} */

#endif
