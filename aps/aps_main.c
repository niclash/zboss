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
PURPOSE: APS layer
*/

#include "zb_common.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_secur.h"
#include "aps_internal.h"
#include "zb_af_globals.h"
#include "zb_zdo.h"

/*! \addtogroup ZB_APS */
/*! @{ */

#include "zb_bank_7.h"

/**
   APS Informational Base in-memory data structure.

   Access to this structure using macros.
 */

static void aps_data_hdr_fill_datareq(zb_uint8_t fc, zb_apsde_data_req_t *req, zb_buf_t *apsdu) ZB_SDCC_REENTRANT;
static void aps_ack_send_handle(zb_buf_t *packet, zb_aps_hdr_t *aps_hdr) ZB_SDCC_REENTRANT;
static zb_uint8_t save_ack_data(zb_uint8_t param, zb_apsde_data_req_t *req, zb_uint8_t *ref) ZB_SDCC_REENTRANT;
static void aps_ack_frame_handle(zb_aps_hdr_t *aps_hdr) ZB_SDCC_REENTRANT;
#ifndef ZB_LIMITED_FEATURES
static zb_ret_t zb_search_dst_in_binding_table(zb_apsde_data_req_t *apsreq, zb_address_ieee_ref_t *dst_ref) ZB_SDCC_REENTRANT;
#endif
static void done_with_this_ack(zb_ushort_t i, zb_uint8_t fc, zb_uint8_t status) ZB_SDCC_REENTRANT;
static void fill_nldereq(zb_uint8_t param, zb_uint16_t addr, zb_uint8_t secure) ZB_SDCC_REENTRANT;

void zb_aps_ack_timer_cb(zb_uint8_t param) ZB_CALLBACK;
void zb_aps_pass_group_msg_up(zb_uint8_t param) ZB_CALLBACK;
void zb_aps_pass_local_group_pkt_up(zb_uint8_t param) ZB_CALLBACK;
void zb_aps_pass_up_group_buf(zb_uint8_t param)  ZB_CALLBACK;
void zb_aps_send_ack_and_continue(zb_uint8_t param) ZB_CALLBACK;
void zb_nlde_data_indication_continue(zb_uint8_t param) ZB_CALLBACK;


#define DUMP_TRAF(cmt, buf, len)



void zb_aps_init()
{
  zb_ushort_t i;

  TRACE_MSG(TRACE_APS1, "+zb_aps_init", (FMT__0));
  /* Initialize APSIB */
  ZG->aps.aib.aps_counter = ZB_RANDOM();
#ifdef APS_RETRANSMIT_TEST
  ZG->aps.retrans.counter = 0;
#endif

  for (i = 0 ; i < ZB_N_APS_RETRANS_ENTRIES ; ++i)
  {
    ZG->aps.retrans.hash[i].addr = (zb_uint16_t)-1;
    ZG->aps.retrans.hash[i].buf = (zb_uint8_t)-1;
  }
  ZG->aps.retrans.ack_buf = (zb_uint8_t)-1;


  TRACE_MSG(TRACE_APS1, "-zb_aps_init", (FMT__0));
}

#ifndef ZB_LIMITED_FEATURES
static zb_ret_t zb_search_dst_in_binding_table(zb_apsde_data_req_t *apsreq, zb_address_ieee_ref_t *dst_ref) ZB_SDCC_REENTRANT
{
  zb_uint8_t ind_src;
  zb_address_ieee_ref_t addr_ref = 0;
  zb_ret_t ret;
  zb_uint8_t i = 0;

  ret = zb_address_by_short(ZB_PIB_SHORT_ADDRESS(), ZB_FALSE, ZB_FALSE, &addr_ref);
  TRACE_MSG(TRACE_APS3, "address_by_short ret %d, short addr %d", (FMT__D_D, ret, ZB_PIB_SHORT_ADDRESS()));
  if (ret == RET_OK)
  {
    ind_src = aps_find_src_ref(addr_ref, apsreq->src_endpoint, apsreq->clusterid);
    TRACE_MSG(TRACE_APS3, "ind_src %hd, src ep %hd, clusterid %d",
              (FMT__H_H_D, ind_src, apsreq->src_endpoint, apsreq->clusterid));

    ret = RET_NO_BOUND_DEVICE;
    if (ind_src != (zb_uint8_t)-1)
    {
      i = 0;
      do
      {
        if ( ZG->aps.binding.dst_table[i].src_table_index == ind_src )
        {
          if ( ZG->aps.binding.dst_table[i].dst_addr_mode == ZB_APS_BIND_DST_ADDR_LONG )
          {
            *dst_ref = ZG->aps.binding.dst_table[i].u.long_addr.dst_addr;
            apsreq->dst_endpoint = ZG->aps.binding.dst_table[i].u.long_addr.dst_end;
            ret = RET_OK;
          }
          else
          {
            /* FIXME: should we implement group addressing here? */
          }
          break;
        }
        i++;
      } while (i < ZG->aps.binding.dst_n_elements);
    }
  }
  else
  {
    ret = RET_NO_BOUND_DEVICE;
  }
  return ret;
}
#endif

void zb_apsde_data_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t fc = 0;
  zb_nlde_data_req_t nldereq;
  zb_buf_t *apsdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *apsreq = ZB_GET_BUF_TAIL(apsdu, sizeof(zb_apsde_data_req_t));

  TRACE_MSG(TRACE_APS1, "+apsde_data_req %hd", (FMT__H, param));
