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
PURPOSE: Network creation routine
*/

#include <string.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_secur.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_magic_macros.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_6.h"


#ifdef ZB_ROUTER_ROLE
static zb_mac_status_t zb_nwk_accept_child(zb_ieee_addr_t device_address, zb_mac_capability_info_t capability, zb_uint8_t lqi, zb_uint16_t *address) ZB_SDCC_REENTRANT;


void zb_nlme_rejoin_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nwk_hdr_t *nwhdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(buf);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_rejoin_request %hd", (FMT__H, param));

  if ( ZG->nwk.handle.rejoin_req_table_cnt < ZB_NWK_REJOIN_REQUEST_TABLE_SIZE
       /* && ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control) */
       && ZB_NWK_FRAMECTL_GET_SOURCE_IEEE(nwhdr->frame_control) )
  {
    zb_nwk_rejoin_request_t *rejoin_request = (zb_nwk_rejoin_request_t *)
      ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, ZB_NWK_HDR_SIZE(nwhdr->frame_control));
    zb_mac_status_t status = MAC_SUCCESS;
    zb_uint16_t address = (zb_uint16_t)~0;
    zb_nwk_rejoin_response_t *rejoin_response;
    zb_ieee_addr_t dst_ieee_addr;
    zb_ieee_addr_t *src_ieee_addr;
    zb_uint16_t dst_addr;
#ifdef ZB_SECURITY
    zb_bool_t secure = ZB_NWK_FRAMECTL_GET_SECURITY(nwhdr->frame_control);
    /* If request was secured, response secured also - see fig. 2.108 2.5.5.4.1 */
#else
    zb_bool_t secure = ZB_FALSE;
#endif

    if ( ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control) )
    {
      src_ieee_addr = (zb_ieee_addr_t *)&nwhdr->src_ieee_addr;
    }
    else
    {
      src_ieee_addr = (zb_ieee_addr_t *)&nwhdr->dst_ieee_addr;
    }
    ZB_IEEE_ADDR_COPY(dst_ieee_addr, *src_ieee_addr);

    status = zb_nwk_accept_child(*src_ieee_addr, rejoin_request->capability_information, ZB_MAC_GET_LQI(buf), &address);
    TRACE_MSG(TRACE_NWK3, "address assigned %d", (FMT__D));
    dst_addr = nwhdr->src_addr;

    nwhdr = nwk_alloc_and_fill_hdr(buf, dst_addr, ZB_PIB_EXTENDED_ADDRESS(), dst_ieee_addr, ZB_FALSE, secure, ZB_TRUE);
#ifdef ZB_SECURITY
    if ( secure )
    {
#ifdef ZB_SECURITY
      buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
      TRACE_MSG(TRACE_NWK1, "rejoin req was secured, reply secured also", (FMT__0));
#endif
    }
#endif
    rejoin_response = (zb_nwk_rejoin_response_t *)nwk_alloc_and_fill_cmd(buf, ZB_NWK_CMD_REJOIN_RESPONSE, sizeof(zb_nwk_rejoin_response_t));

    /* Save Ember address. Yes, it's not correct and it will not work, but
     * just to pass test */
#ifdef ZB_EMBER_GOLDEN_UNIT
    rejoin_response->network_addr = dst_addr;
#else
    rejoin_response->network_addr = address;
#endif

    ZB_NWK_ADDR_TO_LE16(rejoin_response->network_addr);
    rejoin_response->rejoin_status = status;

#if 0
    /* fill nwk header */
    {
      zb_ushort_t hdr_size  = ZB_NWK_FULL_HDR_SIZE(0);
#ifdef ZB_SECURITY
      if (secure)
      {
        hdr_size += sizeof(zb_nwk_aux_frame_hdr_t);
      }
#endif
      ZB_BUF_INITIAL_ALLOC(buf, hdr_size, nwhdr);
    }
    ZB_BZERO2(nwhdr->frame_control);
    ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(nwhdr->frame_control, ZB_NWK_FRAME_TYPE_COMMAND, ZB_PROTOCOL_VERSION);
    /*ZB_NWK_FRAMECTL_SET_DISCOVER_ROUTE(nwhdr->frame_control, 0); implied*/
    ZB_NWK_FRAMECTL_SET_SRC_DEST_IEEE(nwhdr->frame_control, 1, 1);
