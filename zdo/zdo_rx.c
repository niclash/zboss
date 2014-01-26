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
PURPOSE: ZDO RX path
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"

#include "zb_bank_8.h"
/*! \addtogroup ZB_ZDO */
/*! @{ */

#ifdef ZB_ROUTER_ROLE
static void zdo_device_annce_srv(zb_uint8_t param, void *dt) ZB_SDCC_REENTRANT;
#endif

void zb_zdo_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = (zb_apsde_data_indication_t *)ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);
#ifdef ZB_ROUTER_ROLE
  zb_uint8_t *body;
#endif
  zb_uint8_t skip_free_buf = 1;
  zb_ret_t ret;
  zb_uint8_t fc;


  TRACE_MSG(TRACE_ZDO1, "zdo_data_ind %hd clu 0x%hx", (FMT__H_H, param, ind->clusterid));
  fc = *ZB_BUF_BEGIN(asdu); /* APS FC is needed in some response functions */
#ifdef ZB_ROUTER_ROLE
  ZB_APS_HDR_CUT_P(asdu, body);
#else
  ZB_APS_HDR_CUT(asdu);
#endif
/* seems like switch should be replaced. It's broken somehow */  
#if 0
  switch (ind->clusterid)
  {
#ifdef ZB_ROUTER_ROLE
    case ZDO_DEVICE_ANNCE_CLID:
      /* skip tsn from ZDP message */
      if (!ZG->nwk.handle.joined_pro){
		  zdo_device_annce_srv(param, (void *)(body + 1));
      	  skip_free_buf = 0;
	 }
      break;
#endif
#ifndef ZB_LIMITED_FEATURES
    case ZDO_NODE_DESC_REQ_CLID:
    case ZDO_POWER_DESC_REQ_CLID:
      zdo_send_desc_resp(param);
      break;
    case ZDO_SIMPLE_DESC_REQ_CLID:
      zdo_send_simple_desc_resp(param);
      break;
    case ZDO_NWK_ADDR_REQ_CLID:
      zdo_device_nwk_addr_res(param, fc);
      break;
    case ZDO_IEEE_ADDR_REQ_CLID:
      zdo_device_ieee_addr_res(param, fc);
      break;
    case ZDO_ACTIVE_EP_REQ_CLID:
      zdo_active_ep_res(param);
      break;
    case ZDO_MATCH_DESC_REQ_CLID:
      zdo_match_desc_res(param);
      break;
    case ZDO_MGMT_NWK_UPDATE_REQ_CLID:
      zb_zdo_mgmt_nwk_update_handler(param);      
      break;
    case ZDO_SYSTEM_SERVER_DISCOVERY_REQ_CLID:
      zdo_system_server_discovery_res(param);
      break;
    case ZDO_MGMT_LQI_REQ_CLID:
      zdo_lqi_resp(param);
      break;
    case ZDO_MGMT_LEAVE_REQ_CLID:
      zdo_mgmt_leave_srv(param);
      break;
    case ZDO_BIND_REQ_CLID:
      zb_zdo_bind_unbind_res(param, ZB_TRUE);
      break;
    case ZDO_UNBIND_REQ_CLID:
      zb_zdo_bind_unbind_res(param, ZB_FALSE);
      break;
    case ZDO_END_DEVICE_BIND_REQ_CLID:
      zb_zdo_end_device_bind_handler(param);
      break;

    case ZDO_MGMT_PERMIT_JOINING_CLID:
#ifdef ZB_ROUTER_ROLE
      if (!ZG->nwk.handle.joined_pro) zb_zdo_mgmt_permit_joining_handle(param);
	  else skip_free_buf = 0;
#endif
      break;

    case ZDO_NODE_DESC_RESP_CLID:
    case ZDO_POWER_DESC_RESP_CLID:
    case ZDO_SIMPLE_DESC_RESP_CLID:
    case ZDO_NWK_ADDR_RESP_CLID:
    case ZDO_IEEE_ADDR_RESP_CLID:
    case ZDO_ACTIVE_EP_RESP_CLID:
    case ZDO_MATCH_DESC_RESP_CLID:
    case ZDO_MGMT_LEAVE_RESP_CLID:
    case ZDO_MGMT_LQI_RESP_CLID:
    case ZDO_BIND_RESP_CLID:
    case ZDO_UNBIND_RESP_CLID:
    case ZDO_END_DEVICE_BIND_RESP_CLID:
      ret = zdo_af_resp(param);      
      /* do not free buffer if callback was found */
      skip_free_buf = (ret == RET_OK);
      break;

    case ZDO_SYSTEM_SERVER_DISCOVERY_RESP_CLID:
    {
      zb_uint8_t tsn;

      tsn = *(zb_uint8_t *)ZB_BUF_BEGIN(asdu);
      TRACE_MSG(TRACE_ZDO1, "SERVER_DISCOVERY_RESP_CLID tsn %hd, disc_tsn %hd ",
                (FMT__H_H, tsn, ZDO_CTX().system_server_discovery_tsn));

      ZB_BUF_CUT_LEFT(asdu, sizeof(tsn), asdu);
      if (tsn == ZDO_CTX().system_server_discovery_tsn)
      {
        zb_buf_t *buf = ZB_BUF_FROM_REF(param);
        zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
        zb_uint16_t mask_tmp;
        zb_zdo_system_server_discovery_resp_t *resp = (zb_zdo_system_server_discovery_resp_t*)(zdp_cmd);
        mask_tmp = resp->server_mask;
        ZB_HTOLE16(&(resp->server_mask), &mask_tmp);
        zb_schedule_callback(ZDO_CTX().system_server_discovery_cb, param);
      }
      else
      {
        skip_free_buf = 0;
      }
      break;
    }

    case ZDO_MGMT_NWK_UPDATE_NOTIFY_CLID:
    {
      /* nwk update notify can be received without request, so zdo_af_resp()
         can return error - no callback for response */
      ret = zdo_af_resp(param);
      if (ret != RET_OK)
      {
        TRACE_MSG(TRACE_ZDO3, "update_notify was received without request", (FMT__0));
        zdo_change_channel(param);
      }
      break;
    }
#endif  /* ZB_LIMITED_FEATURES */

    default:
      TRACE_MSG(TRACE_ZDO1, "unhandl clu %hd - drop", (FMT__H, ind->clusterid));
      skip_free_buf = 0;
      break;
  }