/*
  That macros are implied (set values to 0):
  ZB_APS_FC_SET_FRAME_TYPE(fc, ZB_APS_FRAME_DATA);
  ZB_APS_FC_SET_ACK_FORMAT(fc, 0);
  ZB_APS_FC_SET_EXT_HDR_PRESENT(fc, 0);
*/
  nldereq.radius = apsreq->radius;
  nldereq.addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
  nldereq.nonmember_radius = 0; /* if multicast, get it from APS IB */
  nldereq.discovery_route = 1;  /* always! see 2.2.4.1.1.3 */
  /* use NWK security only if not use APS security */

  switch ((int)apsreq->addr_mode)
  {
#ifndef ZB_LIMITED_FEATURES
    case ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT:
      {
        zb_address_ieee_ref_t dst_ref = 0;
        zb_ret_t ret = RET_OK;

        /* FIXME: implement group addressing */
        ret = zb_search_dst_in_binding_table(apsreq, &dst_ref);
        if (ret == RET_OK)
        {
          ZB_APS_FC_SET_DELIVERY_MODE(fc, ZB_APS_DELIVERY_UNICAST);
          zb_address_short_by_ref(&nldereq.dst_addr, dst_ref);
          TRACE_MSG(TRACE_APS3, "apsde_data unicast, dst %d", (FMT__D, nldereq.dst_addr));

          if (nldereq.dst_addr == ZB_UNKNOWN_SHORT_ADDR)
          {
            /* TODO: we can fall into infinite loop here, asking for peer short address, fix it!!! */
            zb_start_get_peer_short_addr(dst_ref, zb_apsde_data_request, param);
            TRACE_MSG(TRACE_APS1, "-apsde_data_req, wait for peer addr", (FMT__0));
            return;
          }

          apsreq->dst_addr.addr_short = nldereq.dst_addr;
        }
        else
        {
          TRACE_MSG(TRACE_APS3, "dst addr and endpiont not present in binding table", (FMT__0));
          ZB_SCHEDULE_CALLBACK(zb_apsde_data_confirm, param);
          return;
        }
      }
      break;

    case ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT:
      /* group addressing - NWK broadcast to 0xfffd */
      nldereq.dst_addr = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
      apsreq->tx_options &= (zb_uint8_t)(~(zb_uint16_t)ZB_APSDE_TX_OPT_ACK_TX);
      ZB_APS_FC_SET_DELIVERY_MODE(fc, ZB_APS_DELIVERY_GROUP);
      TRACE_MSG(TRACE_APS3, "apsde_data group, group %d, options 0x%hx", (FMT__D_H, apsreq->dst_addr.addr_short, apsreq->tx_options));
      /* no dest endp */
      break;
#endif /* ZB_LIMITED_FEATURES */

    case ZB_APS_ADDR_MODE_16_ENDP_PRESENT:
      /* unicast - address is known*/

      if (apsreq->dst_addr.addr_short == ZB_NWK_BROADCAST_ALL_DEVICES ||
          apsreq->dst_addr.addr_short == ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE ||
          apsreq->dst_addr.addr_short == ZB_NWK_BROADCAST_ROUTER_COORDINATOR ||
          apsreq->dst_addr.addr_short == ZB_NWK_BROADCAST_LOW_POWER_ROUTER)
      {
        ZB_APS_FC_SET_DELIVERY_MODE(fc, ZB_APS_DELIVERY_BROADCAST);
      }
      else
      {
        ZB_APS_FC_SET_DELIVERY_MODE(fc, ZB_APS_DELIVERY_UNICAST);
      }
      nldereq.dst_addr = apsreq->dst_addr.addr_short;
      TRACE_MSG(TRACE_APS3, "apsde_data unicast, dst %d, options 0x%hx", (FMT__D_H, nldereq.dst_addr, apsreq->tx_options));
      break;
    case ZB_APS_ADDR_MODE_64_ENDP_PRESENT:
      ZB_APS_FC_SET_DELIVERY_MODE(fc, ZB_APS_DELIVERY_UNICAST);
      /* convert long (64) to short (16) address, then unicast */
      nldereq.dst_addr = zb_address_short_by_ieee(apsreq->dst_addr.addr_long);
      apsreq->dst_addr.addr_short = nldereq.dst_addr;
      TRACE_MSG(TRACE_APS3, "apsde_data unicast, dst %d, options 0x%hx", (FMT__D_H, nldereq.dst_addr, apsreq->tx_options));
      break;
    default:
      TRACE_MSG(TRACE_ERROR, "strange addr mode %d", (FMT__D, apsreq->addr_mode));
      break;
  }

  ZB_APS_FC_SET_ACK_FORMAT(fc, 0);
#ifndef ZB_LIMITED_FEATURES2
  if (apsreq->tx_options & ZB_APSDE_TX_OPT_ACK_TX)
  {
    zb_apsde_data_req_t req;
    zb_uint8_t ret = 0;
    zb_uint8_t ref = 0;

    ZB_MEMCPY(&req, apsreq, sizeof(zb_apsde_data_req_t));
    ZB_APS_FC_SET_ACK_REQUEST(fc, 1);
    aps_data_hdr_fill_datareq(fc, apsreq, apsdu);

    ret = save_ack_data(param, &req, &ref);
    if (ret != RET_OK)
    {
      apsdu->u.hdr.status = ret;
      ZB_SCHEDULE_CALLBACK(zb_apsde_data_confirm, param);
      return;
    }
    /* Optimize traffic for ZED: if wait for ACK, send POLL soon to be able to
     * retrive ACK. */
    zb_zdo_reschedule_poll_parent(ZB_APS_POLL_AFTER_REQ_TMO);
    ZB_SCHEDULE_ALARM(zb_aps_ack_timer_cb, ref, ZB_N_APS_ACK_WAIT_DURATION);
  }
  else
#endif
  {
    aps_data_hdr_fill_datareq(fc, apsreq, apsdu);
  }
  ZB_CHK_ARR(ZB_BUF_BEGIN(apsdu), 8); /* check hdr fill */

  /* If not secured at APS layer, enable secure at NWK layer */
  nldereq.security_enable = !apsdu->u.hdr.encrypt_type;
  nldereq.ndsu_handle = 0;

  ZB_MEMCPY(
    ZB_GET_BUF_TAIL(apsdu, sizeof(zb_nlde_data_req_t)),
    &nldereq, sizeof(nldereq));

#ifndef ZB_LIMITED_FEATURES
  /* Check for our group table and transmit buffer to us locally */
  if (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_GROUP
      && ZG->aps.group.n_groups > 0
      && !ZB_RING_BUFFER_IS_FULL(&ZG->aps.group.local_dup_q)
      && !ZB_RING_BUFFER_IS_FULL(&ZG->aps.group.pass_up_q))
  {
    /* Must send this packet to myself as well */
    ZB_RING_BUFFER_PUT(&ZG->aps.group.local_dup_q, param);
    zb_get_in_buf_delayed(zb_aps_pass_local_group_pkt_up);
  }
  else
#endif
  {
    /* Now send (pass to NWK). If address got from the bind, send more then once (?).  */
    ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, param);
  }

  TRACE_MSG(TRACE_APS1, "-apsde_data_req", (FMT__0));
}


#ifndef ZB_LIMITED_FEATURES
void zb_aps_pass_local_group_pkt_up(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t out_param;
  zb_aps_hdr_t *aps_hdr;

  if (!ZB_RING_BUFFER_IS_EMPTY(&ZG->aps.group.local_dup_q))
  {
    out_param = *ZB_RING_BUFFER_GET(&ZG->aps.group.local_dup_q);
    ZB_RING_BUFFER_FLUSH_GET(&ZG->aps.group.local_dup_q);
    ZB_BUF_COPY(buf, ZB_BUF_FROM_REF(out_param));
    /* send original frame to nwk */
    ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, out_param);

    /* Fill parsed aps hdr. Note that no nwk hdr in the buffer. */
    aps_hdr = ZB_GET_BUF_PARAM(buf, zb_aps_hdr_t);
    zb_aps_hdr_parse(buf, aps_hdr, ZB_FALSE);
    /* this packet is from me to me */
    aps_hdr->src_addr = ZB_NIB_NETWORK_ADDRESS();

    ZB_SCHEDULE_CALLBACK(zb_aps_pass_up_group_buf, param);
  }
  else
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
    ZB_ASSERT(0);
  }
}


