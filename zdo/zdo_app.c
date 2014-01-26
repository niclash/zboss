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
PURPOSE: Typical ZDO applications: ZC, ZR, ZE
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

#include "zb_bank_8.h"
/*! \addtogroup ZB_ZDO */
/*! @{ */
#ifdef APS_RETRANSMIT_TEST
static void send_data();
#endif

void zdo_send_device_annce(zb_uint8_t param) ZB_CALLBACK;
void zdo_join_done(zb_uint8_t param) ZB_CALLBACK;
void zb_zdo_force_child_leave(zb_uint8_t param, zb_uint16_t child_addr) ZB_SDCC_REENTRANT;
static void init_config_attr() ZB_SDCC_REENTRANT;

void zb_zdo_init() ZB_CALLBACK
{
  ZDO_CTX().conf_attr.nwk_indirect_poll_rate = ZB_ZDO_INDIRECT_POLL_TIMER;
  ZDO_CTX().max_parent_threshold_retry = ZB_ZDO_MAX_PARENT_THRESHOLD_RETRY;
#if 0
  /* that values already zeroed by global init */
  ZDO_CTX().parent_threshold_retry = 0;
  ZDO_CTX().system_server_discovery_cb = NULL;
  ZDO_CTX().long_timer_cb = NULL;
#endif
  ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_1].end_device_bind_param = ZB_UNDEFINED_BUFFER;
  ZDO_CTX().end_device_bind_ctx.bind_device_info[ZB_ZDO_BIND_DEV_2].end_device_bind_param = ZB_UNDEFINED_BUFFER;
  ZDO_CTX().handle.allow_auth = 1;

  init_config_attr();
}

static void init_config_attr() ZB_SDCC_REENTRANT
{
  ZDO_CTX().conf_attr.nwk_scan_attempts = ZB_ZDO_NWK_SCAN_ATTEMPTS;
  ZDO_CTX().conf_attr.nwk_time_btwn_scans = ZB_ZDO_NWK_TIME_BTWN_SCANS;
  ZDO_CTX().conf_attr.enddev_bind_timeout = ZB_ZDO_ENDDEV_BIND_TIMEOUT;
  ZDO_CTX().conf_attr.permit_join_duration = ZB_DEFAULT_PRMIT_JOINING_DURATION;

#ifndef ZB_LIMITED_FEATURES
#if defined ZB_COORDINATOR_ROLE
  zb_set_default_ffd_descriptor_values(ZB_COORDINATOR);
#elif defined ZB_ROUTER_ROLE
  zb_set_default_ffd_descriptor_values(ZB_ROUTER);
#else
  /* ZB_END_DEVICE */
  zb_set_default_ed_descriptor_values();
#endif
#endif
}

void zdo_main_loop()
{
  while (1)
  {
    zb_sched_loop_iteration();
  }
}

zb_ret_t zdo_dev_start() ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;

  /* zb_zdo_init(); it is called in zb_init() */

  /* Startup procedure as defined in 2.5.5.5.6.2    Startup Procedure */

#ifdef ZB_USE_NVRAM
  zb_read_formdesc_data();
#endif
  if (ZB_EXTPANID_IS_ZERO(ZB_NIB_EXT_PAN_ID()))
  {
    /* This call is here to take into account parameters changed after
     * zb_init() but before zdo_dev_start(). For instance, it can be MAC
     * address and pan id. */
    zb_handle_parms_before_start();
    TRACE_MSG(TRACE_APS1, "ext pan id 0 - startup", (FMT__0));
    if (ZB_AIB().aps_designated_coordinator)
    {
#ifdef ZB_COORDINATOR_ROLE
      /* will start as coordinator: Formation */
      zb_buf_t *buf = zb_get_out_buf();
      zb_nlme_network_formation_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_network_formation_request_t);
      /* we must set nwkExtendedPanID to aspUseExtendedPanID if any */
      if (!ZB_EXTPANID_IS_ZERO(ZB_AIB().aps_use_extended_pan_id))
      {
        ZB_IEEE_ADDR_COPY(ZB_NIB_EXT_PAN_ID(), ZB_AIB().aps_use_extended_pan_id);
      }
      req->scan_channels = ZB_AIB().aps_channel_mask;
      req->scan_duration = ZB_DEFAULT_SCAN_DURATION; /* TODO: configure it somehow? */
                         /* timeout for every channel is
                                   ((1l<<duration) + 1) * 15360 / 1000000

                                   For duration 8 ~ 4s
                                   For duration 5 ~0.5s
                                   For duration 2 ~0.08s
                                   For duration 1 ~0.05s
                                */
      ret = ZB_SCHEDULE_CALLBACK(zb_nlme_network_formation_request, ZB_REF_FROM_BUF(buf));    

#else
      TRACE_MSG(TRACE_MAC1, "Coordinator role is not supported", (FMT__0));
      ret = RET_NOT_IMPLEMENTED;
#endif
    }
