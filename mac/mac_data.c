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
PURPOSE: Roitines specific mac data transfer
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_secur.h"
#include "zb_zdo.h"

#include "zb_bank_1.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

/* sends data request command */
zb_ret_t zb_mlme_send_data_req_cmd(zb_mlme_data_req_params_t *params) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mac_mhr_t mhr;
  zb_uint_t packet_length = 0;
  zb_uint_t mhr_len = 0;
  zb_uint8_t *ptr = NULL;

/*
  7.3.4 Data request command

  TODO: if data request command is being sent in response to the receipt
  of a beacon frame, check spec 7.3.4 Data request command for more details:
  set dst and src addr and addr mode according to values specified in
  the beacon frame

  -= USE THIS INFO ON THE CALLER SIDE =-
  - if MLME-POLL.request triggeres data req cmd, set dst addr mode and
  dst addr the same as in the MLME-POLL.req;
  - src addr/mode: if macShortAddress is assigned (is less than
  0xfffe), then use short addr; use extended addr otherwise

  -= USE THIS INFO ON THE CALLER SIDE =-
  - if association req triggers data req cmd, set dst addr info
  according to coordinator addr to wich the request is directed: if
  macCoordShortAddress == 0xfffe, use extended addressing; use short
  addressing otherwise
  - use extended addressing for src addr/mode

  - if dst addr mode == ZB_ADDR_NO_ADDR, see spec for more details
  (beacon frame case)
  - if dst addr is specified, set FCF.PAN id compression = 1, dst PAN
  id = macPANId, src PAN id is omitted.
  - set FCF.Frame pending = 0
  - set FCF.Acknowledgment Request = 1
*/

#if defined ZB_TRANSPORT_8051_DATA_SPI && defined ZB_ED_ROLE && !defined ZB_DBG_NO_IDLE
  if (ZB_MAC_GET_TRANS_SPLEEPING())
  {
    TRACE_MSG(TRACE_COMMON3, "transceiver, WAKE UP", (FMT__0));
    ZB_MAC_CLEAR_TRANS_SPLEEPING();
    zb_uz2400_register_wakeup();
  }
#endif

  TRACE_MSG(TRACE_MAC2, ">> zb_mlme_send_data_req_cmd %p", (FMT__P, params));

  /* | MHR | Cmd frame id (1 byte) | */

  if (params->dst_addr_mode != ZB_ADDR_NO_ADDR)
  {
    mhr_len = zb_mac_calculate_mhr_length(params->src_addr_mode, params->dst_addr_mode, 1);
  }
  else
  {
    TRACE_MSG(TRACE_MAC1, "dst addr mode ZB_ADDR_NO_ADDR is not supported (beacon frame case)", (FMT__0));
    ZB_ASSERT(0);
  }
  packet_length = mhr_len;
  packet_length += sizeof(zb_uint8_t);

  ZB_MEMCPY(&mhr.dst_addr, &params->dst_addr, sizeof(union zb_addr_u));
  mhr.src_pan_id = 0;
  mhr.dst_pan_id = MAC_PIB().mac_pan_id;
  ZB_MEMCPY(&mhr.src_addr, &params->src_addr, sizeof(union zb_addr_u));
  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();
  MAC_CTX().ack_dsn = mhr.seq_number; /* save DSN to check acks */

/* Fill Frame Controll then call zb_mac_fill_mhr()
   mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  ZB_BZERO(mhr.frame_control, sizeof(mhr.frame_control));

  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  /* security enable is 0 */
  /* frame pending is 0 */
  ZB_FCF_SET_ACK_REQUEST_BIT(mhr.frame_control, 1);

  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, params->dst_addr_mode);
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, params->src_addr_mode);

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);

  zb_mac_fill_mhr(ptr, &mhr);

  ptr += mhr_len;
  *ptr = MAC_CMD_DATA_REQUEST;

  MAC_ADD_FCS(MAC_CTX().operation_buf);
/* wake up! */

  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  TRACE_MSG(TRACE_MAC2, "<< zb_mlme_send_data_req_cmd ret %i", (FMT__D, ret));
  return ret;
}


