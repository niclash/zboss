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
PURPOSE: Roitines specific to mlme scan for coordinator/router
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"

#include "zb_bank_2.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
void zb_mlme_associate_response(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  if (!ZG->nwk.handle.joined_pro)
  {
    TRACE_MSG(TRACE_MAC2, ">>mlme_ass_resp %hd", (FMT__H, param));
    MAC_CTX().mac_status = MAC_SUCCESS;
    MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
    /* Q: why do not insert here
       zb_handle_associate_response code?
       
       A: I think there's no
       need to do it, compiler will inline this function, because here's
       the only place, which it called from. */
    ret = zb_handle_associate_response();
    TRACE_MSG(TRACE_MAC2, "<<mlme_ass_resp", (FMT__0));
  }
  else
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
}

zb_ret_t zb_handle_associate_response() ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mlme_associate_response_t *params;
  zb_mac_mhr_t mhr;
  zb_uint_t packet_length;
  zb_uint_t mhr_len;
  zb_uint8_t *ptr;
  zb_mac_pending_data_t pend_data;

/*
  7.1.3.3 MLME-ASSOCIATE.response
  - send Association response command, using indirect transmission
  - add packet to send to pending list; if there is no space, set
  status TRANSACTION_OVERFLOW
  - if packet is not handled during macTransactionPersistenceTime,
  set status TRANSACTION_EXPIRED
  - if any parameter value is incorrect, set status INVALID_PARAMETER
  - if frame is transmited and ack is received, set status SUCCESS
  - send indication primitive
*/

  TRACE_MSG(TRACE_MAC2, ">>handle_ass_resp", (FMT__0));

  params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_associate_response_t);
  ZB_ASSERT(params);

/*
  7.3.2 Association response command
  | MHR | cmd frame id 1b | short addr 2b | ass status 1b |
  - command id = MAC_CMD_ASSOCIATION_RESPONSE
  - dst addr mode = ZB_ADDR_64BIT_DEV
  - src addr mode = ZB_ADDR_64BIT_DEV
  - frame pending = 0
  - ack req = 1
  - pan id compress = 1
  - dst pan id = macPANid
  - src pan id = 0
  - dst addr = device ext addr
  - src addr = aExtendedAddress

  - short addr = associated addr
  - ass status = association status
*/

  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_64BIT_DEV, ZB_ADDR_64BIT_DEV, 1);
  packet_length = mhr_len;
  packet_length += (sizeof(zb_uint8_t) + sizeof(zb_uint16_t) +sizeof(zb_uint8_t));

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().pending_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);
/* Fill Frame Controll then call zb_mac_fill_mhr()
   mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  /* security enable is 0 */
  /* frame pending is 0 */
  ZB_FCF_SET_ACK_REQUEST_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);

  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();

  mhr.dst_pan_id = MAC_PIB().mac_pan_id;
  ZB_IEEE_ADDR_COPY(mhr.dst_addr.addr_long, params->device_address);

  TRACE_MSG(TRACE_MAC3, "ASS RESP dst mod %hi addr " TRACE_FORMAT_64, (FMT__H_A,
                                                                       ZB_FCF_GET_DST_ADDRESSING_MODE(mhr.frame_control),
                                                                       TRACE_ARG_64(mhr.dst_addr.addr_long)));
  mhr.src_pan_id = MAC_PIB().mac_pan_id;
  ZB_IEEE_ADDR_COPY(mhr.src_addr.addr_long, MAC_PIB().mac_extended_address);
  TRACE_MSG(TRACE_MAC3, "ASS RESP src mod %hi addr " TRACE_FORMAT_64, (FMT__H_A,
                                                                       ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr.frame_control),
                                                                       TRACE_ARG_64(mhr.src_addr.addr_long)));


  /* TODO: fill Auxiliary Security Header (MAC security - not for ZB 2007) */

  zb_mac_fill_mhr(ptr, &mhr);

  ZB_BZERO(&mhr, sizeof(mhr));
  zb_parse_mhr(&mhr, ptr);

  ptr += mhr_len;
  *ptr = MAC_CMD_ASSOCIATION_RESPONSE;
  ptr++;
  zb_put_next_htole16(&ptr, params->short_address);
  *ptr = params->status;
  MAC_ADD_FCS(MAC_CTX().pending_buf);
  /* Association response is filled, lets put it to pending queue */

  pend_data.pending_data = MAC_CTX().pending_buf;
  pend_data.dst_addr_mode = ZB_ADDR_64BIT_DEV;
  ZB_IEEE_ADDR_COPY(pend_data.dst_addr.addr_long, params->device_address);
  ZB_DUMP_IEEE_ADDR(pend_data.dst_addr.addr_long);

  ret = zb_mac_put_data_to_pending_queue(&pend_data);

  if (ret == RET_ERROR)
  {
    TRACE_MSG(TRACE_MAC3, "calling zb_mac_send_comm_status", (FMT__0));
    zb_mac_send_comm_status(MAC_CTX().pending_buf, ZB_GET_MAC_STATUS(), MAC_CTX().pending_buf);
    MAC_CTX().pending_buf = NULL;
  }
  TRACE_MSG(TRACE_MAC2, "<<handle_ass_resp set mac 1 st %hi ret %i", (FMT__D, ret));
  return ret;
}


/* Coordinator side: get request command, say ACK to end device,
 * signal to high level with associate.indication */
void zb_accept_ass_request_cmd(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_uint8_t *cmd_ptr;
  zb_mac_mhr_t mhr;


  TRACE_MSG(TRACE_MAC2, ">>accept_ass_req_cmd %hd", (FMT__H, param));
  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
  cmd_ptr = ZB_BUF_BEGIN(MAC_CTX().pending_buf);
  cmd_ptr += zb_parse_mhr(&mhr, cmd_ptr);
#ifdef ZB_MANUAL_ACK
  ret = zb_mac_send_ack(mhr.seq_number, 0);
  if (ret == RET_OK)
  {
    ret = zb_check_cmd_tx_status();
  }
#endif
  if (ret == RET_OK)
  {
    zb_mlme_associate_indication_t *ass_indication = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_associate_indication_t);
/*
  Coordinator side
  - on receiving association request, send acknowledgement

  TODO: check who is doing all that stuff!!!
  - if this device was previously associated on this PAN, remove all
  information about it
  - if capability.allocate_address == 1, generate 16 bit short
  address, otherwise set short adress to 0xFFFE
  - generate 16-bit short address for device, if it is possible; set
  failure status otherwise
  - send assosiation responce indiectly
*/
/* 7.3.1 Association request command
   | MHR | Command frame id 1 byte | Capability info 1 byte | */

    ZB_MEMCPY(&ass_indication->device_address, &mhr.src_addr, sizeof(zb_ieee_addr_t));
    cmd_ptr++;
    ass_indication->capability = *cmd_ptr;
    ass_indication->lqi = ZB_MAC_GET_LQI((zb_buf_t*)MAC_CTX().pending_buf);
    ZB_SCHEDULE_CALLBACK(zb_mlme_associate_indication, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }
  TRACE_MSG(TRACE_MAC2, "<<accept_ass_req_cmd", (FMT__0));
}
/*! @} */

#endif /*ZB_COORDINATOR_ROLE || ZB_ROUTER_ROLE */