#ifndef ZB_LIMITED_FEATURES
    else if (!ZB_EXTPANID_IS_ZERO(ZB_AIB().aps_use_extended_pan_id))
    {
      /* try to rejoin */
      zb_buf_t *buf = zb_get_out_buf();
      ret = zdo_initiate_rejoin(buf);
    }
#endif
    else
    {
      /* ZR or ZC: discovery, then join */
      zb_buf_t *buf = zb_get_out_buf();
      zb_nlme_network_discovery_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_network_discovery_request_t);
#ifdef ZB_ROUTER_ROLE
      if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE)
      {
        ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
      }
#endif
      req->scan_channels = ZB_AIB().aps_channel_mask;
      req->scan_duration = ZB_DEFAULT_SCAN_DURATION; /* TODO: configure it somehow? */
      TRACE_MSG(TRACE_APS1, "disc, then join by assoc", (FMT__0));

      ZDO_CTX().zdo_ctx.discovery_ctx.disc_count = ZDO_CTX().conf_attr.nwk_scan_attempts;
      ret = ZB_SCHEDULE_CALLBACK(zb_nlme_network_discovery_request, ZB_REF_FROM_BUF(buf));
    }
  }
  else
  {
#ifdef ZB_USE_NVRAM
    zb_buf_t *buf = zb_get_out_buf();
	buf->u.hdr.status = ZB_NWK_STATUS_ALREADY_PRESENT;
    ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, ZB_REF_FROM_BUF(buf));
#endif
    TRACE_MSG(TRACE_APS1, "already in nw", (FMT__0));
    /* TODO: verify that we have right channel ID and, maybe, do active scan to
     * find the channel. */
  }
  //temp trace
  TRACE_MSG(TRACE_APS1, "return now", (FMT__0));
  return ret;
}


#ifndef ZB_LIMITED_FEATURES2
zb_ret_t zdo_initiate_rejoin(zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  zb_nlme_join_request_t *req = ZB_GET_BUF_PARAM(buf, zb_nlme_join_request_t);

  TRACE_MSG(TRACE_APS1, ">>zdo_initiate_rejoin ", (FMT__0));
  ZB_BZERO(req, sizeof(*req)); /* all defaults to 0 */

  ZG->zdo.handle.started = 0;
  ZB_EXTPANID_COPY(req->extended_pan_id, ZB_AIB().aps_use_extended_pan_id);
#ifdef ZB_ROUTER_ROLE
  if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE)
  {
    ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
    ZB_MAC_CAP_SET_ROUTER_CAPS(req->capability_information); /* join as ZR */
    TRACE_MSG(TRACE_APS1, "Rejoin to pan " TRACE_FORMAT_64 " as ZR", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
  }
  else
#endif
  {
    TRACE_MSG(TRACE_APS1, "Rejoin to pan " TRACE_FORMAT_64 " as ZE", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));
    if (MAC_PIB().mac_rx_on_when_idle)
    {
      ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(req->capability_information, 1);
    }
  }
  /* if join as ZE - all cap to 0 (set by memset) */
  ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(req->capability_information, 1);
  req->rejoin_network = ZB_NLME_REJOIN_METHOD_REJOIN;
  req->scan_channels = ZB_AIB().aps_channel_mask;
  req->scan_duration = ZB_DEFAULT_SCAN_DURATION; /* TODO: configure it somehow? */
  ZG->zdo.handle.rejoin = 1;
  ZG->nwk.handle.joined = 0;
#ifndef ZB_NS_BUILD
  //ZB_CLEAR_SHORT_ADDR(); /* to prevent from receiving packets during rejoin */
#endif
  TRACE_MSG(TRACE_APS1, "<<zdo_initiate_rejoin ", (FMT__0));
  return ZB_SCHEDULE_CALLBACK(zb_nlme_join_request, ZB_REF_FROM_BUF(buf));
}
#endif


#ifdef ZB_COORDINATOR_ROLE

void zb_nlme_network_formation_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_permit_joining_request_t *request = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_permit_joining_request_t);
  TRACE_MSG(TRACE_NWK1, "formation conf st %hd", (FMT__H, ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status));

#if defined ZB_SECURITY
  secur_tc_init();