/*
  Checks if acknowledgement is received or not, checks ack timeout
  and retry counter.
  @return RET_OK, RET_ERROR, RET_BLOCKED, RET_PENDING.
  RET_PENDING means ack is not received during time interval, but we
  can do retry.

  KLUDGE: MAC_CTX().ack_dsn should be set before calling this
  function. ack_dsn is used in zb_mlme_ack_accept() for checking
*/
#if defined ZB_MANUAL_ACK
zb_ret_t zb_mac_check_ack()
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_MAC2, ">> zb_mac_check_ack", (FMT__0));

  {
    /* check ack is called in a cycle, start ack waiting procedure should be
     * done only once */
    ZB_MAC_START_ACK_WAITING();
  }
  while (ret == RET_OK)
  {
    /* check for the ack during <ack_timeout> */
    if (ZB_MAC_GET_ACK_OK())
    {
      break;
    }
    else
    {
      ret = RET_BLOCKED; /* ACK is not received yet */
      /* condition: ack recv is not timed out yet or recv has started */
      if (ZB_MAC_GET_ACK_FAIL() || ZB_MAC_GET_ACK_TIMEOUT())
      {
        /* ack is not received during timeout period or ack is failed */
        /* increase re-try counter, check it and try again to send command */
        ZB_SCHEDULE_ALARM_CANCEL(zb_mac_ack_timeout, ZB_ALARM_ANY_PARAM); /* cancel scheduled alarm */

        MAC_CTX().retry_counter++;
        if (MAC_CTX().retry_counter > MAC_PIB().mac_max_frame_retries)
        {
          TRACE_MSG(TRACE_MAC1, "Error, ack fail", (FMT__0));
          ret = RET_ERROR;
          ZB_SET_MAC_STATUS(MAC_NO_ACK);
        }
        else
        {
          /* retry data send and wait for ack again */
          ret = RET_PENDING;
        }
      }
      else
      {
        /* FIXME: there's no states and a lot of other changes. */
        zb_buf_t *buf = zb_get_in_buf();
        if (buf)
        {
#ifdef ZB_NS_BUILD
          ZG->sched.mac_receive_pending = 0;
#endif
          zb_mac_recv_data(ZB_REF_FROM_BUF(buf));
        }
      }
      if (ZB_MAC_GET_ACK_OK())
      {
       ret = RET_OK;
        break;
      }
    }
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_mac_check_ack ret %i", (FMT__D, ret));
  return ret;
}
#endif /* ZB_MANUAL_ACK */

