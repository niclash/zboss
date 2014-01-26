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

#include <string.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_secur.h"


/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_6.h"


#ifndef ZB_LIMITED_FEATURES
void zb_nlme_leave_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_neighbor_tbl_ent_t *nbt = NULL;
  zb_nwk_status_t status = ZB_NWK_STATUS_SUCCESS;
  zb_buf_t *nsdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_leave_request_t r;

  ZB_MEMCPY(&r, ZB_GET_BUF_PARAM(nsdu, zb_nlme_leave_request_t), sizeof(r));
  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_leave_request %hd", (FMT__H, param));
  if (!ZG->nwk.handle.joined)
  {
    TRACE_MSG(TRACE_ERROR, "got leave.request when not joined", (FMT__H, param));
    ret = RET_ERROR;
    status = ZB_NWK_STATUS_INVALID_REQUEST;
  }
#ifdef ZB_ROUTER_ROLE
  else if (!ZB_IEEE_ADDR_IS_ZERO(r.device_address)
           && (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR
               || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER))
  {
    if (zb_nwk_neighbor_get_by_ieee(r.device_address, &nbt) != RET_OK)
    {
      ret = RET_ERROR;
      status = ZB_NWK_STATUS_UNKNOWN_DEVICE;
      TRACE_MSG(TRACE_NWK2, "device is not in the neighbor", (FMT__0));
    }
    else if (nbt->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD)
    {
      /*
        see 3.6.1.10.2  Method for a Device to Remove Its Child from the Network

        Must not force leave of non-authenticated child.

        LEAVE.CONFIRM with status = 0
      */
      TRACE_MSG(TRACE_NWK2, "device is not authenticated - do't send LEAVE to it (status 0)", (FMT__0));
      ret = RET_ERROR;
    }
    else if (nbt->relationship != ZB_NWK_RELATIONSHIP_CHILD)
    {
      /* anyway, only child will accept LEAVE request */
      ret = RET_ERROR;
      TRACE_MSG(TRACE_NWK2, "device is not child", (FMT__0));
      status = ZB_NWK_STATUS_UNKNOWN_DEVICE;
    }
  }
#endif  /* router */
  if (ret == RET_OK)
  {
    /*
      a) local leave (device_address is empty)
      b) force remote device leave

      Anyway, send LEAVE_REQUEST
    */
#ifdef ZB_SECURITY
    zb_bool_t secure = (zb_bool_t)(ZG->nwk.nib.secure_all_frames && ZG->nwk.nib.security_level && secur_has_preconfigured_key());
#else
    zb_bool_t secure = ZB_FALSE;
#endif
    zb_nwk_hdr_t *nwhdr = NULL;
    zb_uint8_t *lp = NULL;

    if (!nbt)
    {
      /* if no neighbor table entry, this is leave for us */
      ZG->nwk.leave_context.rejoin_after_leave = r.rejoin;
      nwhdr = nwk_alloc_and_fill_hdr(nsdu,
                                     ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE, /* see 3.4.4.2  */
                                     ZB_PIB_EXTENDED_ADDRESS(), NULL,
                                     ZB_FALSE, secure, ZB_TRUE);
    }
#ifdef ZB_ROUTER_ROLE
    else
    {
      zb_uint16_t child_addr;
      zb_ieee_addr_t ieee_addr;
      zb_address_short_by_ref(&child_addr, nbt->addr_ref);
      /* must use temp variable! */
      zb_address_ieee_by_ref(ieee_addr, nbt->addr_ref);
      nwhdr = nwk_alloc_and_fill_hdr(nsdu,
                                     child_addr,
                                     ZB_PIB_EXTENDED_ADDRESS(), ieee_addr, ZB_FALSE, secure, ZB_TRUE);
    }
#endif  /* router */

#ifdef ZB_SECURITY
    if (secure)
    {
      nsdu->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
    }
#endif
    /* Don't want it to be routed - see 3.4.4.2 */
    nwhdr->radius = 1;

    lp = (zb_uint8_t *)nwk_alloc_and_fill_cmd(nsdu, ZB_NWK_CMD_LEAVE, sizeof(zb_uint8_t));
    *lp = 0;
    ZB_LEAVE_PL_SET_REJOIN(*lp, r.rejoin);

#ifdef ZB_ROUTER_ROLE
    ZB_LEAVE_PL_SET_REMOVE_CHILDREN(*lp, r.remove_children);
    if (nbt)
    {
      ZB_LEAVE_PL_SET_REQUEST(*lp);
      TRACE_MSG(TRACE_NWK3, "send leave.request request 1 rejoin %hd remove_children %hd",
                (FMT__A_H_H, r.rejoin, r.remove_children));
    }
    else
#endif  /* router */
    {
      TRACE_MSG(TRACE_NWK3, "send leave.request request 0 (I am leaving) rejoin %hd", (FMT__H, r.rejoin));
    }
  }

  if (ret == RET_OK)
  {

	ZB_SET_BUF_PARAM(nsdu, ZB_NWK_INTERNAL_LEAVE_CONFIRM_AT_DATA_CONFIRM_HANDLE, zb_uint8_t);
#ifdef ZB_ROUTER_ROLE
    {
	    zb_uint16_t child_addr;
	    zb_address_short_by_ref(&child_addr, nbt->addr_ref);
		/* if we removed last joined device, we could decrease number of child to */
		/* to save some address space */

		if (child_addr == ZB_NWK_ED_ADDRESS_ASSIGN()-1)
		{
			ZG->nwk.nib.ed_child_num--;
		}
	}
#endif
	ZB_SCHEDULE_CALLBACK(zb_nwk_forward, ZB_REF_FROM_BUF(nsdu));
  }
  else
  {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_nlme_leave_confirm_t *lc = NULL;

    TRACE_MSG(TRACE_NWK1, "leave.request failed %hd", (FMT__H, status));
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_leave_confirm_t), lc);
    lc->status = status;
    ZB_IEEE_ADDR_COPY(lc->device_address, r.device_address);
    ZB_SCHEDULE_CALLBACK(zb_nlme_leave_confirm, param);
  }
  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_leave_request %d", (FMT__D, ret));
}
#endif


