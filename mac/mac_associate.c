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
PURPOSE: Roitines specific to mlme scan
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"

#include "zb_bank_2.h"


/*! \addtogroup ZB_MAC */
/*! @{ */


/*
  7.1.3.1 MLME-ASSOCIATE.request
  - check if device is not associated yet (is done on nwk layer)
  - set parameters:
  phyCurrentChannel = req.logical_channel
  phyCurrentPage = req.channel_page (not used in zigbee)
  macPANId = req.coord_pan_id
  macCoordExtendedAddress or macCoordShortAddress = req.coord_address
  - send Association request command (mac spec 7.3.1)
  - if send fails, notify higher layer
  - wait for association request acknowledgement
  - after ack is received, wait for macResponseWaitTime symbols and
  then extract associate responce from coordinator (not-beacon mode)
  - if response can not be extracted, set status NO_DATA
  - if response is received, send acknowledgement
  - if assosiation was not successful, set macPANId to 0xFFFF
*/
void zb_mlme_associate_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_mlme_associate_params_t *params = NULL;
  TRACE_MSG(TRACE_MAC2, ">>mlme_ass_req %hd", (FMT__H, param));
  /* process request immediately*/
  /* MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);*/
  /* zb_handle_associate_request(); */
  params = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_associate_params_t);
  ZB_ASSERT(params);
  if (ret == RET_OK)
  {
    if (params->coord_addr_mode == ZB_ADDR_64BIT_DEV)
    {
      TRACE_MSG(TRACE_MAC3, "associate to long, addr " TRACE_FORMAT_64,
                (FMT__A, TRACE_ARG_64(params->coord_addr.addr_long)));
      ret = zb_mac_setup_for_associate(params->logical_channel, params->pan_id,
                                       (zb_uint16_t)~0, params->coord_addr.addr_long);
    }
    else
    {
      zb_ieee_addr_t laddr;
      ZB_IEEE_ADDR_ZERO(laddr);
      ret = zb_mac_setup_for_associate(params->logical_channel, params->pan_id,
                                       params->coord_addr.addr_short, laddr);
    }
  }
  if (ret == RET_OK)
  {
#ifdef ZB_MANUAL_ACK
command_send:
#endif
  TRACE_MSG(TRACE_MAC2, "<<mlme_ass_req, continue scheduled", (FMT__0));
#ifndef ZB_NS_BUILD
  ZB_SCHEDULE_TX_CB(zb_mlme_send_association_req_cmd, param) ;
#else
  zb_mlme_send_association_req_cmd(param);
#endif
  }
#ifdef ZB_MANUAL_ACK
  if (ret == RET_OK)
  {
    /* associate request is trasnmitted successfully, wait ack
     * for a macAckWaitDuration time; start timer to measure
     * this interval */
    /* timeout is calculated in beacon intervals */

    /* ack_dsn was set in zb_mlme_send_association_req_cmd() */

    ret = zb_mac_check_ack();

    if (ret == RET_PENDING)
    {
      /* retry data send */
      goto command_send;
    }
  }
#endif
  TRACE_MSG(TRACE_MAC2, "<<mlme_ass_req", (FMT__0));
}

void zb_mlme_send_association_req_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  ZVUNUSED(param);
  ZB_SKIP_TX_CHK();
  ret = zb_check_cmd_tx_status();
  TRACE_MSG(TRACE_MAC2, ">>mlme_ass_req_continue", (FMT__0));

  if (ret == RET_OK)
  {
    /* wait for macResponseWaitTime time interval
     * and try to get associate resp (inderect transmission) */
    TRACE_MSG(TRACE_MAC2, "resp_tmout cleared", (FMT__0));
    ret = ZB_SCHEDULE_ALARM(zb_mac_resp_timeout, 0, ZB_MAC_PIB_RESPONSE_WAIT_TIME);
  }
  TRACE_MSG(TRACE_MAC2, "<<mlme_ass_req_continue", (FMT__0));
}


