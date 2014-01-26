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
PURPOSE: AF layer
*/

#include "zb_common.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_af_globals.h"
#include "zb_zdo.h"

#include "zb_bank_9.h"
/*! \addtogroup ZB_AF */
/*! @{ */

#ifndef ZB_LIMITED_FEATURES
void zb_set_node_descriptor(zb_logical_type_t device_type, zb_int8_t power_src, zb_int8_t rx_on_when_idle, zb_int8_t alloc_addr) ZB_SDCC_REENTRANT
{
  zb_mac_capability_info_t mac_cap = 0;

  /* we should not set this bit, because "The alternate PAN */
  /* coordinator bit is set to 0 in devices implementing this specification" */
  /* ZB_MAC_CAP_SET_ALTERNATE_PAN_COORDINATOR(mac_cap, (device_type == * ZB_COORDINATOR || device_type == ZB_R OUTER));*/
  ZB_MAC_CAP_SET_DEVICE_TYPE(mac_cap, (device_type == ZB_COORDINATOR || device_type == ZB_ROUTER));
  ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(mac_cap, rx_on_when_idle);

  ZB_MAC_CAP_SET_POWER_SOURCE(mac_cap, power_src);
  ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(mac_cap, alloc_addr);

  ZB_SET_NODE_DESCRIPTOR(
    device_type,
    ZB_FREQ_BAND_2400, /* specific for current UBEC adapter */
    mac_cap,
    ZB_MANUFACTORER_CODE,
    ZB_NSDU_MAX_LEN,
    /* ZB_ASDU_MAX_LEN */ 0, /* 0 - is described in certification tests */
    device_type == ZB_COORDINATOR ? ZB_PRIMARY_TRUST_CENTER | ZB_NETWORK_MANAGER : 0,
    /* ZB_ASDU_MAX_LEN */ 0,  /* 0 - is described in certification tests */
    0);

  TRACE_MSG(TRACE_ZDO3, "node desc, server_mask %x mac cap 0x%x",
    (FMT__D_H, ZB_ZDO_NODE_DESC()->server_mask, ZB_ZDO_NODE_DESC()->mac_capability_flags));

}

/*
  Set node descriptor for FFD
  param device_type - FFD device type ZB_COORDINATOR or ZB_ROUTER
*/
void zb_set_ffd_node_descriptor(zb_logical_type_t device_type) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO3, "zb_set_ffd_node_descriptor", (FMT__0));
  zb_set_node_descriptor(device_type, 0, 1, 0);
}

/*
  Set node descriptor for end device
  param power_src - 1 if the current power source is mains power, 0 otherwise
  param rx_on_when_idle - receiver on when idle sub-field
  param alloc_addr - allocate address sub-field
*/
void zb_set_ed_node_descriptor(zb_int8_t power_src, zb_int8_t rx_on_when_idle, zb_int8_t alloc_addr) ZB_SDCC_REENTRANT
{
    TRACE_MSG(TRACE_ZDO3, "zb_set_ed_node_descriptor", (FMT__0));
  zb_set_node_descriptor(ZB_END_DEVICE, power_src, rx_on_when_idle, alloc_addr);
}

/*
  Set node power descriptor
  param current_power_mode - current power mode
  param available_power_sources - available power sources
  param current_power_source - current power source
  param current_power_source_level - current power source level
*/
void zb_set_node_power_descriptor(zb_current_power_mode_t current_power_mode, zb_uint8_t available_power_sources,
                                  zb_uint8_t current_power_source, zb_power_source_level_t current_power_source_level) ZB_SDCC_REENTRANT
{
  ZB_SET_POWER_DESC_CUR_POWER_MODE(ZB_ZDO_NODE_POWER_DESC(), current_power_mode);
  ZB_SET_POWER_DESC_AVAIL_POWER_SOURCES(ZB_ZDO_NODE_POWER_DESC(), available_power_sources);
  ZB_SET_POWER_DESC_CUR_POWER_SOURCE(ZB_ZDO_NODE_POWER_DESC(), current_power_source);
  ZB_SET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(ZB_ZDO_NODE_POWER_DESC(), current_power_source_level);
}

