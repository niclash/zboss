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
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_ubec24xx.h"
#include "zb_mac_globals.h"
#ifdef ZB_CC25XX
#include "zb_cc25xx.h"
#endif

/*! \addtogroup ZB_MAC */
/*! @{ */

/* Both MAC and NWK placed to the bank1, so all MAC calls are not banked */
#include "zb_bank_1.h"


static zb_bool_t check_frame_dst_addr(zb_mac_mhr_t *mhr) ZB_SDCC_REENTRANT;
#ifdef ZB_LIMIT_VISIBILITY
static zb_bool_t mac_is_frame_visible(zb_mac_mhr_t *mhr);
#endif


void zb_mac_init() /* __reentrant for sdcc, to save DSEG space */
{
  TRACE_MSG( TRACE_MAC1, ">>mac_init", (FMT__0 ));

  /* Q: Do we really need all that parameters to be
     configurable at runtime? Maybe, use constants instead?

     A: Yes, in general we need all PIB
     attributes configurable, because they could be changed via
     MLME-SET.request. This is a part of mac_layer API. For zigbee, we
     don't need all these attributes configurable. In zigbee2007 only
     the following PIB attributes could be changed:
     1. macShortAddress
     2. macAssociationPermit
     3. macAutoRequest
     4. macPANID
     5. macBeaconPayload
     6. macBeaconPayloadLength

     Replacing an attribute with a constant could save ~5-7 bytes of code for
     each occurrance. Currently, ZB_CONFIGURABLE_MAC_PIB in zb_config.h is implemented to
     switch between configurable and const PIB mode
  */
#if defined (ZB_CONFIGURABLE_MAC_PIB) || defined(ZB_NS_BUILD) || defined(ZB_CC25XX)
  MAC_PIB().mac_ack_wait_duration =
    (ZB_UINT_BACKOFF_PERIOD + ZB_TURNAROUND_TIME + ZB_PHY_SHR_DURATION + (zb_uint16_t)(6 * ZB_PHY_SYMBOLS_PER_OCTET));

/*
  macAckWaitDuration =  aUnitBackoffPeriod + aTurnaroundTime + phySHRDuration + (6 * phySymbolsPerOctet)
*/
/* this time value should be calculated according to the described
 * formula, but it is too complex, so use estimated predefined value */
  MAC_PIB().mac_max_frame_retries = ZB_MAC_MAX_FRAME_RETRIES;
#endif

#ifdef ZB_CONFIGURABLE_MAC_PIB

/*
  macMaxFrameTotalWaitTime = ( SUM (0..m-1) (2 ^ (macMinBE +k)) + (2 ^ macMaxBE - 1)*(macMaxCSMABackoffs - m) ) * aUnitBackoffPeriod + phyMaxFrameDuration

  m is min(macMaxBE-macMinBE, macMaxCSMABackoffs).
  macMaxBE = 3-8
  macMinBE = 0 - macMaxBE
  macMaxCSMABackoffs = 0 - 5
  aUnitBackoffPeriod = 20
  phyMaxFrameDuration = 55, 212, 266, 1064
*/
/* this time value should be calculated according to the described
 * formula, but it is too complex, so use estimated predefined value */
  MAC_PIB().mac_max_frame_total_wait_time = ZB_MAX_FRAME_TOTAL_WAIT_TIME;

/*
  aBaseSlotDuration = 60
  aNumSuperframeSlots = 16
  aBaseSuperframeDuration (symbols) = aBaseSlotDuration * aNumSuperframeSlots
*/
  MAC_PIB().mac_base_superframe_duration = ZB_MAC_BASE_SLOT_DURATION * ZB_MAC_NUM_SUPERFRAME_SLOTS;

/* The maximum time (in unit periods) that a transaction is stored by
 * a coordinator and indicated in its beacon. range 0x0000 -
 * 0xffff. unit period = aBaseSuperframeDuration (== beacon interval)*/
  MAC_PIB().mac_transaction_persistence_time = ZB_MAC_TRANSACTION_PERSISTENCE_TIME;
#endif
  MAC_CTX().operation_buf = zb_get_out_buf();

  MAC_CTX().operation_recv_buf = zb_get_in_buf();
#ifdef ZB_SECURITY
  MAC_CTX().encryption_buf = zb_get_out_buf();
#endif
#ifndef ZB_NS_BUILD
  /* TODO: move HW init to the mcps.start */
  init_zu2400();
#endif
  MAC_PIB().mac_dsn = ZB_RANDOM();
  MAC_PIB().mac_bsn = ZB_RANDOM();
#ifdef ZB_USE_RX_QUEUE
  {
    int i = ZB_RX_QUEUE_CAP;
    /* point for further refactoring and tests */
    /* this loop takes much more code, than plain code */
    while (i--)
    {
      MAC_CTX().mac_rx_queue.ring_buf[i] = zb_get_in_buf();
    }
  }
#endif

  TRACE_MSG( TRACE_MAC1, "<<mac_init", (FMT__0 ));
}


/**
   Read data from transceiver

   This routine can be called in both cases: if mac state is idle or some mac logic is in process
   The routine sets and resets state on mac rx layer.

   @param wait_for_data - specifies to wait for incoming data or not. If wait_for_data is ZB_TRUE,
   routine will return RET_BLOCKED in case incoming data is available, will return RET_OK otherwise
   @return RET_OK, RET_BLOCKED
*/
void zb_mac_recv_data(zb_uint8_t param)
{
  TRACE_MSG(TRACE_MAC1, ">>zb_mac_recv_data", (FMT__0));
  /* Q: Do we need MAC_CTX().recv_buf here? It used by
   * ZB_GET_RX_QUEUE() only. Pass it as macro parameter, get result, again, as
   * macro parameter?
   *
   * A: No, we don't, but passing a param with
   * all related stuff will cost 1b RAM and ~25 bytes CODE. And BTW, changing
   * a param variable in macro isn't the best for code readability
   */

  MAC_CTX().recv_buf = ZB_BUF_FROM_REF(param);
#ifdef ZB_USE_RX_QUEUE
  ZB_GET_RX_QUEUE();
  DUMP_RESERVED_REGS();
#else
  ZB_TRANS_RECV_PACKET(MAC_CTX().recv_buf);
#endif
#ifdef ZB_NS_BUILD
  zb_mac_parse_recv_data(ZB_REF_FROM_BUF(MAC_CTX().recv_buf));
#else
  ZB_SCHEDULE_CALLBACK(zb_mac_parse_recv_data, ZB_REF_FROM_BUF(MAC_CTX().recv_buf));
#endif /* ZB_NS_BUILD */
  MAC_CTX().recv_buf = NULL;
  TRACE_MSG(TRACE_MAC1, "<<zb_mac_recv_data", (FMT__D));
}

