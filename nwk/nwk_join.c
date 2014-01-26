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

#include "zb_bank_4.h"

static void nwk_join_failure_confirm(zb_uint8_t param, zb_uint8_t s) ZB_SDCC_REENTRANT
{
  zb_buf_t ZB_SDCC_XDATA *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_join_confirm_t *join_confirm = ZB_GET_BUF_PARAM(buf, zb_nlme_join_confirm_t);
  join_confirm->status = (zb_mac_status_t)(s);
  buf->u.hdr.status = s;
  join_confirm->network_address = (s == MAC_SUCCESS) ? ZB_NIB_NETWORK_ADDRESS() : 0xffff;
  ZB_EXTPANID_COPY(join_confirm->extended_pan_id, ZB_NIB_EXT_PAN_ID());
  ZB_SCHEDULE_CALLBACK(zb_nlme_join_confirm, param);
}

static zb_ext_neighbor_tbl_ent_t *nwk_choose_parent(zb_address_pan_id_ref_t panid_ref, zb_mac_capability_info_t capability_information) ZB_SDCC_REENTRANT
{
  zb_ext_neighbor_tbl_ent_t *ret = NULL;
  zb_uint_t i;

  ZVUNUSED(capability_information);
  TRACE_MSG(TRACE_NWK1, ">>nwk_choose_parent panid_ref %hd cap %hx nib_upd_id %hd",
            (FMT__H_H_H, panid_ref, capability_information, ZB_NIB_UPDATE_ID()));

  /*
    Search its neighbor table for a suitable
    parent device, i.e. a device for which following conditions are true:

    - The device belongs to the network identified by the ExtendedPANId
    parameter.

    - The device is open to join requests and is advertising capacity of the correct
    device type.

    - The link quality for frames received from this device is such that a link cost of
    at most 3 is produced when calculated as described in sub-clause 3.6.3.1.

    - If the neighbor table entry contains a potential parent field for this device, that
    field shall have a value of 1 indicating that the device is a potential
    parent.

    - The device shall have the most recent update id, where the determination of
    most recent needs to take into account that the update id will wrap back to zero.
    In particular the update id given in the beacon payload of the device should be
    greater than or equal to - again, compensating for wrap -  the nwkUpdateId
    attribute of the NIB.

    If the neighbor table has more than one device that could be
    a suitable parent, the device which is at a minimum depth from the ZigBee
    coordinator may be chosen.
  */

  for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
  {
    zb_ext_neighbor_tbl_ent_t *ne = &ZG->nwk.neighbor.ext_neighbor[i];

    TRACE_MSG(TRACE_NWK2,"ne %p panid_ref %hd, potential_par %hd, permit_join %hd, cap r/e %hd/%hd, lqi %hu, upd_id %hd, NIB_uid %hu", (FMT__P_H_H_H_H_H_H_H, ne, panid_ref, (zb_uint8_t)ne->potential_parent,(zb_uint8_t) ne->permit_joining,(zb_uint8_t) ne->router_capacity, (zb_uint8_t)ne->end_device_capacity,(zb_uint8_t) ne->lqi, (zb_uint8_t)ne->update_id, ZB_NIB_UPDATE_ID()));



    if (ne->panid_ref == panid_ref
        && ne->potential_parent
        && ne->permit_joining == 1
#ifndef ZB_PRO_COMPATIBLE
		&& ne->stack_profile==1
#endif
#ifdef ZB_ROUTER_ROLE
  && ((ZB_MAC_CAP_GET_DEVICE_TYPE(capability_information)&&(ne->stack_profile==1))?
           ne->router_capacity : ne->end_device_capacity)
#else
        && ne->end_device_capacity
#endif
        && ZB_LINK_QUALITY_IS_OK_FOR_JOIN(ne->lqi)
       && ZB_NWK_UPDATE_ID1_GE_ID2(ne->update_id, ZB_NIB_UPDATE_ID()))
    {
      if (ret
          && (ne->depth >= ret->depth
              || ZB_LINK_QUALITY_1_IS_BETTER(ret->lqi, ne->lqi)))
      {
        TRACE_MSG(TRACE_NWK2, "best_par %d %p ne %p dep %d / %d, lqi %hd / %hd - skip", (FMT__D_P_P_D_D_H_H, ret, ne, ne->depth, ret->depth, ret->lqi, ne->lqi));
        continue;
      }
	  TRACE_MSG(TRACE_NWK2, "parent found", (FMT__0));
      ret = ne;
    }
    else
    {
      TRACE_MSG(TRACE_NWK2, "ne %p is not potential parent: panid_ok %hd potent_prnt_ok %hd permit_j_ok %hd cap_ok %hd lqi_ok %hd upd_id_ok %hd",
                (FMT__P_H_H_H_H_H_H,
                 ne,
                 (zb_uint8_t)(ne->panid_ref == panid_ref),
                 (zb_uint8_t)ne->potential_parent,
                 (zb_uint8_t)ne->permit_joining,
                 (zb_uint8_t)(ZB_MAC_CAP_GET_DEVICE_TYPE(capability_information) ?
                              (zb_uint8_t)ne->router_capacity : (zb_uint8_t)ne->end_device_capacity),
                 (zb_uint8_t)ZB_LINK_QUALITY_IS_OK_FOR_JOIN(ne->lqi),
                 (zb_uint8_t)ZB_NWK_UPDATE_ID1_GE_ID2(ne->update_id, ZB_NIB_UPDATE_ID())));
    }
  } /* for */

  TRACE_MSG(TRACE_NWK1, "<<nwk_choose_parent %p", (FMT__P, ret));
#if defined ZB_ROUTER_ROLE && defined ZB_PRO_COMPATIBLE
  if (ret->stack_profile != 1)
  {
	 ZG->nwk.handle.joined_pro = 1;
     ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ED;
	 MAC_CTX().dev_type = MAC_DEV_RFD;
  }
#endif
  return ret;
}