#endif

  if (ZG->nwk.nib.max_children > 0)
  {
    request->permit_duration = ZDO_CTX().conf_attr.permit_join_duration;
  }
  else
  {
    request->permit_duration = 0;
  }
  ZB_SCHEDULE_CALLBACK(zb_nlme_permit_joining_request, param);
}


void zb_nlme_permit_joining_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_NWK1, "permit j conf st %hd", (FMT__H, ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status));

  ZB_BUF_FROM_REF(param)->u.hdr.status = 0;

  zb_address_by_short(ZB_PIB_SHORT_ADDRESS(), ZB_TRUE, ZB_FALSE, &addr_ref);
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);

}
#endif  /* ZB_COORDINATOR_ROLE */

void zb_nlme_join_indication(zb_uint8_t param) ZB_CALLBACK
{
#ifdef ZB_TRACE_LEVEL
  zb_nlme_join_indication_t *ind = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_join_indication_t);
  TRACE_MSG(TRACE_NWK1, "JOINED (st %hd) dev 0x%x/" TRACE_FORMAT_64 " cap: dev type %hd rx.w.i. %hd rejoin %hd secur %hd",
            (FMT__H_D_A_H_H_H_H,
             (zb_uint8_t)(((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status),
             (zb_uint16_t)ind->network_address,
             TRACE_ARG_64(ind->extended_address),
              ZB_MAC_CAP_GET_DEVICE_TYPE(ind->capability_information),
              ZB_MAC_CAP_GET_RX_ON_WHEN_IDLE(ind->capability_information),
              ind->rejoin_network, ind->secure_rejoin, ZB_NIB_SECURITY_LEVEL()));
#endif

#if defined ZB_SECURITY && defined ZB_COORDINATOR_ROLE
  if (ZG->nwk.nib.security_level != 0)
  {
    /* Authenticate device: send network key to it */
    ZB_SCHEDULE_CALLBACK(secur_authenticate_child, param);
  }
  else
#endif
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }

}


void zb_nlme_network_discovery_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_network_discovery_confirm_t *cnf;
  zb_nlme_network_descriptor_t *dsc;
  zb_ushort_t i;
  zb_nlme_join_request_t *req;

  TRACE_MSG(TRACE_NWK1, "disc st %hd", (FMT__H, ((zb_buf_t *)ZB_BUF_FROM_REF(param))->u.hdr.status));
  cnf = (zb_nlme_network_discovery_confirm_t *)ZB_BUF_BEGIN((zb_buf_t *)ZB_BUF_FROM_REF(param));
  dsc = (zb_nlme_network_descriptor_t *)(cnf + 1);

  TRACE_MSG(TRACE_NWK1, "Disc res: st %hd, nw_cnt %hd", (FMT__H_H,
            (int)cnf->status, (int)cnf->network_count));

#ifdef ZB_TRACE_LEVEL
  for (i = 0 ; i < cnf->network_count ; ++i)
  {
    TRACE_MSG(TRACE_NWK1,
              "net %hd: xpanid " TRACE_FORMAT_64 ", ch %hd, s.prof %hd, zb v %hd, beacon_ord %hd, superf_ord %hd, permit_j %hd, rtr_cap %hd, ed_cap %hd", (FMT__H_A_H_H_H_H_H_H_H_H,
              i, TRACE_ARG_64(dsc->extended_pan_id),
              (int)dsc->logical_channel, (int)dsc->stack_profile, (int)dsc->zigbee_version,
              (int)dsc->beacon_order,
              (int)dsc->superframe_order, (int)dsc->permit_joining,
              (int)dsc->router_capacity, (int)dsc->end_device_capacity));
    dsc++;
  }
#endif

  req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_join_request_t);
  dsc = (zb_nlme_network_descriptor_t *)(cnf + 1);

  /* Now join thru Association */
  for (i = 0 ; i < cnf->network_count ; ++i)
  {
    if (ZB_EXTPANID_IS_ZERO(ZB_AIB().aps_use_extended_pan_id)
        || ZB_EXTPANID_CMP(dsc->extended_pan_id, ZB_AIB().aps_use_extended_pan_id))
    {
      /*
        Now join to the first network or network with desired ext pan id.
        TODO: find best pan to join to.
      */

      ZB_BZERO(req, sizeof(*req)); /* all defaults to 0 */
      ZB_EXTPANID_COPY(req->extended_pan_id, dsc->extended_pan_id);
#ifdef ZB_ROUTER_ROLE
/* joined_pro, here's one of the key moments */
      if ((ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE
          || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)
#ifdef ZB_PRO_COMPATIBLE
          &&(dsc->stack_profile == 1)
#endif
         )
      {
        ZB_MAC_CAP_SET_ROUTER_CAPS(req->capability_information);  /* join as router */
      }
      else
#endif
      {
        if (MAC_PIB().mac_rx_on_when_idle)
        {
          ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(req->capability_information, 1);
        }
      }
      ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(req->capability_information, 1);

      ZB_SCHEDULE_CALLBACK(zb_nlme_join_request, param);
      break;
    }
  } /* for */
  if (i == cnf->network_count)
  {
    TRACE_MSG(TRACE_APS1, "Can't find PAN to join to!", (FMT__0));
    /* Indicate startup failure */
    ZB_BUF_FROM_REF(param)->u.hdr.status = ZB_NWK_STATUS_NOT_PERMITTED;
    ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
  }
}