/*
  gets pending data from a coordinator (get data indirectly)
  @param data_req_cmd_params - parameters for data request command
  @return RET_OK, RET_ERROR, RET_BLOCKED, RET_PENDING
  RET_PENDING is returned if there is more data available on
  coordinator
*/
zb_ret_t zb_mac_get_indirect_data(zb_mlme_data_req_params_t *data_req_cmd_params)
{
  zb_ret_t ret = RET_OK;

/*
  7.5.6.3 Extracting pending data from a coordinator
  NON-beacon-enabled case is implememnted

  - send data request command with ack = 1
  - wait for ack for macAckWaitDuration symbols (7.5.6.4.3 Retransmissions)
  - if ack did not come, retry send data req cmd up to macMaxFrameRetries
  - on ack recv, check FCF.Frame pending subfield
  - if pending data is present, wait for macMaxFrameTotalWaitTime symbols to recv data
  - if no data is available on coord, it sends packet with zero payload, no ack is needed
  - if no data is received during timeout, no data is available, set NO_DATA
  - if data is received and ack is needed, send ack
  - if FCF.Frame pending == 1, more data is available
*/

  TRACE_MSG(TRACE_MAC2, ">>get_indirect_data", (FMT__0));

  /* send data req cmd */
  /* we need to know, if data_request sent */
#ifdef ZB_MANUAL_ACK
command_send:
#endif
  ZB_MAC_SET_INDIRECT_DATA_REQUEST();
  TRACE_MSG(TRACE_MAC2, "ZB_MAC_SET_INDIRECT_DATA_REQUEST()", (FMT__0));
  ret = zb_mlme_send_data_req_cmd(data_req_cmd_params);
  if (ret == RET_OK)
  {
    /* Seems like there's no reason to do it synchronously */
    ret = zb_check_cmd_tx_status();
  }
#if defined ZB_MANUAL_ACK
  if (ret == RET_OK)
  {
    /* ack_dsn was set in zb_mlme_send_data_req_cmd() */

    ret = zb_mac_check_ack();
    if (ret == RET_PENDING)
    {
      goto command_send;
    }
  }
#endif
  if (ret == RET_OK)
  {
    TRACE_MSG( TRACE_MAC1, "pending data %hd", (FMT__H, ZB_MAC_GET_PENDING_DATA()));
    if (ZB_MAC_GET_PENDING_DATA())
    {
      ZB_MAC_SET_INDIRECT_DATA_REQUEST();
      MAC_CTX().rt_ctx.indirect_data.buf_ref = 0;
      ZB_SCHEDULE_ALARM(zb_mac_indirect_data_timeout, data_req_cmd_params->cb_type, ZB_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME);
    }
    else
    {
      /* no data is available on remote */
      ZB_SET_MAC_STATUS(MAC_NO_DATA);
      ret = RET_ERROR;
    }
  }
  else if ( ret == RET_ERROR )
  {
    /* ZB_SET_MAC_STATUS(MAC_NO_DATA);*/
    ZB_MAC_CLEAR_PENDING_DATA();
    TRACE_MSG(TRACE_MAC2, "Pending data clear", (FMT__0));
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_mac_get_indirect_data, ret %i", (FMT__D, ret));
  return ret;
}

/*
  Adds mac header to the data contained already in data_buf
  forms fata frame according to 7.2.2.2 Data frame format
  return RET_OK, RET_ERROR, RET_PENDING
  RET_PENDING is returned if indirect transmission is used and data frame is put to pending queue
*/
zb_ret_t zb_mcps_data_request_fill_hdr(zb_buf_t *data_req)
{
  zb_uint8_t*        ptr;
  zb_mac_mhr_t mhr;
  zb_ret_t ret = RET_OK;
  zb_mcps_data_req_params_t *data_req_params;

  TRACE_MSG(TRACE_MAC2, ">> zb_mcps_data_request_fill_hdr p %p", (FMT__P, data_req));

/* TODO: add correct parameters check */

  data_req_params = ZB_GET_BUF_PARAM(data_req, zb_mcps_data_req_params_t);

#ifndef ZB_MAC_EXT_DATA_REQ
  /* always short addr is used */
  data_req_params->mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_16BIT_DEV_OR_BROADCAST, ZB_ADDR_16BIT_DEV_OR_BROADCAST, 1);
#else
  data_req_params->mhr_len = zb_mac_calculate_mhr_length(
    data_req_params->src_addr_mode,
    data_req_params->dst_addr_mode,
    (data_req_params->dst_pan_id == MAC_PIB().mac_pan_id));
#endif

  /* In the current implementation only DATA can be encrypted, so it is ok to
   * analyze security only here. */
#ifdef ZB_MAC_SECURITY
  if (data_req_params->security_level)
  {
    /* currently hard-code security level 5 and key_id_mode 1 */
    data_req_params->mhr_len += MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE; /* control(1), frame counter(4), key id (1) */
    data_req->u.hdr.encrypt_type = ZB_SECUR_MAC_ENCR;

    if (MAC_PIB().mac_frame_counter == (zb_uint32_t)~0)
    {
      ret = RET_ERROR;
      ZB_SET_MAC_STATUS(MAC_COUNTER_ERROR);
      TRACE_MSG(TRACE_MAC2, "<< zb_mcps_data_request_fill_hdr ret %d", (FMT__D, ret));
      return ret;
    }
  }
#endif

  ZB_ASSERT((data_req->u.hdr.len + (zb_ushort_t)data_req_params->mhr_len) <= 255);

/* Fill Frame Controll then call zb_mac_fill_mhr()
   mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_DATA);
  /* security enable is 0 */
  /* frame pending is 0 */
  ZB_FCF_SET_ACK_REQUEST_BIT(
    mhr.frame_control, !!(data_req_params->tx_options & MAC_TX_OPTION_ACKNOWLEDGED_BIT));
#ifndef ZB_MAC_EXT_DATA_REQ
  /* Set panid compress, overwise Ember don't understand rejoin response */
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  /* 7.2.3 Frame compatibility: All unsecured frames specified in this
     standard are compatible with unsecured frames compliant with IEEE Std 802.15.4-2003 */
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
#else
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control,
                                   (data_req_params->dst_pan_id == MAC_PIB().mac_pan_id));
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, data_req_params->dst_addr_mode);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, data_req_params->src_addr_mode);
#endif
  /* 7.2.3 Frame compatibility: All unsecured frames specified in this
     standard are compatible with unsecured frames compliant with IEEE Std 802.15.4-2003 */
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_IEEE_802_15_4_2003);
#ifdef ZB_MAC_SECURITY
  if (data_req_params->security_level)
  {
    ZB_FCF_SET_SECURITY_BIT(mhr.frame_control, 1);
    /* frame security compatible with 2006 */
    ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_IEEE_802_15_4);
  }