/**
   3.6.1.4.1  Joining a Network Through Association

   This routine can be called when upper layer called join first time or for
   attempt to join to another parent.
 */
static zb_ret_t nwk_association_join(zb_buf_t *buf, zb_nlme_join_request_t *request) ZB_SDCC_REENTRANT
{
  zb_ext_neighbor_tbl_ent_t *best_parent = NULL;
  zb_address_pan_id_ref_t panid_ref = 0; /* shutup sdcc */
  zb_uint16_t panid;
  zb_ret_t ret = ZB_NWK_STATUS_SUCCESS;

  TRACE_MSG(TRACE_NWK1, ">>assoc_join buf %p req %p", (FMT__P_P, buf, request));

  /* upper layer logic (ZDO) has chosen PAN */

  /* see 3.6.1.4.1.1    Child Procedure */


  /*
    Only those devices that are not already joined to a network shall initiate the join
    procedure. If any other device initiates this procedure, the NLME shall terminate
    the procedure and notify the next higher layer of the illegal request by issuing the
    NLME-JOIN.confirm primitive with the Status parameter set to
    INVALID_REQUEST.
   */
  if (ZG->nwk.handle.joined)
  {
    TRACE_MSG(TRACE_ERROR, "Association join req while joined", (FMT__0));
    ret = ZB_NWK_STATUS_INVALID_REQUEST;
  }
  else if ( zb_address_get_pan_id_ref(request->extended_pan_id, &panid_ref) != RET_OK )
  {
    TRACE_MSG(TRACE_NWK1, "No dev with xpanid " TRACE_FORMAT_64,
              (FMT__A, TRACE_ARG_64(request->extended_pan_id)));
    ret = ZB_NWK_STATUS_NOT_PERMITTED;
  }

  if (ret == ZB_NWK_STATUS_SUCCESS)
  {
    best_parent = nwk_choose_parent(panid_ref, request->capability_information);

    if (!best_parent)
    {
      /*
        If the neighbor table contains no devices that are suitable parents, the NLME shall
        respond with an NLME-JOIN.confirm with a Status parameter of
        NOT_PERMITTED.
      */

      TRACE_MSG(TRACE_ERROR, "No dev for join", (FMT__0));
      ret = ZB_NWK_STATUS_NOT_PERMITTED;
    }
  }   /* if ok */

  if (best_parent)
  {
    /*
      Issue an MLME-ASSOCIATE.request primitive to the  MAC sub-layer
     */
    zb_address_get_short_pan_id(panid_ref, &panid);
    /* Remember my PAN id in NIB just now: don't want to store it
     * externally. Anyway, it is illegal until Join complete. */
    ZB_NIB_PAN_ID() = panid;
    ZB_UPDATE_PAN_ID();

    /* save request to be able to reattempt connect */
    ZB_MEMCPY(&ZG->nwk.handle.saved_req.join, request, sizeof(*request));

    if (!ZB_ADDRESS_COMPRESED_IS_ZERO(best_parent->long_addr))
    {
      zb_ieee_addr_t long_addr;
      ZB_ADDRESS_DECOMPRESS(long_addr, best_parent->long_addr);

      TRACE_MSG(TRACE_NWK2, "Will assoc to pan " TRACE_FORMAT_64 "/%d, dev " TRACE_FORMAT_64 " %p", (FMT__A_D_A_P,
                TRACE_ARG_64(request->extended_pan_id),
                panid, TRACE_ARG_64(long_addr), (void *)best_parent));

      TRACE_MSG(TRACE_NWK2, "ch %hd", (FMT__H, best_parent->logical_channel));
      ZB_MLME_BUILD_ASSOCIATE_REQUEST(buf, best_parent->logical_channel,
                                      panid,
                                      ZB_ADDR_64BIT_DEV, long_addr,
                                      ZG->nwk.handle.saved_req.join.capability_information);

    }
    else
    {
      TRACE_MSG(TRACE_NWK2, "Will assoc to pan " TRACE_FORMAT_64 "/%d, dev %d %p ch %hd", (FMT__A_D_D_P_H,
                TRACE_ARG_64(request->extended_pan_id), panid, best_parent->short_addr,
                best_parent, best_parent->logical_channel));

      ZB_MLME_BUILD_ASSOCIATE_REQUEST(buf, best_parent->logical_channel,
                                      panid,
                                      ZB_ADDR_16BIT_DEV_OR_BROADCAST, &best_parent->short_addr,
                                      ZG->nwk.handle.saved_req.join.capability_information);
    }

    /* remember parent to be able to ref it after association complete */
    ZG->nwk.handle.tmp.join.parent = best_parent;

    ZB_SCHEDULE_CALLBACK(zb_mlme_associate_request, ZB_REF_FROM_BUF(buf));
  }

  TRACE_MSG(TRACE_NWK1, "<<assoc_join %d", (FMT__D, ret));
  return ret;
}


