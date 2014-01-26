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
PURPOSE: APSME security routines
*/


/*! \addtogroup ZB_SECUR */
/*! @{ */

#include "zb_common.h"
#include "zb_aps.h"
#include "zb_secur.h"

#include "zb_bank_common.h"
#ifdef ZB_SECURITY

void zb_aps_pass_nwk_key_to_children(zb_uint8_t param) ZB_CALLBACK;


#ifdef ZB_ROUTER_ROLE

void zb_apsme_transport_key_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_bool_t secur_transfer = ZB_FALSE;
  zb_uint16_t dest_short = 0;
  {
    zb_apsme_transport_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_req_t);

#ifdef ZB_STACK_PROFILE_2007
    if (req->key_type == ZB_STANDARD_NETWORK_KEY)
    {
      /* nwk key */
      zb_transport_key_nwk_key_dsc_pkt_t *dsc;

      ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(*dsc), dsc);
      dsc->key_type = req->key_type;
      ZB_MEMCPY(dsc->key, req->key.nwk.key, ZB_CCM_KEY_SIZE);
      dsc->seq_number = req->key.nwk.key_seq_number;
      if (req->addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
      {
        ZB_IEEE_ADDR_ZERO(dsc->dest_address);
      }
      else
      {
        ZB_IEEE_ADDR_COPY(dsc->dest_address, req->dest_address.addr_long);
      }
      ZB_IEEE_ADDR_COPY(dsc->source_address, ZB_PIB_EXTENDED_ADDRESS());
      if (req->key.nwk.use_parent)
      {
        dest_short = zb_address_short_by_ieee(req->key.nwk.parent_address);
        /* transfer key securely to the parent, unsecurely to the device */
        secur_transfer = ZB_TRUE;
      }
      else
      {
        if (req->addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
        {
          dest_short = req->dest_address.addr_short;
          /* Broadcast transfer used for key update in the entire network. It
           * is secured. */
          secur_transfer = ZB_NWK_IS_ADDRESS_BROADCAST(dest_short);
        }
        else
        {
          /* transfer to the child is not secured */
          dest_short = zb_address_short_by_ieee(req->dest_address.addr_long);

          /* TODO: handle all-zero destination address: send secured unicast to
           * all children. Seems, not used in 2007 (Standard security). */
        }
        /* TODO: Update Neighbor table entry for the unauthenticated child when
         * command successfully sent */
      }

      TRACE_MSG(TRACE_SECUR3, "send std nwk key #%hd from " TRACE_FORMAT_64,
                (FMT__H_A, dsc->seq_number, TRACE_ARG_64(dsc->source_address)));

    }
    else
    {
      /* link key */
      TRACE_MSG( TRACE_ERROR, "TODO: implement link key!", (FMT__0));
    }
#else
#error Implement PRO!
#endif
  }

  zb_aps_send_command(param,
                      dest_short,
                      APS_CMD_TRANSPORT_KEY, secur_transfer);
}


/**
   Send UPDATE-DEVICE.request from ZR to TC.

   Inform TC that new devive joined network.
   TC must send nwk key to it.
 */
void zb_apsme_update_device_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_update_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_update_device_req_t);
  zb_apsme_update_device_pkt_t *p;

  ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(zb_apsme_update_device_pkt_t), p);

  ZB_IEEE_ADDR_COPY(p->device_address, req->device_address);
  ZB_HTOLE16(&p->device_short_address, &req->device_short_address);
  p->status = req->status;
  TRACE_MSG(TRACE_SECUR3, "send UPDATE-DEVICE " TRACE_FORMAT_64 " %d st %hd to " TRACE_FORMAT_64, (FMT__A_D_H, TRACE_ARG_64(p->device_address),
                          p->device_short_address, p->status, TRACE_ARG_64(req->dest_address)));

  zb_aps_send_command(param,
                      zb_address_short_by_ieee(req->dest_address),
                      APS_CMD_UPDATE_DEVICE, ZB_TRUE);
}