void zdo_join_done(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>join_done %hd", (FMT__H, param));

  /* Not sure this is right, but let's send annonce after authentication complete */
#ifdef ZB_SECURITY
  if (ZG->nwk.nib.security_level != 0)
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
  else
#endif
  {
    ZB_SCHEDULE_CALLBACK(zdo_send_device_annce, param);
  }

  /* clear poll retry count */
  ZDO_CTX().parent_threshold_retry = 0;
  TRACE_MSG(TRACE_NWK1, "mac_rx_on_when_idle %hd", (FMT__H, MAC_PIB().mac_rx_on_when_idle));
  /* Start polling function */
  TRACE_MSG(TRACE_COMMON1, "Join done, scheduling poll request with appropriate tmout", (FMT__0));

  zb_zdo_reschedule_poll_parent(ZG->zdo.conf_attr.nwk_indirect_poll_rate);
  ZB_P3_ON();
  TRACE_MSG(TRACE_NWK1, "<<join_done", (FMT__0));
}

#ifndef ZB_ED_ROLE
void zb_nlme_start_router_confirm(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">> start_router_confirm", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zdo_join_done, param);

  TRACE_MSG(TRACE_NWK1, "<< start_router_confirm", (FMT__0));
}
#endif

void zb_nlme_join_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_join_confirm_t *confirm = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_join_confirm_t);

  TRACE_MSG(TRACE_NWK1, ">>nlme_join_conf %hd", (FMT__H, param));

  if (confirm->status == 0)
  {
    TRACE_MSG(TRACE_COMMON1, "CONGRATULATIONS! joined status %hd, addr %d, xpanid " TRACE_FORMAT_64 ", ch %hd, addr 0x%x", (FMT__H_D_A_H_D,
                           confirm->status, confirm->network_address,
                           TRACE_ARG_64(confirm->extended_pan_id),
                           confirm->active_channel,
                           ZB_PIB_SHORT_ADDRESS()));

    ZG->zdo.handle.started = 0;

#ifdef ZB_ROUTER_ROLE
    if ( ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER )
    {
      zb_nlme_start_router_request_t *request;

      ZB_BUF_REUSE((zb_buf_t *)ZB_BUF_FROM_REF(param));

      request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_start_router_request_t);
      request->beacon_order = ZB_TURN_OFF_ORDER;
      request->superframe_order = ZB_TURN_OFF_ORDER;
      request->battery_life_extension = 0;
      ZB_SCHEDULE_CALLBACK(zb_nlme_start_router_request, param);
    }
    else
#endif
    {
      ZB_SCHEDULE_CALLBACK(zdo_join_done, param);
    }
  }
  else if (ZG->zdo.handle.rejoin && ZB_AIB().aps_insecure_join)
  {
    zb_nlme_network_discovery_request_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_network_discovery_request_t);
    TRACE_MSG(TRACE_ZDO1, "rejoin failed st %hd - try assoc", (FMT__H, (int)confirm->status));
    ZG->zdo.handle.rejoin = 0;
#ifdef ZB_ROUTER_ROLE
    if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_NONE)
    {
      ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ROUTER;
    }
    else
#endif
    {
      ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_ED;
    }
    req->scan_channels = ZB_AIB().aps_channel_mask;
    req->scan_duration = ZB_DEFAULT_SCAN_DURATION; /* TODO: configure it somehow? */
    ZDO_CTX().zdo_ctx.discovery_ctx.disc_count = ZDO_CTX().conf_attr.nwk_scan_attempts;
    ZB_SCHEDULE_CALLBACK(zb_nlme_network_discovery_request, param);
  }
  else
  {
    TRACE_MSG(TRACE_ZDO1, "assoc j failed st %hd", (FMT__H, (int)confirm->status));
    ZB_BUF_FROM_REF(param)->u.hdr.status = ZB_NWK_STATUS_NOT_PERMITTED;
    ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
  }

  TRACE_MSG(TRACE_NWK1, "<<nlme_join_conf", (FMT__0));
}