void zb_mlme_associate_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = 0;
  zb_mlme_associate_confirm_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_associate_confirm_t);
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_NWK1, ">>zb_mlme_associate_confirm %hd", (FMT__H, param));

  CHECK_PARAM_RET_ON_ERROR(request);

  if ( request->status == MAC_SUCCESS )
  {
    zb_nwk_set_device_type(
      ZB_MAC_CAP_GET_DEVICE_TYPE(ZG->nwk.handle.saved_req.join.capability_information) ?
      ZB_NWK_DEVICE_TYPE_ROUTER : ZB_NWK_DEVICE_TYPE_ED);
    ZG->nwk.handle.joined = 1;

    /* short pan id is already stored in NIB - see nwk_association_join */

    ZB_EXTPANID_COPY(ZB_NIB_EXT_PAN_ID(), ZG->nwk.handle.saved_req.join.extended_pan_id);

    /* remember my short network address */
    ZB_NIB_NETWORK_ADDRESS() = request->assoc_short_address;

    /* update nwkUpdateId */
    ZB_NIB_UPDATE_ID() = ZG->nwk.handle.tmp.join.parent->update_id;
    TRACE_MSG(TRACE_NWK3, "new update_id %hd", (FMT__H, ZB_NIB_UPDATE_ID()));

    /* remember this device long addr + short addr in the addr table */
    zb_address_update(ZB_PIB_EXTENDED_ADDRESS(), request->assoc_short_address, ZB_TRUE, &addr_ref);

    /* remember potential parent address to find it in the neighbor table */
    {
      zb_neighbor_tbl_ent_t *nent = NULL; /* shutup sdcc */
      zb_uint16_t short_addr = ZG->nwk.handle.tmp.join.parent->short_addr;
      zb_ieee_addr_t long_addr;

      if (short_addr == (zb_uint16_t)~0)
      {
        ZB_ADDRESS_DECOMPRESS(long_addr, ZG->nwk.handle.tmp.join.parent->long_addr);
      }
      else
      {
        ZB_IEEE_ADDR_COPY(long_addr, request->parent_address);
      }

      /* see 3.6.1.4.1.1:

         The network depth is set to one more than the parent network depth
         unless the parent network depth has a value of 0x0f, i.e. the maximum
         value for the 4-bit device depth field in the beacon payload. In this
         case, the network depth shall also be set to 0x0f.
       */
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
      ZB_NIB_DEPTH() = ZG->nwk.handle.tmp.join.parent->depth;
      if (ZB_NIB_DEPTH() != 0xf)
      {
        ZB_NIB_DEPTH() += 1;
      }
      /* Calculate CScip value for tree routing */
      ZG->nwk.nib.cskip = zb_nwk_daa_calc_cskip(ZB_NIB_DEPTH());
#endif

      /* stop ext neighbor table - convert it to the normal neighbor. */
      zb_nwk_exneighbor_stop(short_addr);

      /*
       * 'parent' pointed to the ext neighbor and now is invalid.
       * Find my parent in the neighbor table and mark it using Relationship
       * field. Find either by long or short address. */

      if (short_addr == (zb_uint16_t)~0)
      {
        /* use long address */
        ret = zb_address_by_ieee(long_addr, ZB_FALSE, ZB_FALSE, &addr_ref);
      }
      else
      {
        ret = zb_address_by_short(short_addr,
                                  ZB_FALSE, ZB_FALSE, &addr_ref);
      }
      if (ret != RET_OK)
      {
        TRACE_MSG(TRACE_NWK1, "Couldn't find addr", (FMT__0));
        ZB_ASSERT(0);
      }
      else
      {
        if (!ZB_IEEE_ADDR_IS_ZERO(long_addr) && short_addr != (zb_uint16_t)~0)
        {
          /* association request was sent to the short address, got responce
           * from the long address - can update address translation table */
          zb_address_update(request->parent_address, short_addr, ZB_FALSE, &addr_ref);
        }

        ret = zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nent);
      }
      if (ret == RET_OK)
      {
        nent->relationship = ZB_NWK_RELATIONSHIP_PARENT;
        TRACE_MSG(TRACE_NWK3, "nb ent %p type %hd rel %hd",
                  (FMT__P_H_H, nent, (zb_uint8_t)nent->device_type, (zb_uint8_t)nent->relationship));

        ZG->nwk.handle.parent = addr_ref;
      }
    }

    /* Notify upper layer */
    {
      zb_uint8_t status = request->status;
      zb_nlme_join_confirm_t *join_confirm = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_confirm_t);

      join_confirm->status = (zb_mac_status_t)status;
      join_confirm->network_address = ZB_NIB_NETWORK_ADDRESS();
      ZB_EXTPANID_COPY(join_confirm->extended_pan_id, ZG->nwk.handle.saved_req.join.extended_pan_id);
      join_confirm->active_channel = ZG->nwk.handle.tmp.join.parent->logical_channel;
      ZB_SCHEDULE_CALLBACK(zb_nlme_join_confirm, param);
    }
  }
  else
  {
    /* Unsuccessful associate. Attempt to join to another parent or join as ZE,
     * if failed - pass error up. */
    zb_uint8_t status = request->status;
    /* try to join to another device. */

    ZG->nwk.handle.tmp.join.parent->potential_parent = 0;

    ret = nwk_association_join((zb_buf_t *)ZB_BUF_FROM_REF(param), &ZG->nwk.handle.saved_req.join);
    TRACE_MSG(TRACE_NWK1, "Assoc retr ret %d", (FMT__D, ret));
    if (ret == ZB_NWK_STATUS_NOT_PERMITTED
        && ZB_MAC_CAP_GET_DEVICE_TYPE(ZG->nwk.handle.saved_req.join.capability_information))
    {
      /*
       * If no more devices to join to and we tried to connect as router, try
       * to connect as end device.
       * FIXME: not sure can do it transparrently.
       */
      ZB_MAC_CAP_SET_DEVICE_TYPE(ZG->nwk.handle.saved_req.join.capability_information, 0);

      {
        /* reset potential_parent: try all parents again */
        zb_uint_t i;
        for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_size ; ++i)
        {
          ZG->nwk.neighbor.ext_neighbor[i].potential_parent = 1;
        }
      }

      ret = nwk_association_join((zb_buf_t *)ZB_BUF_FROM_REF(param), &ZG->nwk.handle.saved_req.join);
      TRACE_MSG(TRACE_NWK1, "Assoc as ZE retr ret %d", (FMT__D, ret));
    }
    if (ret != 0)
    {
      TRACE_MSG(TRACE_NWK1, "Assoc fail - pass MAC st %d up", (FMT__D, status));
      nwk_join_failure_confirm(param, status);
      /* FIXME: not sure: stop ext neighbor table here? */
    }
    else
    {
      /* if ret is ok, let's wait for the next zb_mlme_associate_confirm() */
      TRACE_MSG(TRACE_NWK3, "wait for assoc conf", (FMT__0));
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<assoc_conf", (FMT__0));
}

