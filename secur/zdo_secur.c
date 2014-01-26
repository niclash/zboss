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
PURPOSE: Security support in ZDO
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"

#include "zb_bank_common.h"
#ifdef ZB_SECURITY

/*! \addtogroup ZB_SECUR */
/*! @{ */

void zb_secur_rejoin_after_security_failure(zb_uint8_t param) ZB_CALLBACK;

void zb_apsme_transport_key_indication(zb_uint8_t param) ZB_CALLBACK
{

  zb_apsme_transport_key_indication_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_indication_t);

  switch (ind->key_type)
  {
    case ZB_STANDARD_NETWORK_KEY:
    {
      zb_ushort_t i = 0;

      if (ZB_IEEE_ADDR_IS_ZERO(ZB_AIB().trust_center_address))
      {
        ZB_IEEE_ADDR_COPY(ZB_AIB().trust_center_address, ind->src_address);
        TRACE_MSG(TRACE_SECUR1, "TC is " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(ZB_AIB().trust_center_address)));

        /*
          Have a chance to remember long address of TC.
          But, short address is not known.
          Currently, it hard-coded to 0, but in the future TC can migrate to
          other device.


          I try to solve following scenario:
          - ZR1 joins ZC. ZC transport key to ZR1
          - ZR2 joins ZR1. ZR1 transports key to ZR2.
          ZR2 doesn't know ZC address now.
          - ZC broadcasts key transport, APS encrypt. ZR2 can't decrypt it
          because TC long address is unknown (zero)
         */
#ifdef ZB_TC_AT_ZC
        {
          zb_address_ieee_ref_t addr_ref;
          (void)zb_address_update(ZB_AIB().trust_center_address, 0, ZB_FALSE, &addr_ref);
        }
#else
        /*
          Can try to additionally check that ths packet is from device which we
          know (both long and short) and device is TC. It can be device other
          then ZC (short 0).
         */
#error Dont know what to do here!
#endif
      }

      /* check for all-zero key: is is so if we have pre-defined key */
      if (secur_nwk_key_is_empty(ind->key.nwk.key) && ind->key.nwk.key_seq_number == 0)
      {
        /* confirmed our key */
        TRACE_MSG(TRACE_SECUR1, "got zero nwk key", (FMT__0));
      }
      else
      {
        /* this calculation automatically takes key_seq_number overflow into account */
        i = (ZG->nwk.nib.active_secur_material_i +
             (zb_uint8_t)(ind->key.nwk.key_seq_number - ZG->nwk.nib.active_key_seq_number))
          % ZB_SECUR_N_SECUR_MATERIAL;
        ZB_MEMCPY(ZG->nwk.nib.secur_material_set[i].key, ind->key.nwk.key, ZB_CCM_KEY_SIZE);
        ZG->nwk.nib.secur_material_set[i].key_seq_number = ind->key.nwk.key_seq_number;
        TRACE_MSG(TRACE_SECUR1, "update key i %hd seq %hd ",
                  (FMT__H_H_A_A, i, ind->key.nwk.key_seq_number));
        if (i == ZG->nwk.nib.active_secur_material_i
            && ind->key.nwk.key_seq_number != ZG->nwk.nib.active_key_seq_number)
        {
          /* Probably, too old key. Recover by switching current key */
          ZG->nwk.nib.active_key_seq_number = ind->key.nwk.key_seq_number;
          ZG->nwk.nib.outgoing_frame_counter = 0;
          TRACE_MSG(TRACE_SECUR1, "switch current key", (FMT__0));
        }
      }

      if (!ZG->aps.authenticated)
      {
        ZG->aps.authenticated = 1;

        ZG->nwk.nib.active_key_seq_number = ind->key.nwk.key_seq_number;
        ZG->nwk.nib.outgoing_frame_counter = 0;
        ZG->nwk.nib.active_secur_material_i = i;

        TRACE_MSG(TRACE_SECUR1, "authenticated; curr key #%hd", (FMT__H, ind->key.nwk.key_seq_number));

        zdo_send_device_annce(param);
      }
      else
      {
        zb_free_buf(ZB_BUF_FROM_REF(param));
      }
    }
      break;
    default:
      TRACE_MSG(TRACE_SECUR1, "unsupported key type %hd", (FMT__H, ind->key_type));
      zb_free_buf(ZB_BUF_FROM_REF(param));
      break;
  }
}

#ifdef ZB_ROUTER_ROLE