static zb_bool_t can_accept_frame(zb_mac_mhr_t mhr)
{
  zb_bool_t ret = ZB_FALSE;

#ifdef ZB_LIMIT_VISIBILITY
  if (!mac_is_frame_visible(&mhr))
  {
    TRACE_MSG(TRACE_COMMON1, "filtered frame dropped", (FMT__0));
  }
  else
#endif
  {
#ifdef ZB_BLOCK_BROADCASTS_SLEEPY_ED
    if ((!ZB_PIB_RX_ON_WHEN_IDLE())&&(mhr.dst_addr.addr_short == 0xffff))
    {
      TRACE_MSG(TRACE_COMMON1, "drop broadcast", (FMT__0));
    }
    else
    {
#endif
      if (ZB_FCF_GET_SECURITY_BIT(mhr.frame_control)
          && ZB_FCF_GET_FRAME_VERSION(mhr.frame_control) < MAC_FRAME_IEEE_802_15_4)
      {
        TRACE_MSG(TRACE_MAC1, "unsupported 2003 security frame dropped", (FMT__0));
      }
      else
      {
        ret = ZB_TRUE;
      }
    }
  }
  TRACE_MSG(TRACE_MAC1, "<<can_accept_fr, ret %i", (FMT__D, ret));
  return ret;
}

void zb_mac_main_loop()
{
  if (ZB_GET_TRANS_INT())
  {
    TRACE_MSG(TRACE_COMMON2, "Interrupt occured, processing...", (FMT__0 ));
    ZB_CHECK_INT_STATUS();
  }

#ifdef ZB_CC25XX
  if (ZB_MAC_GET_ACK_OK())
  {
#endif
    /* check for cb waiting for tx finished, it's also indicats that tx in progress */
    if (MAC_CTX().tx_wait_cb)
    {
      /* Somebody is waiting for TX complete */
      /* Q: is tx_cnt really counter or just flag?

      A: Flag, but could be used as counter for debug */
      if (MAC_CTX().tx_cnt)
      {
        /* TX complete. Call function waiting for it. */
        MAC_CTX().tx_cnt = 0;
        TRACE_MSG(TRACE_COMMON2, "tx waiting callback called, func: %p", (FMT__P, MAC_CTX().tx_wait_cb));
        (MAC_CTX().tx_wait_cb)(MAC_CTX().tx_wait_cb_arg);
        MAC_CTX().tx_wait_cb = NULL;
      }
    }
    else
    {
      /* Use separate callbacks queue for TX: can call next callback only
       * after previous TX finished.
       * If no tx_wait_cb assigned, TX is idle.
       */
      /* calling callbacks, that need free tx */
      zb_mac_cb_ent_t *mac_cb_ent;
      if ((mac_cb_ent = ZB_RING_BUFFER_PEEK(&ZG->sched.mac_tx_q)) != NULL)
      {
        TRACE_MSG(TRACE_COMMON2, "tx callback called", (FMT__0 ));
        (*mac_cb_ent->func)(mac_cb_ent->param);
        ZB_RING_BUFFER_FLUSH_GET(&ZG->sched.mac_tx_q);
      }
    }
#ifdef ZB_CC25XX
  }
  else
  {
    if (ZB_MAC_GET_ACK_TIMEOUT())
    {
      // TODO need add retransmit
      /* ack is not received during timeout period or ack is failed */
      /* increase re-try counter, check it and try again to send command */
      ZB_SCHEDULE_ALARM_CANCEL(zb_mac_ack_timeout, ZB_ALARM_ANY_PARAM); /* cancel scheduled alarm */
      MAC_CTX().retry_counter++;
      if (MAC_CTX().retry_counter > MAC_PIB().mac_max_frame_retries)
      {
        TRACE_MSG(TRACE_MAC1, "Error, ack fail", (FMT__0));
        ZB_MAC_CLEAR_ACK_TIMEOUT();
        ZB_MAC_CLEAR_ACK_FAIL();
        ZB_MAC_CLEAR_ACK_OK();
        ZB_SET_MAC_STATUS(MAC_NO_ACK);
      }
      else
      {
        ZB_MAC_START_ACK_WAITING();
        /* retransmit */
        RFST = 0xE3; /* rx on    */
      }
    }
  }
#endif
#ifdef ZB_USE_RX_QUEUE
  if (((!ZB_RING_BUFFER_IS_EMPTY(&MAC_CTX().mac_rx_queue))||MAC_CTX().recv_buf_full))
#else
    /* change it to flag?? */
#ifdef ZB_NS_BUILD
    if (ZG->sched.mac_receive_pending)
#else
    if (ZB_UBEC_GET_RX_DATA_STATUS())
#endif
#endif
    {
      zb_buf_t *buf = zb_get_in_buf();
      if (buf)
      {
        /* We could call it directly */
        /* ZB_SCHEDULE_MAC_CB(zb_mac_recv_data, ZB_REF_FROM_BUF(buf));*/
#ifndef ZB_NS_BUILD
        ZB_UBEC_CLEAR_RX_DATA_STATUS();
#else
        ZG->sched.mac_receive_pending = 0;
#endif
        zb_mac_recv_data(ZB_REF_FROM_BUF(buf));
        MAC_CTX().rx_need_buf = 0;
      }
      else
      {
        MAC_CTX().rx_need_buf = 1;
      }
    }

/* high priority mac layer queue. Could be used to manage callbacks priority. Currently unused. */
#if 0
  { /* checking mac callback queue */
    zb_mac_cb_ent_t *mac_cb_ent;
    while ((mac_cb_ent = ZB_RING_BUFFER_PEEK(&ZG->sched.mac_cb_q)) != NULL)
    {
      TRACE_MSG(TRACE_COMMON2, "Mac loop", (FMT__0 ));
      (*mac_cb_ent->func)(mac_cb_ent->param);
      ZB_RING_BUFFER_FLUSH_GET(&ZG->sched.mac_cb_q);
    }
  }
#endif
}