#endif


#ifdef ZB_ROUTER_ROLE
  if ((ind->clusterid == ZDO_DEVICE_ANNCE_CLID)&&(!ZG->nwk.handle.joined_pro))
  {
      zdo_device_annce_srv(param, (void *)(body + 1));     
      skip_free_buf = 0;
  } else    
#endif              
#ifndef ZB_LIMITED_FEATURES
  if ((ind->clusterid == ZDO_NODE_DESC_REQ_CLID)&&(ind->clusterid == ZDO_POWER_DESC_REQ_CLID))  
  {
      zdo_send_desc_resp(param);
  } else if (ind->clusterid == ZDO_SIMPLE_DESC_REQ_CLID)
  {
       zdo_send_simple_desc_resp(param);
  } else if (ind->clusterid == ZDO_NWK_ADDR_REQ_CLID)
  {  
       zdo_device_nwk_addr_res(param, fc);
  } else if (ind->clusterid == ZDO_IEEE_ADDR_REQ_CLID)
  {
        zdo_device_ieee_addr_res(param, fc);
  } else if (ind->clusterid == ZDO_ACTIVE_EP_REQ_CLID)
  {
    zdo_active_ep_res(param);
  } else if (ind->clusterid == ZDO_MATCH_DESC_REQ_CLID)
  {
    zdo_match_desc_res(param);
  } else if (ind->clusterid == ZDO_MGMT_NWK_UPDATE_REQ_CLID)
  {
    zb_zdo_mgmt_nwk_update_handler(param);  
  } else if (ind->clusterid == ZDO_SYSTEM_SERVER_DISCOVERY_REQ_CLID)
  {
    zdo_system_server_discovery_res(param);
  } else if (ind->clusterid == ZDO_MGMT_LQI_REQ_CLID)
  {
      zdo_lqi_resp(param);
  } else if (ind->clusterid == ZDO_MGMT_LEAVE_REQ_CLID)
  {
      zdo_mgmt_leave_srv(param);
  } else if (ind->clusterid == ZDO_BIND_REQ_CLID)
  { 
    zb_zdo_bind_unbind_res(param, ZB_TRUE);
  } else if (ind->clusterid == ZDO_UNBIND_REQ_CLID)
  {
      zb_zdo_bind_unbind_res(param, ZB_FALSE);
  } else if (ind->clusterid == ZDO_END_DEVICE_BIND_REQ_CLID) 
  {  
      zb_zdo_end_device_bind_handler(param);
  } else if (ind->clusterid == ZDO_MGMT_PERMIT_JOINING_CLID)
  {
#ifdef ZB_ROUTER_ROLE
      if (!ZG->nwk.handle.joined_pro) zb_zdo_mgmt_permit_joining_handle(param);
        else skip_free_buf = 0;
#endif    
  } else if ((ind->clusterid == ZDO_NODE_DESC_RESP_CLID)||(ind->clusterid == ZDO_POWER_DESC_RESP_CLID)||
     (ind->clusterid == ZDO_SIMPLE_DESC_RESP_CLID)||(ind->clusterid == ZDO_NWK_ADDR_RESP_CLID)||
       (ind->clusterid == ZDO_IEEE_ADDR_RESP_CLID)||(ind->clusterid == ZDO_ACTIVE_EP_RESP_CLID)||
         (ind->clusterid == ZDO_MATCH_DESC_RESP_CLID)||(ind->clusterid == ZDO_MGMT_LEAVE_RESP_CLID)||
           (ind->clusterid == ZDO_MGMT_LQI_RESP_CLID)||(ind->clusterid == ZDO_BIND_RESP_CLID)||
             (ind->clusterid == ZDO_UNBIND_RESP_CLID)||(ind->clusterid == ZDO_END_DEVICE_BIND_RESP_CLID))
  {
      ret = zdo_af_resp(param);      
      /* do not free buffer if callback was found */
      skip_free_buf = (ret == RET_OK);
  } else if (ind->clusterid == ZDO_SYSTEM_SERVER_DISCOVERY_RESP_CLID)
  {
      zb_uint8_t tsn;

      tsn = *(zb_uint8_t *)ZB_BUF_BEGIN(asdu);
      TRACE_MSG(TRACE_ZDO1, "SERVER_DISCOVERY_RESP_CLID tsn %hd, disc_tsn %hd ",
                (FMT__H_H, tsn, ZDO_CTX().system_server_discovery_tsn));

      ZB_BUF_CUT_LEFT(asdu, sizeof(tsn), asdu);
      if (tsn == ZDO_CTX().system_server_discovery_tsn)
      {
        zb_buf_t *buf = ZB_BUF_FROM_REF(param);
        zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
        zb_uint16_t mask_tmp;
        zb_zdo_system_server_discovery_resp_t *resp = (zb_zdo_system_server_discovery_resp_t*)(zdp_cmd);
        mask_tmp = resp->server_mask;
        ZB_HTOLE16(&(resp->server_mask), &mask_tmp);
        zb_schedule_callback(ZDO_CTX().system_server_discovery_cb, param);
      }
      else
      {
        skip_free_buf = 0;
      }
   } else if (ind->clusterid == ZDO_MGMT_NWK_UPDATE_NOTIFY_CLID)
   {
      /* nwk update notify can be received without request, so zdo_af_resp()
         can return error - no callback for response */
      ret = zdo_af_resp(param);
      if (ret != RET_OK)
      {
        TRACE_MSG(TRACE_ZDO3, "update_notify was received without request", (FMT__0));
        zdo_change_channel(param);
      }
    } else