void zb_aps_pass_up_group_buf(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (ZB_RING_BUFFER_IS_FULL(&ZG->aps.group.pass_up_q))
  {
    TRACE_MSG(TRACE_APS3, "no space in the queue - drop this packet", (FMT__0));
    zb_free_buf(buf);
  }
  else
  {
    /* Use handle as iterator in the group table */
    buf->u.hdr.handle = 0;

    if (!ZG->aps.group.active_pass_up_buf)
    {
      /* If queue is empty, group data passing up is not schedules */
      TRACE_MSG(TRACE_APS3, "schedule pass up group message", (FMT__0));
      ZG->aps.group.active_pass_up_buf = buf;
      zb_get_in_buf_delayed(zb_aps_pass_group_msg_up);
    }
    else
    {
      /* Passing up is scheduled (via schedule or delayed alloc) - just
       * add buffer to the queue */
      ZB_RING_BUFFER_PUT(&ZG->aps.group.pass_up_q, param);
      TRACE_MSG(TRACE_APS3, "pass up group message is in progress - add to q", (FMT__0));
    }
  }
}
#endif


/**
   Fill APS header in the packe for data request.

   @param fc - FC field (created at upper layer)
   @param req - APS data request
 */
static void aps_data_hdr_fill_datareq(zb_uint8_t fc, zb_apsde_data_req_t *req, zb_buf_t *apsdu) ZB_SDCC_REENTRANT
{
  zb_uint8_t *aps_hdr;
  zb_uint8_t is_group = (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_GROUP);
  zb_short_t aps_hdr_size =
      1              /* fc */
    + 1              /* dst_endpoint (if not group addressing) */
    + 2              /* cluster id*/
    + 2              /* profile id */
    + 1              /* src_endpoint */
    + 1;             /* APS counter */

  /* If group addressing, dst_endpoint is absent but instead of it 2-bytes
   * Group address exists */
  aps_hdr_size += is_group;

#ifdef ZB_SECURITY
  apsdu->u.hdr.encrypt_type = ZB_SECUR_NO_ENCR;
#ifdef APS_FRAME_SECURITY
  if (req->tx_options & ZB_APSDE_TX_OPT_SECURITY_ENABLED)
  {
    /* Need to secure this frame. Must decide which key use: NWK or  */
    if (req->tx_options & ZB_APSDE_TX_OPT_USE_NWK_KEY)
    {
      if (!ZG->nwk.nib.secure_all_frames)
      {
        TRACE_MSG(TRACE_SECUR3, "Secure APS by NWK key", (FMT__0));
        aps_hdr_size += sizeof(zb_aps_nwk_aux_frame_hdr_t);
        ZB_APS_FC_SET_SECURITY(fc, 1);
        apsdu->u.hdr.encrypt_type = ZB_SECUR_APS_ENCR;
      }
    }
    else
    {
      TRACE_MSG(TRACE_SECUR1, "Secure APS by app key: TODO - implment!", (FMT__0));
      aps_hdr_size += sizeof(zb_aps_data_aux_frame_hdr_t);
      ZB_APS_FC_SET_SECURITY(fc, 1);
      apsdu->u.hdr.encrypt_type = ZB_SECUR_APS_ENCR;
      ZB_ASSERT(0);
    }
  }
#endif
#endif

  /* TODO: handle fragmentation and Extended header. Now suppose no Extended header */
  ZB_BUF_ALLOC_LEFT(apsdu, aps_hdr_size, aps_hdr);

  *aps_hdr++ = fc;

  /* Assume always ZB_APS_FC_GET_ACK_FORMAT(fc) == 0: command can't be here! */
  ZB_ASSERT(ZB_APS_FC_GET_ACK_FORMAT(fc) == 0);

  /* If Group addressing, no dest endpoint but have Group address */
  if (is_group)
  {
    zb_put_next_htole16(&aps_hdr, req->dst_addr.addr_short);
  }
  else
  {
    *aps_hdr++ = req->dst_endpoint;
  }

  /*
    If data or ack, has cluster and profile id.
    Command can't be here.
   */
  zb_put_next_htole16(&aps_hdr, req->clusterid);
  zb_put_next_htole16(&aps_hdr, req->profileid);
  *aps_hdr++ = req->src_endpoint;
  *aps_hdr++ = ZB_AIB_APS_COUNTER();
  ZB_AIB_APS_COUNTER_INC();

#ifdef ZB_SECURITY
#ifdef APS_FRAME_SECURITY
  if (ZB_APS_FC_GET_SECURITY(fc))
  {
    /* Anytime APS retransmitted APS header must use same counter - see
     * 2.2.8.4.4.
     * So, fill aux header now. Payload will be encrypted layer, in MAC
     */
    zb_secur_aps_aux_hdr_fill(aps_hdr, (req->tx_options & ZB_APSDE_TX_OPT_USE_NWK_KEY));
  }
#endif
#endif
}


zb_ushort_t zb_aps_full_hdr_size(zb_uint8_t *pkt)
{
  zb_ushort_t size = ZB_APS_HDR_SIZE(*pkt);

#ifdef ZB_SECURITY
#ifdef APS_FRAME_SECURITY
  if (ZB_APS_FC_GET_SECURITY(*pkt))
  {
    if (ZB_SECUR_AUX_HDR_GET_KEY_TYPE(((zb_aps_nwk_aux_frame_hdr_t *)&pkt[size])->secur_control) == ZB_SECUR_DATA_KEY)
    {
      size += sizeof(zb_aps_data_aux_frame_hdr_t);
    }
    else
    {
      size += sizeof(zb_aps_nwk_aux_frame_hdr_t);
    }
  }
#endif
#endif

  return size;
}


void zb_nlde_data_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *nsdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *fc_p;

  TRACE_MSG(TRACE_APS2, "+zb_nlde_data_confirm %hd status %hd", (FMT__H_H, param, nsdu->u.hdr.status));

  {
    zb_nwk_hdr_t *nwk_hdr;
    ZB_MAC_CUT_HDR_WITHOUT_TRAILER(nsdu, nwk_hdr);
    ZVUNUSED(nwk_hdr);
  }
  ZB_NWK_HDR_CUT(nsdu, fc_p);

  if (ZB_APS_FC_GET_FRAME_TYPE(*fc_p) == ZB_APS_FRAME_ACK)
  {
    /* This is APS ACK - free it */
    zb_free_buf(nsdu);
  }
  else
    /* For sdcc only imitate APS ACKs: send ACK but pass confirm up even before
     * ACk requested. We need more code space!!! */