static void zb_nlme_rejoin(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_join_request_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_request_t);
  zb_uint32_t scan_channels = request->scan_channels;
  zb_uint8_t scan_duration = request->scan_duration;

  TRACE_MSG(TRACE_NWK1, ">>rejoin %hd", (FMT__H, param));

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_IDLE )
  {
    TRACE_MSG(TRACE_NWK1, "xpanid " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(request->extended_pan_id)));
    /* Save request params */
    ZB_EXTPANID_COPY(ZG->nwk.handle.tmp.rejoin.extended_pan_id, request->extended_pan_id);
    ZG->nwk.handle.tmp.rejoin.capability_information = request->capability_information;

    /* Extend neighor table. */
#ifndef ZB_ED_ROLE
    zb_nwk_exneighbor_start();
#endif

    /* start active scan */
    ZB_MLME_BUILD_SCAN_REQUEST(buf, scan_channels, ACTIVE_SCAN, scan_duration);
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
    ZG->nwk.handle.state = ZB_NLME_STATE_REJOIN;
    ZG->nwk.handle.router_started = 0;
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "nwk is busy, state %hd", (FMT__H, ZG->nwk.handle.state));
    nwk_join_failure_confirm(param, ZB_NWK_STATUS_NOT_PERMITTED);
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin", (FMT__0));
}

