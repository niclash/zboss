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
PURPOSE: IB save/load/set defaults
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_secur.h"

#include "zb_bank_6.h"
#ifdef ZB_TRANSPORT_LINUX_PIPES
#include <ctype.h>
#endif


/*! \addtogroup ZB_BASE */
/*! @{ */


void zb_ib_set_defaults(zb_char_t *rx_pipe) ZB_CALLBACK
{
  ZVUNUSED(rx_pipe);
#if (defined ZB_NS_BUILD && defined ZB8051) || (defined ZB8051 && defined NO_NVRAM) || defined ZB_PLATFORM_LINUX_ARM_2400
  /* special trick for ns build run on 8051 simulator: get node number from the
   * rx pipe name parameter. Same for 8051 HW without NVRAM: need to get
   * address somewhere */
  ZB_IEEE_ADDR_ZERO(ZB_PIB_EXTENDED_ADDRESS());
  ZB_PIB_EXTENDED_ADDRESS()[7] = 8;
  if (rx_pipe)
  {
    ZB_PIB_EXTENDED_ADDRESS()[0] = rx_pipe[0] - '0';
  }
#elif defined ZB_TRANSPORT_LINUX_PIPES
  /* Parse rpipe_path. Before trailing point it has node number. */
  {
    zb_uint_t node_number = 0;
    zb_char_t *p = strrchr(rx_pipe, '.');
    if (!p)
    {
      /* no extension */
      p = rx_pipe + strlen(rx_pipe) - 1;
    }
    else
    {
      p--;
    }

    while (isdigit(*p))
    {
      node_number *= 10;
      node_number += *p - '0';
      p--;
    }

    ZB_IEEE_ADDR_ZERO(ZB_PIB_EXTENDED_ADDRESS());
    ZB_PIB_EXTENDED_ADDRESS()[0] = node_number & 0xff;
    ZB_PIB_EXTENDED_ADDRESS()[1] = (node_number >> 8) & 0xff;
    ZB_PIB_EXTENDED_ADDRESS()[7] = 0x8; /* assign one byte to have non-zero
                                         * address for node #0 */
  }
#endif

  /* pand id to join or form */
  /*ZB_EXTPANID_ZERO(ZB_AIB().aps_use_extended_pan_id);
  ZB_AIB().aps_use_extended_pan_id[7] = 8;*/

  ZB_AIB().aps_channel_mask = ZB_DEFAULT_APS_CHANNEL_MASK;

  TRACE_MSG(TRACE_APS3, "aps_channel_mask 0x%x", (FMT__D, ZB_AIB().aps_channel_mask));

  ZB_AIB().aps_insecure_join = 1;

#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = ZB_SECURITY_LEVEL;
  ZG->nwk.nib.secure_all_frames = ZB_DEFAULT_SECURE_ALL_FRAMES;


#if defined ZB_TC_GENERATES_KEYS && defined ZB_COORDINATOR_ROLE
  secur_generate_keys();
#endif  /* ZB_TC_GENERATES_KEYS */

#endif  /* security */

#ifdef ZB_ROUTER_ROLE
  MAC_PIB().mac_rx_on_when_idle = 1;
  ZG->nwk.nib.max_children = ZB_DEFAULT_MAX_CHILDREN;
#endif
  ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_NONE;
}


void zb_ib_load() ZB_CALLBACK
{
}


void zb_ib_save() ZB_CALLBACK
{
}


/*! @} */