static zb_ushort_t zb_secur_gen_upd_dev_status(zb_ushort_t rejoin_network, zb_ushort_t secure_rejoin);

#ifdef ZB_COORDINATOR_ROLE

void secur_send_key_sw_next(zb_uint8_t param) ZB_CALLBACK;


/**
   TC initialization
 */
void secur_tc_init()
{
  TRACE_MSG(TRACE_SECUR1, "TC init", (FMT__0));
  ZG->nwk.handle.is_tc = 1;
  ZB_IEEE_ADDR_COPY(ZB_AIB().trust_center_address, ZB_PIB_EXTENDED_ADDRESS());

  /* TC is always authenticated */
  ZG->aps.authenticated = 1;
}

#ifdef ZB_TC_GENERATES_KEYS
static void secur_generate_key(zb_uint8_t i, zb_uint_t key_seq)
{
  zb_ushort_t j;
  for (j = 0 ; j < ZB_CCM_KEY_SIZE ; ++j)
  {
    ZG->nwk.nib.secur_material_set[i].key[j] = (ZB_RANDOM() >> 4) & 0xff;
  }
  ZG->nwk.nib.secur_material_set[i].key_seq_number = key_seq;
}


void secur_generate_keys()
{
  zb_ushort_t i;

  for (i = 0 ; i < ZB_SECUR_N_SECUR_MATERIAL ; ++i)
  {
    /* active_key_seq_number, active_secur_material_i set to 0 by global init -
       not need to init it here */
    secur_generate_key(i, i);
  }
}
#endif


#endif  /* ZB_COORDINATOR_ROLE */


/**
   Authnticate child - see 4.6.2.2

   Called from nlme-join.indication at ZC or ZR
 */
void secur_authenticate_child(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_join_indication_t ind;
  zb_uint8_t upd_dev_status;
  ind = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_join_indication_t); /* save parameter */
  upd_dev_status = zb_secur_gen_upd_dev_status(ind.rejoin_network, ind.secure_rejoin);
  TRACE_MSG(TRACE_SECUR3, "authenticate child %d, st %hd", (FMT__D_H, ind.network_address, upd_dev_status));
#ifdef ZB_COORDINATOR_ROLE
  if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR
      && ZG->nwk.handle.is_tc)
  {
    /* We are TC - send TRANSPORT-KEY directly */
    zb_apsme_transport_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_req_t);
    ZB_IEEE_ADDR_COPY(req->dest_address.addr_long, ind.extended_address);
    req->addr_mode = ZB_ADDR_64BIT_DEV;
#ifdef ZB_STACK_PROFILE_2007
    req->key_type = ZB_STANDARD_NETWORK_KEY;
    /* see table 4.40 */
    if (upd_dev_status == ZB_STD_SEQ_SECURED_REJOIN)
    {
      /* device already has nwk key. Send empty key to it. */
      ZB_BZERO(req->key.nwk.key, ZB_CCM_KEY_SIZE);
      req->key.nwk.key_seq_number = 0;
      TRACE_MSG(TRACE_SECUR3, "Send empty nwk key to " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(ind.extended_address)));
    }
    else
    {
      ZB_MEMCPY(req->key.nwk.key, ZG->nwk.nib.secur_material_set[ZG->nwk.nib.active_secur_material_i].key, ZB_CCM_KEY_SIZE);
      req->key.nwk.key_seq_number = ZG->nwk.nib.active_key_seq_number;
      TRACE_MSG(TRACE_SECUR3, "Send nwk key #%hd to " TRACE_FORMAT_64, (FMT__H_A,
                               req->key.nwk.key_seq_number, TRACE_ARG_64(ind.extended_address)));
    }
    req->key.nwk.use_parent = 0; /* send key directly: we are parent! */
#else
#error Implement PRO!
#endif
    ZB_SCHEDULE_CALLBACK(zb_apsme_transport_key_request, param);
  }
  else
#endif  /* ZB_COORDINATOR_ROLE */
  {
    /* 4.6.3.2.1  Router Operation */
    /* send UPDATE-DEVICE to TC */
    zb_apsme_update_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_update_device_req_t);
    req->status = upd_dev_status;
    ZB_IEEE_ADDR_COPY(req->dest_address, ZB_AIB().trust_center_address);
    ZB_IEEE_ADDR_COPY(req->device_address, ind.extended_address);
    req->device_short_address = ind.network_address;
    TRACE_MSG(TRACE_SECUR3, "update device %d status %hd; send cmd to " TRACE_FORMAT_64, (FMT__D_H, ind.network_address, upd_dev_status, TRACE_ARG_64(req->dest_address)));
    ZB_SCHEDULE_CALLBACK(zb_apsme_update_device_request, param);
  }
}