void zb_apsme_switch_key_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_switch_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_switch_key_req_t);
  zb_apsme_switch_key_pkt_t *p;
  zb_uint16_t dest_short;

  ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(*p), p);
  /*
    See 4.4.7.1.3:
    The sequence number field of this command frame shall be set to the same
    value as the KeySeqNumber parameter.

    Its strange, but..
  */
  /*ZB_AIB_APS_COUNTER() = req->key_seq_number;*/
  /* Setting aps counter causes false APS dups! */
  if (req->addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
  {
    TRACE_MSG(TRACE_SECUR1, "zb_apsme_switch_key_req dest %d # %d",
              (FMT__D_D, req->dest_address.addr_short, req->key_seq_number));
    ZB_ASSERT(ZB_NWK_IS_ADDRESS_BROADCAST(req->dest_address.addr_short));
    dest_short = req->dest_address.addr_short;
  }
  else
  {
    dest_short = zb_address_short_by_ieee(req->dest_address.addr_long);
    TRACE_MSG(TRACE_SECUR1, "zb_apsme_switch_key_req dest " TRACE_FORMAT_64 " %d # %d",
              (FMT__A_D_D, TRACE_ARG_64(req->dest_address.addr_long), req->dest_address.addr_short, req->key_seq_number));
  }
  p->key_seq_number = req->key_seq_number;
  zb_aps_send_command(param,
                      dest_short,
                      APS_CMD_SWITCH_KEY, ZB_TRUE);
}


#endif  /* ZB_ROUTER_ROLE */


/**
   Reaction on TRANSPORT-KEY APS command
 */
void zb_aps_in_transport_key(zb_uint8_t param)
{
  zb_transport_key_nwk_key_dsc_pkt_t *dsc =
    (zb_transport_key_nwk_key_dsc_pkt_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));

  TRACE_MSG(TRACE_SECUR3, ">>zb_aps_in_transport_key %d", (FMT__H, param));

  /* See 4.4.3.3  Upon Receipt of a Transport-Key Command */

  switch (dsc->key_type)
  {
    case ZB_STANDARD_NETWORK_KEY:
      if (
        /* key is for me */
        ZB_IEEE_ADDR_CMP(dsc->dest_address, ZB_PIB_EXTENDED_ADDRESS())
        /* key is for all */
          || ZB_IEEE_ADDR_IS_ZERO(dsc->dest_address))
      {
        /* This key is for me. Issue APSME-TRANSPORT-KEY.indication. ZDO will
         * setup keys and remember TC address.
         */
        zb_apsme_transport_key_indication_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_indication_t);

        TRACE_MSG(TRACE_SECUR3, "in std nwk key #%d for me", (FMT__D, dsc->seq_number));
        ind->key_type = dsc->key_type;
        ZB_IEEE_ADDR_COPY(ind->src_address, dsc->source_address);
        ind->key.nwk.key_seq_number = dsc->seq_number;
        ZB_MEMCPY(ind->key.nwk.key, dsc->key, ZB_CCM_KEY_SIZE);
        ZB_SCHEDULE_CALLBACK(zb_apsme_transport_key_indication, param);

/* #ifdef ZB_ROUTER_ROLE */
/* This feature should be processed at request */
#if 0
        if (ZB_IEEE_ADDR_IS_ZERO(dsc->dest_address)
            /* && check for secured transfer at nwk level */
            && ZB_IEEE_ADDR_CMP(dsc->source_address, ZB_AIB().trust_center_address))
        {
          /*
           * Need to pass key to all rx-off-when-idle children. Need another
           * packet buffer for it.
           * Do the rest in the calback: this is blocked
           * buffer alloc. Not need to save current key: it will be aleady
           * assigned, so can send my own key.
           */
          ZG->aps.tmp.neighbor_table_iterator = zb_nwk_neighbor_next_ze_children_rx_off_i(0);
          ZG->aps.tmp.key_seq_number = ind->key.nwk.key_seq_number;
          if (ZG->aps.tmp.neighbor_table_iterator != (zb_ushort_t)~0)
          {
            TRACE_MSG(TRACE_SECUR3, "send key #%hd to all ZE", (FMT__H, dsc->seq_number));
            zb_get_out_buf_delayed(zb_aps_pass_nwk_key_to_children);
          }
        }
#endif  /* ZB_ROUTER_ROLE */
      }