#endif  /* ZB_LIMITED_FEATURES */
  {
      TRACE_MSG(TRACE_ZDO1, "unhandl clu %hd - drop", (FMT__H, ind->clusterid));
      skip_free_buf = 0;
  }
























  TRACE_MSG(TRACE_ZDO3, "skip_free_buf %hd", (FMT__H, skip_free_buf));
  if (!skip_free_buf)
  {
    zb_free_buf(asdu);
  }
}


#ifdef ZB_ROUTER_ROLE
static void zdo_device_annce_srv(zb_uint8_t param, void *dt) ZB_SDCC_REENTRANT
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_device_annce_t *da = dt;
  zb_neighbor_tbl_ent_t *ne;
  zb_uint16_t addr;

  TRACE_MSG(TRACE_ZDO1, ">> zdo_device_annce_srv %p", (FMT__P, dt));

  ZB_LETOH16(&addr, &da->nwk_addr);
  ne = zdo_device_info_upd(buf, da->ieee_addr, addr);
  if (ne)
  {
    if (ne->device_type == ZB_NWK_DEVICE_TYPE_NONE
        || ne->relationship == ZB_NWK_RELATIONSHIP_PREVIOUS_CHILD)
    {
      ne->device_type = ZB_MAC_CAP_GET_DEVICE_TYPE(da->capability) ? ZB_NWK_DEVICE_TYPE_ROUTER : ZB_NWK_DEVICE_TYPE_ED;
    }
    ne->rx_on_when_idle = ZB_MAC_CAP_GET_RX_ON_WHEN_IDLE(da->capability);
    /* It was our child, now its address has changed - it is previous child */
    if (ne->relationship == ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE
        && (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER
            || ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR)
        && ZB_MAC_CAP_GET_DEVICE_TYPE(da->capability))
    {
      /* relationshop was unknown, both our and remote device are Routers, we got this
       * packet - this is our sibling */
      ne->relationship = ZB_NWK_RELATIONSHIP_SIBLING;
    }
    if (ne->relationship == ZB_NWK_RELATIONSHIP_CHILD
        || ne->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD
        || ne->relationship == ZB_NWK_RELATIONSHIP_PREVIOUS_CHILD)
    {
      if (ZG->nwk.nib.depth < ZG->nwk.nib.max_depth)
      {
        ne->depth = ZG->nwk.nib.depth + 1;
      }
    }
    if (ne->device_type == ZB_NWK_DEVICE_TYPE_ED)
    {
      ne->permit_joining = 0;
    }
    TRACE_MSG(TRACE_ZDO3, "DEV_ANNCE: upd addr " TRACE_FORMAT_64 "/%d ne %p dev_t %hd, rx.o.i %hd rel %hd",
              (FMT__A_D_P_H_H_H, TRACE_ARG_64(da->ieee_addr), addr, ne, ne->device_type,
               ne->rx_on_when_idle, ne->relationship));
  }
}


