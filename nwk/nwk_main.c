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
PURPOSE: Network layer main module
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_secur.h"


/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_3.h"

/**
   NIB database in memory
 */

void zb_nwk_btr_expiry(zb_uint8_t param) ZB_CALLBACK;
void nwk_frame_indication(zb_uint8_t param) ZB_CALLBACK;
#ifndef ZB_LIMITED_FEATURES
void zb_nwk_leave_ind_prnt(zb_uint8_t param) ZB_CALLBACK;
static void zb_nwk_call_leave_ind(zb_uint8_t param, zb_uint8_t rejoin, zb_address_ieee_ref_t addr_ref);
static void zb_nwk_leave_handler(zb_uint8_t param, zb_nwk_hdr_t *nwk_hdr, zb_uint8_t lp) ZB_SDCC_REENTRANT;
#endif
void zb_nwk_call_br_confirm(zb_uint8_t param) ZB_CALLBACK;

/* Start btr entries expiry if btt was empty and now it has entries */
#if 1
#define ZB_NWK_START_BTR_EXPIRY()                                 \
do                                                                \
{                                                                 \
  if ( ZG->nwk.handle.btt_cnt == 1 )                              \
  {                                                               \
    ZB_SCHEDULE_ALARM_CANCEL(zb_nwk_btr_expiry, ZB_ALARM_ANY_PARAM); \
    ZB_SCHEDULE_ALARM(zb_nwk_btr_expiry, 0, ZB_TIME_ONE_SECOND);  \
  }                                                               \
}                                                                 \
while(0)
#endif

void zb_nwk_init()
{
  TRACE_MSG(TRACE_NWK1, "+nwk_init", (FMT__0));

  /* Initialize internal structures */
  zb_nwk_set_device_type((zb_nwk_device_type_t)ZB_NIB_DEVICE_TYPE());

  zb_nwk_nib_init();

  zb_nwk_neighbor_init();

#ifdef ZB_NWK_TREE_ROUTING
  zb_nwk_tree_routing_init();
#endif

#ifdef ZB_NWK_MESH_ROUTING
  zb_nwk_mesh_routing_init();
#endif

  TRACE_MSG(TRACE_NWK1, "-nwk_init", (FMT__0));
}

void zb_nwk_nib_init()
{
  TRACE_MSG(TRACE_NWK1, ">>nib_init", (FMT__0));

  ZG->nwk.nib.passive_ack_timeout = ZB_NWK_PASSIVE_ACK_TIMEOUT;
  ZG->nwk.nib.max_broadcast_retries = ZB_NWK_MAX_BROADCAST_RETRIES;
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
  ZG->nwk.nib.max_routers = ZB_NWK_MAX_ROUTERS;
#endif
  ZB_NIB_MAX_DEPTH() = ZB_NWK_MAX_DEPTH;
  ZB_NIB_SET_USE_TREE_ROUTING(1);
#ifdef  ZB_NWK_MESH_ROUTING
  ZG->nwk.nib.aps_rreq_addr = -1;
#endif

  TRACE_MSG(TRACE_NWK1, "<<nib_init", (FMT__0));
}

/*
  Alloc and fill nwk hdr, return pointer to the allocated hdr
*/
zb_nwk_hdr_t *nwk_alloc_and_fill_hdr(zb_buf_t *buf,
                                     zb_uint16_t dst_addr,
                                     zb_uint8_t *src_ieee_addr, zb_uint8_t *dst_ieee_addr,
                                     zb_bool_t is_multicast, zb_bool_t is_secured, zb_bool_t is_cmd_frame) ZB_SDCC_REENTRANT
{
  zb_nwk_hdr_t *nwhdr;
  zb_ushort_t hdr_size = ( src_ieee_addr && dst_ieee_addr ) ? ZB_NWK_FULL_HDR_SIZE(is_multicast)
    : ( (src_ieee_addr || dst_ieee_addr) ? ZB_NWK_HALF_HDR_SIZE(is_multicast) : ZB_NWK_SHORT_HDR_SIZE(is_multicast) );

#ifdef ZB_SECURITY
  if ( is_secured )
  {
    hdr_size += sizeof(zb_nwk_aux_frame_hdr_t);
  }
#else
  ZVUNUSED(is_secured);
#endif

  /* fill nwk header */
  if ( is_cmd_frame )
  {
    ZB_BUF_INITIAL_ALLOC(buf, hdr_size, nwhdr);
  }
  else
  {
    ZB_BUF_ALLOC_LEFT(buf, hdr_size, nwhdr);
  }

  /* fill frame control fields */
  ZB_BZERO2(nwhdr->frame_control);
  if ( is_cmd_frame )
  {
    ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(nwhdr->frame_control, ZB_NWK_FRAME_TYPE_COMMAND, ZB_PROTOCOL_VERSION);
  }
  else
  {
    ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(nwhdr->frame_control, ZB_NWK_FRAME_TYPE_DATA, ZB_PROTOCOL_VERSION);
  }
  ZB_NWK_FRAMECTL_SET_SRC_DEST_IEEE(nwhdr->frame_control, (src_ieee_addr) ? 1 : 0, (dst_ieee_addr) ? 1 : 0);
#ifdef ZB_SECURITY
  if ( is_secured )
  {
    ZB_NWK_FRAMECTL_SET_SECURITY(nwhdr->frame_control, 1);
    buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
  }
  else
  {
    buf->u.hdr.encrypt_type = ZB_SECUR_NO_ENCR;
  }
#endif
  nwhdr->src_addr = ZB_NIB_NETWORK_ADDRESS();
  nwhdr->dst_addr = dst_addr;
  nwhdr->radius = 1;
  nwhdr->seq_num = ZB_NIB_SEQUENCE_NUMBER();
  ZB_NIB_SEQUENCE_NUMBER_INC();

  if ( src_ieee_addr && dst_ieee_addr )
  {
    ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, dst_ieee_addr);
    ZB_IEEE_ADDR_COPY(nwhdr->src_ieee_addr, src_ieee_addr);
  }
  else if ( dst_ieee_addr )
  {
    ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, dst_ieee_addr);
  }
  else if ( src_ieee_addr )
  {
    /* Yes, here should be dst_ieee_addr, because src_ieee_addr is absent */
    ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, src_ieee_addr);
  }

  return nwhdr;
}

zb_uint8_t *nwk_alloc_and_fill_cmd(zb_buf_t *buf, zb_nwk_cmd_t cmd, zb_uint8_t cmd_size)
{
  zb_uint8_t *ptr = NULL;
  ZB_NWK_ALLOC_COMMAND_GET_PAYLOAD_PTR(buf, cmd, cmd_size, ptr);
  return ptr;
}


void zb_nlde_data_request(zb_uint8_t param)   ZB_CALLBACK
{
  zb_buf_t *nsdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlde_data_req_t *nldereq = ZB_GET_BUF_TAIL(nsdu, sizeof(zb_nlde_data_req_t));
  zb_nwk_hdr_t *nwhdr;
  zb_bool_t secure = ZB_FALSE;

  ZB_ASSERT(nldereq);

  TRACE_MSG(TRACE_NWK1, "+nlde_data_request %p", (FMT__P, nldereq));

  /* check that we are associated */
  if (!ZG->nwk.handle.joined)
  {
    NWK_CONFIRM_STATUS(nsdu, ZB_NWK_STATUS_INVALID_REQUEST, zb_nlde_data_confirm);
    return;
  }

  /* TODO: if not broadcast or multicast, send source and destination ieee
   * address.
   * Not sure when really needs to send it.
   * Sure need to send it for some comman frames (it depends on frame). Usually:
   * source - always, destination - depends on command.
   */

#ifdef ZB_SECURITY
  TRACE_MSG(TRACE_NWK1, "security_enable %hd authenticated %hd secure_all_frames %hd security_level %hd",
            (FMT__H_H_H_H, nldereq->security_enable, ZG->aps.authenticated, ZG->nwk.nib.secure_all_frames, ZG->nwk.nib.security_level));

  secure = (zb_bool_t)(nldereq->security_enable && ZG->aps.authenticated && ZG->nwk.nib.secure_all_frames
            && ZG->nwk.nib.security_level);
#endif
  nwhdr = nwk_alloc_and_fill_hdr(nsdu,
                                 nldereq->dst_addr,
                                 NULL, NULL,
                                 (zb_bool_t)(nldereq->addr_mode == ZB_ADDR_16BIT_MULTICAST), secure,
                                 ZB_FALSE);
  nwhdr->radius = nldereq->radius ? nldereq->radius : ZB_NIB_MAX_DEPTH() * 2;

  if ( nldereq->discovery_route )
  {
    ZB_NWK_FRAMECTL_SET_DISCOVER_ROUTE(nwhdr->frame_control, 1);
  }

  {
    zb_uint8_t ndsu_handle = nldereq->ndsu_handle;

    ZB_SET_BUF_PARAM(nsdu, ndsu_handle, zb_uint8_t);
    ZB_SCHEDULE_CALLBACK(zb_nwk_forward, ZB_REF_FROM_BUF(nsdu));
  }

  TRACE_MSG(TRACE_NWK1, "-nlde_data_request", (FMT__0));
}