#ifndef ZB_LIMITED_FEATURES2
void zb_nlme_rejoin_scan_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = ZB_NWK_STATUS_SUCCESS;
  zb_address_pan_id_ref_t panid_ref = 0;
  zb_ext_neighbor_tbl_ent_t *best_parent;

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_rejoin_scan_confirm %hd", (FMT__H, param));

  if ( zb_address_get_pan_id_ref(ZG->nwk.handle.tmp.rejoin.extended_pan_id, &panid_ref) != RET_OK )
  {
    TRACE_MSG(TRACE_NWK1, "No dev with xpanid " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(ZG->nwk.handle.tmp.rejoin.extended_pan_id)));
    ret = ZB_NWK_STATUS_NOT_PERMITTED;
  }
  else
  {
    /* try to find suitable parent, also we could have it's extended address
     * to rejoin */
    best_parent = nwk_choose_parent(panid_ref, ZG->nwk.handle.tmp.rejoin.capability_information);
    if ( !best_parent )
    {
      TRACE_MSG(TRACE_NWK1, "can't find prnt", (FMT__0));
      ret = ZB_NWK_STATUS_NOT_PERMITTED;
    }
    else
    {
      zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
      zb_nwk_hdr_t *nwhdr;
      zb_nwk_rejoin_request_t *rejoin_request;
      zb_address_ieee_ref_t addr_ref;
      zb_ieee_addr_t ieee_addr;

#ifdef ZB_SECURITY
      zb_bool_t secure = (zb_bool_t)(ZG->nwk.nib.secure_all_frames && ZG->nwk.nib.security_level && secur_has_preconfigured_key());
#else
      zb_bool_t secure = ZB_FALSE;
#endif

      {
        /* Assign panid to fill mac hdr by it */
        zb_uint16_t panid;
        zb_address_get_short_pan_id(panid_ref, &panid);
        ZB_NIB_PAN_ID() = panid;
      }

      ZB_ADDRESS_DECOMPRESS(ieee_addr, best_parent->long_addr);
      if (ZB_IEEE_ADDR_IS_ZERO(ieee_addr))
      {
        /*
         * Active scan does not give us long address.
         * This is rejoin so, maybe, this device is already in the address
         * translation table?
         */
        if (zb_address_ieee_by_short(best_parent->short_addr, ieee_addr) == RET_OK)
        {
          TRACE_MSG(TRACE_NWK1, "short %d, long " TRACE_FORMAT_64, (FMT__D_A, best_parent->short_addr,
                    TRACE_ARG_64(ieee_addr)));
          zb_ieee_addr_compress(ieee_addr, &best_parent->long_addr);
        }
      }

      /* set random network address if not set */
      while ( ZB_NIB_NETWORK_ADDRESS() == 0x00
              || ZB_NIB_NETWORK_ADDRESS() == 0xffff )
      {
        ZB_NIB_NETWORK_ADDRESS() = ZB_RANDOM();
        ZB_UPDATE_SHORT_ADDR();
      }

      nwhdr = nwk_alloc_and_fill_hdr(buf, best_parent->short_addr,
                                     ZB_PIB_EXTENDED_ADDRESS(), ZB_IEEE_ADDR_IS_ZERO(ieee_addr) ? NULL : ieee_addr,
                                     ZB_FALSE, secure, ZB_TRUE);
      nwhdr->radius = (zb_uint8_t)(ZB_NIB_MAX_DEPTH() << 1);
#ifdef ZB_SECURITY
      if (secure)
      {
        buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
      }
#endif
      rejoin_request = (zb_nwk_rejoin_request_t *)nwk_alloc_and_fill_cmd(buf, ZB_NWK_CMD_REJOIN_REQUEST, sizeof(zb_nwk_rejoin_request_t));
      rejoin_request->capability_information = ZG->nwk.handle.tmp.rejoin.capability_information;

#if 0
      {
        zb_ushort_t hdr_size = ZB_IEEE_ADDR_IS_ZERO(ieee_addr) ? ZB_NWK_HALF_HDR_SIZE(0) : ZB_NWK_FULL_HDR_SIZE(0);
#ifdef ZB_SECURITY
        if (secure)
        {
          hdr_size += sizeof(zb_nwk_aux_frame_hdr_t);
        }
#endif
        /* fill nwk header */
        ZB_BUF_INITIAL_ALLOC(buf, hdr_size, nwhdr);
      }
      ZB_BZERO(nwhdr->frame_control, sizeof(nwhdr->frame_control));
      ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(nwhdr->frame_control, ZB_NWK_FRAME_TYPE_COMMAND, ZB_PROTOCOL_VERSION);
      /*ZB_NWK_FRAMECTL_SET_DISCOVER_ROUTE(nwhdr->frame_control, 0); implied*/
      ZB_NWK_FRAMECTL_SET_SRC_DEST_IEEE(nwhdr->frame_control, 1, ZB_IEEE_ADDR_IS_ZERO(ieee_addr) ? 0 : 1);

      nwhdr->src_addr = ZB_NIB_NETWORK_ADDRESS();
      nwhdr->dst_addr = best_parent->short_addr;
      nwhdr->radius = (zb_uint8_t)(ZB_NIB_MAX_DEPTH() << 1);
      nwhdr->seq_num = ZB_NIB_SEQUENCE_NUMBER();
      ZB_NIB_SEQUENCE_NUMBER_INC();
      if ( !ZB_IEEE_ADDR_IS_ZERO(ieee_addr) )
      {
        ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, ieee_addr);
        ZB_IEEE_ADDR_COPY(nwhdr->src_ieee_addr, ZB_PIB_EXTENDED_ADDRESS());
      }
      else
      {
        /* Dirty hack here!!! Dst address is absent in nwk header, so it's
         * source address actually */
        ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, ZB_PIB_EXTENDED_ADDRESS());
      }

#ifdef ZB_SECURITY
      if (secure)
      {
        ZB_NWK_FRAMECTL_SET_SECURITY(nwhdr->frame_control, 1);
        buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
      }
#endif

      /* fill rejoin cmd & payload */
      ZB_NWK_ALLOC_COMMAND_GET_PAYLOAD_PTR(buf, ZB_NWK_CMD_REJOIN_REQUEST, zb_nwk_rejoin_request_t, rejoin_request);
      ZB_ASSERT(rejoin_request);
      rejoin_request->capability_information = ZG->nwk.handle.tmp.rejoin.capability_information;