void zdo_send_device_annce(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_ZDO1, "device_annce", (FMT__0));

  {
    zb_zdo_device_annce_t *da;

    ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), sizeof(*da), da);
    ZDO_CTX().tsn++;
    da->tsn = ZDO_CTX().tsn;
    ZB_HTOLE16(&da->nwk_addr, &ZB_PIB_SHORT_ADDRESS());
    ZB_IEEE_ADDR_COPY(da->ieee_addr, ZB_PIB_EXTENDED_ADDRESS());
    da->capability = 0;
#ifdef ZB_ROUTER_ROLE
    if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)
    {
      ZB_MAC_CAP_SET_ROUTER_CAPS(da->capability);
      /* ZB_MAC_CAP_SET_SECURITY means high security mode - never set it */
    }
    else
#endif
    {
      if (MAC_PIB().mac_rx_on_when_idle)
      {
        ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(da->capability, 1);
      }
    }
  }

  {
    zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

    ZB_BZERO(dreq, sizeof(*dreq));
    /* Broadcast to all devices for which macRxOnWhenIdle = TRUE.
       MAC layer in ZE sends unicast to its parent.
    */
    dreq->dst_addr.addr_short = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
    dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    /* use default radius, max_depth * 2 */
    dreq->clusterid = ZDO_DEVICE_ANNCE_CLID;
    ZG->zdo.handle.dev_annce = param;
  }

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);

#if 0
  zb_zdo_device_annce(param);
#endif
}


#if 0
/**
  Device_annce is special primitive: no reply to it.

  It is part of Discovery primitives section but, indeed, it is very special.

  Directly fill APS packet here.

  See 2.4.3.1.11
 */
void zb_zdo_device_annce(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  {
    zb_uint8_t *tsn_p;
    ZB_BUF_ALLOC_LEFT(ZB_BUF_FROM_REF(param), 1, tsn_p);
    ZDO_CTX().tsn++;
    *tsn_p = ZDO_CTX().tsn;
  }
  {
    zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(ZB_BUF_FROM_REF(param), sizeof(zb_apsde_data_req_t));

    TRACE_MSG(TRACE_ZDO1, "device_annce", (FMT__0));

    ZB_BZERO(dreq, sizeof(*dreq));
    /* Broadcast to all devices for which macRxOnWhenIdle = TRUE.
       MAC layer in ZE sends unicast to its parent.
    */
    dreq->dst_addr.addr_short = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
    dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    /* use default radius, max_depth * 2 */
    dreq->clusterid = ZDO_DEVICE_ANNCE_CLID;
    ZG->zdo.handle.dev_annce = param;
  }

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, param);
}
#endif

void zb_apsde_data_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_APS3, "apsde_data_conf: param %hd status %hd dev_annce %hd key_sw %hd",
            (FMT__H_H_H_H, param, buf->u.hdr.status, ZG->zdo.handle.dev_annce, ZG->zdo.handle.key_sw));

  if (ZG->zdo.handle.dev_annce == param)
  {
    zb_uint8_t status = buf->u.hdr.status;
    ZB_BUF_REUSE(buf);
    ZG->zdo.handle.dev_annce = 0;
    /* Indicate startup complete */
    buf->u.hdr.status = status;
    zb_address_by_short(ZB_PIB_SHORT_ADDRESS(), ZB_TRUE, ZB_TRUE, &addr_ref);
    TRACE_MSG(TRACE_ZDO1, "was device_annce, start compl, st %hd", (FMT__H, buf->u.hdr.status));
    if ( !ZG->zdo.handle.started )
    {
      ZG->zdo.handle.started = 1;
      ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
    }
  }
#if defined ZB_SECURITY && defined ZB_COORDINATOR_ROLE
  else if (ZG->zdo.handle.key_sw == param)
  {
    TRACE_MSG(TRACE_SECUR3, "switch nwk key after this frame sent", (FMT__0));
    ZG->zdo.handle.key_sw = 0;
    secur_nwk_key_switch(ZG->nwk.nib.active_key_seq_number + 1);
  }
#endif
#ifndef ZB_LIMITED_FEATURES
  else if (!ZG->nwk.leave_context.leave_after_mgmt_leave_rsp_conf
           || !zdo_try_mgmt_leave_complete(param))
  {
    /* RET_OK and RET_NO_ACK statuses mean that confirm was
     * called from aps_ack_check_handle()  */
    if (buf->u.hdr.status == 0 || buf->u.hdr.status == (zb_uint8_t)RET_NO_ACK)
    {
      TRACE_MSG(TRACE_ZDO1, "buffer status %hd - call zb_apsde_data_acknowledged", (FMT__H, buf->u.hdr.status));
      ZB_SCHEDULE_CALLBACK(zb_apsde_data_acknowledged, param);
    }
    else
    {
      TRACE_MSG(TRACE_ZDO1, "buffer status %hd - free buf", (FMT__H, buf->u.hdr.status));
      zb_free_buf(buf);
    }
#ifdef APS_RETRANSMIT_TEST
    send_data();
#endif
  }