/**
   Calculate destination address.

   The calculation the destination address:
   1) if it is broadcast
   2) in neighbor table (the dest is a neighbor of our)
   3) routing table (the route path exists)
   4) route discovery (the route path does not exists but mesh routing is in
   progress)
   5) tree (none of the above and the route discovery flag is disabled)

   @param nsdu - packet to send; NWK header is filled already
   @param handle - ndsu packet handle
   @param mac_dst - output parameter, short MAC address of the next hop if know
   it now, 0 if will wait
   @param indirect - output parameter, true if neighbor does not keep its rx on
   when idle (i.e. sleeping), then we need to send frame indirectly

   @return RET_OK on success, RET_BUSY - when route discovery has been initiated or
   route discovery is in progress, error code otherwise
 */
static zb_ret_t nwk_calc_destination(zb_buf_t *nsdu, zb_uint8_t handle, zb_uint16_t *mac_dst, zb_short_t *indirect, zb_nwk_command_status_t *cmd_status) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_nwk_hdr_t *nwhdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(nsdu);
#ifdef ZB_ROUTER_ROLE
  zb_neighbor_tbl_ent_t *nbh = NULL;
  zb_address_ieee_ref_t addr_ref;
#endif
#ifdef ZB_NWK_MESH_ROUTING
  zb_nwk_routing_t *route = NULL;
#endif

  ZVUNUSED(handle);
  ZVUNUSED(cmd_status);
  TRACE_MSG(TRACE_NWK1, ">>calc_dest nsdu %p handle %hd mac_dst %p indir %p cmd_status %p", (FMT__P_H_P_P_P, nsdu, handle, mac_dst, indirect, cmd_status));

  /* set most frequent calculation result */
  ZB_ASSERT(mac_dst && indirect);
  *mac_dst = nwhdr->dst_addr;
  *indirect = ZB_FALSE;

  /* ED sends all to its parent */
#ifdef ZB_ROUTER_ROLE
  if (ZG->nwk.nib.device_type == ZB_NWK_DEVICE_TYPE_ED)
#endif
  {
    /* but not during rejoin */
    if ( ZG->nwk.handle.state != ZB_NLME_STATE_REJOIN )
    {
      zb_address_short_by_ref(mac_dst, ZG->nwk.handle.parent);
      TRACE_MSG(TRACE_NWK1, "ED - send to parent %d", (FMT__D, *mac_dst));
    }
  }
#ifdef ZB_ROUTER_ROLE
  else
  {
    TRACE_MSG(TRACE_NWK1, "dest addr %d dev type %hd", (FMT__D_H, nwhdr->dst_addr, ZG->nwk.nib.device_type));
    /* is the destination address is broadcast */
    if ( ZB_NWK_IS_ADDRESS_BROADCAST(nwhdr->dst_addr) )
    {
      *mac_dst = ZB_NWK_BROADCAST_ALL_DEVICES;
      TRACE_MSG(TRACE_NWK1, "broadcast, mac dst ~0", (FMT__0));
    }
    /* is the destination is our neighbor */
    else if ( ((ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control) == 0
                &&  zb_address_by_short(nwhdr->dst_addr, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK)
               || ((ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control) != 0
                    &&  zb_address_by_ieee(nwhdr->dst_ieee_addr, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK)))
              && zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nbh) == RET_OK )
    {
      TRACE_MSG(TRACE_NWK1, "dst is neighb, rx_on %hd", (FMT__D, nbh->rx_on_when_idle));
      if ( !nbh->rx_on_when_idle )
      {
        *indirect = ZB_TRUE;
      }
    }
#ifdef ZB_NWK_MESH_ROUTING
    /* is the destination in our routing table */
    else if ( ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(nwhdr->frame_control)
              && (route = zb_nwk_mesh_find_route(nwhdr->dst_addr))
              && (route->status == ZB_NWK_ROUTE_STATE_ACTIVE
                  || route->status == ZB_NWK_ROUTE_STATE_VALIDATION_UNDERWAY) )
    {
      TRACE_MSG(TRACE_NWK1, "found route for %hd, next hop %hd", (FMT__H_H, nwhdr->dst_addr, route->next_hop_addr));
      *mac_dst = route->next_hop_addr;
      route->status = ZB_NWK_ROUTE_STATE_ACTIVE;
    }
    /* discovery is in progress */
    else if ( ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(nwhdr->frame_control)
              && route
              && route->status == ZB_NWK_ROUTE_STATE_DISCOVERY_UNDERWAY )
    {
      TRACE_MSG(TRACE_NWK2, "disc in progress - who is it?", (FMT__0));

      /* add buffer to the pending list */
      ret = zb_nwk_mesh_add_buf_to_pending(nsdu, handle);
      TRACE_MSG(TRACE_NWK2, "add buf to pending %d", (FMT__D, ret));
      if ( ret == RET_OK )
      {
        /* check who initiate discovery */
        zb_nwk_route_discovery_t *disc_entry = zb_nwk_mesh_find_route_discovery_entry(nwhdr->dst_addr);

        if ( !disc_entry
             || disc_entry->source_addr != ZB_NIB_NETWORK_ADDRESS() )
        {
          /* initiate route discovery */
          zb_nwk_mesh_route_discovery(NULL, nwhdr->dst_addr, 0);
        }
        else
        {
          TRACE_MSG(TRACE_NWK3, "already init r disc to %d", (FMT__D, nwhdr->dst_addr));
        }

        /* discovery is in progress */
        ret = RET_BUSY;
      }
      else
      {
        *cmd_status = ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE;
      }
    }
    /* are we allowed to discover the route? */
    else if ( ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(nwhdr->frame_control)
              && !route )
    {
      TRACE_MSG(TRACE_NWK2, "init r disc to %d", (FMT__D, nwhdr->dst_addr));

      /* add buffer to the pending list */
      ret = zb_nwk_mesh_add_buf_to_pending(nsdu, handle);
      TRACE_MSG(TRACE_NWK3, "add buf to pend.l. %d", (FMT__D, ret));
      if ( ret == RET_OK )
      {
        /* initiate route discovery */
        zb_nwk_mesh_route_discovery(NULL, nwhdr->dst_addr, 0);

        /* discovery is in progress */
        ret = RET_BUSY;
      }
      else
      {
        *cmd_status = ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE;
      }
    }
#endif  /* ZB_NWK_MESH_ROUTING */
/* seems like we need to use tree routing, when DISCOVER_ROUTE bit in framectl
 * is 0, but we have no such frames */
#ifdef ZB_NWK_TREE_ROUTING
    else if ( ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(nwhdr->frame_control)
              && ZB_NIB_GET_USE_TREE_ROUTING() )
    {
      /* calc next hop */
      nbh = zb_nwk_tree_routing_route(nwhdr->dst_addr);
      TRACE_MSG(TRACE_NWK2, "route using tree r, to the neighb %p", (FMT__P, nbh));
      if ( nbh )
      {
        zb_address_short_by_ref(mac_dst, nbh->addr_ref);
        TRACE_MSG(TRACE_NWK3, "neighb addr %d", (FMT__D, *mac_dst));
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "tree route fail, n.t. empty?", (FMT__0));
        ret = RET_ERROR;
        *cmd_status = ZB_NWK_COMMAND_STATUS_TREE_LINK_FAILURE;
      }
    }
#endif  /* ZB_NWK_TREE_ROUTING */
    else
    {
      TRACE_MSG(TRACE_ERROR, "Unable to route!, get_discovery_route %hd", (FMT__H, ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(nwhdr->frame_control)));
      ret = RET_ERROR;
      *cmd_status = ZB_NWK_COMMAND_STATUS_NO_ROUTING_CAPACITY;
    }
  }
#endif  /* ZB_ROUTER_ROLE */

  TRACE_MSG(TRACE_NWK1, "<<calc_dest %d", (FMT__D, ret));
  return ret;
}

#ifdef ZB_ROUTER_ROLE
/* Retransmit broadcast packets nwkMaxBroadcastRetries times and unicast
 * broadcast packets to EDs with rx_on_when_idle == false */