#endif  /* 0 */

      (void)zb_mac_setup_for_associate(best_parent->logical_channel, ZB_NIB_PAN_ID(),
                                       best_parent->short_addr, ieee_addr);

      /* send request */
      ZB_SET_BUF_PARAM(buf, ZB_NWK_INTERNAL_REJOIN_CMD_HANDLE, zb_uint8_t);
      ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);

      /* save parent */
      ZG->nwk.handle.tmp.rejoin.parent = best_parent;

      /* add addr_ref to address space */
      zb_address_update(ieee_addr, best_parent->short_addr, ZB_TRUE, &addr_ref);

      /* add temporary ext entry to base */
      zb_nwk_neighbor_ext_to_base_tmp(best_parent);

      /* join response timer will be started inside confirm callback */
    }
  }

  if ( ret != RET_OK )
  {
    ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
    zb_nwk_exneighbor_stop(0);
    nwk_join_failure_confirm(param, ret);
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin_scan_confirm", (FMT__0));
}

void remove_parent_from_potential_parents(zb_ext_neighbor_tbl_ent_t *parent)
{
  if ( parent )
  {
    zb_address_ieee_ref_t addr_ref;
    zb_ieee_addr_t ieee_addr;

    /* do not choose this parent again */
    parent->potential_parent = 0;

    /* Remove tmp neighbor */
    ZB_ADDRESS_DECOMPRESS(ieee_addr, parent->long_addr);
    if ( zb_address_update(ieee_addr, parent->short_addr, ZB_FALSE, &addr_ref) == RET_OK )
    {
      zb_nwk_neighbor_delete(addr_ref);
    }
  }
}

void zb_nlme_rejoin_response_timeout(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>rejoin_resp_tmo %hd", (FMT__H, param));

  if ( ZG->nwk.handle.tmp.rejoin.parent
       && !MAC_PIB().mac_rx_on_when_idle )
  {
    /* call mlme-poll */
    zb_mlme_poll_request_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_poll_request_t);

    req->coord_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    req->coord_addr.addr_short = ZG->nwk.handle.tmp.rejoin.parent->short_addr;
    req->coord_pan_id = ZB_PIB_SHORT_PAN_ID();
    ZB_SCHEDULE_TX_CB(zb_handle_poll_request, param);
  }
  else if ( ZG->nwk.handle.tmp.rejoin.parent )
  {
    /* do not choose this parent again */
    remove_parent_from_potential_parents(ZG->nwk.handle.tmp.rejoin.parent);

    /* try to choose another parent and send join request again */
    ZB_SCHEDULE_CALLBACK(zb_nlme_rejoin_scan_confirm, param);
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin_resp_tmo", (FMT__0));
}