#ifndef ZB_LIMITED_FEATURES2
    if (!ZB_APS_FC_GET_ACK_REQUEST(*fc_p))
#endif
  {
    /* Not need to wait for APS ACK - pass confirm up, to AF */
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_confirm, param);
  }
#ifndef ZB_LIMITED_FEATURES2
  else
  {
    zb_ushort_t i;

    /* This is confirm to APS packet that must be acked, or acked already */
    for ( i = 0 ; i < ZB_N_APS_RETRANS_ENTRIES ; ++i)
    {
      if (ZG->aps.retrans.hash[i].buf == param)
      {
        if (ZG->aps.retrans.hash[i].state == ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_APS_ACKED_ALRM_RUNNING)
        {
          /* That packet ACKed while we were sending retransmit. */
          TRACE_MSG(TRACE_APS2, "APS ACK OK, retransmit of %hd confirmed", (FMT__H, param));
          done_with_this_ack(i, *fc_p, RET_OK);
        }
        else
        {
          ZB_ASSERT(ZG->aps.retrans.hash[i].state == ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING);
          TRACE_MSG(TRACE_APS2, "packet %hd len %hd confirm state %hd, set st %hd, still wait for ack",
                    (FMT__H_H_H_H, param, ZB_BUF_LEN(nsdu), ZG->aps.retrans.hash[i].state, ZB_APS_RETRANS_ENT_SENT_MAC_CONFIRMED_ALRM_RUNNING));
          ZG->aps.retrans.hash[i].state = ZB_APS_RETRANS_ENT_SENT_MAC_CONFIRMED_ALRM_RUNNING;
          /* will call upper-layer confirm at ack receive, or retransmit this
           * buffer, so can't free it now */
        }
        break;
      }
    }
    if (i == ZB_N_APS_RETRANS_ENTRIES)
    {
      /* not found (hmm...) - free it */
      TRACE_MSG(TRACE_APS2, "strange buf %hd - free it", (FMT__H, param));
      zb_free_buf(nsdu);
    }
  }
#endif

  TRACE_MSG(TRACE_APS2, "-nlde_data_conf", (FMT__0));
}

/*
   This is APS handler for nlde-data.indication - receive path entry point
*/
void zb_nlde_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *packet = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t fc;
  {
    zb_nwk_hdr_t *nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(packet);
    fc = *(ZB_BUF_BEGIN(packet) + ZB_NWK_HDR_SIZE(nwk_hdr->frame_control));
  }

  /* Result of the headers parse */

  TRACE_MSG(TRACE_APS1, "+nlde_data_ind %hd", (FMT__H, param));

  /* Parse NWK header */
  /* get source address from the NWK header */

#ifndef ZB_LIMITED_FEATURES
  /* Detect ACK packet */
  if (ZB_APS_FC_GET_EXT_HDR_PRESEBT(fc))
  {
    /* TODO: handle fragmentation */
    TRACE_MSG(TRACE_ERROR, "Ext hdr - not implemented yet!", (FMT__0));
  }
#endif
  if (ZB_APS_FC_GET_FRAME_TYPE(fc) == ZB_APS_FRAME_ACK)
  {
    /* if all-all limited, just drop ACK packet */
#ifndef ZB_LIMITED_FEATURES2
    zb_aps_hdr_t aps_hdr;
    zb_aps_hdr_parse(packet, &aps_hdr, ZB_TRUE);
    aps_ack_frame_handle(&aps_hdr);
#endif
    zb_free_buf(packet);
  }
  else
  {
    if (ZB_APS_FC_GET_ACK_REQUEST(fc)
        && !ZB_RING_BUFFER_IS_FULL(&ZG->aps.retrans.ack_q))
    {
      ZB_RING_BUFFER_PUT(&ZG->aps.retrans.ack_q, param);
      zb_get_out_buf_delayed(zb_aps_send_ack_and_continue);
    }
    else
    {
      zb_nlde_data_indication_continue(param);
    }
  }
}


void zb_aps_send_ack_and_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t in_pkt = *ZB_RING_BUFFER_GET(&ZG->aps.retrans.ack_q);
  ZB_RING_BUFFER_FLUSH_GET(&ZG->aps.retrans.ack_q);
  ZG->aps.retrans.ack_buf = param;
  zb_nlde_data_indication_continue(in_pkt);
}


void zb_nlde_data_indication_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *packet = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_aps_hdr_t aps_hdr;

  TRACE_MSG(TRACE_APS1, "+zb_nlde_data_indication_continue %hd", (FMT__H, param));

  zb_aps_hdr_parse(packet, &aps_hdr, ZB_TRUE);

  if (ZG->aps.retrans.ack_buf != (zb_uint8_t)-1)
  {
    aps_ack_send_handle(ZB_BUF_FROM_REF(ZG->aps.retrans.ack_buf), &aps_hdr);
    ZG->aps.retrans.ack_buf = (zb_uint8_t)-1;
  }

  /* Detect and reject dup */
#ifndef ZB_LIMITED_FEATURES
  if (aps_check_dups(aps_hdr.src_addr, aps_hdr.aps_counter))
  {
    TRACE_MSG(TRACE_APS2, "pkt #%d is a dup - drop", (FMT__D, aps_hdr.aps_counter));
    zb_free_buf(packet);
  }
  else
#endif
  {
    zb_buf_assign_param(packet, (zb_uint8_t *)&aps_hdr, sizeof(zb_apsde_data_indication_t));
#ifdef ZB_SECURITY
#ifdef APS_FRAME_SECURITY
    if (ZB_APS_FC_GET_SECURITY(aps_hdr.fc)
        && zb_aps_unsecure_frame(packet) != RET_OK)
    {
      zb_free_buf(packet);
    }
    else
#endif
#endif
    {
#ifndef ZB_LIMITED_FEATURES
      if (ZB_APS_FC_GET_FRAME_TYPE(aps_hdr.fc) == ZB_APS_FRAME_COMMAND)
      {
        zb_aps_in_command_handle(param);
      }
      else
        if (!(ZB_APS_FC_GET_FRAME_TYPE(aps_hdr.fc) == ZB_APS_FRAME_DATA
              && ZB_APS_FC_GET_DELIVERY_MODE(aps_hdr.fc) == ZB_APS_DELIVERY_GROUP))
#endif
      {
        TRACE_MSG(TRACE_APS3, "data pkt", (FMT__0));
        ZB_SCHEDULE_CALLBACK(zb_apsde_data_indication, param);
      }
#ifndef ZB_LIMITED_FEATURES
      else
      {
        TRACE_MSG(TRACE_APS3, "data pkt - group addressed", (FMT__0));
        ZB_SCHEDULE_CALLBACK(zb_aps_pass_up_group_buf, param);
      } /* else (group) */
#endif
    } /* else (unsecured ok) */
  } /* else (not dup) */

  TRACE_MSG(TRACE_APS1, "-zb_nlde_data_indication_continue", (FMT__0));
}