/*
  Set simple descriptor parameters
  param simple_desc - pointer to simple descriptor
  param endpoint - Endpoint
  param app_profile_id - Application profile identifier
  param app_device_id - Application device identifier
  param app_device_version - Application device version
  param app_input_cluster_count - Application input cluster count
  param app_output_cluster_count - Application output cluster count
*/
void zb_set_simple_descriptor(zb_af_simple_desc_1_1_t *simple_desc,
                              zb_uint8_t  endpoint, zb_uint16_t app_profile_id,
                              zb_uint16_t app_device_id, zb_bitfield_t app_device_version,
                              zb_uint8_t app_input_cluster_count, zb_uint8_t app_output_cluster_count) ZB_SDCC_REENTRANT
{
  simple_desc->endpoint = endpoint;
  simple_desc->app_profile_id = app_profile_id;
  simple_desc->app_device_id = app_device_id;
  simple_desc->app_device_version = app_device_version;
  simple_desc->app_input_cluster_count = app_input_cluster_count;
  simple_desc->app_output_cluster_count = app_output_cluster_count;
  simple_desc->reserved = 0;
}

/*
  Set input cluster item
  param simple_desc - pointer to simple descriptor
  param cluster_number - cluster item number
  param cluster_id - cluster id
*/
void zb_set_input_cluster_id(zb_af_simple_desc_1_1_t *simple_desc, zb_uint8_t cluster_number, zb_uint16_t cluster_id) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO2, "cluster_id 0x%hx, cluster_number %hu, input count %hd",
            (FMT__H_H_H, cluster_id, cluster_number, simple_desc->app_input_cluster_count));
  ZB_ASSERT(cluster_number < simple_desc->app_input_cluster_count);
  simple_desc->app_cluster_list[cluster_number] = cluster_id;
}

/*
  Set output cluster item
  param simple_desc - pointer to simple descriptor
  param cluster_number - cluster item number
  param cluster_id - cluster id
*/
void zb_set_output_cluster_id(zb_af_simple_desc_1_1_t *simple_desc, zb_uint8_t cluster_number, zb_uint16_t cluster_id) ZB_SDCC_REENTRANT
{
TRACE_MSG(TRACE_ZDO2, "cluster_id 0x%hx, cluster_number %hd, output count %hd",
          (FMT__H_H_H, cluster_id, cluster_number, simple_desc->app_output_cluster_count));
 ZB_ASSERT(cluster_number < simple_desc->app_output_cluster_count);
  simple_desc->app_cluster_list[simple_desc->app_input_cluster_count + cluster_number] = cluster_id;
}

void zb_set_zdo_descriptor() ZB_SDCC_REENTRANT
{
  zb_set_simple_descriptor((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(),
                           0 /* endpoint */,                0 /* app_profile_id */,
                           0 /* app_device_id */,           0 /* app_device_version*/,
                           7 /* app_input_cluster_count */, 8 /* app_output_cluster_count */);
  ZB_ZDO_SIMPLE_DESC_NUMBER() = 0;

  {
    static zb_uint16_t ZB_CODE s_iclids[] = {
      ZDO_NWK_ADDR_RESP_CLID,
      ZDO_IEEE_ADDR_RESP_CLID,
      ZDO_NODE_DESC_RESP_CLID,
      ZDO_POWER_DESC_RESP_CLID,
      ZDO_SIMPLE_DESC_RESP_CLID,
      ZDO_ACTIVE_EP_RESP_CLID,
      ZDO_MATCH_DESC_RESP_CLID
    };

    static zb_uint8_t ZB_CODE s_oclids[] = {
      ZDO_NWK_ADDR_REQ_CLID,
      ZDO_IEEE_ADDR_REQ_CLID,
      ZDO_NODE_DESC_REQ_CLID,
      ZDO_POWER_DESC_REQ_CLID,
      ZDO_SIMPLE_DESC_REQ_CLID,
      ZDO_ACTIVE_EP_REQ_CLID,
      ZDO_MATCH_DESC_REQ_CLID,
      ZDO_DEVICE_ANNCE_CLID
    };

    zb_uint8_t i;
    for (i = 0 ; i < sizeof(s_iclids)/sizeof(s_iclids[0]) ; ++i)
    {
      zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), i, s_iclids[i]);
    }
    for (i = 0 ; i < sizeof(s_oclids)/sizeof(s_oclids[0]) ; ++i)
    {
      zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), i, s_oclids[i]);
    }
  }