void nwk_broadcast_transmition(zb_uint8_t param) ZB_CALLBACK
{
  int i = 0;
  zb_nwk_broadcast_retransmit_t *ent = NULL;
  zb_neighbor_tbl_ent_t *nent = NULL;
  zb_buf_t *buf = NULL;
  zb_uint8_t passive_ack_device_num = 0;

  TRACE_MSG(TRACE_NWK1, ">>nwk_broadcast_transmition %hd", (FMT__H, param));

  /* first transmit broadcast packets for the first time */
  for (i = 0; i < ZB_NWK_BRR_TABLE_SIZE && !ent ; i++)
  {
    if ( ZG->nwk.handle.brrt[i].used
         && ZG->nwk.handle.brrt[i].retries == 0 )
    {
      TRACE_MSG(TRACE_NWK2, "found brdcst packet %hd retries %hd ready to be retransmitted",
                (FMT__H_H, ZG->nwk.handle.brrt[i].buf, ZG->nwk.handle.brrt[i].retries));
      ent = &ZG->nwk.handle.brrt[i];
    }
  }

  /* next unicast packets to childrens */
  for (i = 0; i < ZB_NWK_BRR_TABLE_SIZE && !ent ; i++)
  {
    if ( ZG->nwk.handle.brrt[i].used
         && ZG->nwk.handle.brrt[i].neighbor_table_iterator != (zb_ushort_t)~0 )
    {
      zb_uint16_t addr;

      ent = &ZG->nwk.handle.brrt[i];
      nent = &ZG->nwk.neighbor.base_neighbor[ZG->nwk.handle.brrt[i].neighbor_table_iterator];
      zb_address_short_by_ref(&addr, nent->addr_ref);
      ZB_NWK_ADDR_TO_LE16(addr);

      /* do not unicast to the originator */
      if ( addr == ((zb_nwk_hdr_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(ent->buf)))->src_addr )
      {
        TRACE_MSG(TRACE_NWK2, "do not unicast brdcst frame to the originator", (FMT__0));
        ent->neighbor_table_iterator = zb_nwk_neighbor_next_ze_children_i(ent->dst_addr, ent->neighbor_table_iterator + 1);
        ent = NULL;
        nent = NULL;
        continue;
      }
      TRACE_MSG(TRACE_NWK2, "found brdcst packet %hd to be unicasted iterator %hd",
                (FMT__H_D, ZG->nwk.handle.brrt[i].buf, ZG->nwk.handle.brrt[i].neighbor_table_iterator));
    }
  }

  /* next retransmit broadcast packets */
  for (i = 0; i < ZB_NWK_BRR_TABLE_SIZE && !ent ; i++)
  {
    if ( ZG->nwk.handle.brrt[i].used
         && ZB_TIME_GE(ZB_TIMER_GET(), ZG->nwk.handle.brrt[i].next_retransmit) )
    {
      TRACE_MSG(TRACE_NWK2, "found brdcst packet %hd with ready to be retransmitted", (FMT__H, ZG->nwk.handle.brrt[i].buf));
      ent = &ZG->nwk.handle.brrt[i];
    }
  }

  if (ent)
  {
    TRACE_MSG(TRACE_NWK2, "ent %p rerties %hd nbtbl_iter %d", (FMT__P_H_D, ent, ent->retries, ent->neighbor_table_iterator));

    /* check we have buffer to be sent */
    if (!param)
    {
      /* get new one */
      TRACE_MSG(TRACE_NWK2, "brdcst retransmition need additional buffer %hd", (FMT__H, param));
      ZB_GET_OUT_BUF_DELAYED(nwk_broadcast_transmition);
      goto done;
    }

    if (nent)
    {
      /* mark this child as we get passive ack from */
      TRACE_MSG(TRACE_NWK2, "mark neighbor %hd as got ack from", (FMT__H, ent->neighbor_table_iterator));
      ZB_SET_BIT_IN_BIT_VECTOR(ent->passive_ack, ent->neighbor_table_iterator);

      /* find next child */
      ent->neighbor_table_iterator = zb_nwk_neighbor_next_ze_children_i(ent->dst_addr, ent->neighbor_table_iterator + 1);
    }
    else
    {
      ent->next_retransmit = ZB_TIMER_GET() + ZB_NWK_RREQ_RETRY_INTERVAL;
      ent->retries++;
    }

    /* Calc device numbers to get passive ack from */
    ZB_NWK_GET_PASSIVE_ACK_DEV_NUM(passive_ack_device_num);
    ZB_CALC_NONE_ZERO_BITS_IN_BIT_VECTOR(ent->passive_ack, ZB_NWK_BRCST_PASSIVE_ACK_ARRAY_SIZE, i);
    TRACE_MSG(TRACE_NWK2, "number of got passive acks %hd total %hd", (FMT__H_H, i, passive_ack_device_num));
    /* last packet retransmition?
       Could be in two cases:
       - if we unicast to all childs and broadcast max retries times
       - if we unicast to all childs and got passive acks from all neighbors (all
       routers coordinators and childs)
    */
    if (ent->neighbor_table_iterator == (zb_ushort_t)~0
        && ( ent->retries >= ZB_NWK_MAX_BROADCAST_RETRIES
             /* if this packet is from us, we should have exactly neighbor size
              * number acks */
             || ((unsigned)i == passive_ack_device_num
                 && ent->src_addr == ZB_PIB_SHORT_ADDRESS())
             /* if this packet is from our neighbor, we should have neighbor size-1
              * acks */
             || (passive_ack_device_num
                 && i == passive_ack_device_num - 1
                 && ent->src_addr != ZB_PIB_SHORT_ADDRESS())) )
    {
      /* clear entrie */
      TRACE_MSG(TRACE_NWK2, "last retransmition for the entrie", (FMT__0));
      /* TODO: Optimization could be done here, we could use ent->buf for the
       * last packet retransmission when no additional buffer is required. But
       * it's quite tricky. */
      if (param)
      {
        zb_free_buf(ZB_BUF_FROM_REF(param));
      }
      /* last time transmit this buffer to pass it up in .confirm */
      param = ent->buf;
      NWK_ARRAY_PUT_ENT(ZG->nwk.handle.brrt, ent, ZG->nwk.handle.brrt_cnt);
    }

    buf = ZB_BUF_FROM_REF(param);
    if (param != ent->buf)
    {
      ZB_BUF_COPY(buf, ZB_BUF_FROM_REF(ent->buf));
      /* This is not last buffer. It must be dropped after transmit complete
       * inside confirm. Only last transmission of buffer must be confirmed for
       * upper layer.
       */
      {
        zb_mcps_data_req_params_t *data_req = ZB_GET_BUF_PARAM(buf, zb_mcps_data_req_params_t);
        data_req->msdu_handle = ZB_NWK_INTERNAL_NSDU_HANDLE;
      }
    }

    if (nent)
    {
      /* unicast packet */
      zb_mcps_data_req_params_t *data_req = ZB_GET_BUF_PARAM(buf, zb_mcps_data_req_params_t);

      TRACE_MSG(TRACE_NWK2, "unicast transmition", (FMT__0));

      zb_address_short_by_ref(&data_req->dst_addr, nent->addr_ref);
      TRACE_MSG(TRACE_NWK2, "addr %d", (FMT__D, data_req->dst_addr));

      /* unicast packet */
      data_req->tx_options |= (MAC_TX_OPTION_INDIRECT_TRANSMISSION_BIT|MAC_TX_OPTION_ACKNOWLEDGED_BIT);
      ZB_SCHEDULE_CALLBACK(zb_mcps_data_request, ZB_REF_FROM_BUF(buf));
    }
    else
    {
      /* broadcast packet */
      TRACE_MSG(TRACE_NWK2, "broadcast transmition", (FMT__0));
      ZB_SCHEDULE_CALLBACK(zb_mcps_data_request, ZB_REF_FROM_BUF(buf));
    }

  }
  else if (param)
  {
    /* free unnecessary buffer, could it ever happen? */
    zb_free_buf(ZB_BUF_FROM_REF(param));
    param = 0;
  }


  /* schedule next execution */
  if ( ZG->nwk.handle.brrt_cnt )
  {
    if ( ent )
    {
      ZB_SCHEDULE_CALLBACK(nwk_broadcast_transmition, 0);
    }
    else
    {
      zb_time_t next_execution = ZB_TIMER_GET() + ZB_NWK_MAX_BROADCAST_JITTER_INTERVAL;

      /* calc next execution time */
      for (i = 0; i < ZB_NWK_BRR_TABLE_SIZE; i++)
      {
        if ( ZG->nwk.handle.brrt[i].used
             && ZG->nwk.handle.brrt[i].retries < ZB_NWK_MAX_BROADCAST_RETRIES )
        {
          if ( ZG->nwk.handle.brrt[i].next_retransmit < next_execution )
          {
            next_execution = ZG->nwk.handle.brrt[i].next_retransmit;
          }
        }
      }
      TRACE_MSG(TRACE_NWK1, "schedule next retransmition at %d", (FMT__D, next_execution));
      ZB_SCHEDULE_ALARM_CANCEL(nwk_broadcast_transmition, 0);
      ZB_SCHEDULE_ALARM(nwk_broadcast_transmition, 0, next_execution - ZB_TIMER_GET());
    }
  }

  done:
  TRACE_MSG(TRACE_NWK1, "<<nwk_broadcast_transmition", (FMT__0));
  return;
}
#endif  /* router role */


/**
   Forward (or send) packet

   Define destination, if it already known - send packet to MAC

   @param param - NWK packet to proceed.
 */