zb_neighbor_tbl_ent_t *zdo_device_info_upd(zb_buf_t *buf, zb_ieee_addr_t ieee_addr, zb_uint16_t addr) ZB_SDCC_REENTRANT
{
  zb_neighbor_tbl_ent_t *ne;
  zb_address_ieee_ref_t addr_ref;
  /*
    First detect former child.
    For 2007 profile is is simple: short address is changed.
    No ideas for PRO: see 2.5.5.5.4.3:
    "ZDO shall arrange that any IEEE address to short
    address mappings which have become known to applications running on this
    device be updated. This behavior is mandatory, but the mechanism by which it is
    achieved is outside the scope of this specification."
   */
  if (zb_address_by_ieee(ieee_addr, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK)
  {
    zb_uint16_t old_short;
    zb_address_short_by_ref(&old_short, addr_ref);
    if (old_short != addr && old_short != (zb_uint16_t)~0
        && zb_nwk_neighbor_get(addr_ref, ZB_FALSE, &ne) == RET_OK
        && (ne->relationship == ZB_NWK_RELATIONSHIP_CHILD
            || ne->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD))
    {
      /* It was our child, now its address has changed - it is previous child */
      ne->relationship = ZB_NWK_RELATIONSHIP_PREVIOUS_CHILD;
    }
  }

  if (zb_address_update(ieee_addr, addr, ZB_TRUE, &addr_ref) == RET_OK)
  {
    zb_mac_mhr_t mac_hdr;

    /* check if it's a neighbor mac_addr == nwk_addr */
    if ( zb_parse_mhr(&mac_hdr, buf->buf + buf->u.hdr.mac_hdr_offset)
         && ZB_FCF_GET_SRC_ADDRESSING_MODE(mac_hdr.frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST
         && mac_hdr.src_addr.addr_short == addr
         && zb_nwk_neighbor_get(addr_ref, ZB_TRUE, &ne) == RET_OK)
    {
      TRACE_MSG(TRACE_ZDO1, "new neighbor added %d", (FMT__D, addr));
      return ne;
    }
    else
    {
      TRACE_MSG(TRACE_ZDO1, "seems device %d is not our neighbor", (FMT__D, addr));
    }
  }

  return NULL;
}

#endif

/*! @} */