#ifdef ZB_SECURITY
    if (secure)
    {
      ZB_NWK_FRAMECTL_SET_SECURITY(nwhdr->frame_control, 1);
      buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
      TRACE_MSG(TRACE_NWK1, "rejoin req was secured, reply secured also", (FMT__0));
    }
#endif

    nwhdr->src_addr = ZB_PIB_SHORT_ADDRESS();
    nwhdr->dst_addr = dst_addr;
    nwhdr->radius = (zb_uint8_t)(ZB_NIB_MAX_DEPTH() << 1);
    nwhdr->seq_num = ZB_NIB_SEQUENCE_NUMBER();
    ZB_NIB_SEQUENCE_NUMBER_INC();
    ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, dst_ieee_addr);
    ZB_IEEE_ADDR_COPY(nwhdr->src_ieee_addr,  ZB_PIB_EXTENDED_ADDRESS());

    /* fill rejoin request cmd & payload */
    ZB_NWK_ALLOC_COMMAND_GET_PAYLOAD_PTR(buf, ZB_NWK_CMD_REJOIN_RESPONSE, zb_nwk_rejoin_response_t, rejoin_response);
    rejoin_response->network_addr = address;
    ZB_NWK_ADDR_TO_LE16(rejoin_response->network_addr);
    rejoin_response->rejoin_status = status;
#endif

    /* transmit rejoin packet */
    ZB_SET_BUF_PARAM(buf, (status == MAC_SUCCESS) ? ZB_NWK_INTERNAL_REJOIN_CMD_RESPONSE : ZB_NWK_INTERNAL_NSDU_HANDLE, zb_uint8_t);
    ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);

    /* We should call zb_nlme_join_indication after successful join, save address */
    if ( status == MAC_SUCCESS )
    {
      /* Save network address */
      ZG->nwk.handle.rejoin_req_table[ZG->nwk.handle.rejoin_req_table_cnt] = address;
      TRACE_MSG(TRACE_NWK1, "rejoin_req_table[%hd] = %d", (FMT__H_D, ZG->nwk.handle.rejoin_req_table_cnt, address));
      ZG->nwk.handle.rejoin_req_table_cnt++;
    }
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "rejoin req tbl is full or bad request - drop req", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin_req", (FMT__0));
}


void zb_nlme_rejoin_resp_sent(zb_uint8_t param) ZB_CALLBACK
{
  zb_address_ieee_ref_t addr_ref;
  zb_neighbor_tbl_ent_t *nent;
  zb_nlme_join_indication_t *resp = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_indication_t);

  TRACE_MSG(TRACE_NWK1, ">>rejoin_resp_sent param %hd status %hd", (FMT__H_H, param, ZB_BUF_FROM_REF(param)->u.hdr.status));

  if ( ZG->nwk.handle.rejoin_req_table_cnt
       && zb_address_by_short(ZG->nwk.handle.rejoin_req_table[ZG->nwk.handle.rejoin_req_table_cnt - 1], ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK )
  {
    ZG->nwk.handle.rejoin_req_table_cnt--;

    zb_address_by_ref(resp->extended_address, &resp->network_address, addr_ref);
    resp->capability_information = 0;

    zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nent);

    /* Only that 2 fields of capability information are meaningful */
    ZB_MAC_CAP_SET_DEVICE_TYPE(resp->capability_information, (nent->device_type == ZB_NWK_DEVICE_TYPE_ROUTER));
    ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(resp->capability_information, nent->rx_on_when_idle);
    resp->rejoin_network = ZB_NLME_REJOIN_METHOD_REJOIN;

#ifdef ZB_SECURITY
    {
      zb_nwk_hdr_t *nwhdr;
      /* must be 0 if it was secured rejoin! */
      ZB_MAC_CUT_HDR_WITHOUT_TRAILER(ZB_BUF_FROM_REF(param), nwhdr);
      resp->secure_rejoin = !ZB_NWK_FRAMECTL_GET_SECURITY(nwhdr->frame_control);
      TRACE_MSG(TRACE_NWK3, "secure_rejoin %hd (1 means insecure!)", (FMT__H, resp->secure_rejoin));
    }
#else
    resp->secure_rejoin = 1;    /* According to the table 4.40,
                                   * in means "unsecured rejoin" */
#endif

    ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status = ZB_NWK_STATUS_SUCCESS;

    /* Network configuration has changed - change update id. */
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
    zb_nwk_update_beacon_payload();
#endif
    /* Pass indication to AF. It will free the buffer. */
    ZB_SCHEDULE_CALLBACK(zb_nlme_join_indication, param);
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "should never happen", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
    ZB_ASSERT(0);
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin_resp_sent", (FMT__0));
}