void zb_nlme_rejoin_response(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nwk_hdr_t *nwhdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(buf);
  zb_nwk_rejoin_response_t *rejoin_response = (zb_nwk_rejoin_response_t *)
    ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, ZB_NWK_HDR_SIZE(nwhdr->frame_control));
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_NWK1, ">>rejoin_resp %hd", (FMT__H, param));

  ZB_NWK_ADDR_TO_H16(rejoin_response->network_addr);

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_REJOIN )
  {
    zb_ieee_addr_t parent_addr;
    ZB_ADDRESS_DECOMPRESS(parent_addr, ZG->nwk.handle.tmp.rejoin.parent->long_addr);

    TRACE_MSG(TRACE_NWK1, "status %hd nwk_addr %d", (FMT__H_D, rejoin_response->rejoin_status, rejoin_response->network_addr));
    TRACE_MSG(TRACE_NWK1, "nwhdr->dst_ieee_addr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(nwhdr->dst_ieee_addr)));
    TRACE_MSG(TRACE_NWK1, "nwhdr->src_ieee_addr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(nwhdr->src_ieee_addr)));
    TRACE_MSG(TRACE_NWK1, "MAC_PIB().mac_extended_address " TRACE_FORMAT_64, (FMT__A,
              TRACE_ARG_64(MAC_PIB().mac_extended_address)));
    TRACE_MSG(TRACE_NWK1, "parent_addr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(parent_addr)));

    /* check if parent answer is correct */
    if ( rejoin_response->rejoin_status == MAC_SUCCESS
         && !ZB_NWK_IS_ADDRESS_BROADCAST(rejoin_response->network_addr)
         && ZB_NWK_FRAMECTL_GET_SOURCE_IEEE(nwhdr->frame_control)
         && (!ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control)
             || ZB_IEEE_ADDR_CMP(nwhdr->dst_ieee_addr, MAC_PIB().mac_extended_address))
         && (ZB_IEEE_ADDR_IS_ZERO(parent_addr)
             || ZB_IEEE_ADDR_CMP(nwhdr->src_ieee_addr, parent_addr)) )
    {
      /* remember this device long addr + short addr in the addr table */
      zb_address_update(ZB_PIB_EXTENDED_ADDRESS(), rejoin_response->network_addr, ZB_TRUE, &addr_ref);
      /* save parent long address if necessary */
      /* dirty hack here!!! if dst address is absent, dst pointer will be src */
      zb_address_update(ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(nwhdr->frame_control) ? nwhdr->src_ieee_addr : nwhdr->dst_ieee_addr,
                        ZG->nwk.handle.tmp.rejoin.parent->short_addr,
                        ZB_TRUE, /* lock! */
                        &ZG->nwk.handle.parent);

      /* save assigned address */
      ZB_NIB_NETWORK_ADDRESS() = rejoin_response->network_addr;
      ZB_UPDATE_SHORT_ADDR();
      ZG->nwk.handle.joined = 1;

      /* see 3.6.1.4.1.1:

         The network depth is set to one more than the parent network depth
         unless the parent network depth has a value of 0x0f, i.e. the maximum
         value for the 4-bit device depth field in the beacon payload. In this
         case, the network depth shall also be set to 0x0f.
       */
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
      ZB_NIB_DEPTH() = ZG->nwk.handle.tmp.rejoin.parent->depth + (ZB_NIB_DEPTH() != 0xf);
      /* Calculate CScip value for tree routing */
      ZG->nwk.nib.cskip = zb_nwk_daa_calc_cskip(ZB_NIB_DEPTH());
#endif

      ZB_EXTPANID_COPY(ZB_NIB_EXT_PAN_ID(), ZG->nwk.handle.tmp.rejoin.extended_pan_id);

      /* extended neighbor table is useless now */
      zb_nwk_exneighbor_stop(ZG->nwk.handle.tmp.rejoin.parent->short_addr);
      ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;

      {
        zb_neighbor_tbl_ent_t *nent;
        if (zb_nwk_neighbor_get(ZG->nwk.handle.parent, ZB_FALSE, &nent) == RET_OK)
        {
          nent->relationship = ZB_NWK_RELATIONSHIP_PARENT;
        }
        else
        {
          TRACE_MSG(TRACE_ERROR, "Oops: no our parent in the neighbor table!", (FMT__0));
        }
      }


      /* confirm join result */
      nwk_join_failure_confirm(param, MAC_SUCCESS);
    }
    else
    {
      TRACE_MSG(TRACE_NWK1, "bad rejoin resp - ignore", (FMT__0));

      /* do not choose this parent again */
      ZG->nwk.handle.tmp.rejoin.parent->potential_parent = 0;

      /* try to choose another parent and send join request again */
      ZB_SCHEDULE_CALLBACK(zb_nlme_rejoin_scan_confirm, param);
    }

    /* cancel rejoin timeout */
    ZB_SCHEDULE_ALARM_CANCEL(zb_nlme_rejoin_response_timeout, ZB_ALARM_ANY_PARAM);
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "got rejoin resp, state differ - drop", (FMT__0));
    zb_free_buf(buf);
  }

  TRACE_MSG(TRACE_NWK1, "<<rejoin_resp", (FMT__0));
}
#endif  /* ZB_LIMITED_FEATURES2 */


#ifndef ZB_LIMITED_FEATURES
static void zb_nlme_orphan_scan(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_join_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_join_request_t);

  TRACE_MSG(TRACE_NWK1, ">> orphan_scan prm %hd", (FMT__H, param));

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_IDLE )
  {
    /* perform orphan scan */
    ZB_MLME_BUILD_SCAN_REQUEST(buf, request->scan_channels, ORPHAN_SCAN, request->scan_duration);
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
    ZG->nwk.handle.state = ZB_NLME_STATE_ORPHAN_SCAN;
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "nwk is busy, state %d", (FMT__D, ZG->nwk.handle.state));
    nwk_join_failure_confirm(param, ZB_NWK_STATUS_NOT_PERMITTED);
  }

  TRACE_MSG(TRACE_NWK1, "<< orphan_scan", (FMT__0));
}

void zb_nlme_orphan_scan_confirm(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mac_scan_confirm_t *confirm = ZB_GET_BUF_PARAM(buf, zb_mac_scan_confirm_t);
  zb_uint8_t status = confirm->status;

  TRACE_MSG(TRACE_NWK1, ">> orphan_scan_cnfrm prm %hd status %hd", (FMT__H_H, param, buf->u.hdr.status));

  nwk_join_failure_confirm(param, (status == MAC_SUCCESS) ? ZB_NWK_STATUS_SUCCESS : ZB_NWK_STATUS_NO_NETWORKS);

  //ZB_EXTPANID_COPY(join_confirm->extended_pan_id, MAC_PIB().mac_beacon_payload.extended_panid);

  ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;

  TRACE_MSG(TRACE_NWK1, "<< orphan_scan_cnfrm", (FMT__0));
}