#ifndef ZB_LIMITED_FEATURES
void zb_nlme_reset_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_reset_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_reset_request_t);

  TRACE_MSG(TRACE_NWK1, ">>reset_req %hd", (FMT__H, param));

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_IDLE
       && !ZG->nwk.handle.joined )
  {
    if ( request->warm_start )
    {
      /* clear neighbor table */
      zb_nwk_neighbor_clear();

      /* clear route table */
#if defined ZB_NWK_MESH_ROUTING
      NWK_ARRAY_CLEAR(ZG->nwk.nib.routing_table, zb_nwk_routing_t, ZG->nwk.nib.routing_table_cnt);
      NWK_ARRAY_CLEAR(ZG->nwk.nib.route_disc_table, zb_nwk_route_discovery_t, ZG->nwk.nib.route_disc_table_cnt);
#endif

      NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_SUCCESS, zb_nlme_reset_confirm);
    }
    else
    {
      zb_mlme_reset_request_t *req = ZB_GET_BUF_PARAM(buf, zb_mlme_reset_request_t);

      req->set_default_pib = ZB_TRUE;
      ZB_SCHEDULE_CALLBACK(zb_mlme_reset_request, param);

      ZG->nwk.handle.state = ZB_NLME_STATE_RESET;
    }
  }
  else
  {
    NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_INVALID_REQUEST, zb_nlme_reset_confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<reset_req", (FMT__0));
}

void zb_mlme_reset_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>reset_cnfrm %hd", (FMT__H, param));

  /* reinit nib */
  zb_nwk_nib_init();

  /* clear all internal variables */
  ZB_BZERO(&ZG->nwk.handle, sizeof(ZG->nwk.handle));

  /* neighbor table */
  zb_nwk_neighbor_clear();

  /* call higher layers */
  NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_SUCCESS, zb_nlme_reset_confirm);

  TRACE_MSG(TRACE_NWK1, "<<reset_cnfrm", (FMT__0));
}
#endif  /* ZB_LIMITED_FEATURES */

void zb_nlme_sync_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_sync_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_sync_request_t);

  TRACE_MSG(TRACE_NWK1, ">>sync_req %hd", (FMT__H, param));

  /* 3.2.2.22.3 Zigbee works only in non-beacon. And if track is true - just
   * return invalid param */
  if ( request->track )
  {
    NWK_CONFIRM_STATUS(buf, ZB_NWK_STATUS_INVALID_PARAMETER, zb_nlme_sync_confirm);
  }
  else
  {
    /* call mlme-poll */
    zb_mlme_poll_request_t *req = ZB_GET_BUF_PARAM(buf, zb_mlme_poll_request_t);
    zb_uint16_t parent_addr;

    zb_address_short_by_ref(&parent_addr, ZG->nwk.handle.parent);
    req->coord_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    req->coord_addr.addr_short = parent_addr;
    req->coord_pan_id = ZB_PIB_SHORT_PAN_ID();
    ZB_SCHEDULE_TX_CB(zb_handle_poll_request, param);
  }

  TRACE_MSG(TRACE_NWK1, "<<sync_req", (FMT__0));
}