#endif

  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();
  if (data_req_params->tx_options & MAC_TX_OPTION_ACKNOWLEDGED_BIT)
  {
    MAC_CTX().ack_dsn = mhr.seq_number; /* save DSN to check acks */
  }

#ifndef ZB_MAC_EXT_DATA_REQ
  /* put our pan id as src and dst pan id */
  mhr.dst_pan_id = MAC_PIB().mac_pan_id;
  mhr.dst_addr.addr_short = data_req_params->dst_addr;
  mhr.src_pan_id = MAC_PIB().mac_pan_id;
  mhr.src_addr.addr_short = data_req_params->src_addr;
#else
  mhr.dst_pan_id = data_req_params->dst_pan_id;
  ZB_MEMCPY(&mhr.dst_addr, &data_req_params->dst_addr, sizeof(union zb_addr_u));
  mhr.src_pan_id = MAC_PIB().mac_pan_id;
  ZB_MEMCPY(&mhr.src_addr, &data_req_params->src_addr, sizeof(union zb_addr_u));
#endif

#ifdef ZB_ROUTER_ROLE
  if (data_req_params->tx_options & MAC_TX_OPTION_INDIRECT_TRANSMISSION_BIT)
  {
    /* put data request to pending queue */
    zb_mac_pending_data_t pend_data;

    pend_data.pending_data = data_req;
#ifndef ZB_MAC_EXT_DATA_REQ
    pend_data.dst_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    pend_data.dst_addr.addr_short = data_req_params->dst_addr;
#else
    pend_data.dst_addr_mode = data_req_params->dst_addr_mode;
    ZB_MEMCPY(&pend_data.dst_addr, &data_req_params->dst_addr, sizeof(union zb_addr_u));
#endif

    /* returns RET_PENDING on success */
    ret = zb_mac_put_data_to_pending_queue(&pend_data);
  }
#endif

  if (ret == RET_OK || ret == RET_PENDING)
  {
    /* get pointer to put header there */
    ZB_BUF_ALLOC_LEFT(data_req, data_req_params->mhr_len,  ptr);
    ZB_ASSERT(ptr);
    zb_mac_fill_mhr(ptr, &mhr);

#ifdef ZB_MAC_SECURITY
    if (data_req_params->security_level)
    {
      /* fill Aux security header */
      ptr += (data_req_params->mhr_len - MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE);
      /* security control: always level 5, key id mode 1 */
      *ptr = ZB_MAC_SECURITY_LEVEL | (ZB_MAC_KEY_ID_MODE << 3);
      ptr++;
      /* frame counter */
      ZB_HTOLE32(ptr, &MAC_PIB().mac_frame_counter);
      MAC_PIB().mac_frame_counter++;
      ptr += 4;
      /* key identifier */
      *ptr = data_req_params->key_index;
    }
#endif

  }