void zb_mac_parse_recv_data(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_uint8_t *fcf;
  zb_uint8_t *cmd_ptr;
  zb_mac_mhr_t mhr;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
#ifdef ZB_MAC_TESTING_MODE
  zb_uint8_t mhr_len;
#endif

  TRACE_MSG(TRACE_MAC1, ">>mac_parse_recv_dt", (FMT__0));

  ZB_ASSERT(buf);
  ZB_DUMP_INCOMING_DATA(buf);
  /* ZB_MAC_STOP_IO(); moved to interrupt handlers */
  ZB_TRANS_CUT_SPECIFIC_HEADER(buf);

  cmd_ptr = ZB_BUF_BEGIN(buf);
#ifdef ZB_MAC_TESTING_MODE
  mhr_len = zb_parse_mhr(&mhr, cmd_ptr);
#else
  /* TODO: partial MHR parse? */
  zb_parse_mhr(&mhr, cmd_ptr);
#endif
  TRACE_MSG(TRACE_MAC2, "frame_control %hx:%hx, seq_number %hu, dst_pan_id 0x%x, src_pan_id 0x%x dst mode %hi addr " TRACE_FORMAT_64 " src mode %hi addr " TRACE_FORMAT_64,
            (FMT__H_H_H_D_D_H_A_H_A,
             mhr.frame_control[0], mhr.frame_control[1], mhr.seq_number, mhr.dst_pan_id, mhr.src_pan_id,
             ZB_FCF_GET_DST_ADDRESSING_MODE((mhr).frame_control), TRACE_ARG_64((mhr).dst_addr.addr_long),
             ZB_FCF_GET_SRC_ADDRESSING_MODE((mhr).frame_control), TRACE_ARG_64((mhr).src_addr.addr_long)));

  fcf = ZB_MAC_GET_FCF_PTR(cmd_ptr);

  TRACE_MSG(TRACE_MAC3, "set mac rx , frm %hi len %hi", (FMT__H_H,
                                                         ZB_FCF_GET_FRAME_TYPE(fcf), ZB_BUF_LEN(buf)));
  if (can_accept_frame(mhr))
  {
    if (ZB_MAC_GET_INDIRECT_DATA_REQUEST())
    {
/* if there is indirect request, check if received frame is sent as a
 * response to this request: dst addr == current device addr; maybe it
 * will be good to check source addr. Handle received frame right
 * now. */
      TRACE_MSG(TRACE_MAC3, "addr %i / " TRACE_FORMAT_64, (FMT__D_A,
                                                           MAC_PIB().mac_short_address, TRACE_ARG_64(MAC_PIB().mac_extended_address)));
      if (check_frame_dst_addr(&mhr))
      {
        /* data is received indirectly */
        /* cancel indirect data transfer timeout */
        ZB_SCHEDULE_ALARM_CANCEL(zb_mac_indirect_data_timeout, ZB_ALARM_ANY_PARAM);
        ZB_MAC_CLEAR_INDIRECT_DATA_REQUEST();
        TRACE_MSG(TRACE_MAC2, "got indir dt, OK", (FMT__0));
        if (ZB_FCF_GET_FRAME_PENDING_BIT(fcf))
        {
          /* set flag - there is more pending data on coordinator */
          ZB_MAC_SET_PENDING_DATA();
          TRACE_MSG(TRACE_MAC2, "Pending data set, OK", (FMT__0));
        }
        /* check if poll request */
        if ( ZB_MAC_GET_POLL_REQUEST() /* poll request? */
             && ZB_FCF_GET_FRAME_TYPE(fcf) == MAC_FRAME_DATA
             && MAC_CTX().rt_ctx.indirect_data.buf_ref )
        {
          ZB_MAC_CLEAR_POLL_REQUEST();
          ZB_BUF_FROM_REF(MAC_CTX().rt_ctx.indirect_data.buf_ref)->u.hdr.status = MAC_CTX().mac_status;
          ZB_SCHEDULE_CALLBACK(zb_mlme_poll_confirm, MAC_CTX().rt_ctx.indirect_data.buf_ref);
        }
      }
    }

    switch (ZB_FCF_GET_FRAME_TYPE(fcf))
    {
      case MAC_FRAME_DATA:
        ZB_SCHEDULE_CALLBACK(zb_handle_data_frame, ZB_REF_FROM_BUF(buf));
        break;
      case MAC_FRAME_BEACON:
        TRACE_MSG(TRACE_MAC3, "BEACON frame", (FMT__0));

#ifdef ZB_MAC_TESTING_MODE
        {
          zb_uint8_t payload_len;

          payload_len = ZB_BUF_LEN(buf) - ZB_BEACON_PAYLOAD_OFFSET(mhr_len) - ZB_BEACON_PAYLOAD_TAIL;
          TRACE_MSG(TRACE_NWK1, "payload len %hd, auto_req %hd", (FMT__H_H, payload_len, MAC_PIB().mac_auto_request));

          /* if mac auto request == FALSE or beacon patyload length is not 0, call beacon notify indication */
          if (!MAC_PIB().mac_auto_request || payload_len)
          {
            zb_mlme_beacon_notify_indication(param);
          }

          /* 7.1.11.1 MLME-SCAN.request
           * if macAutoRequest == TRUE, MLME-SCAN.confirm primitive
           * will contain PAN descriptor list; otherwise, MLME-BEACON-NOTIFY is
           * called only if not-empty beacon payload */
          if (MAC_PIB().mac_auto_request)
          {
            zb_mac_store_pan_desc(buf);
          }
        }

#else /* ZB_MAC_TESTING_MODE */

        ZB_SCHEDULE_CALLBACK(zb_mlme_beacon_notify_indication, param);

#endif /* ZB_MAC_TESTING_MODE */
        /* signal that at least 1 beacon came during active scan */
        if (MAC_CTX().mlme_scan_in_progress)
        {
          MAC_CTX().rt_ctx.active_scan.beacon_found = 1;
          TRACE_MSG(TRACE_MAC3, "beacon found!", (FMT__0));
        }
        break;
      case MAC_FRAME_ACKNOWLEDGMENT:
        /*ZB_SCHEDULE_CALLBACK(zb_mlme_ack_accept, ZB_REF_FROM_BUF(MAC_CTX().recv_buf));*/
        /* it seems scheduler usage is not needed here */
        TRACE_MSG(TRACE_MAC2, "Ack acccepted", (FMT__0));
#if defined(ZB_NS_BUILD) || defined(ZB_CC25XX)
        zb_mlme_ack_accept(param);
#else
        ZB_SCHEDULE_CALLBACK(zb_mlme_ack_accept, param);
#endif
        break;
      case MAC_FRAME_COMMAND:
        TRACE_MSG(TRACE_MAC3, "sched cmd accept", (FMT__0));
        ZB_SCHEDULE_CALLBACK(zb_mlme_command_accept, param);
        break;
      default:
        break;
    }
  }
  else
  {
    zb_free_buf(buf);
    buf = NULL;
  }
  TRACE_MSG(TRACE_MAC1, "<<mac_parse_recv_data %d", (FMT__D, ret));
}



#ifndef ZB_LIMITED_FEATURES
void zb_mlme_reset_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_mlme_reset_request_t *reset_req = NULL;

  /* TODO: init MAC HW here, not in mac_init()! */
  buf->u.hdr.status = 0;
  reset_req = ZB_GET_BUF_PARAM(buf, zb_mlme_reset_request_t);
  if (reset_req->set_default_pib == 1)
  {
    /* TODO: re-init PIB if needed */
  }

#ifndef ZB_NS_BUILD
//  init_zu2400(); /* ret code is ignored according to spec */
  /* TODO: check if we need to re-init mac */
  /* TODO: fill confirm buffer correctly */
#endif
  ZB_SCHEDULE_CALLBACK(zb_mlme_reset_confirm, param);
}
#endif


/**
   Calculates length of mac header (MHR) inside MAC frame

   param src_addr_mode   - source addressing mode one of values of zb_addr_mode_e enum
   param dst_addr_mode   - destination addressing mode one of values of zb_addr_mode_e enum
*/
zb_uint8_t zb_mac_calculate_mhr_length(zb_uint8_t src_addr_mode, zb_uint8_t dst_addr_mode, zb_uint8_t panid_compression)
{
  zb_uint8_t len = ZB_MAC_DST_PANID_OFFSET;

  if( ( src_addr_mode ) != ZB_ADDR_NO_ADDR )
  {
    ZB_ASSERT( ( src_addr_mode ) == ZB_ADDR_16BIT_DEV_OR_BROADCAST ||
               ( src_addr_mode ) == ZB_ADDR_64BIT_DEV);

    if (!panid_compression)
    {
      len += PAN_ID_LENGTH;
    }

    len += 2;
    if (src_addr_mode == ZB_ADDR_64BIT_DEV)
    {
      len += 6;
    }
  }

  if( ( dst_addr_mode ) != ZB_ADDR_NO_ADDR )
  {
    ZB_ASSERT( ( dst_addr_mode ) == ZB_ADDR_16BIT_DEV_OR_BROADCAST ||
               ( dst_addr_mode ) == ZB_ADDR_64BIT_DEV);

    len += (PAN_ID_LENGTH + 2);
    if (dst_addr_mode == ZB_ADDR_64BIT_DEV)
    {
      len += 6;
    }
  }
  return len;
}