void zb_mlme_poll_confirm(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>poll_cnfrm %hd", (FMT__H, param));

  TRACE_MSG(TRACE_NWK1, "state %hd", (FMT__H, ZG->nwk.handle.state));
#ifndef ZB_LIMITED_FEATURES
  if ( ZG->nwk.handle.state == ZB_NLME_STATE_REJOIN )
  {
    TRACE_MSG(TRACE_NWK1, "status %hd", (FMT__H, ZB_BUF_FROM_REF(param)->u.hdr.status));
    if ( ZB_BUF_FROM_REF(param)->u.hdr.status != MAC_SUCCESS )
    {
      /* do not choose this parent again */
      remove_parent_from_potential_parents(ZG->nwk.handle.tmp.rejoin.parent);

      /* try to choose another parent and send join request again */
      ZB_SCHEDULE_CALLBACK(zb_nlme_rejoin_scan_confirm, param);
    }
    else
    {
      /* do nothing */
    }
  }
  else
#endif
  {
    /* call sync confirm with status ret by poll */
    ZB_SCHEDULE_CALLBACK(zb_nlme_sync_confirm, param);
  }

  TRACE_MSG(TRACE_NWK1, "<<poll_cnfrm", (FMT__0));
}


#ifndef ZB_LIMITED_FEATURES
void zb_mlme_sync_loss_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_mlme_sync_loss_ind_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_sync_loss_ind_t);

  TRACE_MSG(TRACE_NWK1, ">>sync_loss_ind %hd", (FMT__H, param));

  /* 3.2.2.23.2  NLME-SYNC-LOSS.indication, when generated */
  /* TODO: Seems this code is useless, cause we work only in non-becon mode */
  if ( ind->loss_reason == ZB_SYNC_LOSS_REASON_BEACON_LOST )
  {
    /* no params for zb_mlme_sync_loss_indication */
    ZB_SCHEDULE_CALLBACK(zb_mlme_sync_loss_indication, param);
  }
  else
  {
    /* do nothing, just drop frame */
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }

  TRACE_MSG(TRACE_NWK1, "<<sync_loss_ind", (FMT__0));
}


void zb_nlme_get_request(zb_uint8_t param) ZB_CALLBACK
{
#ifdef ZB_LIMITED_FEATURES
  (void)param;
#else
  zb_nwk_status_t status = ZB_NWK_STATUS_SUCCESS;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_get_confirm_t *conf = NULL;
  zb_nlme_get_request_t req;

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_get_request %hd", (FMT__H, param));
  ZB_MEMCPY(&req, ( zb_nlme_get_request_t *)ZB_BUF_BEGIN(buf), sizeof(req));

  if (req.nib_attribute == ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZB_NIB_SEQUENCE_NUMBER();
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_PASSIVE_ASK_TIMEOUT)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->attribute_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.passive_ack_timeout;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.max_broadcast_retries;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_MAX_CHILDREN)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.max_children;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_MAX_DEPTH)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZB_NIB_MAX_DEPTH();
  }
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_MAX_ROUTERS)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.max_routers;
  }
#endif
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_ADDR_ALLOC)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.addr_alloc;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_USE_TREE_ROUTING)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.use_tree_routing;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_MANAGER_ADDR)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint16_t), conf);
    conf->attribute_length = sizeof(zb_uint16_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.nwk_manager_addr;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_UPDATE_ID)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.update_id;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_EXTENDED_PANID)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_ext_pan_id_t), conf);
    conf->attribute_length = sizeof(zb_ext_pan_id_t);
    ZB_64BIT_ADDR_COPY((((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)), ZG->nwk.nib.extended_pan_id);
  }
#ifdef ZB_SECURITY
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_SECURITY_LEVEL)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.security_level;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_SECURITY_MATERIAL_SET)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.active_secur_material_i;
  }
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_ACTIVE_KEY_SEQ_NUMBER)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.active_key_seq_number;
  }
#endif
#ifdef ZB_SECURITY
  else if (req.nib_attribute == ZB_NIB_ATTRIBUTE_SECURE_ALL_FRAMES)
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t) + sizeof(zb_uint8_t), conf);
    conf->attribute_length = sizeof(zb_uint8_t);
    *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t)) = ZG->nwk.nib.secure_all_frames;
  }
#endif
  else
  {
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_confirm_t), conf);
    status = ZB_NWK_STATUS_UNSUPPORTED_ATTRIBUTE;
    conf->attribute_length = 0;
  }
  conf->status = status;
  conf->nib_attribute = req.nib_attribute;
  ZB_SCHEDULE_CALLBACK(zb_nlme_get_confirm, param);

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_get_request", (FMT__0));
#endif /* ZB_LIMITED_FEATURES */
}

