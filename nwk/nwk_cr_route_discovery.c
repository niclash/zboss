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
PURPOSE: Network creation routine
*/

#include <string.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_magic_macros.h"

/*! \addtogroup ZB_NWK */
/*! @{ */
#include "zb_bank_5.h"
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE

void zb_nlme_route_discovery_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_route_discovery_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_route_discovery_request_t);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_route_discovery_request %hd", (FMT__H, param));
  CHECK_PARAM_RET_ON_ERROR(request);

  /* check that we are router or coordinator */
  if ( ret == RET_OK
       && !(ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR
            || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER ))
  {
    NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_INVALID_REQUEST, ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE);
    ret = RET_ERROR;
    TRACE_MSG(TRACE_NWK1, "no routing capacity available, ret error", (FMT__0));
    goto done;
  }

  /* TODO: implement many to one route discovery */
  /* check for broadcast address */
  if ( ret == RET_OK
       && request->address_mode != ZB_ADDR_NO_ADDR
       && ZB_NWK_IS_ADDRESS_BROADCAST(request->network_addr) )
  {
    NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_INVALID_REQUEST, ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE);
    TRACE_MSG(TRACE_NWK1, "got broadcast address, ret error", (FMT__0));
    goto done;
  }

  /* check it's not our own address */
  if ( ret == RET_OK
       && request->address_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST
       && request->network_addr == ZB_NIB_NETWORK_ADDRESS() )
  {
    NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_SUCCESS, 0xff);
    TRACE_MSG(TRACE_NWK1, "truing to discover our own address, send success", (FMT__0));
    goto done;
  }

  /* check it's not our neighbour */
  {
    zb_neighbor_tbl_ent_t *nbt;

    if ( ret == RET_OK
         && zb_nwk_neighbor_get_by_short(request->network_addr, &nbt) == RET_OK )
    {
      NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_SUCCESS, 0xff);
      TRACE_MSG(TRACE_NWK1, "discovery device found in neighbour table, ret success", (FMT__0));
      goto done;
    }
  }

#if defined ZB_NWK_MESH_ROUTING && defined ZB_ROUTER_ROLE
  if ( ret == RET_OK )
  {
    if ( request->address_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST )
    {
      /* save dest address, to call confirm on error or success */
      ZG->nwk.nib.aps_rreq_addr = request->network_addr;
      zb_nwk_mesh_route_discovery(buf, request->network_addr, request->radius);
    }
    else if ( request->address_mode == ZB_ADDR_16BIT_MULTICAST )
    {
      /* TODO: Implement */
      NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_INVALID_REQUEST, ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE);
    }
    else
    {
      /* TODO: Implement */
      NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_INVALID_REQUEST, ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE);
    }
  }
#else
  NWK_ROUTE_DISCOVERY_CONFIRM(buf, ZB_NWK_STATUS_ROUTE_ERROR, ZB_NWK_COMMAND_STATUS_NO_ROUTING_CAPACITY);
#endif

  done:
  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_route_discovery_request %d", (FMT__D, ret));
}
#endif
/*! @} */