zb_ret_t zb_mac_setup_for_associate(zb_uint8_t logical_channel, zb_uint16_t pan_id,
                                    zb_uint16_t short_addr, zb_ieee_addr_t long_addr)
{
  zb_ret_t ret = RET_OK;

  ZVUNUSED(logical_channel);
  ZVUNUSED(pan_id);
  ZVUNUSED(short_addr);
  ZVUNUSED(long_addr);

  TRACE_MSG(TRACE_MAC1, "mac hw setup before associate", (FMT__0));

  MAC_CTX().mac_status = MAC_SUCCESS;
  MAC_CTX().retry_counter = 0;
#ifndef ZB_NS_BUILD
  /* TODO: make logical|physical channels uniformly in set_channel calls */

  ZB_TRANSCEIVER_SET_CHANNEL(logical_channel);
#if 0
  ZB_TRANSCEIVER_SET_PAN_ID(pan_id);
  ZB_UPDATE_LONGMAC();
  if (short_addr == (zb_uint16_t)~0)
  {
    short_addr = zb_address_short_by_ieee(long_addr);
  }
  if (ZB_IEEE_ADDR_IS_ZERO(long_addr))
  {
    zb_address_ieee_by_short(short_addr, long_addr);
  }

  if (!ZB_IEEE_ADDR_IS_ZERO(long_addr))
  {
    TRACE_MSG(TRACE_MAC3, "set coordinator long address " TRACE_FORMAT_64,
              (FMT__A, TRACE_ARG_64(long_addr)));
    ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(long_addr);
  }
  if (short_addr != (zb_uint16_t)~0)
  {
    ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(short_addr);
  }
#endif
#endif
  return ret;
}

/*
  sends association request command
  return RET_OK, RET_ERROR
*/
void zb_mlme_send_association_req_cmd(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  zb_mac_mhr_t mhr;
  zb_uint_t packet_length;
  zb_uint_t mhr_len;
  zb_uint8_t *ptr;
  zb_mlme_associate_params_t *params;

/*
  7.3.1 Association request command
  - fill frame control:
  - source addr mode = ZB_ADDR_64BIT_DEV
  - dst addr mode = req.coord_addr_mode
  - frame pending = 0
  - ack request = 1
  - fill MHR
  - dst pan id = req.coord_pan_id
  - dst address = req.coord_addr
  - src pan id = 0xffff (broadcast pan id)
  - src addr = aExtendedAddress
  - command id = MAC_CMD_ASSOCIATION_REQUEST
*/

/* | MHR | Command frame id (1 byte) | Capability Info (1 byte) | */

  params = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_associate_params_t);
  TRACE_MSG(TRACE_MAC2, ">>mlme_send_ass_req_cmd %p", (FMT__P, params));
  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_64BIT_DEV, params->coord_addr_mode, 0);
  packet_length = mhr_len;
  packet_length += sizeof(zb_uint8_t) * 2;

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);

/* Fill Frame Controll then call zb_mac_fill_mhr()
   mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  ZB_BZERO(mhr.frame_control, sizeof(mhr.frame_control));

  /* TODO: optimize fc fill */
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  /* security enable is 0 */
  /* frame pending is 0 */
  ZB_FCF_SET_ACK_REQUEST_BIT(mhr.frame_control, 1);
  /* pan id compress is 0 */
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, params->coord_addr_mode);
  /* MAC_FRAME_VERSION defined in zb_config.h */
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);

  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();
  MAC_CTX().ack_dsn = mhr.seq_number; /* save DSN to check acks */

  mhr.dst_pan_id = params->pan_id;
  ZB_MEMCPY(&mhr.dst_addr, &params->coord_addr, sizeof(union zb_addr_u));
  mhr.src_pan_id = ZB_BROADCAST_PAN_ID;
  ZB_MEMCPY(&mhr.src_addr, MAC_PIB().mac_extended_address, sizeof(zb_ieee_addr_t));
  /* TODO: fill Auxiliary Security Header */

  zb_mac_fill_mhr(ptr, &mhr);

/* | MHR | Command frame id 1 byte | Capability info 1 byte | */

  ptr += mhr_len;
  *ptr = MAC_CMD_ASSOCIATION_REQUEST;
  ptr++;
  *ptr = params->capability;

  MAC_ADD_FCS(MAC_CTX().operation_buf);
  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);
  MAC_CTX().tx_wait_cb = zb_mlme_send_association_req_continue;

  TRACE_MSG(TRACE_MAC2, "<<mlme_send_ass_req_cmd, ret %i", (FMT__D, ret));
}


/* associate responce - coordinator side. send response to device asynchronously */

/*! @} */
