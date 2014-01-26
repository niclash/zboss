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

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_magic_macros.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_3.h"

void zb_mlme_start_confirm(zb_uint8_t param) ZB_CALLBACK
{
#ifdef ZB_ROUTER_ROLE
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
#else
  ZVUNUSED(param);
#endif
  TRACE_MSG(TRACE_NWK1, ">>mlme_start_conf %hd", (FMT__H, param));

  TRACE_MSG(TRACE_NWK1, "state %hd", (FMT__H, ZG->nwk.handle.state));
#ifdef ZB_ROUTER_ROLE
  if (buf->u.hdr.status != 0)
  {
    TRACE_MSG(TRACE_ERROR, "mlme_start FAILED %hd", (FMT__H, buf->u.hdr.status));
  }
  else
  {
    /* Ext neighbor was filled during Active scan at Formation time. Free it
     * (will find no devices in our PAN, so put nothing into Neighbor table).
     * Note that it is safe to call zb_nwk_exneighbor_stop() even if ext neighbor
     * was not started before.
     */
    zb_nwk_exneighbor_stop(-1);

    switch ( ZG->nwk.handle.state )
    {
      case ZB_NLME_STATE_ROUTER:
        zb_nwk_set_device_type(ZB_NWK_DEVICE_TYPE_ROUTER);
        MAC_PIB().mac_association_permit = (ZG->nwk.nib.max_children > ZG->nwk.nib.router_child_num + ZG->nwk.nib.ed_child_num);
        TRACE_MSG(TRACE_NWK1, "max_children %hd, zr# %hd, ze# %hd, mac_association_permit %hd", (FMT__H_H,
                  ZG->nwk.nib.max_children, ZG->nwk.nib.router_child_num, ZG->nwk.nib.ed_child_num, MAC_PIB().mac_association_permit));
        zb_nwk_update_beacon_payload();
        NWK_CONFIRM_STATUS((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_NWK_STATUS_SUCCESS, zb_nlme_start_router_confirm);
        break;

      case ZB_NLME_STATE_FORMATION:
        zb_nwk_set_device_type(ZB_NWK_DEVICE_TYPE_COORDINATOR);
        /* See 3.6.7.
           The NWK layer of the ZigBee coordinator shall update the beacon payload
           immediately following network formation. All other ZigBee devices shall update
           it immediately after the join is completed and any time the network configuration
           (any of the parameters specified in Table3.10) changes.
        */
        zb_nwk_update_beacon_payload();
        /* simplify all checks: set 'joined' flag. */
        ZG->nwk.handle.joined = 1;
        NWK_CONFIRM_STATUS((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_NWK_STATUS_SUCCESS, zb_nlme_network_formation_confirm);
        break;

      case ZB_NLME_STATE_PANID_CONFLICT_RESOLUTION:
        TRACE_MSG(TRACE_NWK1, "done panid update after panid conflict resolution", (FMT__0));
        zb_free_buf(ZB_BUF_FROM_REF(param));
        break;

      default:
        TRACE_MSG(TRACE_NWK1, "wrong nwk ste %hd", (FMT__H, ZG->nwk.handle.state));
        ZB_ASSERT(0);
        break;
    }
  }
#endif  /* router */
  ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;

  TRACE_MSG(TRACE_NWK1, "<<mlme_start_conf", (FMT__0));
}

/*! @} */
