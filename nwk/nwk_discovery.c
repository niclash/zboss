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
PURPOSE: Network discovery routine
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_5.h"

void zb_nlme_network_discovery_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_network_discovery_request_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_network_discovery_request_t);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_network_discovery_request %p", (FMT__P, request));
  CHECK_PARAM_RET_ON_ERROR(request);

  /*
    see 3.6.1.3  Network Discovery
    Can be run before or after join. If run before join, fill neighbor table
    (both its "main" and "additional" parts.
    If run after join - not sure yet. When could it be used?


    Calls sequence:
    > NETWORK-DISCOVERY.request
      > MLME-SCAN.request
      < NETWORK-DISCOVERY.request
      < NETWORK-DISCOVERY.request
      ...
      < MLME-SCAN.confirm
    < NLME-NETWORK-DISCOVERY.confirm
   */

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_IDLE )
  {
    /* start writing to the extended neigbor table: eat some space from the
     * neighbor table */
    /* For ED neighbor table has size 1 for parent entrie only, and ext table has fixed size */
#ifndef ZB_ED_ROLE
    zb_nwk_exneighbor_start();
#endif
    /* Don't forget to call zb_nwk_exneighbor_stop() after successful join (not
     * discovery! ext neighbor used for potential parent search). */

    {
      zb_nlme_network_discovery_request_t rq;
      ZB_MEMCPY(&rq, request, sizeof(rq));
      TRACE_MSG(TRACE_NWK1, "scan channels 0x%x scan_duration %hd", (FMT__D_H, rq.scan_channels, rq.scan_duration));
      ZB_MLME_BUILD_SCAN_REQUEST((zb_buf_t *)ZB_BUF_FROM_REF(param), rq.scan_channels, ACTIVE_SCAN, rq.scan_duration);
    }
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
    ZG->nwk.handle.state = ZB_NLME_STATE_DISC;
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "nwk is busy, state %d", (FMT__D, ZG->nwk.handle.state));
    ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status = RET_BUSY;
    ZB_SCHEDULE_CALLBACK(zb_nlme_network_discovery_confirm, param);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_network_discovery_request", (FMT__0));
}