void zb_mlme_orphan_indication(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
#ifdef ZB_ROUTER_ROLE
  zb_mac_orphan_ind_t *ind = ZB_GET_BUF_PARAM(buf, zb_mac_orphan_ind_t);
  zb_address_ieee_ref_t ref;
  zb_neighbor_tbl_ent_t *nbt;
#endif
  TRACE_MSG(TRACE_NWK1, ">> orphan_ind prm %hd", (FMT__H, param));

#ifdef ZB_ROUTER_ROLE
  /* 3.6.1.4.3.3 Try to find dev in neighbor table */
  if ( zb_address_by_ieee(ind->orphan_addr, ZB_FALSE, ZB_FALSE, &ref) == RET_OK
       && zb_nwk_neighbor_get(ref, ZB_FALSE, &nbt) == RET_OK
       && nbt->relationship == ZB_NWK_RELATIONSHIP_CHILD )
  {
    zb_mac_orphan_response_t *resp = ZB_GET_BUF_PARAM(buf, zb_mac_orphan_response_t);

    /* prepare and send orphan response */
    zb_address_ieee_by_ref(resp->orphan_addr, ref);
    zb_address_short_by_ref(&resp->short_addr, ref);
    resp->associated = ZB_TRUE;

    /* send orph resp */
    ZB_SCHEDULE_TX_CB(zb_mlme_handle_orphan_response, param);
  }
  else
  {
    /* orphan dev is not our child, drop */
    TRACE_MSG(TRACE_NWK1, "not a chld, drop", (FMT__0));
    zb_free_buf(buf);
  }
#else
  /* ed doen't support orpan scan, just drop packet */
  TRACE_MSG(TRACE_NWK1, "orph ind for ed dev, drop", (FMT__0));
  zb_free_buf(buf);
#endif

  TRACE_MSG(TRACE_NWK1, "<< orphan_ind", (FMT__0));
}
#endif  /* ZB_LIMITED_FEATURES */


void zb_nlme_join_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_nlme_join_request_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_request_t);

  TRACE_MSG(TRACE_NWK1, ">>join_req %hd", (FMT__H, param));
  CHECK_PARAM_RET_ON_ERROR(request);

  ZG->nwk.handle.router_started = 0;
  switch ( request->rejoin_network )
  {
    case ZB_NLME_REJOIN_METHOD_ASSOCIATION:
      /* On receipt of this primitive by a device that is not currently joined
         to a network and with the RejoinNetwork parameter equal to 0x00, the
         device attempts to join the network specified by the 64-bit
         ExtendedPANId parameter as described in sub-clause3.6.1.4.1.1. */
      ret = nwk_association_join((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_request_t));
      if (ret)
      {
        nwk_join_failure_confirm(param, ret);
        ret = 0;
      }
      break;

    case ZB_NLME_REJOIN_METHOD_DIRECT:
#ifndef ZB_LIMITED_FEATURES
      zb_nlme_orphan_scan(param);
#endif
      break;

    case ZB_NLME_REJOIN_METHOD_REJOIN:
      zb_nlme_rejoin(param);
      break;

    case ZB_NLME_REJOIN_METHOD_CHANGE_CHANNEL:
      TRACE_MSG(TRACE_ERROR, "Change channel not impl", (FMT__0));
      break;

    default:
      break;
  }

  TRACE_MSG(TRACE_NWK1, "<<join_req %d", (FMT__D, ret));
}


void zb_mlme_comm_status_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  zb_address_ieee_ref_t addr_ref;
  zb_neighbor_tbl_ent_t *nent = NULL;
  
  zb_mlme_comm_status_indication_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_comm_status_indication_t);

  TRACE_MSG(TRACE_NWK1, ">>mlme_comm_status_ind", (FMT__0));
  {
	  zb_uint8_t i;
	  for (i=0; i<8;i++)
	  {
	  	TRACE_MSG(TRACE_NWK1, "addr[] %hd %hx", (FMT__H_H, i, request->dst_addr.addr_long[i] ));
	  }
  }
  if (request->dst_addr_mode == ZB_ADDR_64BIT_DEV)
  {
    ret = zb_address_by_ieee(request->dst_addr.addr_long, ZB_FALSE, ZB_FALSE, &addr_ref);
  }
  else
  {
    ret = zb_address_by_short(request->dst_addr.addr_short, ZB_FALSE, ZB_FALSE, &addr_ref);
  }

  if ( ret == RET_OK )
  {
    ret = zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &nent);
  }

  if ( ret == RET_OK )
  {
    if (request->status == MAC_SUCCESS)
    {
      /* Done. Issue NLME-JOIN.indication, update  */

      zb_nlme_join_indication_t *resp = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_indication_t);

      zb_address_by_ref(resp->extended_address, &resp->network_address, addr_ref);
      resp->capability_information = 0;
      /* Only that 2 fields of capability information are meaningful */
      ZB_MAC_CAP_SET_DEVICE_TYPE(resp->capability_information, (nent->device_type == ZB_NWK_DEVICE_TYPE_ROUTER));
      ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(resp->capability_information, nent->rx_on_when_idle);
      /* TODO: fill that fields */
      resp->rejoin_network = ZB_NLME_REJOIN_METHOD_ASSOCIATION;
      resp->secure_rejoin = 0;
      ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status = ZB_NWK_STATUS_SUCCESS;

      /* Network configuration has changed - change update id. */
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
      zb_nwk_update_beacon_payload();
#endif
      ZB_SCHEDULE_CALLBACK(zb_nlme_join_indication, param);
    }
    else
    {
      /* Failed. Remove this device from address and neighbor tables. */
      zb_nwk_neighbor_delete(addr_ref);
      zb_address_delete(addr_ref);
    }
  }
  TRACE_MSG(TRACE_NWK1, "<<mlme_comm_status_ind", (FMT__0));
}

/*! @} */
