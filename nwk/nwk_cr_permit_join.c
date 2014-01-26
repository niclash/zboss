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
PURPOSE: Permit join API
*/

#include <string.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_4.h"
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE

void nwk_permit_timeout(zb_uint8_t param) ZB_CALLBACK;

void nwk_permit_timeout(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_NWK1, ">>nwk_permit_timeout %hd", (FMT__H, param));

  ZG->mac.pib.mac_association_permit = 0;
  ZG->nwk.handle.permit_join = 0;

  TRACE_MSG(TRACE_NWK1, "<<nwk_permit_timeout", (FMT__0));
}

void zb_nlme_permit_joining_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_permit_joining_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_permit_joining_request_t);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_permit_joining_request %hd", (FMT__H, param));
  CHECK_PARAM_RET_ON_ERROR(request);

  TRACE_MSG(TRACE_NWK1, "permit join %d device type %d", (FMT__D_D, (int)ZG->nwk.handle.permit_join, (int)ZB_NIB_DEVICE_TYPE()));

  /* cancel any other permit timeout */
  if ( ZG->nwk.handle.permit_join )
  {
    ZB_SCHEDULE_ALARM_CANCEL(nwk_permit_timeout, ZB_ALARM_ANY_PARAM);
    ZG->nwk.handle.permit_join = 0;
  }

  if ( ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR
       || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER )
  {
    TRACE_MSG(TRACE_NWK1, "permit duration %d", (FMT__D, (int)request->permit_duration));
    switch ( request->permit_duration )
    {
      case 0x00:
        MAC_PIB().mac_association_permit = 0;
        break;

      case 0xff:
        MAC_PIB().mac_association_permit = 1;
        break;

      default:
        ZG->nwk.handle.permit_join = 1;
        MAC_PIB().mac_association_permit = 1;
        ZB_SCHEDULE_ALARM(nwk_permit_timeout, 0, request->permit_duration*ZB_TIME_ONE_SECOND);
        break;
    }

    zb_nwk_update_beacon_payload();
    NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_SUCCESS, zb_nlme_permit_joining_confirm);
  }
  else
  {
    NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_INVALID_REQUEST, zb_nlme_permit_joining_confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_permit_joining_request %d", (FMT__D, ret));
}
#endif

/*! @} */