void zb_secur_send_nwk_key_update_br(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint16_t dest_br = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_uint16_t);
  zb_apsme_transport_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_req_t);

  TRACE_MSG(TRACE_SECUR3, "secur_send_nwk_key_update %hd", (FMT__H, param));

  req->addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
  req->dest_address.addr_short = dest_br; /* broadcast - see 4.4.9.2.3.2    Network Key Descriptor Field */
  req->key.nwk.use_parent = 0; /* send key directly: we are parent! */
  req->key_type = ZB_STANDARD_NETWORK_KEY;
  req->key.nwk.key_seq_number = ZG->nwk.nib.active_key_seq_number + 1;
  {
    zb_uint8_t *key = secur_nwk_key_by_seq(req->key.nwk.key_seq_number);
    if (!key)
    {
      TRACE_MSG(TRACE_ERROR, "No nwk key # %hd", (FMT__H, req->key.nwk.key_seq_number));
      return;
    }
    ZB_MEMCPY(req->key.nwk.key, key, ZB_CCM_KEY_SIZE);
  }
  TRACE_MSG(TRACE_SECUR3, "Send nwk key #%hd update to all, dest_br 0x%x",
            (FMT__H_D, req->key.nwk.key_seq_number, dest_br));
  ZB_SCHEDULE_CALLBACK(zb_apsme_transport_key_request, param);
}


void zb_secur_send_nwk_key_switch(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint16_t dest_br = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_uint16_t);
  zb_apsme_switch_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_switch_key_req_t);

  TRACE_MSG(TRACE_SECUR1, "zb_secur_send_nwk_key_switch param %hd dest_br %d", (FMT__H_D, param, dest_br));
  if (dest_br == 0)
  {
    /* Switch key, then unicast to all rx-on-when-idle */
    secur_nwk_key_switch(ZG->nwk.nib.active_key_seq_number + 1);
    TRACE_MSG(TRACE_SECUR3, "send key switch to #%hd unicast to all devices from n.t.", (FMT__D, ZG->nwk.nib.active_key_seq_number));
    ZG->aps.tmp.neighbor_table_iterator = zb_nwk_neighbor_next_rx_on_i(0);
    if (ZG->aps.tmp.neighbor_table_iterator != (zb_ushort_t)~0)
    {
      secur_send_key_sw_next(param);
    }
    else
    {
      TRACE_MSG(TRACE_SECUR3, "have nobody to send to", (FMT__0));
      zb_free_buf(ZB_BUF_FROM_REF(param));
    }
  }
  else
  {
    req->addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    req->dest_address.addr_short = dest_br;
    req->key_seq_number = ZG->nwk.nib.active_key_seq_number + 1;
    /* Broadcast, then switch key: remember this buffer */
    ZG->zdo.handle.key_sw = param;
    TRACE_MSG(TRACE_SECUR3, "broadcast key switch, remember key_sw %hd", (FMT__H, ZG->zdo.handle.key_sw));
    ZB_SCHEDULE_CALLBACK(zb_apsme_switch_key_request, param);
  }
}


/**
   Send switch-key.request to the next rx-on-when-idle device from the neighbor table

   Iteration thru neighbor table made using aps.tmp.neighbor_table_iterator
   variable.
   Send command, iterate next device, if it exist, allocate buffer and call
   myself with it.

   @param - buffer to use.
 */
void secur_send_key_sw_next(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_switch_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_switch_key_req_t);

  req->addr_mode = ZB_ADDR_64BIT_DEV;
  req->key_seq_number = ZG->nwk.nib.active_key_seq_number;
  zb_address_ieee_by_ref(req->dest_address.addr_long,
                         ZG->nwk.neighbor.base_neighbor[ZG->aps.tmp.neighbor_table_iterator].addr_ref);
  ZB_SCHEDULE_CALLBACK(zb_apsme_switch_key_request, param);

  ZG->aps.tmp.neighbor_table_iterator = zb_nwk_neighbor_next_rx_on_i(ZG->aps.tmp.neighbor_table_iterator + 1);
  if (ZG->aps.tmp.neighbor_table_iterator != (zb_ushort_t)~0)
  {
    zb_get_out_buf_delayed(secur_send_key_sw_next);
  }
}
#endif