void zb_nlme_get_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_get_confirm_t *conf = NULL;

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_get_confirm %p", (FMT__P, buf));
  conf = ( zb_nlme_get_confirm_t *)ZB_BUF_BEGIN(buf);
  if ((conf->nib_attribute == ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER)
      || (conf->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES)
      || (conf->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_CHILDREN))
  {
    TRACE_MSG(TRACE_NWK2, "attr %hd get param %hd", (FMT__H_H, conf->nib_attribute,  *(((zb_uint8_t *)conf) + sizeof(zb_nlme_get_confirm_t))));
  }
  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_get_confirm status %hd", (FMT__H, conf->status));
  zb_free_buf(buf);
}

void zb_nlme_set_request(zb_uint8_t param) ZB_CALLBACK
{
#ifdef ZB_LIMITED_FEATURES
  (void)param;
#else
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *ptr = ZB_BUF_BEGIN(buf);
  zb_nlme_set_request_t *req = (zb_nlme_set_request_t *)ptr;
  zb_nlme_set_confirm_t conf;
  zb_nlme_set_confirm_t *conf_p = NULL;

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_set_request %hd", (FMT__H, param));

  conf.nib_attribute = req->nib_attribute;
  conf.status = ZB_NWK_STATUS_SUCCESS;
  ptr += sizeof(zb_nlme_set_request_t);
  if (req->nib_attribute == ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER)
  {
    ZB_NIB_SEQUENCE_NUMBER() = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_PASSIVE_ASK_TIMEOUT)
  {
    ZG->nwk.nib.passive_ack_timeout = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES)
  {
    ZG->nwk.nib.max_broadcast_retries = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_CHILDREN)
  {
    ZG->nwk.nib.max_children = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_DEPTH)
  {
    ZB_NIB_MAX_DEPTH() = *ptr;
  }
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_MAX_ROUTERS)
  {
    ZG->nwk.nib.max_routers = *ptr;
  }
#endif
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_ADDR_ALLOC)
  {
    ZG->nwk.nib.addr_alloc = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_USE_TREE_ROUTING)
  {
    ZG->nwk.nib.use_tree_routing = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_MANAGER_ADDR)
  {
    ZG->nwk.nib.nwk_manager_addr = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_UPDATE_ID)
  {
    ZG->nwk.nib.update_id = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_EXTENDED_PANID)
  {
    ZB_64BIT_ADDR_COPY(ZG->nwk.nib.extended_pan_id, ptr);
  }
#ifdef ZB_SECURITY
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_SECURITY_LEVEL)
  {
    ZG->nwk.nib.security_level = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_SECURITY_MATERIAL_SET)
  {
    ZG->nwk.nib.active_secur_material_i = *ptr;
  }
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_ACTIVE_KEY_SEQ_NUMBER)
  {
    ZG->nwk.nib.active_key_seq_number = *ptr;
  }
#endif
#ifdef ZB_SECURITY
  else if (req->nib_attribute == ZB_NIB_ATTRIBUTE_SECURE_ALL_FRAMES)
  {
    ZG->nwk.nib.secure_all_frames = *ptr;
  }
#endif
  else
  {
    conf.status = ZB_NWK_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_set_confirm_t), conf_p);
  ZB_MEMCPY(conf_p, &conf, sizeof(*conf_p));
  ZB_SCHEDULE_CALLBACK(zb_nlme_set_confirm, param);

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_set_request", (FMT__0));
#endif /* ZB_LIMITED_FEATURES */
}

void zb_nlme_set_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_set_confirm %hd", (FMT__H, param));

#ifdef ZB_TRACE_LEVEL
  {
  zb_nlme_set_confirm_t *conf = ( zb_nlme_set_confirm_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_set_confirm status %hd", (FMT__H, conf->status));
  }
#endif
  zb_free_buf(buf);
}
#endif /* ZB_LIMITED_FEATURES */


#ifdef ZB_ROUTER_ROLE
void zb_nlme_send_status(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_send_status_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_send_status_t);
  zb_nlme_status_indication_t *status_cmd;

  TRACE_MSG(TRACE_NWK1, ">> zb_nlme_send_status param %hd", (FMT__H, param));

  nwk_alloc_and_fill_hdr(buf, request->dest_addr, NULL, NULL, ZB_FALSE, ZB_FALSE, ZB_TRUE);
  status_cmd = (zb_nlme_status_indication_t *)nwk_alloc_and_fill_cmd(buf, ZB_NWK_CMD_NETWORK_STATUS, sizeof(zb_nlme_status_indication_t));
  status_cmd->status = request->status.status;
  status_cmd->network_addr = request->status.network_addr;
  ZB_NWK_ADDR_TO_LE16(status_cmd->network_addr);

  /* transmit route request packet */
  ZB_SET_BUF_PARAM(buf, request->ndsu_handle, zb_uint8_t);
  ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);

  TRACE_MSG(TRACE_NWK1, "<< zb_nlme_send_status", (FMT__0));
}
#endif

/*! @} */