#endif
}

#ifdef APS_RETRANSMIT_TEST
static void send_data()
{
  zb_buf_t *buf = NULL;
  zb_apsde_data_req_t req;
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  buf = zb_get_out_buf();
  req.dst_addr.addr_short = 0; /* send to ZC */
  req.addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req.tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req.radius = 1;
  req.profileid = 2;
  req.src_endpoint = 10;
  req.dst_endpoint = 10;

  buf->u.hdr.handle = 0x11;
  ZB_BUF_INITIAL_ALLOC(buf, 80, ptr);

  for (i = 0 ; i < ZB_TEST_DATA_SIZE ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }
  ZB_MEMCPY(
    ZB_GET_BUF_TAIL(buf, sizeof(req)),
    &req, sizeof(req));

  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}
#endif

void zb_nlme_sync_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_bool_t sched_poll = (zb_bool_t)ZG->nwk.handle.joined;

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_sync_confirm %hd", (FMT__H, param));

  ZDO_CTX().inside_poll = 0;

  TRACE_MSG(TRACE_NWK1, "status %hd", (FMT__H, ZB_BUF_FROM_REF(param)->u.hdr.status));
#ifndef ZB_LIMITED_FEATURES
  if ( ZB_BUF_FROM_REF(param)->u.hdr.status == MAC_SUCCESS
       || ZB_BUF_FROM_REF(param)->u.hdr.status == MAC_NO_DATA )
  {
    /* nothing to do */
  }
  else
  {
    ZDO_CTX().parent_threshold_retry++;

    if ( ZDO_CTX().parent_threshold_retry >= ZDO_CTX().max_parent_threshold_retry )
    {
      sched_poll = ZB_FALSE;
      /* rejoin to current pan */
      ZB_EXTPANID_COPY(ZB_AIB().aps_use_extended_pan_id, ZB_NIB_EXT_PAN_ID());

      /* rejoin */
      zdo_initiate_rejoin(ZB_BUF_FROM_REF(param));

      /* prevent buffer from being free */
      param = 0;
    }
  }
#endif
  if (sched_poll)
  {
    TRACE_MSG(TRACE_NWK1, "schedule poll if needed rx_on_when_idle %hd", (FMT__H, MAC_PIB().mac_rx_on_when_idle));
    /* Start polling function if necessary */
    zb_zdo_reschedule_poll_parent(ZG->zdo.conf_attr.nwk_indirect_poll_rate);
  }
  if ( param )
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_sync_confirm", (FMT__0));
}


void zb_zdo_reschedule_poll_parent(zb_uint16_t timeout)
{
  /* reschedule alarm only if we are not waiting for the poll confirm and
   * really need polls */
  if (!(MAC_PIB().mac_rx_on_when_idle
        || ZDO_CTX().inside_poll))
  {
    ZB_SCHEDULE_ALARM_CANCEL(zb_zdo_poll_parent, 0);
    /* If FFD have some pending data for us, we schedule poll w/o timeout */
    ZB_SCHEDULE_ALARM(zb_zdo_poll_parent, 0,  ZB_MAC_GET_PENDING_DATA()? 1 : timeout);
  }
}


void zb_zdo_poll_parent(zb_uint8_t param) ZB_CALLBACK
{

  TRACE_MSG(TRACE_NWK1, ">>poll_prnt %hd", (FMT__H, param));
  if ( !param )
  {
    if (ZG->nwk.handle.joined)
    {
      ZB_GET_OUT_BUF_DELAYED(zb_zdo_poll_parent);
    }
  }
  else
  {
    zb_nlme_sync_request_t *request = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_sync_request_t);
    request->track = ZB_FALSE;

    ZDO_CTX().inside_poll = 1;

    ZB_SCHEDULE_CALLBACK(zb_nlme_sync_request, param);
  }
  TRACE_MSG(TRACE_NWK1, "<<poll_prnt", (FMT__0));
}

/* 3.2.2.30 NLME-NWK-STATUS.indication */
void zb_nlme_status_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_status_indication_t *status = ZB_GET_BUF_PARAM(buf, zb_nlme_status_indication_t);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_status_indication  %hd", (FMT__H, param));

  TRACE_MSG(TRACE_NWK1, "Got nwk status indication: status %hd address %d", (FMT__H_D, status->status, status->network_addr));