#if 0
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 0,  ZDO_NWK_ADDR_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 1,  ZDO_IEEE_ADDR_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 2,  ZDO_NODE_DESC_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 3,  ZDO_POWER_DESC_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 4,  ZDO_SIMPLE_DESC_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 5,  ZDO_ACTIVE_EP_RESP_CLID);
  zb_set_input_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 6,  ZDO_MATCH_DESC_RESP_CLID);

  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 0,  ZDO_NWK_ADDR_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 1,  ZDO_IEEE_ADDR_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 2,  ZDO_NODE_DESC_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 3,  ZDO_POWER_DESC_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 4,  ZDO_SIMPLE_DESC_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 5,  ZDO_ACTIVE_EP_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 6,  ZDO_MATCH_DESC_REQ_CLID);
  zb_set_output_cluster_id((zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC(), 7,  ZDO_DEVICE_ANNCE_CLID);  
#endif
}

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
void zb_set_default_ffd_descriptor_values(zb_logical_type_t device_type) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO3, "zb_set_default_ffd_descriptor_values", (FMT__0));
  zb_set_node_descriptor(device_type, 0, 1, 0);

  zb_set_node_power_descriptor(ZB_POWER_MODE_SYNC_ON_WHEN_IDLE,
                               ZB_POWER_SRC_CONSTATNT | ZB_POWER_SRC_RECHARGEABLE_BATTERY | ZB_POWER_SRC_DISPOSABLE_BATTERY,
                               ZB_POWER_SRC_CONSTATNT | ZB_POWER_SRC_RECHARGEABLE_BATTERY, ZB_POWER_LEVEL_100);
  zb_set_zdo_descriptor();
}
#endif


void zb_set_default_ed_descriptor_values() ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_ZDO3, "zb_set_default_ed_descriptor_values", (FMT__0));
  zb_set_node_descriptor(ZB_END_DEVICE, 0, 1, 0);
  zb_set_node_power_descriptor(ZB_POWER_MODE_SYNC_ON_WHEN_IDLE, ZB_POWER_SRC_CONSTATNT,
                               ZB_POWER_SRC_CONSTATNT, ZB_POWER_LEVEL_100);
  zb_set_zdo_descriptor();
}

/*
  Adds simple descriptor.
  param simple_desc - pointer to simple descriptor to add
  return RET_OK, RET_OVERFLOW
 */
zb_ret_t zb_add_simple_descriptor(zb_af_simple_desc_1_1_t *simple_desc) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;

  if (ZB_ZDO_SIMPLE_DESC_NUMBER() < ZB_MAX_EP_NUMBER)
  {
    ZB_ZDO_SIMPLE_DESC_LIST()[ZB_ZDO_SIMPLE_DESC_NUMBER()++] = simple_desc;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Error, simple_desc_number overflows %i", (FMT__D, (int)ZB_ZDO_SIMPLE_DESC_NUMBER()));
    ret = RET_OVERFLOW;
  }
  return ret;
}

#endif  /* ZB_LIMITED_FEATURES */

/*! @} */