void zb_aps_hdr_parse(zb_buf_t *packet, zb_aps_hdr_t *aps_hdr, zb_bool_t cut_nwk_hdr) ZB_SDCC_REENTRANT
{
  zb_nwk_hdr_t *nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(packet);
  zb_uint8_t *apshdr = NULL;

  /* Parse NWK header */
  /* get src and dst address from the NWK header */
  /* if packet is stored in APS retransmit queue it doesn't have
   * nwk header */
  if (cut_nwk_hdr)
  {
    aps_hdr->src_addr = nwk_hdr->src_addr;
    aps_hdr->dst_addr = nwk_hdr->dst_addr;
    /* Remove NWK header from the packet */
    ZB_NWK_HDR_CUT(packet, apshdr);
  }
  else
  {
    /* src and dst addr are not available in this case */
    aps_hdr->src_addr = 0;
    aps_hdr->dst_addr = 0;
    apshdr = ZB_BUF_BEGIN(packet);
  }

  aps_hdr->src_endpoint = (zb_uint8_t)~0;
  aps_hdr->fc = *apshdr;
  apshdr++;

  /* init */

  if (ZB_APS_FC_GET_ACK_FORMAT(aps_hdr->fc) == 0)
  {
    if (ZB_APS_FC_GET_DELIVERY_MODE(aps_hdr->fc) != ZB_APS_DELIVERY_GROUP)
    {
      /* has endpoint if not group addressing */
      aps_hdr->dst_endpoint = *apshdr;
      apshdr++;
    }
    else
    {
      zb_get_next_letoh16(&aps_hdr->group_addr, &apshdr);
      TRACE_MSG(TRACE_APS3, "Group addressing, group %d", (FMT__D, aps_hdr->group_addr));
    }
    /* if data or ack, has cluster and profile id */
    /* Not sure pointer is aligned to 2. Use macro. */
    zb_get_next_letoh16(&aps_hdr->clusterid, &apshdr);
    zb_get_next_letoh16(&aps_hdr->profileid, &apshdr);
    aps_hdr->src_endpoint = *apshdr;
    apshdr++;
  }
  aps_hdr->aps_counter = *apshdr;
  if (ZB_APS_FC_GET_EXT_HDR_PRESEBT(aps_hdr->fc))
  {
    /* TODO: handle fragmentation and Extended header here */
    TRACE_MSG(TRACE_APS3, "Ext hdr present!", (FMT__0));
  }
}


#ifndef ZB_LIMITED_FEATURES
void zb_aps_pass_group_msg_up(zb_uint8_t param) ZB_CALLBACK
{
  /*
    handle - index in the endpoints array
   */
  zb_ushort_t g_i;

  /* search for a group by group address */
  {
    zb_uint16_t group_addr = ZB_GET_BUF_PARAM(ZG->aps.group.active_pass_up_buf, zb_aps_hdr_t)->group_addr;

    for (g_i = 0 ;
         g_i < ZG->aps.group.n_groups
           && ZG->aps.group.groups[g_i].group_addr != group_addr; ++g_i)
    {
    }
  }
  if (g_i == ZG->aps.group.n_groups
      || ZG->aps.group.active_pass_up_buf->u.hdr.handle >= ZG->aps.group.groups[g_i].n_endpoints)
  {
    /* No such group, or done with this buffer. Free buffer and reschedule myself to check next buffer */
    TRACE_MSG(TRACE_APS3, "done with buf %p/%hd",
              (FMT__P_H, ZG->aps.group.active_pass_up_buf, ZB_REF_FROM_BUF(ZG->aps.group.active_pass_up_buf)));
    zb_free_buf(ZG->aps.group.active_pass_up_buf);
    if (!ZB_RING_BUFFER_IS_EMPTY(&ZG->aps.group.pass_up_q))
    {
      ZG->aps.group.active_pass_up_buf = ZB_BUF_FROM_REF(*ZB_RING_BUFFER_GET(&ZG->aps.group.pass_up_q));
      ZB_RING_BUFFER_FLUSH_GET(&ZG->aps.group.pass_up_q);
      ZB_SCHEDULE_CALLBACK(zb_aps_pass_group_msg_up, param);
    }
    else
    {
      ZG->aps.group.active_pass_up_buf = NULL;
      zb_free_buf(ZB_BUF_FROM_REF(param));
      TRACE_MSG(TRACE_APS3, "No more job of passing group data up", (FMT__0));
    }
  }
  else
  {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);

    /* pass up allocated buffer */
    ZB_BUF_COPY(buf, ZG->aps.group.active_pass_up_buf);
    buf->u.hdr.handle = 0;
    /* assign endpoint */
    ZB_GET_BUF_PARAM(buf, zb_aps_hdr_t)->dst_endpoint =
      ZG->aps.group.groups[g_i].endpoints[ZG->aps.group.active_pass_up_buf->u.hdr.handle];
    ZG->aps.group.active_pass_up_buf->u.hdr.handle++;

    TRACE_MSG(TRACE_APS3, "Pass group packet up to endp %hd", (FMT__H, ZB_GET_BUF_PARAM(buf, zb_aps_hdr_t)->dst_endpoint));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_indication, param);

    /* schedule next pass up iteration */
    zb_get_in_buf_delayed(zb_aps_pass_group_msg_up);
  }
}
#endif

#ifdef ZB_SECURITY
void zb_aps_send_command(zb_uint8_t param, zb_uint16_t dest_addr, zb_uint8_t command, zb_bool_t secure)
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_ushort_t need_ack = !ZB_NWK_IS_ADDRESS_BROADCAST(dest_addr);
#ifdef ZB_DISABLE_APS_ACK_REQ
  /* Specially for test with Daintree: do not ask ACK for key transport:
     Daintree wants to encrypt ACK by its predefined key before it receive key from us */
  if (command == APS_CMD_TRANSPORT_KEY)
  {
    need_ack = 0;
  }
