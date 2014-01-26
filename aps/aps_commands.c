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
#include "zb_aps.h"
#include "aps_internal.h"
#include "zb_secur.h"

#include "zb_bank_6.h"

/*! \addtogroup ZB_APS */
/*! @{ */

void zb_aps_in_command_handle(zb_uint8_t param) ZB_CALLBACK
{
#ifdef ZB_SECURITY
  zb_uint8_t *cmd_id_p;
  zb_ushort_t hdr_size = sizeof(zb_aps_command_pkt_header_t);

#ifdef APS_FRAME_SECURITY
  {
  zb_aps_command_pkt_header_t *hdr = (zb_aps_command_pkt_header_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  if (ZB_APS_FC_GET_SECURITY(hdr->fc))
  {
    hdr_size += zb_aps_secur_aux_size((zb_uint8_t*)(hdr + 1));
    TRACE_MSG(TRACE_SECUR3, "secured aps cmd frame, hdr+aux size %hd", (FMT__H, hdr_size));
  }
  }
#endif

  ZB_BUF_CUT_LEFT(ZB_BUF_FROM_REF(param), hdr_size + 1, cmd_id_p);
  cmd_id_p--;
  TRACE_MSG(TRACE_SECUR3, "in aps cmd %hd", (FMT__H, *cmd_id_p));
  switch (*cmd_id_p)
  {
    case APS_CMD_TRANSPORT_KEY:
      if (!ZG->nwk.handle.joined_pro)  zb_aps_in_transport_key(param);
      break;
#ifdef ZB_ROUTER_ROLE
    case APS_CMD_UPDATE_DEVICE:
      if (!ZG->nwk.handle.joined_pro)  zb_aps_in_update_device(param);
      break;
    case APS_CMD_REMOVE_DEVICE:
      if (!ZG->nwk.handle.joined_pro)  zb_aps_in_remove_device(param);
      break;
    case APS_CMD_REQUEST_KEY:
      if (!ZG->nwk.handle.joined_pro)  zb_aps_in_request_key(param);
      break;
#else  /* ZB_ROUTER_ROLE */
	case APS_CMD_SWITCH_KEY:
      /* Not need to process switch-key at ZED: it has only 1 slot for key so
       * key switched anyway. */
      /* according to new PICS, we need to process a switch key by end device */
      /* because it also should keep up to 3 keys */
      zb_aps_in_switch_key(param);
#endif
      break;


    default:
      TRACE_MSG(TRACE_ERROR, "unknown aps cmd %hd", (FMT__H, *cmd_id_p));
      zb_free_buf(ZB_BUF_FROM_REF(param));
      break;
  }
#else
  zb_free_buf(ZB_BUF_FROM_REF(param));
#endif
}
/*! @} */