#ifdef ZB_ED_ROLE
  if ( status->status == ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE )
  {
    ZDO_CTX().parent_link_failure++;
  }

  TRACE_MSG(TRACE_NWK1, "parent link failure %hd", (FMT__H, ZDO_CTX().parent_link_failure));
#ifndef ZB_LIMITED_FEATURES
  if ( ZDO_CTX().parent_link_failure >= ZB_ZDO_PARENT_LINK_FAILURE_CNT
       /* ED must rejoin at first failed unsecure: that is key miss, probably */
#ifndef ZB_DISABLE_REJOIN_AFTER_SEC_FAIL
       || ZB_NWK_COMMAND_STATUS_IS_SECURE(status->status)
#endif
    )
  {
    ZB_MAC_CLEAR_CHANNEL_ERROR_TEST();

    ZDO_CTX().parent_link_failure = 0;

    /* rejoin to current pan */
    ZB_EXTPANID_COPY(ZB_AIB().aps_use_extended_pan_id, ZB_NIB_EXT_PAN_ID());
    zdo_initiate_rejoin(buf);
  }
  else
#endif
  {
    zb_free_buf(buf);
  }
#else  /* ZR/ZC*/
#ifdef ZB_SECURITY
#ifndef ZB_DISABLE_REJOIN_AFTER_SEC_FAIL
  if (ZB_NWK_COMMAND_STATUS_IS_SECURE(status->status))
  {
    zb_neighbor_tbl_ent_t *nbe;
    if (zb_nwk_neighbor_get_by_short(status->network_addr, &nbe) == RET_OK)
    {
      TRACE_MSG(TRACE_SECUR3, "nwk status %hd addr %d relationship %hd",
                (FMT__H_D_H, status->status, status->network_addr, nbe->relationship));
      if (nbe->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD)
      {
        TRACE_MSG(TRACE_SECUR3, "Child %d security error - force its leave", (FMT__D, status->network_addr));
        zb_zdo_force_child_leave(param, status->network_addr);
        param = 0;
      }
      else if (nbe->relationship == ZB_NWK_RELATIONSHIP_PARENT)
      {
        TRACE_MSG(TRACE_SECUR3, "Security error with my parent - rejoin", (FMT__0));
        /* rejoin to current pan */
        ZB_EXTPANID_COPY(ZB_AIB().aps_use_extended_pan_id, ZB_NIB_EXT_PAN_ID());
        ZB_NIB_DEVICE_TYPE() = ZB_NWK_DEVICE_TYPE_NONE;
        secur_clear_preconfigured_key();
        ZG->aps.authenticated = 0;
        zdo_initiate_rejoin(buf);
        param = 0;
      }
      /* Don't care about security errors not from my child or parent */
    }
  }
#endif
  if (param)
#endif  /* ZB_SECURITY */
  {
    zb_free_buf(buf);
  }
#endif  /* role */

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_status_indication", (FMT__0));
}


#ifdef ZB_ROUTER_ROLE


void zb_zdo_force_child_leave(zb_uint8_t param, zb_uint16_t child_addr) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_leave_request_t *lr = NULL;
  zb_address_ieee_ref_t addr_ref;
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_NWK1, ">>zb_zdo_force_child_leave", (FMT__0));
  lr = ZB_GET_BUF_PARAM(buf, zb_nlme_leave_request_t);

  ret = zb_address_by_short(child_addr, ZB_FALSE, ZB_FALSE, &addr_ref);
  if (ret == RET_OK)
  {
    zb_ieee_addr_t ieee_addr;

    zb_address_ieee_by_ref(ieee_addr, addr_ref);
    ZB_MEMCPY(lr->device_address, ieee_addr, sizeof(zb_ieee_addr_t));
    lr->remove_children = ZB_FALSE;
    lr->rejoin = ZB_TRUE;
    ZB_SCHEDULE_CALLBACK(zb_nlme_leave_request, param);
  }
  TRACE_MSG(TRACE_NWK1, "<<zb_zdo_force_child_leave status %d", (FMT__D, ret));
}

#endif  /* ZB_ROUTER_ROLE */

#ifndef ZB_LIMITED_FEATURES
void zb_nlme_reset_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_reset_confirm %p", (FMT__P, buf));

  if ( ZDO_CTX().reset_confirm_cb )
  {
    ZB_SCHEDULE_CALLBACK(ZDO_CTX().reset_confirm_cb, param);
    ZDO_CTX().reset_confirm_cb = NULL;
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "Reset confirm callback is not set", (FMT__0));
    zb_free_buf(buf);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_reset_confirm", (FMT__0));
}