#endif
  TRACE_MSG(TRACE_SECUR3, ">>zb_aps_send_command param %hd cmd %hd secur %hd to %d need_ack %hd", (FMT__H_H_H_D_H, param, command, secure, dest_addr, need_ack));

  if (need_ack)
  {
    zb_uint8_t i;
    for (i = 0 ; i < ZB_N_APS_RETRANS_ENTRIES ; ++i)
    {
      if (ZG->aps.retrans.hash[i].state == ZB_APS_RETRANS_ENT_FREE)
      {
        ZG->aps.retrans.hash[i].addr = dest_addr;
        ZG->aps.retrans.hash[i].aps_counter = ZB_AIB_APS_COUNTER();
        ZG->aps.retrans.hash[i].buf = param;
        ZG->aps.retrans.hash[i].nwk_insecure = !secure;
        ZG->aps.retrans.hash[i].aps_retries = ZB_N_APS_MAX_FRAME_ENTRIES;
        ZG->aps.retrans.hash[i].state = ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING;
        TRACE_MSG(TRACE_APS2, "Store buf %hd len %hd in retrans hash %d", (FMT__H_H, param, ZB_BUF_LEN(buf), i));

        DUMP_TRAF("sending aps cmd", ZB_BUF_BEGIN(buf), ZB_BUF_LEN(buf));

        break;
      }
    }
    if (i == ZB_N_APS_RETRANS_ENTRIES)
    {
      TRACE_MSG(TRACE_APS2, "ACK table is FULL", (FMT__0));
    }
    else
    {
      ZB_SCHEDULE_ALARM(zb_aps_ack_timer_cb, i, ZB_N_APS_ACK_WAIT_DURATION);
    }
  }

  /* Fill APS command header - see 2.2.5.2.2  APS Command Frame Format.
     At the same time alloc and fill aux security header
   */

  {
    zb_aps_command_header_t *hdr;

#ifdef ZB_SECURITY
#ifdef APS_FRAME_SECURITY
    buf->u.hdr.encrypt_type = ZB_SECUR_NO_ENCR;
    if (secure)
    {
      /* Allocate here space for APS command header, aux header and command id
       * (it is in payload). */
      secure = zb_aps_command_add_secur(buf, command);
      hdr = (zb_aps_command_header_t *)ZB_BUF_BEGIN(buf);
    }
    else
#endif
#endif
    {
      /* no security - just aps command header */
      ZB_BUF_ALLOC_LEFT(buf, sizeof (*hdr), hdr);
      hdr->fc = 0;
      hdr->aps_command_id = command;
    }

    ZB_APS_FC_SET_COMMAND(hdr->fc, need_ack);
    hdr->aps_counter = ZB_AIB_APS_COUNTER();
  }
  ZB_AIB_APS_COUNTER_INC();

  fill_nldereq(param, dest_addr, secure);
  TRACE_MSG(TRACE_SECUR3, "send APS cmd %hd secur %hd to %d", (FMT__H_H_D, command, secure, dest_addr));

  ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, param);
}
#endif

static void fill_nldereq(zb_uint8_t param, zb_uint16_t addr, zb_uint8_t secure) ZB_SDCC_REENTRANT
{
  zb_nlde_data_req_t *nldereq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_nlde_data_req_t));
  ZB_BZERO(nldereq, sizeof(zb_nlde_data_req_t));
  nldereq->radius = ZB_APS_COMMAND_RADIUS;
  nldereq->addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
  nldereq->discovery_route = 1;
  nldereq->dst_addr = addr;
  nldereq->security_enable = secure;
}


static void aps_ack_send_handle(zb_buf_t *packet, zb_aps_hdr_t *aps_hdr) ZB_SDCC_REENTRANT
{
  zb_short_t aps_hdr_size;
  zb_uint8_t *apshdr = NULL;
  zb_uint8_t fc = 0;

  TRACE_MSG(TRACE_APS2, "+aps_ack_send_handle %hd", (FMT__H, ZB_REF_FROM_BUF(packet)));
  ZB_APS_FC_SET_FRAME_TYPE(fc, ZB_APS_FRAME_ACK);

  if (ZB_APS_FC_GET_FRAME_TYPE(aps_hdr->fc) != ZB_APS_FRAME_COMMAND)
  {
    aps_hdr_size = (2 +       /* fc + aps counter */
                    2 +       /* src & dest endpoint */
                    4 +       /* cluster id, profile id */
                    /* TODO: handle fragmentation and Extended header. Now suppose no Extended header */
                    0
      );
    ZB_APS_FC_SET_ACK_FORMAT(fc, 0);
  }
  else
  {
    aps_hdr_size = 2;         /* fc + aps counter */
    ZB_APS_FC_SET_ACK_FORMAT(fc, 1);
  }

  ZB_BUF_INITIAL_ALLOC(packet, aps_hdr_size, apshdr);

  *apshdr = fc;
  apshdr++;

  if (ZB_APS_FC_GET_ACK_FORMAT(fc) == 0)
  {
#ifdef APS_RETRANSMIT_TEST
    zb_uint16_t ci = 1;
#endif
    *apshdr = aps_hdr->src_endpoint;
    apshdr++;

#ifdef APS_RETRANSMIT_TEST
    if (ZG->aps.retrans.counter % 2)
    {
      zb_put_next_htole16(&apshdr, ci);
    }
    else
    {
      zb_put_next_htole16(&apshdr, aps_hdr->clusterid);
    }
    ZG->aps.retrans.counter++;
#else
    TRACE_MSG(TRACE_APS2, "clusterid %d", (FMT__D, aps_hdr->clusterid));
    zb_put_next_htole16(&apshdr, aps_hdr->clusterid);
#endif

    zb_put_next_htole16(&apshdr, aps_hdr->profileid);

    TRACE_MSG(TRACE_APS2, "src_endpoint %d", (FMT__D, aps_hdr->src_endpoint));
    *apshdr = aps_hdr->dst_endpoint;
    apshdr++;
  }
  *apshdr = aps_hdr->aps_counter;

  fill_nldereq(ZB_REF_FROM_BUF(packet), aps_hdr->src_addr, 1);

  ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, ZB_REF_FROM_BUF(packet));

  TRACE_MSG(TRACE_APS2, "-aps_ack_send_handle", (FMT__0));
}