void zb_nwk_forward(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_short_t indirect;
  zb_uint16_t mac_dst;
  zb_buf_t *packet = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nwk_hdr_t *nwhdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(packet);
  zb_uint8_t handle = *ZB_GET_BUF_PARAM(packet, zb_uint8_t);
  zb_nwk_command_status_t cmd_status = ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE;

  TRACE_MSG(TRACE_NWK1, ">>forward parm %hd", (FMT__H, param));

  ret = nwk_calc_destination(packet, handle, &mac_dst, &indirect, &cmd_status);
  if ( ret == RET_OK )
  {
    /* check address is broadcast. */
    if ( ZB_NWK_IS_ADDRESS_BROADCAST(nwhdr->dst_addr) )
    {
      zb_nwk_btr_t *ent;

      /* check if we proceed this broadcast frame already, or add new btt entry */
      NWK_ARRAY_FIND_ENT(ZG->nwk.handle.btt, ent, (ent->source_addr == nwhdr->src_addr) && (ent->sequence_number == nwhdr->seq_num));
      if ( !ent )
      {
        NWK_ARRAY_GET_ENT(ZG->nwk.handle.btt, ent, ZG->nwk.handle.btt_cnt);
        if ( ent )
        {
          ent->source_addr = nwhdr->src_addr;
          ent->sequence_number = nwhdr->seq_num;
          ent->expiration_time = ZB_NWK_BROADCAST_DELIVERY_TIME();
          TRACE_MSG(TRACE_NWK3, "btt add new ent addr %d seq %hd expire t %hd",
                    (FMT__D_H_H, ent->source_addr, ent->sequence_number, ent->expiration_time));
          /* start expiry routine if it was not started before */
          ZB_NWK_START_BTR_EXPIRY();
        }
        else
        {
          TRACE_MSG(TRACE_NWK3, "btt tbl full, drop pkt", (FMT__0));
          ret = RET_ERROR;
          goto done;
        }
      }
    }

    TRACE_MSG(TRACE_NWK1, "mcps data req src %hu dst %hu indirect %hd h %hd", (FMT__H_H_H_H, ZB_PIB_SHORT_ADDRESS(), mac_dst, indirect, handle));
    ZB_MCPS_BUILD_DATA_REQUEST(packet, ZB_PIB_SHORT_ADDRESS(), mac_dst,
                               ((indirect ? MAC_TX_OPTION_INDIRECT_TRANSMISSION_BIT : 0) |
                                /* if not broadcast, wants ack */
                                ((mac_dst != 0xffff) ? MAC_TX_OPTION_ACKNOWLEDGED_BIT : 0x0)),
                               handle);

#ifdef ZB_ROUTER_ROLE
    TRACE_MSG(TRACE_NWK3, "dst %d is_br %hd dev_t %hd brrt_cnt %hd",
              (FMT__D_H_H_D, nwhdr->dst_addr,
               ZB_NWK_IS_ADDRESS_BROADCAST(nwhdr->dst_addr),
               ZB_NIB_DEVICE_TYPE(), ZG->nwk.handle.brrt_cnt));

    if ( ZB_NWK_IS_ADDRESS_BROADCAST(nwhdr->dst_addr)
         && ZB_NIB_DEVICE_TYPE() != (zb_uint8_t)ZB_NWK_DEVICE_TYPE_ED )
    {
      /* begin unicast to children with rx_on_when_idle == false and transmit
       * broadcast frames nwkMaxBroadcastRetries times  */
      if ( ZG->nwk.handle.brrt_cnt < ZB_NWK_BRR_TABLE_SIZE )
      {
        zb_nwk_broadcast_retransmit_t *ent;

        NWK_ARRAY_GET_ENT(ZG->nwk.handle.brrt, ent, ZG->nwk.handle.brrt_cnt);
        ZB_ASSERT(ent);

        ent->buf = param;
        ent->retries = 0;
        ent->src_addr = nwhdr->src_addr;
        ent->dst_addr = nwhdr->dst_addr;
        ent->seq_num = nwhdr->seq_num;
        ent->neighbor_table_iterator = zb_nwk_neighbor_next_ze_children_i(ent->dst_addr, 0);
        TRACE_MSG(TRACE_NWK3, "neighbor_table_iterator %hd", (FMT__H, ent->neighbor_table_iterator));
        /* clear passive ack array */
        ZB_BZERO(ent->passive_ack, sizeof(ent->passive_ack));

        ZB_SCHEDULE_ALARM_CANCEL(nwk_broadcast_transmition, 0);
        /* call without buffer first time: allocate buffer only if it really necessary  */
        ZB_GET_OUT_BUF_DELAYED(nwk_broadcast_transmition);
      }
      else
      {
        TRACE_MSG(TRACE_NWK3, "brrt tbl full, drop pkt", (FMT__0));
        ret = RET_ERROR;
        goto done;
      }
    }
    else
#endif
    {
      ZB_SCHEDULE_CALLBACK(zb_mcps_data_request, ZB_REF_FROM_BUF(packet));
    }

    /* Convert addresses to LE order */
    ZB_NWK_ADDR_TO_LE16(nwhdr->dst_addr);
    ZB_NWK_ADDR_TO_LE16(nwhdr->src_addr);
  }

  done:
  if ( ret != RET_OK
       && ret != RET_BUSY )
  {
#ifdef ZB_ROUTER_ROLE
    zb_neighbor_tbl_ent_t *nbt = NULL;
#endif

    TRACE_MSG(TRACE_NWK1, "cant deliver packet %p err %d", (FMT__P_D, packet, ret));

    /* check if it's our child */
#ifdef ZB_ROUTER_ROLE
    if ( zb_nwk_neighbor_get_by_short(mac_dst, &nbt) == RET_OK
         && nbt
         && (nbt->relationship == ZB_NWK_RELATIONSHIP_CHILD
             || nbt->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD) )
    {
      zb_nlme_status_indication_t *status =  ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_status_indication_t);
      status->status = cmd_status;
      status->network_addr = mac_dst;

      ZB_SCHEDULE_CALLBACK(zb_nlme_status_indication, param);
    }
    else
#endif
    {
      zb_free_buf(packet);
    }
  }
  TRACE_MSG(TRACE_NWK1, "<<forward", (FMT__0));
}


void call_status_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_status_indication_t *status =  ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_status_indication_t);

  TRACE_MSG(TRACE_NWK1, ">>call_status_indication %hd", (FMT__H, param));

  status->status = (zb_nwk_command_status_t)ZG->nwk.handle.status_ind;
  MAC_STATUS_2_NWK_COMMAND_STATUS(status->status);
  status->network_addr = ZG->nwk.handle.status_addr;
  TRACE_MSG(TRACE_NWK1, "status %hd dest_addr %d", (FMT__H_D, status->status, status->network_addr));

  ZB_SCHEDULE_CALLBACK(zb_nlme_status_indication, param);

  TRACE_MSG(TRACE_NWK1, "<<call_status_indication", (FMT__0));
}


void zb_mcps_data_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_mcps_data_confirm_params_t *confirm = ZB_GET_BUF_PARAM(buf, zb_mcps_data_confirm_params_t);

  /*
    This is NWK handler for mcps.confirm.
    Pass mcps.confirm up, to the APS layer.
  */
  TRACE_MSG(TRACE_NWK1, ">>mcps_data_confirm param %hd handle %hd status %hd",
            (FMT__H_H_H, param, confirm->msdu_handle, buf->u.hdr.status));
#ifndef ZB_LIMITED_FEATURES
  if ( confirm->msdu_handle == ZB_NWK_INTERNAL_REJOIN_CMD_HANDLE
       && ZG->nwk.handle.state == ZB_NLME_STATE_REJOIN )
  {
    TRACE_MSG(TRACE_NWK1, "schedule zb_nlme_rejoin_response_timeout %d", (FMT__D, ZB_NWK_REJOIN_TIMEOUT));
    ZB_SCHEDULE_ALARM_CANCEL(zb_nlme_rejoin_response_timeout, ZB_ALARM_ANY_PARAM);
    ZB_SCHEDULE_ALARM(zb_nlme_rejoin_response_timeout, param, ZB_NWK_REJOIN_TIMEOUT);
  }
  else if ( confirm->msdu_handle == ZB_NWK_INTERNAL_REJOIN_CMD_RESPONSE )
  {
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
    zb_nlme_rejoin_resp_sent(param);
#endif
  }
  else if (confirm->msdu_handle == ZB_NWK_INTERNAL_LEAVE_IND_AT_DATA_CONFIRM_HANDLE)
  {
    TRACE_MSG(TRACE_NWK3, "schedule leave.indication", (FMT__0));
    zb_nwk_call_leave_ind(param, ZG->nwk.leave_context.rejoin_after_leave, (zb_uint8_t)-1);
  }
  else