#ifdef ZB_ROUTER_ROLE
  else if (ret == RET_ERROR && ZB_GET_MAC_STATUS() == MAC_TRANSACTION_OVERFLOW)
  {
    TRACE_MSG(TRACE_MAC2, "put request back to mac out queue", (FMT__0));
    ZB_SET_MAC_STATUS(MAC_SUCCESS);
    zb_mac_put_request_to_queue(data_req, ZB_MAC_DATA_REQUEST);
    ret = RET_PENDING;
  }
#endif

  TRACE_MSG(TRACE_MAC2, "<< zb_mcps_data_request_fill_hdr ret %d", (FMT__D, ret));
  return ret;
}


/*
  7.1.1.1 MCPS-DATA.request
  KLUDGE: call zb_mcps_data_request_fill_hdr() first - it fills header
  and can put data request to pending queue if indirect data transfer
  is requested. In common operation_buf is used to store request, but
  in thios case buffer allocated and filled with data on the higher
  level is used.
*/
void zb_mcps_data_request(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG( TRACE_MAC1, ">>zb_mcps_data_request", (FMT__0));
  {
    ZB_SCHEDULE_TX_CB(zb_handle_mcps_data_req, param);
  }
  TRACE_MSG(TRACE_MAC1, "<<zb_mcps_data_request ", (FMT__0));
}

#ifdef ZB_SECURITY
zb_ret_t zb_mac_check_security(zb_buf_t *data_buf)
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_MAC2, ">> zb_mac_check_security data_buf %p", (FMT__P, data_buf));

  if (data_buf->u.hdr.encrypt_type != ZB_SECUR_NO_ENCR)
  {
    void *mac_hdr = ZB_BUF_BEGIN(data_buf);
    zb_ushort_t hlen = zb_mac_calculate_mhr_length(
      ZB_FCF_GET_SRC_ADDRESSING_MODE(mac_hdr),
      ZB_FCF_GET_DST_ADDRESSING_MODE(mac_hdr),
      ZB_FCF_GET_PANID_COMPRESSION_BIT(mac_hdr));

    TRACE_MSG(TRACE_MAC2, "security is on", (FMT__0));

    /*
     * Encrypt nwk payload now, send encrypted buffer, send confirm by
     * unencrypted buffer (!).
     * It is necessary to handle APS acks and retransmissions: we need
     * unencrypted payload.
     */
    if (data_buf->u.hdr.encrypt_type == ZB_SECUR_NWK_ENCR)
    {
      ret = zb_nwk_secure_frame(data_buf, hlen, MAC_CTX().encryption_buf);
    }
#ifdef ZB_MAC_SECURITY
    else if (data_buf->u.hdr.encrypt_type == ZB_SECUR_MAC_ENCR)
    {
      ret = zb_mac_secure_frame(data_buf, hlen, MAC_CTX().encryption_buf);
    }
#endif
#ifdef APS_FRAME_SECURITY
    else
    {
      zb_aps_secure_frame(data_buf, hlen, MAC_CTX().encryption_buf);
    }
#endif
  }

  TRACE_MSG(TRACE_MAC2, "<< zb_mac_check_security ret %d", (FMT__D, ret));
  return ret;
}
#endif


/* handle mac data request, caller side */
void zb_handle_mcps_data_req(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_mcps_data_req_params_t *data_req_params;
  zb_uint8_t is_unicast;

  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_MAC2, ">> zb_handle_mcps_data_req", (FMT__0));

  data_req_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mcps_data_req_params_t);
  ZB_ASSERT(data_req_params);

#ifndef ZB_MAC_EXT_DATA_REQ
  is_unicast = !ZB_NWK_IS_ADDRESS_BROADCAST(data_req_params->dst_addr);
  TRACE_MSG(TRACE_MAC2, "is_unicast %hd, dst_addr 0x%x", (FMT__H_D, is_unicast, data_req_params->dst_addr));
#else
  is_unicast = !(data_req_params->dst_addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST
                 && ZB_NWK_IS_ADDRESS_BROADCAST(data_req_params->dst_addr.addr_short));
  TRACE_MSG(TRACE_MAC2, "is_unicast %hd", (FMT__H, is_unicast));