#ifndef ZB_LIMITED_FEATURES2
static zb_uint8_t save_ack_data(zb_uint8_t param, zb_apsde_data_req_t *req, zb_uint8_t *ref) ZB_SDCC_REENTRANT
{
  zb_uint8_t i = 0;
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_APS2, "+save_ack_data %hd", (FMT__H, param));
  do
  {
    if (ZG->aps.retrans.hash[i].state == ZB_APS_RETRANS_ENT_FREE)
    {
      switch (req->addr_mode)
      {
        case ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT:
          ZG->aps.retrans.hash[i].addr = req->dst_addr.addr_short;
          break;
        case ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT:
          break;
        case ZB_APS_ADDR_MODE_16_ENDP_PRESENT:
          ZG->aps.retrans.hash[i].addr = req->dst_addr.addr_short;
          break;
        case ZB_APS_ADDR_MODE_64_ENDP_PRESENT:
          ZG->aps.retrans.hash[i].addr = req->dst_addr.addr_short;
          break;
        default:
          break;
      }
      ZG->aps.retrans.hash[i].clusterid = req->clusterid;
      ZG->aps.retrans.hash[i].aps_counter = ZB_AIB_APS_COUNTER() - 1;
      ZG->aps.retrans.hash[i].src_endpoint = req->src_endpoint;
      ZG->aps.retrans.hash[i].dst_endpoint = req->dst_endpoint;
      ZG->aps.retrans.hash[i].buf = param;
      ZG->aps.retrans.hash[i].nwk_insecure = 0;
      ZG->aps.retrans.hash[i].aps_retries = ZB_N_APS_MAX_FRAME_ENTRIES;
      ZG->aps.retrans.hash[i].state = ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING;
      *ref = i;
      TRACE_MSG(TRACE_APS2, "save_ack_data  clusterid %d aps_counter %d src_endpoint %d src_endpoint %d, i %hd",
                (FMT__D_D_D_D_H, ZG->aps.retrans.hash[i].clusterid, ZG->aps.retrans.hash[i].aps_counter,
                 ZG->aps.retrans.hash[i].src_endpoint, ZG->aps.retrans.hash[i].dst_endpoint, i));
      break;
    }
    i++;
  } while (i < ZB_N_APS_RETRANS_ENTRIES);

  if (i == ZB_N_APS_RETRANS_ENTRIES-1)
  {
    TRACE_MSG(TRACE_APS2, "ACK table is FULL", (FMT__0));
    ret = ZB_APS_STATUS_TABLE_FULL;
  }
  return ret;
}


static void aps_ack_frame_handle(zb_aps_hdr_t *aps_hdr) ZB_SDCC_REENTRANT
{
  zb_uint8_t i;
  zb_ushort_t is_command = ZB_APS_FC_GET_ACK_FORMAT(aps_hdr->fc);

  /* Search for retransmit table entry */
  for ( i = 0 ; i < ZB_N_APS_RETRANS_ENTRIES ; ++i)
 {
   TRACE_MSG(TRACE_APS2, "aps_ack_check_handle addr %d, src_addr %d", (FMT__D_D, ZG->aps.retrans.hash[i].addr, aps_hdr->src_addr));
   TRACE_MSG(TRACE_APS2, "retrans.hash i %d, clusterid %d aps_counter %d src_endpoint %d dst_endpoint %d",
             (FMT__D_D_D_D_D, i, ZG->aps.retrans.hash[i].clusterid, ZG->aps.retrans.hash[i].aps_counter,
              ZG->aps.retrans.hash[i].src_endpoint, ZG->aps.retrans.hash[i].dst_endpoint));

    /* Not need to check status here: when entry freed, addr set to -1 */
    if (ZG->aps.retrans.hash[i].addr == aps_hdr->src_addr
        && ZG->aps.retrans.hash[i].aps_counter == aps_hdr->aps_counter
        && (is_command
            || (ZG->aps.retrans.hash[i].clusterid == aps_hdr->clusterid
                && ZG->aps.retrans.hash[i].src_endpoint == aps_hdr->dst_endpoint
                && ZG->aps.retrans.hash[i].dst_endpoint == aps_hdr->src_endpoint)))
    {
      break;
    }
  }

  if (i < ZB_N_APS_RETRANS_ENTRIES)
  {
    TRACE_MSG(TRACE_APS2, "found ent %hd", (FMT__H, i));
    if (ZG->aps.retrans.hash[i].state == ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING)
    {
      TRACE_MSG(TRACE_APS2, "ent %hd: got ACK while waiting for nwk confirm", (FMT__H, i));
      ZG->aps.retrans.hash[i].state = ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_APS_ACKED_ALRM_RUNNING;
    }
    else
    {
      TRACE_MSG(TRACE_APS2, "packet %hd acked ok", (FMT__H, ZG->aps.retrans.hash[i].buf));
      done_with_this_ack(i, aps_hdr->fc, RET_OK);
    }
  }
  else
  {
    TRACE_MSG(TRACE_APS2, "No entry to this ACK - just drop it", (FMT__0));
  }

  TRACE_MSG(TRACE_APS2, "-aps_ack_frame_handle", (FMT__0));
}


static void done_with_this_ack(zb_ushort_t i, zb_uint8_t fc, zb_uint8_t status) ZB_SDCC_REENTRANT
{
  ZB_BUF_FROM_REF(ZG->aps.retrans.hash[i].buf)->u.hdr.status = status;
  if (ZB_APS_FC_GET_ACK_FORMAT(fc))
  {
    TRACE_MSG(TRACE_APS2, "finally done with this commmand i %hd", (FMT__H, i));
    zb_free_buf(ZB_BUF_FROM_REF(ZG->aps.retrans.hash[i].buf));
  }
  else
  {
    TRACE_MSG(TRACE_APS2, "finally done with this data pkt i %hd", (FMT__H, i));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_confirm, ZG->aps.retrans.hash[i].buf);
  }
  ZG->aps.retrans.hash[i].state = ZB_APS_RETRANS_ENT_FREE;
  ZG->aps.retrans.hash[i].addr = (zb_uint16_t)-1;
  ZG->aps.retrans.hash[i].buf = (zb_uint8_t)-1;
  zb_schedule_alarm_cancel(zb_aps_ack_timer_cb, i);
}