#endif
  {
    /* Get mac address */
    zb_uint16_t dest_addr;
    zb_mac_mhr_t mac_hdr;
    zb_parse_mhr(&mac_hdr, ZB_BUF_BEGIN(buf));
    ZB_LETOH16(&dest_addr, &mac_hdr.dst_addr.addr_short);
#ifndef ZB_LIMITED_FEATURES
    if (confirm->msdu_handle == ZB_NWK_INTERNAL_LEAVE_CONFIRM_AT_DATA_CONFIRM_HANDLE)
    {
      zb_nlme_leave_confirm_t *lc;

      ZB_BUF_REUSE(buf);
      lc = ZB_GET_BUF_PARAM(buf, zb_nlme_leave_confirm_t);
      lc->status = (zb_nwk_status_t)buf->u.hdr.status;
      if (dest_addr != 0xffff)
      {
        /* if destination was not broadcast, packet has 'request' bit set */
        zb_address_ieee_by_short(dest_addr, lc->device_address);
      }
      else
      {
        ZB_IEEE_ADDR_ZERO(lc->device_address);
      }
      TRACE_MSG(TRACE_NWK3, "schedule leave.confirm", (FMT__0));
      ZB_SCHEDULE_CALLBACK(zb_nlme_leave_confirm, param);
    }
    else
#endif
      if ( confirm->msdu_handle == ZB_NWK_INTERNAL_NSDU_HANDLE )
    {
      if ( buf->u.hdr.status != MAC_SUCCESS )
      {
        /* send status.indication */
        ZG->nwk.handle.status_ind = buf->u.hdr.status;
        ZG->nwk.handle.status_addr = dest_addr;
        TRACE_MSG(TRACE_NWK3, "internal nwk transmission - send nwk status ind", (FMT__0));
        call_status_indication(param);
      }
      else
      {
        TRACE_MSG(TRACE_NWK3, "free confirm to internal nwk transmission", (FMT__0));
        zb_free_buf(buf);
      }
    }
    else
    {
      TRACE_MSG(TRACE_NWK3, "schedule nlde-data.confirm", (FMT__0));
      ZB_SCHEDULE_CALLBACK(zb_nlde_data_confirm, param);

      if ( buf->u.hdr.status != MAC_SUCCESS )
      {
        /* FIXME: we could rewrite status here and send wrong indication */
        /* send status.indication */
        ZG->nwk.handle.status_ind = buf->u.hdr.status;
        ZG->nwk.handle.status_addr = dest_addr;
        ZB_GET_IN_BUF_DELAYED(call_status_indication);
      }
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<mcps_data_confirm", (FMT__0));
}


#ifdef ZB_ROUTER_ROLE
void new_buffer_allocated(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>new_buffer_allocated %hd", (FMT__H, param));

  if ( param != 0
       && ZG->nwk.handle.wait_alloc_cnt )
  {
    zb_nwk_buf_alloc_wait_t *ent = &ZG->nwk.handle.wait_alloc[ZG->nwk.handle.wait_alloc_cnt - 1];

    switch ( (zb_nwk_wait_reason_t)ent->wait )
    {
      case ZB_NWK_WAIT_REASON_IND_AND_RETRANSMIT:
      {
        /* dup buffer  */
        ZB_BUF_COPY(ZB_BUF_FROM_REF(param), ZB_BUF_FROM_REF(ent->buf));
        /*
          Encrypt forwarded by the same nwk key source packet was encrypted by.
          To be used, for instance, to encrypt forwarded switch-key aps command.
         */
#ifdef ZB_SECURITY
        TRACE_MSG(TRACE_SECUR3, "will use same key for secure", (FMT__0));
        ZB_BUF_FROM_REF(ent->buf)->u.hdr.use_same_key = 1;
#endif

        /* call frame indication */
        ZB_SCHEDULE_CALLBACK(nwk_frame_indication, param);

        /* call data retransmit */
        ZB_SET_BUF_PARAM(ZB_BUF_FROM_REF(ent->buf), ZB_NWK_INTERNAL_NSDU_HANDLE, zb_uint8_t);
        ZB_SCHEDULE_CALLBACK(zb_nwk_forward, ent->buf);
      }
      break;

      default:
        ZB_ASSERT(0);
        zb_free_buf(ZB_BUF_FROM_REF(param));
        zb_free_buf(ZB_BUF_FROM_REF(ent->buf));
        break;
    }

    NWK_ARRAY_PUT_ENT(ZG->nwk.handle.wait_alloc, ent, ZG->nwk.handle.wait_alloc_cnt);

    /* schedule buf allocation if necessary */
    if ( ZG->nwk.handle.wait_alloc_cnt )
    {
      ZB_GET_IN_BUF_DELAYED(new_buffer_allocated);
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<new_buffer_allocated", (FMT__0));
}
#endif  /* ZB_ROUTER_ROLE */


void zb_mcps_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nwk_hdr_t *nwk_hdr = NULL;
  zb_mac_mhr_t mac_hdr;
  zb_uint8_t frame_type = 0;
  zb_ushort_t hdr_size;
  zb_ushort_t mhr_size;
  zb_uint16_t dst_addr;
  zb_uint16_t src_addr;
  zb_bool_t indicate = ZB_FALSE;
#ifdef ZB_ROUTER_ROLE
  zb_bool_t retransmit = ZB_FALSE;
#endif
  zb_uint8_t lqi;

  TRACE_MSG(TRACE_NWK2, ">>zb_mcps_data_indication param %hd", (FMT__H, param));

  /* get link quality */
  lqi = ZB_MAC_GET_LQI(buf);
  TRACE_MSG(TRACE_NWK1, "lqi %hd", (FMT__H, lqi));

  /* parse and remove MAC header */
  mhr_size = zb_parse_mhr(&mac_hdr, ZB_BUF_BEGIN(buf));
  ZB_MAC_CUT_HDR(buf, mhr_size, nwk_hdr);


  /* TODO: if frame is from non-joined device, drop it (how to check it??) */

  hdr_size = ZB_NWK_HDR_SIZE((nwk_hdr)->frame_control);
  frame_type = ZB_NWK_FRAMECTL_GET_FRAME_TYPE(nwk_hdr->frame_control);

  /* check frame consistency */
  TRACE_MSG(TRACE_NWK3, "hdr_size %d radius %hd frame type %hd", (FMT__D_H_H, hdr_size, nwk_hdr->radius, frame_type));
  if ( nwk_hdr->radius == 0
       || ( frame_type != ZB_NWK_FRAME_TYPE_COMMAND
            && frame_type != ZB_NWK_FRAME_TYPE_DATA ))
  {
    TRACE_MSG(TRACE_NWK1, "drop bad packet", (FMT__0));
    zb_free_buf(buf);
    goto done;
  }

  /* addr is in little endian. Can't convert it before unsecure: can't
   * change nwk header. */
  ZB_LETOH16(&dst_addr, &nwk_hdr->dst_addr);
  ZB_LETOH16(&src_addr, &nwk_hdr->src_addr);

  /* See 3.6.5 Skip already precessed broadcast packets and not for us packets */
  if ( ZB_NWK_IS_ADDRESS_BROADCAST(dst_addr) )
  {
    TRACE_MSG(TRACE_NWK3, "broadc addr 0x%x", (FMT__D, dst_addr));

    if ( !(dst_addr == ZB_NWK_BROADCAST_ALL_DEVICES
           || (dst_addr == ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE
               && ZB_PIB_RX_ON_WHEN_IDLE())
#ifdef ZB_ROUTER_ROLE
           || (dst_addr == ZB_NWK_BROADCAST_ROUTER_COORDINATOR && (!ZG->nwk.handle.joined_pro))
           || (dst_addr == ZB_NWK_BROADCAST_LOW_POWER_ROUTER && (!ZG->nwk.handle.joined_pro) )
#endif
           )
      )
    {
      TRACE_MSG(TRACE_NWK1, "unsupported broadc pkt 0x%x - drop", (FMT__D, dst_addr));
      zb_free_buf(buf);
      goto done;
    }

    {
      zb_nwk_btr_t *ent;

#ifdef ZB_ROUTER_ROLE
if (!ZG->nwk.handle.joined_pro)
      {
        /* mark passive ack for this neighbor */
        zb_nwk_broadcast_retransmit_t *brrt_ent;
        zb_neighbor_tbl_ent_t *nbt;
        zb_uint8_t n;

        NWK_ARRAY_FIND_ENT(ZG->nwk.handle.brrt, brrt_ent, (brrt_ent->src_addr == src_addr) && (brrt_ent->seq_num == nwk_hdr->seq_num));
        if ( brrt_ent
             && ZB_FCF_GET_SRC_ADDRESSING_MODE(mac_hdr.frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST
             && zb_nwk_neighbor_get_by_short(mac_hdr.src_addr.addr_short, &nbt) == RET_OK )
        {
          zb_uint8_t passive_ack_dev_num;

          TRACE_MSG(TRACE_NWK3, "mark neighbor %d as got passive ack for brdcst source %d seq %hd", (FMT__D_D_H, mac_hdr.src_addr.addr_short, src_addr, nwk_hdr->seq_num));
          ZB_SET_BIT_IN_BIT_VECTOR(brrt_ent->passive_ack, ZB_NWK_NEIGHBOR_GET_INDEX_BY_ADDRESS(nbt));

          ZB_NWK_GET_PASSIVE_ACK_DEV_NUM(passive_ack_dev_num);

          ZB_CALC_NONE_ZERO_BITS_IN_BIT_VECTOR(brrt_ent->passive_ack, ZB_NWK_BRCST_PASSIVE_ACK_ARRAY_SIZE, n);
          TRACE_MSG(TRACE_NWK2, "number of got passive acks %hd total %hd", (FMT__H_H, n, passive_ack_dev_num));
          if (
            /* if this packet is from us, we should have exactly neighbor size
             * number acks */
            ((unsigned)n == passive_ack_dev_num
             && brrt_ent->src_addr == ZB_PIB_SHORT_ADDRESS())
            /* if this packet is from our neighbor, we should have neighbor size-1
             * acks */
            || (n == passive_ack_dev_num - 1
                && brrt_ent->src_addr != ZB_PIB_SHORT_ADDRESS()) )
          {
            /* clear entrie */
            TRACE_MSG(TRACE_NWK2, "got all passive acks, drop retransmition entrie, call confirm", (FMT__0));
            /* Not need to transmit broadcast more. Must call confirm now. */
            ZB_SCHEDULE_CALLBACK(zb_nwk_call_br_confirm, brrt_ent->buf);
            NWK_ARRAY_PUT_ENT(ZG->nwk.handle.brrt, brrt_ent, ZG->nwk.handle.brrt_cnt);
          }
        }
      }
#endif

      /* check if we proceed this broadcast frame already */
      NWK_ARRAY_FIND_ENT(ZG->nwk.handle.btt, ent, (ent->source_addr == src_addr) && (ent->sequence_number == nwk_hdr->seq_num));
      if ( ent )
      {
        TRACE_MSG(TRACE_NWK1, "brc from %d to 0x%x seq %hd already proceed - drop", (FMT__D_H, src_addr, dst_addr, nwk_hdr->seq_num));
        zb_free_buf(buf);
        goto done;
      }
      else
      {
        NWK_ARRAY_GET_ENT(ZG->nwk.handle.btt, ent, ZG->nwk.handle.btt_cnt);
        if ( ent )
        {
          ent->source_addr = src_addr;
          ent->sequence_number = nwk_hdr->seq_num;
          ent->expiration_time = ZB_NWK_BROADCAST_DELIVERY_TIME();
          TRACE_MSG(TRACE_NWK3, "btt add ent addr %d seq %hd exp t %hd", (FMT__D_H_H,
                                 ent->source_addr, ent->sequence_number, ent->expiration_time));

          /* start expiry routine if it was not started before */
          ZB_NWK_START_BTR_EXPIRY();
        }
        else
        {
          TRACE_MSG(TRACE_NWK1, "btt tbl full - drop", (FMT__0));
          zb_free_buf(buf);
          goto done;
        }
      }
    }
  }

#ifdef ZB_SECURITY
  /* decrypt now, before nwk header modification */
  if ( ZB_NWK_FRAMECTL_GET_SECURITY(nwk_hdr->frame_control)
       && (frame_type == ZB_NWK_FRAME_TYPE_DATA || frame_type == ZB_NWK_FRAME_TYPE_COMMAND) )
  {
    if (zb_nwk_unsecure_frame(param, &mac_hdr, mhr_size) != RET_OK)
    {
      /* Do not free the buffer: it used to send NWK status indication */
      TRACE_MSG(TRACE_NWK1, "pkt unsecure failed - drop", (FMT__0));
      goto done;
    }
    else
    {
      /* Set flag to secure this frame before send if it will be forwarded */
      buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
      /* unsecure frame does left alloc and can move data in the
       * frame. Reassign nwk header. */
      nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(buf);
    }
  }
#endif  /* security */

  /* Convert addresses to host order */
  nwk_hdr->dst_addr = dst_addr;
  nwk_hdr->src_addr = src_addr;
  TRACE_MSG(TRACE_NWK1, "dst addr mac %d nwk %d, src addr mac %d nwk %d",
            (FMT__D_D_D_D, mac_hdr.dst_addr.addr_short, nwk_hdr->dst_addr,
             mac_hdr.src_addr.addr_short, nwk_hdr->src_addr));

  /* decrement radius after unsecure */
  nwk_hdr->radius--;
  TRACE_MSG(TRACE_NWK3, "new radius %hd, frame type %hd",
            (FMT__H_H, nwk_hdr->radius, ZB_NWK_FRAMECTL_GET_FRAME_TYPE(nwk_hdr->frame_control)));

  /* Decide what we should do with this frame */
#ifdef ZB_ROUTER_ROLE
  if ( ZG->nwk.nib.device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR
       || ZG->nwk.nib.device_type == ZB_NWK_DEVICE_TYPE_ROUTER )
  {
    if ( ZB_NWK_IS_ADDRESS_BROADCAST(nwk_hdr->dst_addr))
    {
      zb_uint8_t command_id = ZB_NWK_CMD_FRAME_GET_CMD_ID(buf, hdr_size);

      indicate = ZB_TRUE;
      if ( nwk_hdr->radius /* Radius is enough to retransmit */
           && (frame_type == ZB_NWK_FRAME_TYPE_DATA /* All brd data frames
                                                     * should be retranmited */
               || (frame_type == ZB_NWK_FRAME_TYPE_COMMAND /* and some type
                                                            * of commands */
                   && (command_id == ZB_NWK_CMD_NETWORK_REPORT
                       || command_id == ZB_NWK_CMD_NETWORK_STATUS
                       || command_id == ZB_NWK_CMD_NETWORK_UPDATE))))
      {
        retransmit = ZB_TRUE;
      }
    }
    else if ( nwk_hdr->dst_addr == ZB_NIB_NETWORK_ADDRESS() )
    {
      indicate = ZB_TRUE;
    }
    else if ( nwk_hdr->radius )
    {
      retransmit = ZB_TRUE;
    }
  }
  else
#endif  /* ZB_ROUTER_ROLE */
  {
    if ( nwk_hdr->dst_addr == ZB_NIB_NETWORK_ADDRESS()
         || ZB_NWK_IS_ADDRESS_BROADCAST(nwk_hdr->dst_addr) )
    {
      indicate = ZB_TRUE;
    }
  }

  /* update neighbor lqi */
  {
    zb_neighbor_tbl_ent_t *nbt;

    if ( zb_nwk_neighbor_get_by_short(nwk_hdr->src_addr, &nbt) == RET_OK )
    {
#define LQI_TEST
#ifdef ZB_NS_BUILD
#ifdef LQI_TEST
      lqi = ( nwk_hdr->src_addr*10 > 255 ) ? 255 : nwk_hdr->src_addr*10;
#endif
#endif
      nbt->lqi = lqi;
      TRACE_MSG(TRACE_NWK3, "update lqi for %d to %hd", (FMT__D_H, nwk_hdr->src_addr, lqi));
    }
  }

#ifdef ZB_ROUTER_ROLE
  if ( retransmit
       && indicate )
  {
    zb_nwk_buf_alloc_wait_t *alloc_wait = NULL;
    NWK_ARRAY_GET_ENT(ZG->nwk.handle.wait_alloc, alloc_wait, ZG->nwk.handle.wait_alloc_cnt);
    if ( alloc_wait )
    {
      alloc_wait->buf = param;
      alloc_wait->wait = ZB_NWK_WAIT_REASON_IND_AND_RETRANSMIT;

      /* schedule buff allocation  */
      ZB_GET_IN_BUF_DELAYED(new_buffer_allocated);
      goto done;
    }
    else
    {
      ZB_SCHEDULE_CALLBACK(nwk_frame_indication, param);
    }
  }
  else if ( retransmit )
  {
    /* this packet is not for us, but try to forward it */
    ZB_SET_BUF_PARAM(buf, ZB_NWK_INTERNAL_NSDU_HANDLE, zb_uint8_t);
    ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);
  }
  else
#endif
    if ( indicate )
  {
    ZB_SCHEDULE_CALLBACK(nwk_frame_indication, param);
  }
  else
  {
    /* this frame is not for us, drop it */
    TRACE_MSG(TRACE_NWK1, "drop", (FMT__0));
    zb_free_buf(buf);
    goto done;
  }

  done:
  TRACE_MSG(TRACE_NWK2, "<<mcps_data_ind", (FMT__0));
}


/**
   Call confirm for nwk frame passive acked before all retransmits done.

   Add MAC hdr to the frame, then call mcps_data_confirm
 */
void zb_nwk_call_br_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *p;
  zb_ushort_t hlen;
  zb_mac_mhr_t mhr;

#if ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME != 0
  ZB_BUF_ALLOC_RIGHT(buf, ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME, p);
#endif

  /* fill 'fake' mhr. Really, only src and dst addresses used by higher
   * levels. Don't care about oithers. */
  ZB_BZERO(&mhr, sizeof(mhr));
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  /* this is broadcast - send from me to ffff */
  mhr.dst_addr.addr_short = ZB_NWK_BROADCAST_ALL_DEVICES;
  mhr.src_addr.addr_short = ZB_NIB_NETWORK_ADDRESS();
  hlen = zb_mac_calculate_mhr_length(
    ZB_ADDR_16BIT_DEV_OR_BROADCAST,
    ZB_ADDR_16BIT_DEV_OR_BROADCAST,
    1);
  ZB_BUF_ALLOC_LEFT(buf, hlen, p);
  zb_mac_fill_mhr(p, &mhr);

  zb_mcps_data_confirm(param);
}


void nwk_frame_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nwk_hdr_t *nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(buf);
  zb_uint8_t frame_type = ZB_NWK_FRAMECTL_GET_FRAME_TYPE(nwk_hdr->frame_control);
  zb_uint8_t hdr_size = ZB_NWK_HDR_SIZE((nwk_hdr)->frame_control);

  TRACE_MSG(TRACE_NWK1, ">> nwk_frame_indication %hd", (FMT__H, param));

  /* handle data/command frames */
  TRACE_MSG(TRACE_NWK1, "frame type %hd", (FMT__H, frame_type));

  if (frame_type == ZB_NWK_FRAME_TYPE_DATA)
  {
    ZB_SCHEDULE_CALLBACK(zb_nlde_data_indication, param);
  }
  else if (frame_type == ZB_NWK_FRAME_TYPE_COMMAND)
  {
    zb_uint8_t command_id = ZB_NWK_CMD_FRAME_GET_CMD_ID(buf, hdr_size);

    TRACE_MSG(TRACE_NWK3, "command_id %hd", (FMT__H, command_id));
#ifndef ZB_LIMITED_FEATURES
    if (command_id == ZB_NWK_CMD_LEAVE)
    {
      /* leave payload is one byte */
      zb_uint8_t *leave_pl = (zb_uint8_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);

      TRACE_MSG(TRACE_NWK3, "got leave cmd", (FMT__0));
      zb_nwk_leave_handler(param, nwk_hdr, *leave_pl);
    }
#else
    if (0)
    {
    }
#endif
#ifdef ZB_ROUTER_ROLE
#ifdef ZB_NWK_MESH_ROUTING
    else if ((command_id == ZB_NWK_CMD_ROUTE_REQUEST) && (!ZG->nwk.handle.joined_pro))
    {
      zb_nwk_cmd_rreq_t *rreq = (zb_nwk_cmd_rreq_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);

      TRACE_MSG(TRACE_NWK3, "got r req cmd", (FMT__0));
      ZB_NWK_ADDR_TO_LE16(rreq->dest_addr);
      zb_nwk_mesh_rreq_handler(buf, nwk_hdr, rreq);
    }
    else if ((command_id == ZB_NWK_CMD_ROUTE_REPLY) && (!ZG->nwk.handle.joined_pro))
    {
      zb_nwk_cmd_rrep_t *rrep = (zb_nwk_cmd_rrep_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);

      TRACE_MSG(TRACE_NWK3, "got r repl cmd", (FMT__0));
      zb_nwk_mesh_rrep_handler(buf, nwk_hdr, rrep);
    }
    else if ((command_id == ZB_NWK_CMD_REJOIN_REQUEST) && (!ZG->nwk.handle.joined_pro))
    {
      TRACE_MSG(TRACE_NWK3, "got rejoin request cmd", (FMT__0));
      zb_nlme_rejoin_request(param);
    }
#endif /* ZB_NWK_MESH_ROUTING */
    else if (command_id == ZB_NWK_CMD_NETWORK_REPORT)
    {
      zb_nwk_report_cmd_t *report = (zb_nwk_report_cmd_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);

      TRACE_MSG(TRACE_NWK3, "got network report cmd", (FMT__0));
      if (!ZB_NWK_REPORT_IS_PANID_CONFLICT(report->command_options)
          || !ZB_EXTPANID_CMP(report->epid, ZB_NIB_EXT_PAN_ID()))
      {
        TRACE_MSG(TRACE_ERROR, "drop report cmd %hd", (FMT__H, ZB_NWK_REPORT_COMMAND_ID(report->command_options)));
        zb_free_buf(buf);
      }
      else
      {
        zb_panid_conflict_got_network_report(param, report->panids, ZB_NWK_REPORT_INFO_COUNT(report->command_options));
      }
    }
    else if (command_id == ZB_NWK_CMD_NETWORK_STATUS)
    {
      zb_nlme_status_indication_t *cmd = (zb_nlme_status_indication_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);
      zb_nwk_command_status_t status = cmd->status;
      zb_uint16_t network_addr = cmd->network_addr;

      TRACE_MSG(TRACE_NWK3, "got network status cmd", (FMT__0));
      /* notify higher layers */
      cmd = ZB_GET_BUF_PARAM(buf, zb_nlme_status_indication_t);
      cmd->status = status;
      cmd->network_addr = network_addr;
      ZB_SCHEDULE_CALLBACK(zb_nlme_status_indication, param);
    }
#endif /* ZB_ROUTER_ROLE */
#ifndef ZB_LIMITED_FEATURES
    else if (command_id == ZB_NWK_CMD_REJOIN_RESPONSE)
    {
      TRACE_MSG(TRACE_NWK3, "got rejoin response cmd", (FMT__0));
      zb_nlme_rejoin_response(param);
    }
    else if (command_id == ZB_NWK_CMD_NETWORK_UPDATE)
    {
      zb_nwk_update_cmd_t *upd = (zb_nwk_update_cmd_t *)ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, hdr_size);
      if ( !ZB_NWK_REPORT_IS_PANID_CONFLICT(upd->command_options)
           || !ZB_EXTPANID_CMP(upd->epid, ZB_NIB_EXT_PAN_ID()) )
      {
        TRACE_MSG(TRACE_ERROR, "drop nwk update cmd %hd", (FMT__H, ZB_NWK_REPORT_COMMAND_ID(upd->command_options)));
        zb_free_buf(buf);
      }
      else
      {
        TRACE_MSG(TRACE_NWK3, "received NWK update cmd", (FMT__0));
        zb_panid_conflict_network_update_recv(upd);
      }
    }
    else
#endif  /* ZB_LIMITED_FEATURES */
    {
      TRACE_MSG(TRACE_ERROR, "unknown cmd %hd - drop", (FMT__H, command_id));
      zb_free_buf(buf);
    }
  }
}