/*
  Fill packed mac header

  param ptr - pointer to out data
  param mhr - structure with mhr data
*/
void zb_mac_fill_mhr(zb_uint8_t *ptr, zb_mac_mhr_t *mhr)
{
  zb_uint8_t val;

  TRACE_MSG( TRACE_MAC1, ">>mac_fill_mhr", (FMT__0));
  ZB_ASSERT(ptr && mhr);

  /* mac spec 7.2.1 General MAC frame format */
/* MHR structure
   | Frame     | Seq       | dst PAN | dst addr| src PAN | src addr| aux security      |
   | Control 2b| Number 1b | id 0/2b | 0/2/8b  | id 0/2b | 0/2/8b  | header 0/5/6/10/14|*/

  ZB_MEMCPY(ptr, &mhr->frame_control, 2);
  ptr += 2;
  *ptr = mhr->seq_number;
  ptr += sizeof(zb_uint8_t);

  /* mac spec 7.2.1.1.6 Destination Addressing Mode subfield */
  val = ZB_FCF_GET_DST_ADDRESSING_MODE(mhr->frame_control);
  if (val)
  {
    zb_put_next_htole16(&ptr, mhr->dst_pan_id);

    /* dst addr mode: ZB_ADDR_NO_ADDR, ZB_ADDR_16BIT_DEV_OR_BROADCAST or ZB_ADDR_64BIT_DEV */
    if (val == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
    {
      zb_put_next_htole16(&ptr, mhr->dst_addr.addr_short);
    }
    else
    {
      ZB_HTOLE64(ptr, &mhr->dst_addr.addr_long);
      ptr += sizeof(zb_ieee_addr_t);
    }
  }

  /* mac spec 7.2.1.1.8 Source Addressing Mode subfield */
  val = ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control);
  if (val)
  {
    if (!ZB_FCF_GET_PANID_COMPRESSION_BIT(mhr->frame_control))
    {
      zb_put_next_htole16(&ptr, mhr->src_pan_id);
    }

    /* dst addr mode: ZB_ADDR_NO_ADDR, ZB_ADDR_16BIT_DEV_OR_BROADCAST or ZB_ADDR_64BIT_DEV */
    if (val == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
    {
      zb_put_next_htole16(&ptr, mhr->src_addr.addr_short);
    }
    else
    {
      ZB_HTOLE64(ptr, &mhr->src_addr.addr_long);
    }
  }
  /* TODO: add Aux Security Header (for MAC security only - not for ZB 2007) */
  TRACE_MSG( TRACE_MAC1, "<<mac_fill_mhr", (FMT__0));
}

/*
  Function parses incoming MAC command and executes it
*/
void zb_mlme_command_accept(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *request = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_uint8_t *cmd_ptr;
  zb_mac_mhr_t mhr;

  TRACE_MSG(TRACE_MAC2, ">>mlme_cmd_acc %hd", (FMT__H, param));

/*
  5.5.3.4 MAC command frame
  | MHR (var length) | Command type 1 byte | Command payload (var length) | FCS 2 bytes |
*/
  cmd_ptr = ZB_BUF_BEGIN(request);
  cmd_ptr += zb_parse_mhr(&mhr, cmd_ptr);

  TRACE_MSG(TRACE_MAC2, "cmd t %hu", (FMT__H, (*cmd_ptr)));
#ifdef ZB_ROUTER_ROLE
  if (((*cmd_ptr) == MAC_CMD_BEACON_REQUEST) && (!ZG->nwk.handle.joined_pro)) /* Command type check */
  {
#ifdef ZB_MULTIPLE_BEACONS
    MAC_CTX().beacons_sent = 0;
    TRACE_MSG(TRACE_MAC3, "hanaling beacon req", (FMT__0));
    ZB_MAC_SET_BEACON_REQ();
#endif
    ZB_SCHEDULE_TX_CB(zb_handle_beacon_req, 0);
    TRACE_MSG(TRACE_MAC3, "free buf %p", (FMT__P, request));
    zb_free_buf(request);
    ZIG->ioctx.recv_data_buf = NULL;
  }
  else if ((*cmd_ptr) == MAC_CMD_ASSOCIATION_REQUEST)
  {
    ZB_SCHEDULE_CALLBACK(zb_accept_ass_request_cmd, param);
  }
  else
#endif
    /* end device side */

    /*
      parse response command and make associate confirm call
      7.3.2 Association response command
      | MHR | command id 1 byte | short addr 2 bytes | status 1 byte |
    */
    if ((*cmd_ptr) == MAC_CMD_ASSOCIATION_RESPONSE)
    {
      zb_mlme_associate_confirm_t *ass_confirm_param = ZB_GET_BUF_PARAM((zb_buf_t*)ZB_BUF_FROM_REF(param), zb_mlme_associate_confirm_t);


      if (ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr.frame_control) == ZB_ADDR_64BIT_DEV)
      {
        ZB_IEEE_ADDR_COPY(ass_confirm_param->parent_address, mhr.src_addr.addr_long);
      }
      else
      {
        ZB_IEEE_ADDR_ZERO(ass_confirm_param->parent_address);
      }

#ifdef ZB_MANUAL_ACK
      if (ZB_FCF_GET_ACK_REQUEST_BIT(mhr.frame_control))
      {
        zb_mac_send_ack(mhr.seq_number, 0);
        /* TODO: add zb_check_cmd_tx_status() call here to wait until
         * ack is actually send; not sure it is ever needed - described
         * check is actual for transceiver with manual ack send; to
         * make this check need to make separate func for ass response */
      }
#endif
      if (ZB_MAC_GET_ASS_REQUEST())
      {
        cmd_ptr++;
        zb_get_next_letoh16(&ass_confirm_param->assoc_short_address, &cmd_ptr);
        MAC_PIB().mac_short_address = ass_confirm_param->assoc_short_address;
        TRACE_MSG(TRACE_MAC3, "saddr set: %d", (FMT__D, MAC_PIB().mac_short_address));
        ZB_UPDATE_SHORT_ADDR();
        ass_confirm_param->status = *cmd_ptr;
        ZB_BUF_FROM_REF(param)->u.hdr.status = ass_confirm_param->status;
        TRACE_MSG(TRACE_MAC1, "zb_mlme_asociate_confirm scheduled", (FMT__0));
        ZB_SCHEDULE_CALLBACK(zb_mlme_associate_confirm, param);
        ZB_MAC_CLEAR_ASS_REQUEST();
      }
      else
      {
        zb_free_buf(ZB_BUF_FROM_REF(param));
      }

    }

    else if ((*cmd_ptr)==MAC_CMD_DATA_REQUEST)
    {  /*
         Coordinator side
         7.3.4 Data request command
         | MHR | cmd id |
       */
#ifdef ZB_COORDINATOR_ROLE
      TRACE_MSG(TRACE_MAC3, "CMD_DATA_REQ", (FMT__0));
      ZB_SCHEDULE_TX_CB(zb_accept_data_request_cmd, param);
#else
      TRACE_MSG(TRACE_MAC1, "data req cmd came. not ZC - skip", (FMT__0));
#endif
    } else

      if ((*cmd_ptr)==MAC_CMD_COORDINATOR_REALIGNMENT)
      {
        TRACE_MSG(TRACE_MAC3, "COORD_REAL_CMD", (FMT__0));

        if ( MAC_CTX().mlme_scan_in_progress
             && ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_scan_params_t)->scan_type == ORPHAN_SCAN )
        {
          TRACE_MSG(TRACE_MAC3, "upd network", (FMT__0));
          MAC_CTX().rt_ctx.orphan_scan.got_realignment = ZB_TRUE;

          /* TODO: check realign command length */

          /* skip command id */
          cmd_ptr++;

          /* update panid */
          zb_get_next_letoh16(&MAC_PIB().mac_pan_id, &cmd_ptr);

          /* update coordinator short address */
          zb_get_next_letoh16(&MAC_PIB().mac_coord_short_address, &cmd_ptr);

          /* logicl channel */
          MAC_PIB().phy_current_channel = *cmd_ptr;
          cmd_ptr++;

          /* short address */
          zb_get_next_letoh16(&MAC_PIB().mac_short_address, &cmd_ptr);
          TRACE_MSG(TRACE_MAC3, "saddr set: %d", (FMT__0,MAC_PIB().mac_short_address));
          ZB_UPDATE_SHORT_ADDR();
          TRACE_MSG(TRACE_MAC1, "set pan id %d", (FMT__D, MAC_PIB().mac_pan_id));
          ZB_UPDATE_PAN_ID();
        }
      } else