#ifdef ZB_ROUTER_ROLE
      else
      {
        zb_address_ieee_ref_t addr_ref;
        zb_neighbor_tbl_ent_t *nbe;
        /* Search for child in the Neighbor table, mark child as Authenticated,
         * send key to it using unsecured NWK transfer */
        if (zb_address_by_ieee(dsc->dest_address, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK
            && zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nbe) == RET_OK
            && (nbe->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD
                || nbe->relationship == ZB_NWK_RELATIONSHIP_CHILD))
        {
          zb_uint16_t addr;
          zb_address_short_by_ref(&addr, addr_ref);
          TRACE_MSG(TRACE_SECUR3, "send key #%hd to ZE %d, auth ok", (FMT__H_D, dsc->seq_number, addr));
          zb_aps_send_command(param, addr, APS_CMD_TRANSPORT_KEY,
                              (nbe->relationship != ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD));
          nbe->relationship = ZB_NWK_RELATIONSHIP_CHILD;
        }
        else
        {
          TRACE_MSG(TRACE_SECUR1, "child " TRACE_FORMAT_64 " not found", (FMT__A, TRACE_ARG_64(dsc->dest_address)));
          zb_free_buf(ZB_BUF_FROM_REF(param));
        }
      }
#endif  /* ZB_ROUTER_ROLE */
      break;
    default:
      break;
  }

  TRACE_MSG(TRACE_SECUR3, "<<zb_aps_in_transport_key", (FMT__0));
}


#ifdef ZB_ROUTER_ROLE
/**
   Reaction on incoming UPDATE-DEVICE

   Issue UPDATE-DEVICE.indication
 */
void zb_aps_in_update_device(zb_uint8_t param)
{
  /* get source address from the nwk header and convert it to long address */
  zb_apsme_update_device_pkt_t *pkt = (zb_apsme_update_device_pkt_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_apsme_update_device_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_update_device_ind_t);

  TRACE_MSG(TRACE_SECUR3, ">>zb_aps_in_update_device %d", (FMT__H, param));

  {
    zb_uint16_t src_short_addr = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsde_data_indication_t)->src_addr;
    zb_address_ieee_by_short(src_short_addr, ind->src_address);
  }

  ZB_IEEE_ADDR_COPY(ind->device_address, pkt->device_address);
  ZB_LETOH16(&ind->device_short_address, &pkt->device_short_address);
  /* We have short and long addresses of the device UPDATE-DEVICE is
   * about. Remember it. */
  {
    zb_address_ieee_ref_t ref;
    (void)zb_address_update(ind->device_address, ind->device_short_address, ZB_FALSE, &ref);
  }
  ind->status = pkt->status;

  ZB_SCHEDULE_CALLBACK(zb_apsme_update_device_indication, param);

  TRACE_MSG(TRACE_SECUR3, "<<zb_aps_in_update_device", (FMT__0));
}


/**
   Send key to all children with rx_on_when_idle == 0
 */
void zb_aps_pass_nwk_key_to_children(zb_uint8_t param) ZB_CALLBACK
{
  /* Pass our current nwk key to the next child; key source is TC */
  zb_transport_key_nwk_key_dsc_pkt_t *dsc;
  zb_uint16_t dest_short;

  ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(*dsc), dsc);
  dsc->key_type = ZB_STANDARD_NETWORK_KEY;

  ZB_MEMCPY(dsc->key,
            secur_nwk_key_by_seq(ZG->aps.tmp.key_seq_number),
            ZB_CCM_KEY_SIZE);
  dsc->seq_number = ZG->aps.tmp.key_seq_number;

  zb_address_by_ref(dsc->dest_address, &dest_short,
                    ZG->nwk.neighbor.base_neighbor[ZG->aps.tmp.neighbor_table_iterator].addr_ref);

  /* Child is authenticated now (hmm... not sure) */
  ZG->nwk.neighbor.base_neighbor[ZG->aps.tmp.neighbor_table_iterator].relationship = ZB_NWK_RELATIONSHIP_CHILD;

  ZB_IEEE_ADDR_COPY(dsc->source_address, ZB_AIB().trust_center_address);

  TRACE_MSG(TRACE_SECUR3, "send nwk key #%hd to ch %d", (FMT__H_D, dsc->seq_number, dest_short));
  zb_aps_send_command(param,
                      dest_short,
                      APS_CMD_TRANSPORT_KEY, ZB_FALSE);

  /* handle next child */
  ZG->aps.tmp.neighbor_table_iterator = zb_nwk_neighbor_next_ze_children_rx_off_i(ZG->aps.tmp.neighbor_table_iterator + 1);
  if (ZG->aps.tmp.neighbor_table_iterator != (zb_ushort_t)~0)
  {
    zb_get_out_buf_delayed(zb_aps_pass_nwk_key_to_children);
  }
}