#endif

    TRACE_MSG(TRACE_MAC1, "msdu_handle %hd tx_options %hd", (FMT__H_H, data_req_params->msdu_handle, data_req_params->tx_options));

    MAC_CTX().pending_buf->u.hdr.handle = data_req_params->msdu_handle;
    MAC_CTX().mac_status = MAC_SUCCESS;
    MAC_CTX().retry_counter = 0;

    /* this call may put data to pending queue if this is indirect transfer and
     * return RET_PENDING */
    ret = zb_mcps_data_request_fill_hdr(MAC_CTX().pending_buf);

/* Special case to turn on MAC security */
#ifdef ZB_MAC_TESTING_MODE
    if ( ret == RET_OK
         && data_req_params->msdu_handle == 0xFA )
    {
      ZB_FCF_SET_SECURITY_BIT(ZB_BUF_BEGIN(MAC_CTX().pending_buf), 1);
      TRACE_MSG(TRACE_MAC1, "set security bit!", (FMT__0));
    }
#endif


#ifdef ZB_SECURITY
    if (ret == RET_OK)
    {
      ret = zb_mac_check_security(MAC_CTX().pending_buf);
    }
#endif

    if (ret == RET_OK)
    {
      if (is_unicast)
      {
        ZB_TX_TOTAL_INC();
      }
#ifdef ZB_MANUAL_ACK
send_command:
#endif
      if (ret == RET_OK)
      {
        MAC_CTX().tx_wait_cb = zb_handle_mcps_data_req_continue;
        MAC_CTX().tx_wait_cb_arg = param;
        MAC_ADD_FCS(MAC_CTX().pending_buf);
#ifdef ZB_SECURITY
        if (MAC_CTX().pending_buf->u.hdr.encrypt_type != ZB_SECUR_NO_ENCR)
        {
          MAC_ADD_FCS(MAC_CTX().encryption_buf);
          ret = ZB_TRANS_SEND_COMMAND(data_req_params->mhr_len, MAC_CTX().encryption_buf);
        }
        else
#endif
        {
          ret = ZB_TRANS_SEND_COMMAND(data_req_params->mhr_len, MAC_CTX().pending_buf);
        }
      }
    }
#ifdef ZB_MANUAL_ACK
  if (ret == RET_OK)
  {
    zb_uint8_t *fcf;
    fcf = ZB_MAC_GET_FCF_PTR(ZB_BUF_BEGIN(MAC_CTX().pending_buf));
    if (ZB_FCF_GET_ACK_REQUEST_BIT(fcf))
    {
      /* ack_dsn was set in zb_mlme_send_association_req_cmd() */
      ret = zb_mac_check_ack();

#ifdef ZB_NS_BUILD
      /* superhack */
      if (ret == RET_BLOCKED)
      {
        ret = RET_OK;
      }
#endif
      if (ret == RET_PENDING)
      {
        /* retry data send */
        goto send_command;
      }
    }
  }
#endif
  TRACE_MSG(TRACE_MAC2, "<< zb_handle_mcps_data_req %i", (FMT__D, ret));
}

void zb_handle_mcps_data_req_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  zb_mcps_data_req_params_t *data_req_params;
  zb_uint8_t is_unicast;

  TRACE_MSG(TRACE_MAC2, ">> zb_handle_mcps_data_req_continue", (FMT__0));
  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
  data_req_params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mcps_data_req_params_t);

  ZB_SKIP_TX_CHK();
  ret = zb_check_cmd_tx_status();
  is_unicast = !ZB_NWK_IS_ADDRESS_BROADCAST(data_req_params->dst_addr);
  if (ret != RET_OK && ret != RET_BLOCKED && is_unicast)
  {
    ZB_TX_FAIL_INC();
  }
  if (ret != RET_BLOCKED)
  {
    /* check for inderect data tx */
    if ( ret != RET_PENDING )
    {

      if (ret != RET_OK && MAC_CTX().mac_status == MAC_SUCCESS)
      {
        TRACE_MSG(TRACE_MAC3, "Mac invalid parameter", (FMT__0));
        ZB_SET_MAC_STATUS(MAC_INVALID_PARAMETER);
      }
      {
        zb_mcps_data_confirm_params_t *confirm_params =
          ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mcps_data_confirm_params_t);
        confirm_params->msdu_handle = data_req_params->msdu_handle;
      }
      /* data request params are not actual anymore */
      MAC_CTX().pending_buf->u.hdr.status = ZB_GET_MAC_STATUS();
      TRACE_MSG(TRACE_MAC3, "mac status 0x%hx %hd", (FMT__H_H, ZB_GET_MAC_STATUS(), MAC_CTX().pending_buf->u.hdr.status));
      ZB_SCHEDULE_CALLBACK(zb_mcps_data_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
      MAC_CTX().pending_buf = NULL;
    }
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_handle_mcps_data_req_continue", (FMT__0));
}