#ifndef ZB_LIMITED_FEATURES
        if ((*cmd_ptr)==MAC_CMD_ORPHAN_NOTIFICATION)
        {
          if ( ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr.frame_control) == ZB_ADDR_64BIT_DEV )
          {
            zb_mac_orphan_ind_t *orphan_ind = ZB_GET_BUF_PARAM((zb_buf_t*)ZB_BUF_FROM_REF(param), zb_mac_orphan_ind_t);
            ZB_IEEE_ADDR_COPY(orphan_ind->orphan_addr, mhr.src_addr.addr_long);

            ZB_SCHEDULE_CALLBACK(zb_mlme_orphan_indication, param);
          }
          else
          {
            /* mailformed packet */
            TRACE_MSG(TRACE_MAC1, "orph ind should have long src addr", (FMT__0));
          }
        } else
#if 0
          if ((*cmd_ptr)==MAC_CMD_DISASSOCIATION_NOTIFICATION)
          {
          } else
            if ((*cmd_ptr)==MAC_CMD_PAN_ID_CONFLICT_NOTIFICATION)
            {
            } else
              if ((*cmd_ptr)==MAC_CMD_GTS_REQUEST)
              {
              } else
#endif
#endif
              {
                TRACE_MSG(TRACE_MAC1, "ERROR unsupp cmd %hu", (FMT__H, *cmd_ptr));
              }

  TRACE_MSG(TRACE_MAC2, "<<mlme_cmd_acc", (FMT__0));
}

/*
  Function checks received ack.
*/
void zb_mlme_ack_accept(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *request = (zb_buf_t*)ZB_BUF_FROM_REF(param);
#if defined(ZB_MANUAL_ACK) || defined(ZB_CC25XX)
  zb_uint8_t *cmd_ptr;
  zb_mac_mhr_t mhr;

  TRACE_MSG(TRACE_MAC2, ">>mlme_ack_acc %p", (FMT__P, request));

/*
  7.2.2.3 Acknowledgment frame format
  | FCF 2 bytes | Seq num 1 byte |
*/
  cmd_ptr = ZB_BUF_BEGIN(request);
  cmd_ptr += zb_parse_mhr(&mhr, cmd_ptr);
/*
  7.5.6.4.3 Retransmissions
  - if ack is received within ack timeout period, and sequence number == saved dsn, set Ok status
  - set fail status otherwise
*/
  if (MAC_CTX().ack_dsn == mhr.seq_number)
  {
    ZB_MAC_SET_ACK_OK();
    TRACE_MSG(TRACE_MAC2, "ack ok dsn %hi, frame_pending_bit %d, frame_ctrl %hx %hx",
              (FMT__H_D_H_H, MAC_CTX().ack_dsn, ZB_FCF_GET_FRAME_PENDING_BIT(mhr.frame_control), mhr.frame_control[0], mhr.frame_control[1]));
/* check if there is pending data, 7.2.2.3.1 Acknowledgment frame MHR fields */

    if (ZB_FCF_GET_FRAME_PENDING_BIT(mhr.frame_control))
    {
      ZB_MAC_SET_PENDING_DATA();
      TRACE_MSG(TRACE_MAC2, "Pending data set", (FMT__0));
    }
    else
    {
      ZB_MAC_CLEAR_PENDING_DATA();
      TRACE_MSG(TRACE_MAC2, "Pending data clear", (FMT__0));
    }
    ZB_SCHEDULE_ALARM_CANCEL(zb_mac_ack_timeout, ZB_ALARM_ANY_PARAM); /* cancel scheduled alarm */
  }
  else
  {
#if 0
    /* ignore ack with incorrect dsn: acks are broadcasted and we can
     * receive other's ack */
    ZB_MAC_SET_ACK_FAIL();
#endif

    TRACE_MSG(TRACE_MAC2, "ack failed dsn %hi, recv %hi - ignore", (FMT__H_H, MAC_CTX().ack_dsn, mhr.seq_number));
  }
#endif
  TRACE_MSG(TRACE_MAC3, "free buf %p", (FMT__P, request));
  zb_free_buf(request);

  TRACE_MSG(TRACE_MAC2, "<<mlme_ack_acc", (FMT__0));
}

/*
  sends acknowledgement. zb_mac_send_ack is not blocked function, but
  we need to call blocked func zb_check_cmd_tx_status() to wait until
  ack is send by transceiver
  param ack_dsn - frame sequence number to acknowledge
  param data_pending - signal if there is more pending data
  return RET_OK, RET_ERROR
*/
#ifdef ZB_MANUAL_ACK
zb_ret_t zb_mac_send_ack(zb_uint8_t ack_dsn, zb_uint8_t data_pending)
{
  zb_ret_t ret;
  zb_mac_mhr_t mhr;
  zb_uint_t mhr_len;
  zb_uint8_t *ptr;

  TRACE_MSG(TRACE_MAC2, ">>mac_send_ack %hi", (FMT__H, ack_dsn));
/*
  7.2.2.3 Acknowledgment frame format
  | FCF 2 bytes | Seq num 1 byte |
*/
  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_NO_ADDR, ZB_ADDR_NO_ADDR, 0);

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, mhr_len, ptr);
  ZB_ASSERT(ptr);
  ZB_BZERO(ptr, mhr_len);

  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_ACKNOWLEDGMENT);
  ZB_FCF_SET_FRAME_PENDING_BIT(mhr.frame_control, data_pending);
  mhr.seq_number = ack_dsn;
  zb_mac_fill_mhr(ptr, &mhr);

  MAC_ADD_FCS(MAC_CTX().operation_buf);
  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  TRACE_MSG(TRACE_MAC2, "<<mac_send_ack ret %i", (FMT__D, ret));
  return ret;
}
#endif
/**
   Gets source addressing mode subfield in frame control field ( FCF )
   Return values is one value from zb_addr_mode_e enum.

   @param p_fcf - pointer to 16bit FCF field.
*/