void zb_aps_ack_timer_cb(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_APS2, "+zb_aps_ack_timer_cb %hd state %hd", (FMT__H_H, param, ZG->aps.retrans.hash[param].state));

  /* Note: 'param' here is index, not packet buffer!  */
  {
    zb_uint8_t fc = *ZB_BUF_BEGIN(ZB_BUF_FROM_REF(ZG->aps.retrans.hash[param].buf));

    if (ZG->aps.retrans.hash[param].aps_retries)
    {
      ZG->aps.retrans.hash[param].aps_retries--;
    }
    if (ZG->aps.retrans.hash[param].aps_retries == 0
         && ZG->aps.retrans.hash[param].state == ZB_APS_RETRANS_ENT_SENT_MAC_CONFIRMED_ALRM_RUNNING)
    {
      TRACE_MSG(TRACE_APS2, "out of retransmissions - APS transmission failed", (FMT__0));
      done_with_this_ack(param, fc, (zb_uint8_t)RET_NO_ACK);
    }
    else
    {
      /* If we stuck in some intermediate state, let's schedule alarm again
       * even if counter is already 0 */
      ZB_SCHEDULE_ALARM(zb_aps_ack_timer_cb, param, ZB_N_APS_ACK_WAIT_DURATION);

      if (ZG->aps.retrans.hash[param].state == ZB_APS_RETRANS_ENT_SENT_MAC_CONFIRMED_ALRM_RUNNING)
      {
        fill_nldereq(ZG->aps.retrans.hash[param].buf,
                     ZG->aps.retrans.hash[param].addr,
                     /* use NWK security only if not use APS security */
                     !(ZG->aps.retrans.hash[param].nwk_insecure || ZB_APS_FC_GET_SECURITY(fc)));

        TRACE_MSG(TRACE_APS2, "ACK FAILED - retransmit [%hd] st %hd buf %hd to %d, retries %hd, insecure %hd",
                  (FMT__H_H_H_D_H,
                   param, ZG->aps.retrans.hash[param].state,
                   ZG->aps.retrans.hash[param].addr, ZG->aps.retrans.hash[param].aps_retries,
                   ZG->aps.retrans.hash[param].nwk_insecure));
        ZG->aps.retrans.hash[param].state = ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING;

        DUMP_TRAF("retransmitting aps", ZB_BUF_BEGIN(ZB_BUF_FROM_REF(ZG->aps.retrans.hash[param].buf)), ZB_BUF_LEN(ZB_BUF_FROM_REF(ZG->aps.retrans.hash[param].buf)));

        ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, ZG->aps.retrans.hash[param].buf);
      }
      else
      {
        TRACE_MSG(TRACE_APS2, "slit %hd state %hd - skip this retransmit", (FMT__H_H, param, ZG->aps.retrans.hash[param].state));
      }
    }
  }

  TRACE_MSG(TRACE_APS2, "-zb_aps_ack_timer_cb", (FMT__0));
}
#endif  /* ZB_LIMITED_FEATURES2 */

#ifndef ZB_LIMITED_FEATURES

void zb_apsme_get_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_aps_status_t status = ZB_APS_STATUS_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_get_request_t req_param;
  zb_apsme_get_confirm_t *conf = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_apsme_get_req %d", (FMT__D, param));
  ZB_MEMCPY(&req_param, ZB_BUF_BEGIN(buf), sizeof(req_param));

  if (req_param.aib_attr == ZB_APS_AIB_DESIGNATED_COORD)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->aib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)) = ZB_AIB().aps_designated_coordinator;
  }
  else if (req_param.aib_attr == ZB_APS_AIB_USE_INSECURE_JOIN)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->aib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)) = ZB_AIB().aps_insecure_join;
  }
  else if (req_param.aib_attr == ZB_APS_AIB_CHANNEL_MASK)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t) + sizeof(zb_uint32_t), conf);
    conf->aib_length = sizeof(zb_uint32_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)) = ZB_AIB().aps_channel_mask;
  }
  else if (req_param.aib_attr == ZB_APS_AIB_USE_EXT_PANID)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t) + sizeof(zb_ext_pan_id_t), conf);
    conf->aib_length = sizeof(zb_ext_pan_id_t);
    ZB_64BIT_ADDR_COPY((((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)), ZB_AIB().aps_use_extended_pan_id);
  }
  else if (req_param.aib_attr == ZB_APS_AIB_NONMEMBER_RADIUS)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->aib_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)) = 2; /* it is default value */
  }
  else
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_confirm_t), conf);
    status = ZB_APS_STATUS_UNSUPPORTED_ATTRIBUTE;
    conf->aib_length = 0;
  }
  conf->aib_attr = req_param.aib_attr;
  conf->status = status;
  ZB_SCHEDULE_CALLBACK(zb_apsme_get_confirm, param);
  TRACE_MSG(TRACE_APS2, "<<zb_apsme_get_req %d", (FMT__D, param));
}

void zb_apsme_get_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_get_confirm_t *conf = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_apsme_get_confirm %d", (FMT__D, param));
  conf = ( zb_apsme_get_confirm_t *)ZB_BUF_BEGIN(buf);

  if (conf->aib_attr == ZB_APS_AIB_DESIGNATED_COORD)
  {
    TRACE_MSG(TRACE_APS2, "attr %hd get param %hd", (FMT__H_H, conf->aib_attr, *(((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t))));
  }
  else if (conf->aib_attr == ZB_APS_AIB_USE_EXT_PANID)
  {
    TRACE_MSG(TRACE_APS2, "attr %hd get param "TRACE_FORMAT_64"", (FMT__H_A, conf->aib_attr,
              TRACE_ARG_64((((zb_uint8_t *)conf) + sizeof(zb_apsme_get_confirm_t)))));
  }
  TRACE_MSG(TRACE_APS2, "<<zb_apsme_get_confirm status %hd", (FMT__H, conf->status));
  zb_free_buf(buf);
}

void zb_apsme_set_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  zb_apsme_set_request_t *req_param = (zb_apsme_set_request_t *)ptr;
  zb_apsme_set_confirm_t conf;
  zb_apsme_set_confirm_t *conf_p = NULL;

  TRACE_MSG(TRACE_APS2, ">>zb_apsme_set_req %d", (FMT__D, param));

  conf.aib_attr = req_param->aib_attr;
  conf.status = ZB_APS_STATUS_SUCCESS;
  ptr += sizeof(zb_apsme_set_request_t);
  if (req_param->aib_attr == ZB_APS_AIB_DESIGNATED_COORD)
  {
    ZB_AIB().aps_designated_coordinator = *ptr;
  }
  else if (req_param->aib_attr == ZB_APS_AIB_USE_INSECURE_JOIN)
  {
    ZB_AIB().aps_insecure_join = *ptr;
  }
  else if (req_param->aib_attr == ZB_APS_AIB_CHANNEL_MASK)
  {
    ZB_AIB().aps_channel_mask = *ptr;
  }
  else if (req_param->aib_attr == ZB_APS_AIB_USE_EXT_PANID)
  {
    ZB_EXTPANID_COPY(ZB_AIB().aps_use_extended_pan_id, ptr);
  }
  else
  {
    conf.status = ZB_APS_STATUS_UNSUPPORTED_ATTRIBUTE;
  }
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_set_confirm_t), conf_p);
  ZB_MEMCPY(conf_p, &conf, sizeof(*conf_p));
  ZB_SCHEDULE_CALLBACK(zb_apsme_set_confirm, param);
  TRACE_MSG(TRACE_APS2, "<<zb_apsme_set_req %d", (FMT__D, param));
}


void zb_apsme_set_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS2, ">>zb_apsme_set_confirm %d", (FMT__D, param));
#ifdef ZB_TRACE_LEVEL
  {
  zb_apsme_set_confirm_t *conf = ( zb_apsme_set_confirm_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_APS2, "<<zb_apsme_set_confirm status %hd", (FMT__H, conf->status));
  }
#endif
  zb_free_buf(buf);
}
#endif  /* ZB_LIMITED_FEATURES */

/*! @} */