void zb_nwk_btr_expiry(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_NWK1, ">>nwk_btr_expiry %hd", (FMT__H, param));

  if ( ZG->nwk.handle.btt_cnt )
  {
    zb_ushort_t i;

    TRACE_MSG(TRACE_NWK1, "btt cnt %hd", (FMT__H, ZG->nwk.handle.btt_cnt));
    for (i = 0; i < ZB_NWK_BTR_TABLE_SIZE; i++)
    {
      if ( ZG->nwk.handle.btt[i].used )
      {
        if ( ZG->nwk.handle.btt[i].expiration_time )
        {
          ZG->nwk.handle.btt[i].expiration_time--;
        }
        else
        {
          NWK_ARRAY_PUT_ENT(ZG->nwk.handle.btt, &ZG->nwk.handle.btt[i], ZG->nwk.handle.btt_cnt);
        }
      }
    }

    /* Schedule to call later */
    ZB_SCHEDULE_ALARM(zb_nwk_btr_expiry, 0, ZB_TIME_ONE_SECOND);
  }
  else
  {
    /* do no schedule this function, cause we don't have btr's . */
  }

  TRACE_MSG(TRACE_NWK1, "<<nwk_btr_expiry", (FMT__0));
}


#ifndef ZB_LIMITED_FEATURES
/**
   Handle LEAVE command

   Handle LEAVE packet got from the net.

   This routine always free the packet.

   @param packet - incoming packet
   @param nwk_header - already parsed network header
   @param lp - LEAVE command payload (1 byte)
 */