zb_void_t zb_fcf_set_src_addressing_mode(zb_uint8_t *p_fcf, zb_uint8_t addr_mode)
{
  ZB_ASSERT( ( addr_mode ) == 0 ||
             ( addr_mode ) == 1 ||
             ( addr_mode ) == 2 ||
             ( addr_mode ) == 3 );

  ( ( ( zb_uint8_t* ) ( p_fcf ))[ZB_PKT_16B_FIRST_BYTE] ) &= 0x3F;
  ( ( ( zb_uint8_t* ) ( p_fcf ))[ZB_PKT_16B_FIRST_BYTE] ) |= ( addr_mode ) << 6;
}

/**
   Sets dst addressing subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param addr_mode - 0 or 1.
*/
zb_void_t zb_fcf_set_dst_addressing_mode(zb_uint8_t *p_fcf, zb_uint8_t addr_mode)
{
  ZB_ASSERT( ( addr_mode ) == 0 ||
             ( addr_mode ) == 1 ||
             ( addr_mode ) == 2 ||
             ( addr_mode ) == 3    );
  ( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE]) ) &= 0xF3;
  ( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE]) ) |= ( addr_mode ) << 2;
}


void zb_mac_ack_timeout(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC2, "mac_ack_tmo", (FMT__0));
  ZB_MAC_SET_ACK_TIMEOUT();
}

void zb_mac_resp_timeout(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_mlme_associate_params_t *params = NULL;
  ZVUNUSED(param);
  params = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_associate_params_t);
  TRACE_MSG(TRACE_MAC2, "mac_resp_tmo", (FMT__0));
  {
    zb_mlme_data_req_params_t data_req_cmd_params;
    TRACE_MSG(TRACE_MAC3, "RESP_TIMEOUT", (FMT__0));
    data_req_cmd_params.src_addr_mode = ZB_ADDR_64BIT_DEV;
    ZB_MEMCPY(&data_req_cmd_params.src_addr, &MAC_PIB().mac_extended_address, sizeof(zb_ieee_addr_t));
    data_req_cmd_params.dst_addr_mode = params->coord_addr_mode;
    ZB_MEMCPY(&data_req_cmd_params.dst_addr, &params->coord_addr, sizeof(union zb_addr_u));
    data_req_cmd_params.cb_type = MAC_ASS_CONFIRM_CALLBACK;
    ZB_MAC_SET_ASS_REQUEST();
    /* We could avoid checks for tx etc here, 'cause association is a monolithic process */
    ret = zb_mac_get_indirect_data(&data_req_cmd_params);
  }
  if (ret != RET_OK)
  {
    call_indirect_data_callback(MAC_ASS_CONFIRM_CALLBACK,
                                (ZB_GET_MAC_STATUS() == MAC_SUCCESS) ? MAC_INVALID_PARAMETER : ZB_GET_MAC_STATUS(),
                                MAC_CTX().pending_buf);
  }
  else
  {
    TRACE_MSG(TRACE_MAC2, "free buf %p", (FMT__P, MAC_CTX().pending_buf));
    zb_free_buf(MAC_CTX().pending_buf);
    MAC_CTX().pending_buf = NULL;
  }
}

void zb_mac_scan_timeout(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC2, "mac_scan_tmo", (FMT__0));
  ZB_MAC_SET_SCAN_TIMEOUT();
}


void zb_mac_indirect_data_timeout(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_MAC2, "mac_indir_dt_tmo", (FMT__0));

  /* check if we have buffer */
  if ( MAC_CTX().rt_ctx.indirect_data.buf_ref )
  {
    call_indirect_data_callback((zb_callback_type_t)param, MAC_NO_DATA, ZB_BUF_FROM_REF(MAC_CTX().rt_ctx.indirect_data.buf_ref));
    MAC_CTX().rt_ctx.indirect_data.buf_ref = 0;
  }
  else
  {
    MAC_CTX().rt_ctx.indirect_data.cb_type = (zb_callback_type_t)param;
    MAC_CTX().rt_ctx.indirect_data.cb_status = MAC_NO_DATA;
    zb_get_out_buf_delayed(indirect_data_callback_caller);
  }
}

zb_ret_t zb_check_cmd_tx_status()
{
#if defined ZB_NS_BUILD
  return RET_OK;
#else
  zb_ret_t ret = RET_OK;
  ZB_WAIT_FOR_TX();
  if (ZB_TRANS_CHECK_CHANNEL_ERROR())
  {
    ZB_SET_MAC_STATUS(
      ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() ? MAC_CHANNEL_ACCESS_FAILURE : MAC_NO_ACK);
    TRACE_MSG(TRACE_MAC1, "error busy 0x%hx, tx retry exceeded 0x%hx, mac_status 0x%hx",
              (FMT__H_H,
               (zb_uint8_t)ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR(),
               (zb_uint8_t)ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR(),
               ZB_GET_MAC_STATUS()));
    ret = RET_ERROR;
  }
  TRACE_MSG(TRACE_MAC2, "cmd tx status %i", (FMT__D, ret));
  return ret;
#endif
}