void secur_nwk_key_switch(zb_uint8_t key_number)
{
  zb_ushort_t i;
  zb_ushort_t shift = (ZG->nwk.nib.active_secur_material_i + 1) % ZB_SECUR_N_SECUR_MATERIAL;
  for (i = 0 ;
       i < ZB_SECUR_N_SECUR_MATERIAL
         && (ZG->nwk.nib.secur_material_set[(i + shift) % ZB_SECUR_N_SECUR_MATERIAL].key_seq_number == key_number) ;
       ++i)
  {
  }

  if (i == ZB_SECUR_N_SECUR_MATERIAL)
  {
#ifdef ZB_COORDINATOR_ROLE
    if (ZG->nwk.handle.is_tc)
    {
#ifdef ZB_TC_GENERATES_KEYS
      /* We are here if no key with such key number found. Generate new one. */
      secur_generate_key(shift, key_number);
#endif
      i = shift;
    }
    else
#endif
    {
      TRACE_MSG(TRACE_SECUR1, "Could not find nwk key #%hd !", (FMT__H, key_number));
    }
  }

  if (i != ZB_SECUR_N_SECUR_MATERIAL)
  {
    ZG->nwk.nib.active_secur_material_i = i;
    ZG->nwk.nib.active_key_seq_number = key_number;
    ZG->nwk.nib.prev_outgoing_frame_counter = ZG->nwk.nib.outgoing_frame_counter;
    ZG->nwk.nib.outgoing_frame_counter = 0;
    TRACE_MSG(TRACE_SECUR1, "switched to nwk key #%hd, i %hd", (FMT__H_H, key_number, i));
  }
  else if (!ZG->nwk.handle.is_tc)
  {
#ifndef ZB_DISABLE_REJOIN_AFTER_SEC_FAIL
    TRACE_MSG(TRACE_SECUR1, "Key switch failed. Try to rejoin", (FMT__0));
    zb_get_out_buf_delayed(zb_secur_rejoin_after_security_failure);
#endif
  }
}

#ifdef ZB_ROUTER_ROLE
void zb_secur_switch_nwk_key_br(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint16_t *dest_br_p = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_uint16_t);
  *dest_br_p = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
  TRACE_MSG(TRACE_SECUR1, "zb_secur_switch_nwk_key_br", (FMT__0));
  ZB_SCHEDULE_CALLBACK(zb_secur_send_nwk_key_switch, param);
}


/**
   Calculate 'status' field for 'update device' operation.

   see table 4.40
*/
static zb_ushort_t zb_secur_gen_upd_dev_status(zb_ushort_t rejoin_network, zb_ushort_t secure_rejoin)
{
  zb_ushort_t v = ((!!rejoin_network) << 1) | secure_rejoin;
  switch (v)
  {
    case 0:
      return ZB_STD_SEQ_UNSECURED_JOIN;
    case 2:
      return ZB_STD_SEQ_SECURED_REJOIN;
    case 3:
      return ZB_STD_SEQ_UNSECURED_REJOIN;
    default:
      TRACE_MSG(TRACE_SECUR1, "bad rejoin/secur combination %hd %hd", (FMT__H_H, rejoin_network, secure_rejoin));
      return -1;
  }
}