static void zb_nwk_leave_handler(zb_uint8_t param, zb_nwk_hdr_t *nwk_hdr, zb_uint8_t lp) ZB_SDCC_REENTRANT
{
  zb_ret_t    status = RET_OK;
  zb_address_ieee_ref_t addr_ref;
  zb_neighbor_tbl_ent_t *nbt = NULL;
  zb_buf_t *packet = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK3, "+zb_nwk_leave_handler from %d request %hd remove_ch %hd rejoin %hd",
            (FMT__D_H_H_H, nwk_hdr->src_addr,
             ZB_LEAVE_PL_GET_REQUEST(lp), ZB_LEAVE_PL_GET_REMOVE_CHILDREN(lp),
             ZB_LEAVE_PL_GET_REJOIN(lp)));

  status = zb_address_by_short(nwk_hdr->src_addr, ZB_FALSE, ZB_FALSE, &addr_ref);
  if (status == RET_OK)
  {
    if (zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nbt) != RET_OK)
    {
#ifdef ZB_ROUTER_ROLE
      TRACE_MSG(TRACE_NWK2, "incoming LEAVE not from the neighbor", (FMT__0));
      if (!ZB_LEAVE_PL_GET_REQUEST(lp))
      {
        TRACE_MSG(TRACE_NWK2, "delete address", (FMT__0));
        zb_address_delete(addr_ref);
      }
#endif
      status = RET_ERROR;
    }
  }
  if (status == RET_OK
      && nbt->relationship != ZB_NWK_RELATIONSHIP_PARENT
      && (ZB_LEAVE_PL_GET_REQUEST(lp)
#ifdef ZB_ROUTER_ROLE
          || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ED
#endif
        ))
  {
    /* drop */
    TRACE_MSG(TRACE_NWK1, "REQUEST not from the parent or to ED - drop", (FMT__0));
    status = RET_ERROR;
  }
  if (status == RET_OK)
  {
    if (nbt->relationship == ZB_NWK_RELATIONSHIP_PARENT)
    {
      /* If parent has left, pass up leave.indication. Need a buffer for it. */
      if (!ZB_LEAVE_PL_GET_REQUEST(lp))
      {
        ZG->nwk.leave_context.leave_ind_prnt.addr_ref = addr_ref;
        ZG->nwk.leave_context.leave_ind_prnt.rejoin = ZB_LEAVE_PL_GET_REJOIN(lp);
        zb_get_in_buf_delayed(zb_nwk_leave_ind_prnt);
      }

#ifdef ZB_ROUTER_ROLE
      if ((ZB_LEAVE_PL_GET_REQUEST(lp) || ZB_LEAVE_PL_GET_REMOVE_CHILDREN(lp))
          && ZB_NIB_DEVICE_TYPE() != (zb_uint8_t)ZB_NWK_DEVICE_TYPE_ED)
      {
        /* must remove ourself */
        /* send broadcast LEAVE indicating we are leaving */
#ifdef ZB_SECURITY
        zb_bool_t secure = (ZG->nwk.nib.secure_all_frames && ZG->nwk.nib.security_level && secur_has_preconfigured_key());
#else
        zb_bool_t secure = ZB_FALSE;
#endif
        zb_uint8_t *new_lp;
        zb_nwk_hdr_t *nwhdr;

        nwhdr = nwk_alloc_and_fill_hdr(packet,
                                       ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE,
                                       ZB_PIB_EXTENDED_ADDRESS(),
                                       NULL,
                                       ZB_FALSE,
                                       secure,
                                       ZB_TRUE);
        nwhdr->radius = 1;
#ifdef ZB_SECURITY
        if (secure)
        {
          packet->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
        }
#endif
        new_lp = (zb_uint8_t *)nwk_alloc_and_fill_cmd(packet, ZB_NWK_CMD_LEAVE,
                                                      sizeof(zb_uint8_t));
        *new_lp = 0;
        ZB_LEAVE_PL_SET_REJOIN(*new_lp, ZB_LEAVE_PL_GET_REJOIN(lp));
        ZB_LEAVE_PL_SET_REMOVE_CHILDREN(*new_lp, ZB_LEAVE_PL_GET_REMOVE_CHILDREN(lp));
        /* When will got data.confirm at this request, do actual leave */
        ZG->nwk.leave_context.rejoin_after_leave = ZB_LEAVE_PL_GET_REJOIN(lp);
        ZB_SET_BUF_PARAM(packet, ZB_NWK_INTERNAL_LEAVE_IND_AT_DATA_CONFIRM_HANDLE, zb_uint8_t);
        TRACE_MSG(TRACE_NWK2, "send LEAVE with request 0, rejoin %hd, remove_children %hd",
                  (FMT__H_H, ZB_LEAVE_PL_GET_REJOIN(lp), ZB_LEAVE_PL_GET_REMOVE_CHILDREN(lp)));
        ZB_SCHEDULE_CALLBACK(zb_nwk_forward, ZB_REF_FROM_BUF(packet));
      }
      else
#endif  /* router */
        /*
          Forget the parent - he has leaved.

          But, how will we work after that, if 'remove children' is not set???
          Spec says ED must leave also, but what about router?
          What child must do after paret leaved?? Has it a parent after that?
         */
#ifdef ZB_ROUTER_ROLE
      if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ED)
#endif
      {
        /* ZED silently leave */
        TRACE_MSG(TRACE_NWK2, "ZED - silently leave", (FMT__0));
        zb_nwk_call_leave_ind(param, ZB_LEAVE_PL_GET_REJOIN(lp), (zb_address_ieee_ref_t)-1);
      }
#ifdef ZB_ROUTER_ROLE
      else
      {
        /* forget our parent (hmm?) */
        TRACE_MSG(TRACE_NWK2, "our parent has left - hmm?", (FMT__0));
        zb_nwk_call_leave_ind(param, ZB_LEAVE_PL_GET_REJOIN(lp), addr_ref);
      }
#endif
    }
    else      /* not from the parent */
    {
      TRACE_MSG(TRACE_NWK2, "LEAVE not from parent", (FMT__0));
      zb_nwk_call_leave_ind(param, ZB_LEAVE_PL_GET_REJOIN(lp), addr_ref);
    }
  }
  else
  {
    zb_free_buf(packet);
  }

  TRACE_MSG(TRACE_NWK3, "-zb_nwk_leave_handler", (FMT__0));
}