/* returns true if frame is sent directly to current device */
static zb_bool_t check_frame_dst_addr(zb_mac_mhr_t *mhr) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_MAC3, "dst addr mode %i", (FMT__D, ZB_FCF_GET_DST_ADDRESSING_MODE(mhr->frame_control)));
  if (ZB_FCF_GET_DST_ADDRESSING_MODE(mhr->frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
  {
    TRACE_MSG(TRACE_MAC3, "mhr dst %i, pib short a %i", (FMT__D_D,
                                                         mhr->dst_addr.addr_short, MAC_PIB().mac_short_address));
    return (zb_bool_t)(mhr->dst_addr.addr_short == MAC_PIB().mac_short_address);
  }
  else
  {
    /* 64 bit case */
    TRACE_MSG(TRACE_MAC3, "mhr addr " TRACE_FORMAT_64 " pib ext" TRACE_FORMAT_64, (FMT__A_A,
                                                                                   TRACE_ARG_64(mhr->dst_addr.addr_long),
                                                                                   TRACE_ARG_64(MAC_PIB().mac_extended_address)));
    return ZB_64BIT_ADDR_CMP(mhr->dst_addr.addr_long, MAC_PIB().mac_extended_address);
  }
}


void zb_poll_request(zb_uint8_t param) ZB_CALLBACK
{
  if ( !param )
  {
    ZB_GET_OUT_BUF_DELAYED(zb_poll_request);
  }
  else
  {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_mlme_poll_request_t *req = ZB_GET_BUF_PARAM(buf, zb_mlme_poll_request_t);
    zb_uint16_t parent_addr;
    zb_address_short_by_ref(&parent_addr, ZG->nwk.handle.parent);
    req->coord_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    req->coord_addr.addr_short = parent_addr;
    req->coord_pan_id = ZB_PIB_SHORT_PAN_ID();
    MAC_CTX().mac_status = MAC_SUCCESS;
    ZB_SCHEDULE_TX_CB(zb_handle_poll_request, param);
  }
}

void zb_handle_poll_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_mlme_poll_request_t *request;
  zb_mlme_data_req_params_t data_req;
  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_MAC2, ">>h_poll_req", (FMT__0));

  request = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_poll_request_t);

  /* fill indirect data request cmd params */
  data_req.src_addr_mode = ( ZB_PIB_SHORT_ADDRESS() < ZB_MAC_SHORT_ADDR_NOT_ALLOCATED ) ? ZB_ADDR_16BIT_DEV_OR_BROADCAST : ZB_ADDR_64BIT_DEV;
  if ( data_req.src_addr_mode == ZB_ADDR_64BIT_DEV )
  {
    ZB_IEEE_ADDR_COPY(data_req.src_addr.addr_long, ZB_PIB_EXTENDED_ADDRESS());
  }
  else
  {
    data_req.src_addr.addr_short = ZB_PIB_SHORT_ADDRESS();
  }

  data_req.dst_addr_mode = request->coord_addr_mode;
  if ( data_req.dst_addr_mode == ZB_ADDR_64BIT_DEV )
  {
    ZB_IEEE_ADDR_COPY(data_req.dst_addr.addr_long, request->coord_addr.addr_long);

    TRACE_MSG(TRACE_MAC3, "poll_request: dst_addr (long) " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(data_req.dst_addr.addr_long)));
  }
  else
  {
    data_req.dst_addr.addr_short = request->coord_addr.addr_short;
  }
  data_req.cb_type = MAC_POLL_REQUEST_CALLBACK;

  /* get indirect data. Synchronous for ZED which, usually, uses poll. */
  if ((MAC_CTX().tx_wait_cb)&&(!MAC_CTX().tx_cnt))
  {
    ZB_WAIT_FOR_TX();
  }
  ret = zb_mac_get_indirect_data(&data_req);
  {
    if ( ret == RET_OK )
    {
      ZB_MAC_SET_POLL_REQUEST();
      /* save buffer to return confirm */
      MAC_CTX().rt_ctx.indirect_data.buf_ref = ZB_REF_FROM_BUF(MAC_CTX().pending_buf);
    }
    else
    {
      MAC_CTX().pending_buf->u.hdr.status = ( MAC_CTX().mac_status == MAC_SUCCESS ) ? MAC_INVALID_PARAMETER : MAC_CTX().mac_status;
      ZB_SCHEDULE_CALLBACK(zb_mlme_poll_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
    }
    MAC_CTX().pending_buf = NULL;
  }

  TRACE_MSG(TRACE_MAC2, "<<h_poll_req %hd", (FMT__H, ret));
}

#ifndef ZB_LIMITED_FEATURES

void zb_mlme_get_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_mac_status_t status = MAC_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_request_t req;
  zb_mlme_get_confirm_t *conf = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_mlme_get_req %d", (FMT__D, param));
  ZB_MEMCPY(&req, (zb_mlme_get_request_t *)ZB_BUF_BEGIN(buf), sizeof(req));

  if (req.pib_attr == ZB_PHY_PIB_CURRENT_CHANNEL)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().phy_current_channel;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PHY_PIB_CURRENT_PAGE)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().phy_current_page;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_ACK_WAIT_DURATION)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_PIB_ACK_WAIT_DURATION;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_ASSOCIATION_PERMIT)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_association_permit;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_BATT_LIFE_EXT)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_batt_life_ext;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(ZB_PIB_BEACON_PAYLOAD()), conf);
    conf->pib_length = sizeof(ZB_PIB_BEACON_PAYLOAD());
    ZB_MEMCPY((((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)),
              &ZB_PIB_BEACON_PAYLOAD(), sizeof(zb_mac_beacon_payload_t));
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_beacon_payload_length;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_BEACON_ORDER)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_beacon_order;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_BSN)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_BSN();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_COORD_SHORT_ADDRESS)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_PIB_COORD_SHORT_ADDRESS();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_DSN)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_DSN();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_PANID)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_PIB_SHORT_PAN_ID();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_RX_ON_WHEN_IDLE)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_PIB_RX_ON_WHEN_IDLE();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_SHORT_ADDRESS)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_PIB_SHORT_ADDRESS();
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_SUPER_FRAME_ORDER)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_superframe_order;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_TRANSACTION_PERSISTENCE_TIME)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint16_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_PIB_TRANSACTION_PERSISTENCE_TIME;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_ASSOCIATED_PAN_COORD)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t), conf);
    status = MAC_UNSUPPORTED_ATTRIBUTE;
    conf->pib_length = 0;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_MAX_FRAME_TOTAL_WAIT_TIME)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->pib_length = sizeof(zb_uint16_t);
    *(((zb_uint16_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_MAX_FRAME_RETRIES)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_MAX_FRAME_RETRIES;
    conf->pib_index = 0;
  }
  else if (req.pib_attr == ZB_PIB_ATTRIBUTE_RESPONSE_WAIT_TIME)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = ZB_MAC_PIB_RESPONSE_WAIT_TIME;
    conf->pib_index = 0;
  }
  else
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t), conf);
    status = MAC_UNSUPPORTED_ATTRIBUTE;
    conf->pib_length = 0;
  }
  conf->pib_attr = req.pib_attr;
  conf->status = status;
  ZB_SCHEDULE_CALLBACK(zb_mlme_get_confirm, param);
  TRACE_MSG(TRACE_APS2, "<<zb_mlme_get_req %d", (FMT__D, param));
}

/* FIXME: zb_mlme_get_confirm must be somewhere in the upper layer. */
void zb_mlme_get_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_confirm_t *conf = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_mlme_get_confirm %d", (FMT__D, param));
  conf = ( zb_mlme_get_confirm_t *)ZB_BUF_BEGIN(buf);

  if ((conf->pib_attr == ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH)
      || (conf->pib_attr == ZB_PIB_ATTRIBUTE_BEACON_ORDER)
      || (conf->pib_attr == ZB_PIB_ATTRIBUTE_BSN))
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->pib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t)) = MAC_PIB().mac_beacon_payload_length;
    conf->pib_index = 0;
    TRACE_MSG(TRACE_NWK2, "attr %hd get param %hd", (FMT__H_H, conf->pib_attr, *(((zb_uint8_t *)conf) + sizeof(zb_mlme_get_confirm_t))));
  }

  TRACE_MSG(TRACE_APS2, "<<zb_mlme_get_confirm status %hd", (FMT__H, conf->status));
  zb_free_buf(buf);
}

void zb_mlme_set_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_mac_status_t status = MAC_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  zb_mlme_set_request_t *req = (zb_mlme_set_request_t *)ptr;
  zb_mlme_set_confirm_t conf;
  zb_mlme_set_confirm_t *conf_p = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_mlme_set_req %d", (FMT__D, param));

  conf.status = status;
  conf.pib_attr = req->pib_attr;
  conf.pib_index = 0;
  ptr += sizeof(zb_mlme_set_request_t);
  if (req->pib_attr == ZB_PHY_PIB_CURRENT_CHANNEL)
  {
    MAC_PIB().phy_current_channel = *ptr;
    ZB_TRANSCEIVER_SET_CHANNEL(MAC_PIB().phy_current_channel);
  }
  else if (req->pib_attr == ZB_PHY_PIB_CURRENT_PAGE)
  {
    MAC_PIB().phy_current_page = *ptr;
  }