/**
   UPDATE-DEVICE.indication primitive
*/
void zb_apsme_update_device_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_update_device_ind_t ind;
  ind = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_update_device_ind_t);
  if (ind.status == ZB_DEVICE_LEFT)
  {
    zb_address_ieee_ref_t addr_ref;
    /* remove device from the neighbor table and address translation table */
    TRACE_MSG(TRACE_SECUR3, "Device %d left - forget it", (FMT__D, ind.device_short_address));

    if (zb_address_by_short(ind.device_short_address, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK
        || zb_address_by_ieee(ind.device_address, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK)
    {
      zb_nwk_neighbor_delete(addr_ref);
      zb_address_delete(addr_ref);
      zb_free_buf(ZB_BUF_FROM_REF(param));
    }
  }
  else
  {
    if ( ZDO_CTX().handle.allow_auth )
    {
      zb_apsme_transport_key_req_t *req;
      req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_req_t);
      /* We are TC if we got this packet. */

      /*
        see 4.6.3.2.2.1    Standard Security Mode.

        Trust Center shall send
        the device the active network key.

        Unicast TRANSPORT-KEY.request to the originator of UPDATE-DEVICE.indication,
        secure APS command.
      */
      req->key_type = ZB_STANDARD_NETWORK_KEY;
      req->key.nwk.use_parent = 1;
      ZB_IEEE_ADDR_COPY(req->key.nwk.parent_address, ind.src_address);
      ZB_IEEE_ADDR_COPY(req->dest_address.addr_long, ind.device_address);
      req->addr_mode = ZB_ADDR_64BIT_DEV;
      /* see table 4.40 */
      if (ind.status == ZB_STD_SEQ_SECURED_REJOIN)
      {
        /* device has pre-installed nwk key - send empty nwk key to it */
        ZB_BZERO(req->key.nwk.key, ZB_CCM_KEY_SIZE);
        req->key.nwk.key_seq_number = 0;
        TRACE_MSG(TRACE_SECUR3, "Indirect send empty nwk key to " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(ind.device_address)));
      }
      else if (ind.status == ZB_STD_SEQ_UNSECURED_JOIN || ind.status == ZB_STD_SEQ_UNSECURED_REJOIN)
      {
        /* send nwk key. */
        ZB_MEMCPY(req->key.nwk.key, ZG->nwk.nib.secur_material_set[ZG->nwk.nib.active_secur_material_i].key, ZB_CCM_KEY_SIZE);
        req->key.nwk.key_seq_number = ZG->nwk.nib.active_key_seq_number;
        TRACE_MSG(TRACE_SECUR3, "Indirect send nwk key #%hd to " TRACE_FORMAT_64,
                  (FMT__H_A, req->key.nwk.key_seq_number, TRACE_ARG_64(ind.device_address)));
      }
      ZB_SCHEDULE_CALLBACK(zb_apsme_transport_key_request, param);
    }
    else
    {
      /* auth is disallowed, send remove device request */
      zb_apsme_remove_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_remove_device_req_t);

      ZB_IEEE_ADDR_COPY(req->parent_address, ind.src_address);
      ZB_IEEE_ADDR_COPY(req->child_address, ind.device_address);

      ZB_SCHEDULE_CALLBACK(zb_secur_apsme_remove_device, param);
    }
  }
}


/**
   TC asked us to remove that device.
   Issue LEAVE request.
 */
void zb_apsme_remove_device_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_remove_device_ind_t ind;
  zb_nlme_leave_request_t *lr;

  ind = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_remove_device_ind_t);
  lr = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_leave_request_t);
  TRACE_MSG(TRACE_SECUR2, "zb_apsme_remove_device_ind from " TRACE_FORMAT_64 " child " TRACE_FORMAT_64,
            (FMT__A_A, TRACE_ARG_64(ind.src_address), TRACE_ARG_64(ind.child_address)));

  if (ZB_IEEE_ADDR_CMP(ind.src_address, ZB_AIB().trust_center_address))
  {
    /* see 4.6.3.6  Network Leave, 4.6.3.6.2  Router Operation */
    ZB_IEEE_ADDR_COPY(lr->device_address, ind.child_address);
    lr->remove_children = 1;      /* not sure */
    lr->rejoin = 0;               /* not sure */
    ZB_SCHEDULE_CALLBACK(zb_nlme_leave_request, param);
  }
  else
  {
    TRACE_MSG(TRACE_SECUR1, "zb_apsme_remove_device_ind from non-TC - ignore it", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
}
#endif  /* ZB_ROUTER_ROLE */

/**
   switch-key.indication primitive
 */
void zb_apsme_switch_key_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_switch_key_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_switch_key_ind_t);

  TRACE_MSG(TRACE_SECUR3, "zb_apsme_switch_key_ind from " TRACE_FORMAT_64 " key # %hd",
            (FMT__A_H, TRACE_ARG_64(ind->src_address), ind->key_seq_number));

  secur_nwk_key_switch(ind->key_seq_number);
  zb_free_buf(ZB_BUF_FROM_REF(param));
}



/**
   Remote device asked us for key.


   Application keys are not implemented.
   Send current network key.
   Not sure: send unsecured?
   What is meaning of that command??
   Maybe, idea is that we can accept "previous" nwk key?
   Or encrypt by it?
 */