#ifndef ZB_LIMITED_FEATURES
void zb_nlme_ed_scan_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_ed_scan_request_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_ed_scan_request_t);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_ed_scan_request %p", (FMT__P, request));
  CHECK_PARAM_RET_ON_ERROR(request);

  if (ZB_NIB_DEVICE_TYPE() != ZB_NWK_DEVICE_TYPE_NONE )
  {
    {
      zb_nlme_ed_scan_request_t rq;
      ZB_MEMCPY(&rq, request, sizeof(rq));
      ZB_MLME_BUILD_SCAN_REQUEST((zb_buf_t *)ZB_BUF_FROM_REF(param), rq.scan_channels, ED_SCAN, rq.scan_duration);
    }
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
    ZG->nwk.handle.state = ZB_NLME_STATE_ED_SCAN;
  }
  else
  {
    NWK_CONFIRM_STATUS((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_NWK_STATUS_INVALID_REQUEST, zb_nlme_ed_scan_confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_ed_scan_request", (FMT__0));
}
#endif


void zb_mlme_beacon_notify_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t *mac_hdr = ZB_MAC_GET_FCF_PTR(ZB_BUF_BEGIN((zb_buf_t *)ZB_BUF_FROM_REF(param)));
  zb_mac_mhr_t mhr;
  zb_uint8_t mhr_len;
  zb_mac_beacon_payload_t *beacon_payload;


  TRACE_MSG(TRACE_NWK1, ">>zb_mlme_beacon_notify_indication %hd", (FMT__H, param));

  mhr_len = zb_parse_mhr(&mhr, mac_hdr);
  ZB_MAC_GET_BEACON_PAYLOAD(mac_hdr, mhr_len, beacon_payload);

#ifdef ZB_MAC_TESTING_MODE
  {
    zb_uint8_t payload_len;

    payload_len = ZB_BUF_LEN(ZB_BUF_FROM_REF(param)) - ZB_BEACON_PAYLOAD_OFFSET(mhr_len) - ZB_BEACON_PAYLOAD_TAIL;

    TRACE_MSG(TRACE_NWK1, "beacon payload dump", (FMT__0));
    dump_traf((zb_uint8_t*)beacon_payload, payload_len);
  }
#else  /* ZB_MAC_TESTING_MODE */

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_DISC
       || ZG->nwk.handle.state == ZB_NLME_STATE_REJOIN )
  {
    /*
      Fill neighbor tables here (both base and additional parts).
      Note that additional part has some fields which used to construct
      NetworkDescriptor for the NLME-NETWORK-DISCOVERY.confirm.
      MAC does not keep PAN descriptors table: it not used in PAN and it is
      simpler to us to not analyze it.
      Instead put additional fields into additional neighbor table entry.
     */

    /* Analyze beacon payload - see 3.6.7 */


    TRACE_MSG(TRACE_NWK2, "beacon payload %p: protocol id %d, version %d, stack profile %d", (FMT__P_D_D_D,
                           beacon_payload, (int)beacon_payload->protocol_id,
                           (int)beacon_payload->protocol_version, (int)beacon_payload->stack_profile));

    /*
      Check for "not out" nets. Not sure it is really necessary.
      Also not sure do we need to ignore beacons without payload?
      Seems yes: we returns extended panid only in MLME-SCAN.confirm and it can
      be get from the beacon payload only.
     */
    if ( beacon_payload
         && beacon_payload->protocol_id == 0
         && beacon_payload->protocol_version == ZB_PROTOCOL_VERSION
/* if we are ed, we could connect to any net */
#ifndef ZB_ED_ROLE
#if ZB_STACK_PROFILE == 1
         /* ignore PRO if we are 2007 */
		 /* we should not ignore pro, we just need to join and act as end device, no matter, router or ed */
/*        && beacon_payload->stack_profile == ZB_STACK_PROFILE */
#endif
#endif
      )
    {
      if ( (ZG->nwk.handle.state != ZB_NLME_STATE_REJOIN)
           || ZB_64BIT_ADDR_CMP(beacon_payload->extended_panid, ZG->nwk.handle.tmp.rejoin.extended_pan_id))
        /* In rejoin state we update devices only from defined ExtendedPanId */
      {
        zb_ext_neighbor_tbl_ent_t *enbt = NULL; /* shutup sdcc */
        zb_address_pan_id_ref_t panid_ref;
        zb_ret_t ret;

        /* remember extended pan id */
        ret = zb_address_set_pan_id(mhr.src_pan_id, beacon_payload->extended_panid, &panid_ref);
        if (ret == RET_ALREADY_EXISTS)
        {
          ret = RET_OK;
        }

        /* Fill or update extended neighbor table */
        /* First get address from the MAC header */
        if (ret == RET_OK)
        {
          /* Not sure: can beacon use 64-bit address? Let's handle it anyway */
          if (ZB_FCF_GET_SRC_ADDRESSING_MODE(&mhr.frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
          {
            /* 16 bit address. */
            ret = zb_nwk_exneighbor_by_short(panid_ref, mhr.src_addr.addr_short, &enbt);
          }
#ifndef ZB_LIMITED_FEATURES
          else
          {
            ret = zb_nwk_exneighbor_by_ieee(panid_ref, mhr.src_addr.addr_long, &enbt);
          }
#endif
        }

        if (ret == RET_OK)
        {
          void * superframe_spec;

          ZB_MAC_GET_SUPERFRAME_SPEC(mac_hdr, mhr_len, superframe_spec);

          /* Table 3.48, page 368. Address is assigned already. */
          /* have no rx_on_when_idle here: it could be defined by capabilitries at
             join time only, or by device annonsment */
          /* don't touch relationship: no relationship before join */
          /* don't touch transmit_failure here */
          enbt->lqi = ZB_MAC_GET_LQI((zb_buf_t *)ZB_BUF_FROM_REF(param));
#ifdef ZB_NS_BUILD
          enbt->lqi = enbt->lqi < 100 ? 100 : enbt->lqi;
#endif
          /* don't touch outgoing_cost */
          /* nod't use age */
          /* additional fields - table 3.49 */
          enbt->panid_ref = panid_ref;
          enbt->logical_channel = ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL();
          TRACE_MSG(TRACE_NWK2, "ch %hd", (FMT__H, enbt->logical_channel));
          enbt->depth = beacon_payload->device_depth;
          enbt->beacon_order = ZB_MAC_GET_BEACON_ORDER(superframe_spec);
          enbt->permit_joining = ZB_MAC_GET_ASSOCIATION_PERMIT(superframe_spec);
          /* fields for the Network Descriptor - table 3.8 */
          enbt->stack_profile = beacon_payload->stack_profile;
          /* do not store zigbee version: assume compatible only */
          enbt->superframe_order = ZB_MAC_GET_SUPERFRAME_ORDER(superframe_spec);
          enbt->router_capacity = beacon_payload->router_capacity;
          enbt->end_device_capacity = beacon_payload->end_device_capacity;

          /* Some complex stuff */
#ifdef ZB_ROUTER_ROLE
          if ((ZB_NIB_DEVICE_TYPE() != (zb_uint8_t)ZB_NWK_DEVICE_TYPE_ED)&&(enbt->stack_profile != 2))
          {
            enbt->potential_parent = enbt->permit_joining && enbt->router_capacity;
          }
          else
#endif
          {
            enbt->potential_parent = enbt->permit_joining && enbt->end_device_capacity;
          }

          /* No sure about device type. Can detect some cases, but not all. */
          if (enbt->depth == 0)
          {
            enbt->device_type = ZB_NWK_DEVICE_TYPE_COORDINATOR; /* see table 3.56*/
          }
          else if (enbt->permit_joining || enbt->router_capacity || enbt->end_device_capacity)
          {
            enbt->device_type = ZB_NWK_DEVICE_TYPE_ROUTER;
          }
          else
          {
            /* No more info - device type is unknown */
            enbt->device_type = ZB_NWK_DEVICE_TYPE_NONE;
          }
        } /* if ok */
      } /* if this beacon could be interesting for us */
      else
      {
        TRACE_MSG(TRACE_NWK2, "Skip beacon not for us: its protocol id %hd, version %hd, stack profile %hd", (FMT__H_H_H,
                                                                                                              beacon_payload->protocol_id,
                                                                                                              beacon_payload->protocol_version,
                                                                                                              beacon_payload->stack_profile));
      }
    }
  }

#ifdef ZB_ROUTER_ROLE
/* removed, because seems like there should be not active scan on server side */
/* during PAN_ID conflict resolution										  */
  else if ((ZG->nwk.handle.joined)&&(!ZG->nwk.handle.joined_pro))
  {
    /* Detect PANID conflict, On HW seems not need to check for short panid, but
     * let's do it for ns */
    if (mhr.src_pan_id == ZB_NIB_PAN_ID()
        && beacon_payload
        && !ZB_EXTPANID_CMP(beacon_payload->extended_panid, ZB_NIB_EXT_PAN_ID()))
    {
      if (ZG->nwk.nib.nwk_manager_addr == ZB_PIB_SHORT_ADDRESS())
      {
        TRACE_MSG(TRACE_NWK1, "Got PANID conflict, I am a nwk manager - start panid conflict resolve", (FMT__0));
        ZB_MEMSET(ZG->nwk.handle.known_panids, -1, sizeof(ZG->nwk.handle.known_panids));
        ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
        TRACE_MSG(TRACE_NWK3, "sending Pan ID conflict resolution network update", (FMT__0));
        zb_panid_conflict_network_update(param);
      }
      else
      {
        zb_panid_conflict_schedule_network_report(param, mhr.src_pan_id);
      }
      param = (zb_uint8_t)~0;
    }
  }
#endif
  else
  {
    TRACE_MSG(TRACE_NWK1, "NWK state is %d - not ZB_NLME_STATE_DISC - ignore beacon", (FMT__D, ZG->nwk.handle.state));
  }

#ifdef ZB_ROUTER_ROLE
  if (param != (zb_uint8_t)~0)
#endif
  {
    zb_free_buf((zb_buf_t *)ZB_BUF_FROM_REF(param));
  }
#endif /* ZB_MAC_TESTING_MODE */

  TRACE_MSG(TRACE_NWK1, "<<zb_mlme_beacon_notify_indication", (FMT__0));
}


void zb_mlme_scan_confirm(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>zb_mlme_scan_confirm %hd", (FMT__H, param));

  TRACE_MSG(TRACE_NWK1, "nwk state %d", (FMT__D, (int)ZG->nwk.handle.state));

  if ( ZG->nwk.handle.state == ZB_NLME_STATE_DISC )
  {
    /* scan ext neighbor table created in zb_mlme_beacon_notify_indication() - create Network descriptor  */
    zb_nlme_network_discovery_confirm_t *discovery_confirm = NULL;
    zb_nlme_network_descriptor_t *network_descriptor = NULL;
    zb_ushort_t i, j;
    zb_ushort_t n_nwk_dsc = 0;
    zb_uint8_t dev_type_cmp = 2;

//#if defined ZB_ROUTER_ROLE && !defined ZB_NWK_ONE_SCAN_ATTEMPT
    if (ZDO_CTX().zdo_ctx.discovery_ctx.disc_count)
    {
      ZDO_CTX().zdo_ctx.discovery_ctx.disc_count--;
    }
    TRACE_MSG(TRACE_NWK3, "disc_count %hd", (FMT__H, ZDO_CTX().zdo_ctx.discovery_ctx.disc_count));

    if (ZDO_CTX().zdo_ctx.discovery_ctx.disc_count)
    {
      zb_nlme_network_discovery_request_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_network_discovery_request_t);

      req->scan_channels = ZB_AIB().aps_channel_mask;
      req->scan_duration = ZB_DEFAULT_SCAN_DURATION;
      TRACE_MSG(TRACE_NWK3, "call discovery_request channels 0x%x", (FMT__D, req->scan_channels));
      ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
      ZB_SCHEDULE_ALARM(zb_nlme_network_discovery_request, param, ZDO_CTX().conf_attr.nwk_time_btwn_scans);
      /* TODO: for macAutoRequest == TRUE case store pan descriptor list */
    }
    else
//#endif /* */
    {
#ifdef ZB_MAC_TESTING_MODE
      zb_mac_scan_confirm_t *scan_confirm;
      scan_confirm = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mac_scan_confirm_t);
      TRACE_MSG(TRACE_NWK3, "scan type %hd, status %hd, auto_req %hd",
                (FMT__H_H_H, scan_confirm->scan_type, scan_confirm->status, MAC_PIB().mac_auto_request));
      if (scan_confirm->scan_type == ACTIVE_SCAN && scan_confirm->status == MAC_SUCCESS && MAC_PIB().mac_auto_request)
      {
        zb_pan_descriptor_t *pan_desc;
        pan_desc = (zb_pan_descriptor_t*)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
        TRACE_MSG(TRACE_NWK3, "ative scan res count %hd", (FMT__H, scan_confirm->result_list_size));
        for(i = 0; i < scan_confirm->result_list_size; i++)
        {
          TRACE_MSG(TRACE_NWK3,
                    "pan desc: coord addr mode %hd, coord addr %x, pan id %x, channel %hd, superfame %x, lqi %hx",
                    (FMT__H_D_D_H_D_H, pan_desc->coord_addr_mode, pan_desc->coord_address.addr_short, pan_desc->coord_pan_id,
                     pan_desc->logical_channel, pan_desc->super_frame_spec, pan_desc->link_quality));

          if (pan_desc->coord_addr_mode == ZB_ADDR_64BIT_DEV)
          {
            TRACE_MSG(TRACE_MAC3, "Extended coord addr " TRACE_FORMAT_64,
                      (FMT__A, TRACE_ARG_64(pan_desc->coord_address.addr_long)));
          }

          pan_desc++;
        }
      }
#endif

      /* buf->hdr.status will not change - use it as is */
      /* prepare result buffer and fill it */
      ZB_BUF_INITIAL_ALLOC((zb_buf_t *)ZB_BUF_FROM_REF(param),
                           sizeof(*discovery_confirm) + sizeof(*network_descriptor) * ZB_PANID_TABLE_SIZE,
                           discovery_confirm);
      network_descriptor = (zb_nlme_network_descriptor_t *)(discovery_confirm + 1);

      while (dev_type_cmp)
      {
        dev_type_cmp--;
        /* First pass: get info about coordinators only */

        for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
        {
          if (!dev_type_cmp || ZG->nwk.neighbor.ext_neighbor[i].device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR)
          {
            for (j = 0 ;
                 j < n_nwk_dsc &&
                   !zb_address_cmp_pan_id_by_ref(ZG->nwk.neighbor.ext_neighbor[i].panid_ref, network_descriptor[j].extended_pan_id) ;
                 ++j)
            {
            }
            if (j == n_nwk_dsc)
            {
              /* This ext pan id not found - add this PAN */
              zb_address_get_pan_id(ZG->nwk.neighbor.ext_neighbor[i].panid_ref, network_descriptor[j].extended_pan_id);
              network_descriptor[j].logical_channel = ZG->nwk.neighbor.ext_neighbor[i].logical_channel;
              network_descriptor[j].stack_profile = ZG->nwk.neighbor.ext_neighbor[i].stack_profile;
              network_descriptor[j].zigbee_version = ZB_PROTOCOL_VERSION;
              network_descriptor[j].beacon_order = ZG->nwk.neighbor.ext_neighbor[i].beacon_order;
              network_descriptor[j].superframe_order = ZG->nwk.neighbor.ext_neighbor[i].superframe_order;
              network_descriptor[j].permit_joining = ZG->nwk.neighbor.ext_neighbor[i].permit_joining;
              network_descriptor[j].router_capacity = ZG->nwk.neighbor.ext_neighbor[i].router_capacity;
              network_descriptor[j].end_device_capacity = ZG->nwk.neighbor.ext_neighbor[i].end_device_capacity;
              n_nwk_dsc++;
            }
          }
        } /* ext neighbor table iterate */
      } /* while */
      discovery_confirm->network_count = n_nwk_dsc;
      discovery_confirm->status = (zb_mac_status_t)((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status;
      ZB_SCHEDULE_CALLBACK(zb_nlme_network_discovery_confirm, param);
      ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
    }
  }

#ifndef ZB_LIMITED_FEATURES
  else if ( ZG->nwk.handle.state == ZB_NLME_STATE_ED_SCAN )
  {
    ZB_SCHEDULE_CALLBACK(zb_nlme_ed_scan_confirm, param);
    ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
  }
#endif
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
  else if ( ZG->nwk.handle.state == ZB_NLME_STATE_FORMATION_ED_SCAN )
  {
    nwk_formation_ed_scan_confirm((zb_buf_t *)ZB_BUF_FROM_REF(param));
  }
  else if ( ZG->nwk.handle.state == ZB_NLME_STATE_FORMATION_ACTIVE_SCAN )
  {
    nwk_formation_select_channel((zb_buf_t *)ZB_BUF_FROM_REF(param));
  }
#endif
#ifndef ZB_LIMITED_FEATURES
  else if ( ZG->nwk.handle.state == ZB_NLME_STATE_REJOIN )
  {
    zb_nlme_rejoin_scan_confirm(param);
  }
  else if ( ZG->nwk.handle.state == ZB_NLME_STATE_ORPHAN_SCAN )
  {
    zb_nlme_orphan_scan_confirm(param);
  }
#endif
  else
  {
    TRACE_MSG(TRACE_ERROR, "wrong nwk state %d", (FMT__D, (int)ZG->nwk.handle.state));
    ZB_ASSERT(0);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_mlme_scan_confirm", (FMT__0));
}


/*! @} */