void zb_handle_data_frame(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
#ifdef ZB_MANUAL_ACK
  zb_mac_mhr_t mhr;
#endif

  TRACE_MSG(TRACE_MAC1, ">> zb_handle_data_frame", (FMT__0));
  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);

#ifdef ZB_MANUAL_ACK
  zb_parse_mhr(&mhr, ZB_BUF_BEGIN(MAC_CTX().pending_buf));
  if (ZB_FCF_GET_ACK_REQUEST_BIT(mhr.frame_control))
  {
    ret = zb_mac_send_ack(mhr.seq_number, 0);
    if (ret == RET_OK)
    {
      ret = zb_check_cmd_tx_status();
    }
  }
#endif

#ifdef ZB_MAC_SECURITY
  if (ret == RET_OK)
  {
    zb_uint8_t *fc = ZB_BUF_BEGIN(MAC_CTX().pending_buf);
    if (ZB_FCF_GET_SECURITY_BIT(fc))
    {
      ret = zb_mac_unsecure_frame(ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
    }
  }
#endif

  if (ret == RET_OK)
  {
    ZB_SCHEDULE_CALLBACK(zb_mcps_data_indication, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }
  else if (ret != RET_BLOCKED)
  {
    zb_free_buf(MAC_CTX().pending_buf);
    MAC_CTX().pending_buf = NULL;
  }

  TRACE_MSG(TRACE_MAC1, "<< zb_handle_data_frame, ret %i", (FMT__D, ret));
}

/* function is used to call call_indirect_data_callback() with
 * allocated buffer */
void indirect_data_callback_caller(zb_uint8_t param) ZB_CALLBACK
{
  call_indirect_data_callback(MAC_CTX().rt_ctx.indirect_data.cb_type,
                              MAC_CTX().rt_ctx.indirect_data.cb_status, ZB_BUF_FROM_REF(param));
}

/*
  Calls specific callback during indirect data transfer
*/
zb_ret_t call_indirect_data_callback(zb_callback_type_t cb_type, zb_uint8_t cb_status, zb_buf_t *buf)
{

  TRACE_MSG(TRACE_MAC3, ">>call_ind_cb %i, %hd, %p", (FMT__D_H_P, (int)cb_type, cb_status, buf));
  if (cb_type == MAC_ASS_CONFIRM_CALLBACK)
  {
    ZB_MAC_CLEAR_ASS_REQUEST();
    if (buf)
    {
      zb_mlme_associate_confirm_t *ass_confirm_param;
      ass_confirm_param = ZB_GET_BUF_PARAM(buf, zb_mlme_associate_confirm_t);
      ZB_BZERO(ass_confirm_param, sizeof(zb_mlme_associate_confirm_t));
      ass_confirm_param->status = cb_status;
      ZB_SCHEDULE_CALLBACK(zb_mlme_associate_confirm, ZB_REF_FROM_BUF(buf));
    }
  }
  else if ( cb_type == MAC_POLL_REQUEST_CALLBACK )
  {
    ZB_MAC_CLEAR_POLL_REQUEST();
    if ( buf )
    {
      buf->u.hdr.status = cb_status;
      ZB_SCHEDULE_CALLBACK(zb_mlme_poll_confirm, ZB_REF_FROM_BUF(buf));
    }
  }
  else
  {
    ZB_ASSERT(0);
  }

  TRACE_MSG(TRACE_MAC3, "<<call_ind_cb", (FMT__0));
  return RET_OK;
}


/*! @} */