#ifdef ZB_ROUTER_ROLE
void zb_apsme_request_key_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_request_key_ind_t ind;
  ind = *ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_request_key_ind_t);
  if (ind.key_type == ZB_STANDARD_NETWORK_KEY)
  {
    zb_address_ieee_ref_t ref;
    zb_neighbor_tbl_ent_t *nb;

    /* Send key only if device is in our neighbor (is it right ?) and authenticated */
    if (!(zb_address_by_ieee(ind.src_address, ZB_FALSE, ZB_FALSE, &ref) == RET_OK
          && zb_nwk_neighbor_get(ref, ZB_FALSE, &nb) == RET_OK
          && nb->relationship != ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD
          && nb->relationship != ZB_NWK_RELATIONSHIP_PREVIOUS_CHILD
          && nb->relationship != ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE))
    {
      zb_free_buf(ZB_BUF_FROM_REF(param));
    }
    else
    {
      zb_apsme_transport_key_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_transport_key_req_t);

      req->key_type = ZB_STANDARD_NETWORK_KEY;
      req->key.nwk.use_parent = 0;
      ZB_IEEE_ADDR_COPY(req->dest_address.addr_long, ind.src_address);
      req->addr_mode = ZB_ADDR_64BIT_DEV;
      /* send nwk key. */
      ZB_MEMCPY(req->key.nwk.key, ZG->nwk.nib.secur_material_set[ZG->nwk.nib.active_secur_material_i].key, ZB_CCM_KEY_SIZE);
      req->key.nwk.key_seq_number = ZG->nwk.nib.active_key_seq_number;
      TRACE_MSG(TRACE_SECUR3, "send nwk key #%hd to " TRACE_FORMAT_64,
                (FMT__H_A, req->key.nwk.key_seq_number, TRACE_ARG_64(ind.src_address)));
      /*
        Initiate unsecured key transfer.
        Not sure it is right, but I really have no ideas about meaning of
        request-key for network key.
       */
      ZB_SCHEDULE_CALLBACK(zb_apsme_transport_key_request, param);
    }
  }
  else
  {
    /* TODO: implement application key */
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
}
#endif /* router role */



zb_bool_t secur_has_preconfigured_key()
{
  return (zb_bool_t)!secur_nwk_key_is_empty(&ZG->nwk.nib.secur_material_set[0].key[0]);
}

void zb_secur_setup_preconfigured_key(zb_uint8_t *key, zb_uint8_t i)
{
  if (i >= ZB_SECUR_N_SECUR_MATERIAL)
  {
    TRACE_MSG(TRACE_ERROR, "Too big preconfigured key #%hd", (FMT__H, i));
  }
  else
  {
    ZB_MEMCPY(&ZG->nwk.nib.secur_material_set[i].key[0], key, ZB_CCM_KEY_SIZE);
    ZG->nwk.nib.secur_material_set[i].key_seq_number = i;
  }
}


void secur_clear_preconfigured_key()
{
  ZB_BZERO(&ZG->nwk.nib.secur_material_set[0].key[0], ZB_CCM_KEY_SIZE);
}


zb_bool_t secur_nwk_key_is_empty(zb_uint8_t *key)
{
  zb_ushort_t i;
  for (i = 0 ; i < ZB_CCM_KEY_SIZE && key[i] == 0 ; ++i)
  {
  }
  return (zb_bool_t)(i == ZB_CCM_KEY_SIZE);
}


zb_uint8_t *secur_nwk_key_by_seq(zb_ushort_t key_seq_number)
{
  zb_ushort_t i = ZG->nwk.nib.active_secur_material_i;
  zb_ushort_t cnt = 0;

  while (cnt++ < ZB_SECUR_N_SECUR_MATERIAL
         && ZG->nwk.nib.secur_material_set[i].key_seq_number != key_seq_number)
  {
    i = (i + 1) % ZB_SECUR_N_SECUR_MATERIAL;
  }

  if (cnt > ZB_SECUR_N_SECUR_MATERIAL)
  {
    TRACE_MSG(TRACE_SECUR1, "No nwk key # %hd", (FMT__H, key_seq_number));
    return 0;
  }
  TRACE_MSG(TRACE_SECUR3, "cnt %hd seq %hd - i %hd, ret %p", (FMT__H_H_H_P, cnt, key_seq_number, i, ZG->nwk.nib.secur_material_set[i].key));
  return ZG->nwk.nib.secur_material_set[i].key;
}


/**
   Force rejoin adter security failure

   @param parab - buffer to use
 */
void zb_secur_rejoin_after_security_failure(zb_uint8_t param) ZB_CALLBACK
{
  /* rejoin to current pan */
  ZB_EXTPANID_COPY(ZB_AIB().aps_use_extended_pan_id, ZB_NIB_EXT_PAN_ID());
  /* rejoin */
  zdo_initiate_rejoin(ZB_BUF_FROM_REF(param));
}


#endif  /* ZB_SECURITY */

/*! @} */