#ifdef ZB_CONFIGURABLE_MAC_PIB
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_ACK_WAIT_DURATION)
  {
    MAC_PIB().mac_ack_wait_duration = *((zb_uint16_t *)ptr);
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_TRANSACTION_PERSISTENCE_TIME)
  {
    MAC_PIB().mac_transaction_persistence_time = *((zb_uint16_t*)ptr);
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_MAX_FRAME_TOTAL_WAIT_TIME)
  {
    MAC_PIB().mac_max_frame_total_wait_time = *((zb_uint16_t *)ptr);
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_MAX_FRAME_RETRIES)
  {
    MAC_PIB().mac_max_frame_retries = *ptr;
    /* not necessary to update. max_frame_retries will be used during packet send */
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_RESPONSE_WAIT_TIME)
  {
    MAC_PIB().mac_response_wait_time = *ptr;
  }
#endif
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_ASSOCIATION_PERMIT)
  {
    MAC_PIB().mac_association_permit = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_BATT_LIFE_EXT)
  {
    MAC_PIB().mac_batt_life_ext = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD)
  {
    ZB_MEMCPY(&ZB_PIB_BEACON_PAYLOAD(), ptr, sizeof(zb_mac_beacon_payload_t));
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH)
  {
    MAC_PIB().mac_beacon_payload_length = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_BEACON_ORDER)
  {
    MAC_PIB().mac_beacon_order = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_BSN)
  {
    ZB_MAC_BSN() = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_COORD_SHORT_ADDRESS)
  {
    ZB_PIB_COORD_SHORT_ADDRESS() = *((zb_uint16_t *)ptr);
    ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(ZB_PIB_COORD_SHORT_ADDRESS());
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_COORD_EXTEND_ADDRESS )
  {
    ZB_EXTPANID_COPY(ZG->mac.pib.mac_coord_extended_address, (zb_uint8_t *)ptr);
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_DSN)
  {
    ZB_MAC_DSN() = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_PANID)
  {
    ZB_PIB_SHORT_PAN_ID() = *((zb_uint16_t *)ptr);
    ZB_UPDATE_PAN_ID();
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_RX_ON_WHEN_IDLE)
  {
    ZB_PIB_RX_ON_WHEN_IDLE() = *ptr;
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_SHORT_ADDRESS)
  {
    TRACE_MSG(TRACE_APS3, "set SHORT_ADDRESS %x", (FMT__D, *((zb_uint16_t *)ptr)));
    ZB_PIB_SHORT_ADDRESS() = *((zb_uint16_t *)ptr);
    ZB_UPDATE_SHORT_ADDR();
  }
  else if (req->pib_attr == ZB_PIB_ATTRIBUTE_SUPER_FRAME_ORDER)
  {
    MAC_PIB().mac_superframe_order = *((zb_uint16_t*)ptr);
  }
  else
  {
    status = MAC_UNSUPPORTED_ATTRIBUTE;
  }
  conf.status = status;
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_set_confirm_t), conf_p);
  ZB_MEMCPY(conf_p, &conf, sizeof(*conf_p));
  ZB_SCHEDULE_CALLBACK(zb_mlme_set_confirm, param);
  TRACE_MSG(TRACE_APS2, "<<zb_mlme_set_req %d", (FMT__D, param));
}

#endif  /* ZB_LIMITED_FEATURES */

/*
  void mac_dump_states()
  {
  TRACE_MSG(TRACE_MAC2, "mac st 0 (layer 1) %hd", (FMT__H, MAC_STATE_FOR_LAYER(ZB_MAC_LAYER_1)));
  TRACE_MSG(TRACE_MAC2, "mac st 1 (layer 2) %hd", (FMT__H, MAC_STATE_FOR_LAYER(ZB_MAC_LAYER_2)));
  TRACE_MSG(TRACE_MAC2, "mac st 2 (layer 3) %hd", (FMT__H, MAC_STATE_FOR_LAYER(ZB_MAC_LAYER_3)));
  TRACE_MSG(TRACE_MAC2, "mac st 3 (rx) %hd", (FMT__H, MAC_STATE_FOR_LAYER(ZB_MAC_IO_LAYER_RX)));
  TRACE_MSG(TRACE_MAC2, "mac st 4 (tx) %hd", (FMT__H, MAC_STATE_FOR_LAYER(ZB_MAC_IO_LAYER_TX)));
  }
*/
#ifdef ZB_LIMIT_VISIBILITY
static zb_bool_t mac_is_frame_visible(zb_mac_mhr_t *mhr)
{
  zb_ushort_t i;

  /*
    Invisible short addresses used mainly to limit beacons visibility, when long
    address is not available yet.
  */

  if (ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
  {
    for (i = 0 ; i < MAC_CTX().n_invisible_short_addr ; ++i)
    {
      if (mhr->src_addr.addr_short == MAC_CTX().invisible_short_addreesses[i])
      {
        return ZB_FALSE;
      }
    }
  }

  if (MAC_CTX().n_visible_addr == 0
      || ZB_FCF_GET_FRAME_TYPE(mhr->frame_control) == MAC_FRAME_BEACON)
  {
    /* visibility control is off */
    return ZB_TRUE;
  }

  switch (ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control))
  {
    case ZB_ADDR_16BIT_DEV_OR_BROADCAST:
    {
      for (i = 0 ; i < MAC_CTX().n_visible_addr ; ++i)
      {
        zb_ieee_addr_t long_addr;

        if (zb_address_ieee_by_short(mhr->src_addr.addr_short, long_addr) == RET_OK
            && ZB_IEEE_ADDR_CMP(long_addr, MAC_CTX().visible_addresses[i]))
        {
          TRACE_MSG(TRACE_MAC2, "added visible dev " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(long_addr)));
          return ZB_TRUE;
        }
      }
      break;
    }
    case ZB_ADDR_64BIT_DEV:
      for (i = 0 ; i < MAC_CTX().n_visible_addr ; ++i)
      {
        if (ZB_IEEE_ADDR_CMP(mhr->src_addr.addr_long, MAC_CTX().visible_addresses[i]))
        {
          return ZB_TRUE;
        }
      }
      break;
    default:
      return ZB_TRUE;
  }

  TRACE_MSG(TRACE_COMMON1, "dropped frame from %hd " TRACE_FORMAT_64,
            (FMT__H_A, ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control), TRACE_ARG_64(mhr->src_addr.addr_long)));

  return ZB_FALSE;
}


void mac_add_visible_device(zb_ieee_addr_t long_addr)
{
  ZB_IEEE_ADDR_COPY(MAC_CTX().visible_addresses[MAC_CTX().n_visible_addr], long_addr);

  TRACE_MSG(TRACE_MAC2, "added visible dev " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(long_addr)));
  MAC_CTX().n_visible_addr++;
}


void mac_add_invisible_short(zb_uint16_t addr)
{
  MAC_CTX().invisible_short_addreesses[MAC_CTX().n_invisible_short_addr] = addr;

  TRACE_MSG(TRACE_MAC2, "added invisible dev 0x%x", (FMT__D, addr));
  MAC_CTX().n_invisible_short_addr++;
}

void zb_dummy_cb(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
}
#endif


/*! @} */