void zb_zdo_reset(zb_uint8_t param, zb_uint8_t warm_start, zb_callback_t cb) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_nlme_reset_request_t *request = ZB_GET_BUF_PARAM(buf, zb_nlme_reset_request_t);

  ZDO_CTX().reset_confirm_cb = cb;

  /* schedule reset request */
  request->warm_start = warm_start;
  ZB_SCHEDULE_CALLBACK(zb_nlme_reset_request, param);
}


/**
   NLME-LEAVE.confirm primitive

   Called when device got LEAVE command from the net. It can be request for us
   to leave or intication that other device has left.
*/
void zb_nlme_leave_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_leave_indication_t *request = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_leave_indication_t);

  if (ZB_IEEE_ADDR_IS_ZERO(request->device_address))
  {
    /* it is for us */
    TRACE_MSG(TRACE_ZDO2, "do leave", (FMT__0));
    zb_nwk_do_leave(param, request->rejoin);
  }
  else
  {
    zb_neighbor_tbl_ent_t *nbt;

    if (zb_nwk_neighbor_get_by_ieee(request->device_address, &nbt) == RET_OK)
    {
#ifdef ZB_SECURITY
#ifdef ZB_ROUTER_ROLE
      if (!ZG->nwk.handle.is_tc
          && (nbt->relationship == ZB_NWK_RELATIONSHIP_CHILD
              || nbt->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD))
      {
        /* My child has left, I must inform TC */
        zb_apsme_update_device_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_update_device_req_t);
        TRACE_MSG(TRACE_SECUR3, "sending update-device.request (device left) to TC", (FMT__0));

        req->status = ZB_DEVICE_LEFT;
        ZB_IEEE_ADDR_COPY(req->dest_address, ZB_AIB().trust_center_address);
        zb_address_short_by_ref(&req->device_short_address, nbt->addr_ref);
        zb_address_ieee_by_ref(req->device_address, nbt->addr_ref);
        ZB_SCHEDULE_CALLBACK(zb_apsme_update_device_request, param);
        param = 0;
      }
#endif
#endif  /* security */
      /* forget this device */
      TRACE_MSG(TRACE_ZDO2, "forget device by addr ref %hd", (FMT__H, nbt->addr_ref));
      zb_nwk_forget_device(nbt->addr_ref);
    }
    if (param)
    {
      zb_free_buf(ZB_BUF_FROM_REF(param));
    }
  }
}


/**
   NLME-LEAVE.confirm primitive

   Called when LEAVE initiated by LEAVE.REQUEST sent LEAVE command to net.
 */
void zb_nlme_leave_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_leave_confirm_t *lc = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_leave_confirm_t);
  zb_uint8_t will_leave = (!lc->status && ZB_IEEE_ADDR_IS_ZERO(lc->device_address));
  zb_neighbor_tbl_ent_t *ent = NULL;

  TRACE_MSG(TRACE_ZDO2, "LEAVE.CONFIRM satus %hd will_leave %hd", (FMT__H_H, lc->status, will_leave));

#ifndef ZB_LIMITED_FEATURES
  if (!zdo_try_send_mgmt_leave_rsp(param, lc->status, will_leave))
  {
    /* not need to send resp. Maybe, leave now. */
    if (will_leave)
    {
      zb_nwk_do_leave(param, ZG->nwk.leave_context.rejoin_after_leave);
    }
    else
    {
  /* From 3.6.1.10.2  Method for a Device to Remove Its Child from the Network
     When device leaves newtwork we must clear the neighbor table. 			  */
     TRACE_MSG(TRACE_ZDO2, "removing device from nbt", (FMT__0));
     zb_nwk_neighbor_get_by_ieee(lc->device_address, &ent);
     zb_nwk_neighbor_delete(ent->addr_ref);
     zb_free_buf(ZB_BUF_FROM_REF(param));
    }
  }
  else
  {
    /* leave after mgmt resp will be sent */
    ZG->nwk.leave_context.leave_after_mgmt_leave_rsp_conf = will_leave;
  }
#endif
}

void zb_mlme_set_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

#ifdef ZB_TRACE_LEVEL
  {
  zb_mlme_set_confirm_t *conf = ( zb_mlme_set_confirm_t *)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_APS2, "<<zb_mlme_set_confirm status %hd", (FMT__H, conf->status));
  }
#endif
  zb_free_buf(buf);
}


#endif  /* ZB_LIMITED_FEATURES */

/*! @} */