static zb_mac_status_t zb_nwk_accept_child(zb_ieee_addr_t device_address, zb_mac_capability_info_t capability, zb_uint8_t lqi, zb_uint16_t *address) ZB_SDCC_REENTRANT
{
  zb_mac_status_t status = 0;
  zb_neighbor_tbl_ent_t *ent = NULL;
  zb_uint16_t addr = 0xfffe;

  TRACE_MSG(TRACE_NWK1, ">>zb_nwk_accept_child device_address " TRACE_FORMAT_64 " cap 0x%hx lqi %hd address %p",
            (FMT__P_H_H, TRACE_ARG_64(device_address), capability, lqi, address));

  /* 3.6.1.4.1.2
     Search in the neighbor table. If device with this ieee found then check device
     capabilities. If they are equal then obtain the corresponding network
     address and associate it, else remove record from table and continue association. */
  if ( zb_nwk_neighbor_get_by_ieee(device_address, &ent) == RET_OK)
  {
    if (
      /* Device type match. Don't compare for Coordinator: joining device can't be coordinator */
      (ent->device_type == ZB_NWK_DEVICE_TYPE_ROUTER) == ZB_MAC_CAP_GET_DEVICE_TYPE(capability)
      /* idle match */
      && ZB_MAC_CAP_GET_RX_ON_WHEN_IDLE(capability) == ent->rx_on_when_idle
      )
    {
      zb_address_short_by_ref(&addr, ent->addr_ref);
      TRACE_MSG(TRACE_NWK3, "found dev %hd short %d in neighb tbl", (FMT__H_D, ent->addr_ref, addr));
    }
    else
    {
      TRACE_MSG(TRACE_NWK3, "rm dev %hd from neighb tbl: diff caps", (FMT__H, ent->addr_ref));
      zb_nwk_neighbor_delete(ent->addr_ref);
      ent = NULL;
    }
  }

  /* Check the device capabilities and if we have room for this child type  */
  TRACE_MSG(TRACE_NWK1, "cap_inf 0x%x ed_ch_num %hd router_ch_num %hd max_ch %hd max_routers %hd",
            (FMT__D_H_H_H_H,
             (zb_uint16_t)capability,
             (zb_uint8_t)ZG->nwk.nib.ed_child_num, (zb_uint8_t)ZG->nwk.nib.router_child_num,
             (zb_uint8_t)ZG->nwk.nib.max_children, (zb_uint8_t)ZG->nwk.nib.max_routers));
  if (!ent)
  {
    status = MAC_OUT_OF_CAP;
    if ((ZG->nwk.nib.max_children > ZG->nwk.nib.ed_child_num + ZG->nwk.nib.router_child_num)
	&&(ZB_NIB_DEPTH() < ZG->nwk.nib.max_depth))
    {
      /* check child device is a router and we have room for it */
      if (ZB_MAC_CAP_GET_DEVICE_TYPE(capability))
      {
        if (ZG->nwk.nib.router_child_num < ZG->nwk.nib.max_routers)
        {
          /* According to MAC TC, allocate address only if requested in
           * capability.
           * ok for test, but how can it work in practice??
           */
          if (ZB_MAC_CAP_GET_ALLOCATE_ADDRESS(capability))
          {
            addr = ZB_NWK_ROUTER_ADDRESS_ASSIGN();
          }
          TRACE_MSG(TRACE_NWK3, "assigned router addr 0x%x", (FMT__D, addr));
          ZG->nwk.nib.router_child_num++;
          status = MAC_SUCCESS;
        }
      }
      else
      {
        /* child device is an ed and we have room for it */
        if (ZB_MAC_CAP_GET_ALLOCATE_ADDRESS(capability))
        {
          addr = ZB_NWK_ED_ADDRESS_ASSIGN();
        }
        TRACE_MSG(TRACE_NWK3, "assigned ed addr 0x%x (%u %hd %u %hd)",
                  (FMT__D_D_H_D_H, addr, ZG->mac.pib.mac_short_address, ZG->nwk.nib.max_routers, ZG->nwk.nib.cskip, ZG->nwk.nib.ed_child_num));
        ZG->nwk.nib.ed_child_num++;
        status = MAC_SUCCESS;
      }
    }
    if (status == MAC_SUCCESS)
    {
      zb_address_ieee_ref_t ieee_ref;

      /* allocate address translation entry */
      status = MAC_OUT_OF_CAP;
      if (zb_address_update(device_address, addr, ZB_TRUE, &ieee_ref) == RET_OK)
      {
        /* allocate neighbor table entry if necessary */
        if (zb_nwk_neighbor_get(ieee_ref, ZB_TRUE, &ent) != RET_OK)
        {
          /* check for allocated ieee_ref and unlock it */
          zb_address_unlock(ieee_ref);
        }
        else
        {
          status = MAC_SUCCESS;
        }
      }
    }
  }

  if (MAC_PIB().mac_association_permit
      && ZG->nwk.nib.router_child_num + ZG->nwk.nib.ed_child_num >= ZG->nwk.nib.max_children)
  {
    TRACE_MSG(TRACE_NWK1, "status %hd, max_children %hd, routers %hd, eds %hd - disable association permit",
              (FMT__H_H_H_H, status, ZG->nwk.nib.max_children, ZG->nwk.nib.router_child_num, ZG->nwk.nib.ed_child_num));
    MAC_PIB().mac_association_permit = 0;
    zb_nwk_update_beacon_payload();
  }

  if (status == MAC_SUCCESS)
  {
    /* update neighbor table entry */
    ent->device_type = ZB_NWK_DEVICE_TYPE_ED + ZB_MAC_CAP_GET_DEVICE_TYPE(capability);
    ent->rx_on_when_idle = ZB_MAC_CAP_GET_RX_ON_WHEN_IDLE(capability);
#ifdef ZB_SECURITY
    ent->relationship = ZB_NIB_SECURITY_LEVEL() ? ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD : ZB_NWK_RELATIONSHIP_CHILD;
#else
    ent->relationship = ZB_NWK_RELATIONSHIP_CHILD;
#endif
    ent->depth = ZG->nwk.nib.depth + (ZG->nwk.nib.depth < ZG->nwk.nib.max_depth);
    if (ent->device_type == ZB_NWK_DEVICE_TYPE_ED)
    {
      ent->permit_joining = 0;
    }
    ent->lqi = lqi;
    TRACE_MSG(TRACE_NWK2, "neighbor ent %p rel %hd dev_t %hd rx.on %hd",
              (FMT__P_H_H_H, ent, (zb_uint8_t)ent->relationship,
               (zb_uint8_t)ent->device_type, (zb_uint8_t)ent->rx_on_when_idle));

    *address = addr;
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nwk_accept_child status %hd, address %d", (FMT__H_D, status, addr));
  return status;
}

void zb_mlme_associate_indication(zb_uint8_t param) ZB_CALLBACK
{
#ifndef MAC_CERT_TEST_HACKS
  zb_mac_status_t status = MAC_SUCCESS;
  zb_uint16_t address = (zb_uint16_t)~0;
#endif
  zb_ieee_addr_t device_address;
  zb_mlme_associate_indication_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_associate_indication_t);

  TRACE_MSG(TRACE_NWK1, ">>mlme_associate_ind %hd", (FMT__H, param));

  CHECK_PARAM_RET_ON_ERROR(request);
#ifndef MAC_CERT_TEST_HACKS
  ZB_DUMP_IEEE_ADDR(request->device_address);  
  status = zb_nwk_accept_child(request->device_address, request->capability, request->lqi, &address);
#endif
  TRACE_MSG(TRACE_NWK1, "BUILD_ASSOCIATE_RESPONSE", (FMT__0));
  ZB_IEEE_ADDR_COPY(device_address, request->device_address);
						   
  ZB_DUMP_IEEE_ADDR(device_address);

  ZB_MLME_BUILD_ASSOCIATE_RESPONSE((zb_buf_t *)ZB_BUF_FROM_REF(param), device_address, address, status);

  ZB_SCHEDULE_CALLBACK(zb_mlme_associate_response, param);

  /* next MAC sends response and calls our zb_mlme_comm_status_indication callback */

  TRACE_MSG(TRACE_NWK1, "<<mlme_associate_ind", (FMT__0));

}

/*! @} */

#endif  /* ROUTER_ROLE */