void zb_secur_apsme_remove_device(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_remove_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_remove_device_req_t);
  zb_apsme_remove_device_pkt_t *p;

  ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(zb_apsme_remove_device_pkt_t), p);

  ZB_IEEE_ADDR_COPY(p->child_address, req->child_address);
  TRACE_MSG(TRACE_SECUR2, "send REMOVE-DEVICE " TRACE_FORMAT_64 " to " TRACE_FORMAT_64, (FMT__A_A, TRACE_ARG_64(p->child_address),
                          TRACE_ARG_64(req->parent_address)));

  zb_aps_send_command(param,
                      zb_address_short_by_ieee(req->parent_address),
                      APS_CMD_REMOVE_DEVICE, ZB_TRUE);
}


void zb_aps_in_remove_device(zb_uint8_t param)
{
  /* get source address from the nwk header and convert it to long address */
  zb_apsme_remove_device_pkt_t *pkt = (zb_apsme_remove_device_pkt_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_apsme_remove_device_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_remove_device_ind_t);

  TRACE_MSG(TRACE_SECUR3, ">>zb_aps_in_remove_device %d", (FMT__H, param));

  {
    zb_uint16_t src_short_addr = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsde_data_indication_t)->src_addr;
    zb_address_ieee_by_short(src_short_addr, ind->src_address);
  }
  ZB_IEEE_ADDR_COPY(ind->child_address, pkt->child_address);

  ZB_SCHEDULE_CALLBACK(zb_apsme_remove_device_indication, param);

  TRACE_MSG(TRACE_SECUR3, "<<zb_aps_in_remove_device", (FMT__0));
}


void zb_aps_in_request_key(zb_uint8_t param)
{
  /* get source address from the nwk header and convert it to long address */  
  zb_apsme_request_nwk_key_pkt_t *pkt = (zb_apsme_request_nwk_key_pkt_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));  
  zb_apsme_request_key_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_request_key_ind_t);
  TRACE_MSG(TRACE_SECUR2, ">>zb_aps_in_request_key", (FMT__0));
  {
    zb_uint16_t src_short_addr = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsde_data_indication_t)->src_addr;
    zb_address_ieee_by_short(src_short_addr, ind->src_address);
  }
  ind->key_type = pkt->key_type;
  /* TODO: fill partner_address for application key */
  ZB_SCHEDULE_CALLBACK(zb_apsme_request_key_indication, param);

  TRACE_MSG(TRACE_SECUR2, "<<zb_aps_in_request_key", (FMT__0));
}

#endif  /* ZB_ROUTER_ROLE */
void zb_aps_in_switch_key(zb_uint8_t param)
{
  /* get source address from the nwk header and convert it to long address */
  zb_apsme_switch_key_pkt_t *pkt = (zb_apsme_switch_key_pkt_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
  zb_apsme_switch_key_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_switch_key_ind_t);
  {
    zb_uint16_t src_short_addr = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsde_data_indication_t)->src_addr;
    zb_address_ieee_by_short(src_short_addr, ind->src_address);
  }
  ind->key_seq_number = pkt->key_seq_number;
  ZB_SCHEDULE_CALLBACK(zb_apsme_switch_key_indication, param);

  TRACE_MSG(TRACE_SECUR3, "<<zb_aps_in_switch_key", (FMT__0));
}




void zb_secur_apsme_request_key(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_request_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_request_key_req_t);

  if (req->key_type == ZB_SECUR_NWK_KEY)
  {
    zb_apsme_request_nwk_key_pkt_t *p;
    ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(zb_apsme_request_nwk_key_pkt_t), p);
    p->key_type = req->key_type;
  }
  else
  {
    /* TODO: implement application key */
  }

  TRACE_MSG(TRACE_SECUR3, "send REQUEST-KEY to " TRACE_FORMAT_64,
            (FMT__A, TRACE_ARG_64(req->dest_address)));
  zb_aps_send_command(param,
                      zb_address_short_by_ieee(req->dest_address),
                      APS_CMD_REQUEST_KEY, ZB_TRUE);
}


#endif  /* ZB_SECURITY */


/*! @} */
