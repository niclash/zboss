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
PURPOSE: AF: RX path.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"
#include "zb_test_profile.h"

#include "zb_bank_7.h"
/*! \addtogroup ZB_AF */
/*! @{ */

void zb_zdo_data_indication(zb_uint8_t param) ZB_CALLBACK;

void zb_apsde_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);

  TRACE_MSG(TRACE_APS3, "apsde_data_ind: pkt %p h 0x%hx sz %hd, dst endp %hd",
            (FMT__P_H_H_H, (zb_buf_t *)asdu, asdu->u.hdr.handle, ZB_BUF_LEN(asdu), ind->dst_endpoint));

  switch (ind->dst_endpoint)
  {
    case 0:
      /* special case: endpoint 0 - ZDP */
      TRACE_MSG(TRACE_APS3, "call zdo", (FMT__0));
      ZB_SCHEDULE_CALLBACK(zb_zdo_data_indication, param);
      break;

#ifdef ZB_TEST_PROFILE
    case ZB_TEST_PROFILE_EP:
      TRACE_MSG(TRACE_APS3, "test profile", (FMT__0));
      ZB_SCHEDULE_CALLBACK(zb_test_profile_indication, param);
      break;
#endif

    default:
      /* TODO: support endpoints other then 0 and profiles other then ZDP. */
      if (ZG->zdo.af_data_cb)
      {
        TRACE_MSG(TRACE_ERROR, "APS pkt for ep %hd - call %p", (FMT__H_P, ind->dst_endpoint, ZG->zdo.af_data_cb));
        zb_schedule_callback(ZG->zdo.af_data_cb, param);
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "APS pkt for ep %hd - drop", (FMT__H, ind->dst_endpoint));
        zb_free_buf(asdu);
      }
      break;
  }
}

void zb_af_set_data_indication(zb_callback_t cb)
{
  TRACE_MSG(TRACE_ZDO1, "set apsde_data_ind cb to %p, was %p", (FMT__P_P, cb, ZG->zdo.af_data_cb));
  ZG->zdo.af_data_cb = cb;
}

void zb_apsde_data_acknowledged(zb_uint8_t param) ZB_CALLBACK
{
  zb_aps_hdr_t aps_hdr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

#ifndef ZB_LIMITED_FEATURES2
  TRACE_MSG(TRACE_APS3, ">>apsde_data_acked: param %hd", (FMT__H, param));

  zb_aps_hdr_parse(asdu, &aps_hdr, ZB_FALSE);
  if (
    /* If zdo request send failed, call callback now */
    asdu->u.hdr.status == (zb_uint8_t)RET_NO_ACK
    /* This is ZDP request which has no response - call callback now */
    || asdu->u.hdr.zdo_cmd_no_resp)
  {
    {
      zb_uint8_t *body;
      ZB_APS_HDR_CUT_P(asdu, body);
      if (asdu->u.hdr.status == (zb_uint8_t)RET_NO_ACK)
      {
        /* status is 1-byte field at beginning of every ZDP resp */
        body[1] = ZB_ZDP_STATUS_TIMEOUT;
      }
    }
    if (zdo_af_resp(param) != RET_OK)
    {
      zb_free_buf(asdu);
    }
  }
  else if (ZB_APS_FC_GET_ACK_REQUEST(aps_hdr.fc))
  {
#if 0
    /* Why call af_resp here???
       We are here when our packet transmitted ok, or its transmit failed.

       But it is wrong: we must call user's callback when got response from the
       other side.

       Maybe, we still need to call user's callback if transmit failed, setting
       first byte (status) to some error code in that case.
     */
    if (aps_hdr.src_endpoint == 0)
    {
      zb_ret_t ret = RET_OK;

      TRACE_MSG(TRACE_APS3, "ZDO buf", (FMT__0));
      ZB_APS_HDR_CUT(asdu);
      /* user callback is called and buffer is released in zdo_af_resp() */
      ret = zdo_af_resp(param);
      if ( ret != RET_OK )
      {
        zb_free_buf(asdu);
      }
    }
#endif
#ifdef ZB_TEST_PROFILE
    if (aps_hdr.src_endpoint == ZB_TEST_PROFILE_EP)
    {
      TRACE_MSG(TRACE_APS3, "test_profile buf", (FMT__0));
      tp_packet_ack(param);
    }
    else
#endif
    {
      zb_free_buf(asdu);
    }
  }
  else
#endif  /* ZB_LIMITED_FEATURES2 */
  {
    zb_free_buf(asdu);
  }

  TRACE_MSG(TRACE_APS3, "<<apsde_data_acked", (FMT__0));
}


/*! @} */