void zb_nwk_leave_ind_prnt(zb_uint8_t param) ZB_CALLBACK
{
  zb_nwk_call_leave_ind(param, ZG->nwk.leave_context.leave_ind_prnt.rejoin,
                        ZG->nwk.leave_context.leave_ind_prnt.addr_ref);
}

static void zb_nwk_call_leave_ind(zb_uint8_t param, zb_uint8_t rejoin, zb_address_ieee_ref_t addr_ref)
{
  zb_nlme_leave_indication_t *request = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_leave_indication_t);

  if (addr_ref == (zb_address_ieee_ref_t)-1)
  {
    ZB_IEEE_ADDR_ZERO(request->device_address);
  }
  else
  {
    zb_address_ieee_by_ref(request->device_address, addr_ref);
  }
  request->rejoin = rejoin;
  ZB_SCHEDULE_CALLBACK(zb_nlme_leave_indication, param);
}


void zb_nwk_forget_device(zb_address_ieee_ref_t addr_ref)
{
  zb_nwk_neighbor_delete(addr_ref);
  zb_address_delete(addr_ref);
}

/**
   Do actual leave, potentially followed by rejoin

   @param param - buffer to be used for rejoin
   @param rejoin - rejoin flag
 */
void zb_nwk_do_leave(zb_uint8_t param, zb_uint8_t rejoin) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, "zb_nwk_do_leave ", (FMT__0));
  ZG->nwk.leave_context.leave_after_mgmt_leave_rsp_conf = 0;
  ZG->zdo.handle.started = 0;
  ZG->nwk.handle.joined = 0;
  ZG->nwk.handle.router_started = 0;
  ZG->nwk.leave_context.pending_list_size = 0;
  ZG->nwk.handle.btt_cnt = 0;
  ZG->nwk.handle.parent = (zb_address_ieee_ref_t)-1;
#ifdef ZB_ROUTER_ROLE
  ZG->nwk.handle.brrt_cnt = 0;
#endif
  ZG->nwk.handle.rejoin_req_table_cnt = 0;

  ZB_BUF_REUSE(buf);
  if (rejoin)
  {
    zb_nlme_join_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_join_request_t);

    ZB_BZERO(req, sizeof(*req)); /* all defaults to 0 */

    /* join to the same PAN */
    ZB_EXTPANID_COPY(req->extended_pan_id, ZB_NIB_EXT_PAN_ID());
#ifdef ZB_ROUTER_ROLE
    if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)
    {
      ZB_MAC_CAP_SET_ROUTER_CAPS(req->capability_information); /* join as ZR */
      TRACE_MSG(TRACE_NWK1, "Rejoin to pan " TRACE_FORMAT_64 " as ZR", (FMT__A,
                TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
    }
    else
#endif
    {
      TRACE_MSG(TRACE_NWK1, "Rejoin to pan " TRACE_FORMAT_64 " as ZE", (FMT__A,
                TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
    }
    if (MAC_PIB().mac_rx_on_when_idle)
    {
      ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(req->capability_information, 1);
    }
    /* if join as ZE - all cap to 0 (set by memset) */
    ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(req->capability_information, 1);
    req->rejoin_network = ZB_NLME_REJOIN_METHOD_REJOIN;
    req->scan_channels = ZB_AIB().aps_channel_mask;
    req->scan_duration = ZB_DEFAULT_SCAN_DURATION; /* TODO: configure it somehow? */
    ZG->zdo.handle.rejoin = 1;

    ZB_SCHEDULE_CALLBACK(zb_nlme_join_request, ZB_REF_FROM_BUF(buf));
  }
  else
  {
    zb_address_ieee_ref_t addr_ref;
    if (zb_address_by_short(ZB_NIB_NETWORK_ADDRESS(), ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK)
    {
      zb_nwk_forget_device(addr_ref);
    }
    ZB_IEEE_ADDR_ZERO(ZB_NIB_EXT_PAN_ID());
    ZB_NIB_PAN_ID() = (zb_uint16_t)-1;
    ZB_NIB_NETWORK_ADDRESS() = (zb_uint16_t)-1; /* was commented with comment: comment for leave ED device (leave befo mgmt_rsp) */
    zb_free_buf(buf);
    TRACE_MSG(TRACE_NWK1, "LEAVE without rejoin", (FMT__0));
  }
}

#endif  /* ZB_LIMITED_FEATURES */


zb_void_t zb_nwk_set_device_type(zb_nwk_device_type_t device_type)
{
  ZB_NIB_DEVICE_TYPE() = device_type;
#ifdef ZB_ROUTER_ROLE
  if (device_type == ZB_NWK_DEVICE_TYPE_ROUTER || device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR)
  {
    ZB_MAC_SET_DEVICE_TYPE(MAC_DEV_FFD);
  }
  else if (device_type == ZB_NWK_DEVICE_TYPE_NONE)
  {
    ZB_MAC_SET_DEVICE_TYPE(MAC_DEV_UNDEFINED);
  }
  else
#endif
  {
    ZB_MAC_SET_DEVICE_TYPE(MAC_DEV_RFD);
  }
}

zb_ushort_t zb_nwk_hdr_size(zb_uint8_t *fctl)
{
#ifdef ZB_SECURITY
  return (
    ZB_NWK_SHORT_HDR_SIZE(ZB_NWK_FRAMECTL_GET_MULTICAST_FLAG(fctl)) +
    (ZB_NWK_FRAMECTL_GET_SECURITY(fctl) ? sizeof(zb_nwk_aux_frame_hdr_t) : 0) +
    (ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(fctl) + ZB_NWK_FRAMECTL_GET_SOURCE_IEEE(fctl)) * sizeof(zb_ieee_addr_t) +
  0                             /* TODO: add source route subframe here */ \
    );
#else
  return (
    ZB_NWK_SHORT_HDR_SIZE(ZB_NWK_FRAMECTL_GET_MULTICAST_FLAG(fctl)) +
    (ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(fctl) + ZB_NWK_FRAMECTL_GET_SOURCE_IEEE(fctl)) * sizeof(zb_ieee_addr_t) +
  0                             /* TODO: add source route subframe here */
    );
#endif
}


/*! @} */
