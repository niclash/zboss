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
PURPOSE: NWK MIB database
*/

#ifndef ZB_NWK_MIB_H
#define ZB_NWK_MIB_H 1

/*! \addtogroup nwk_ib */
/*! @{ */

/**
   \par NWK NIB

   Some NIB fields are indeed PIB fields. Use macros to access it.
 */

/**
   NWK NIB Attributes
 */
typedef enum zb_nib_attribute_e
{
  ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER                   = 0X81,
  ZB_NIB_ATTRIBUTE_PASSIVE_ASK_TIMEOUT               = 0X82,
  ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES             = 0X83,
  ZB_NIB_ATTRIBUTE_MAX_CHILDREN                      = 0X84,
  ZB_NIB_ATTRIBUTE_MAX_DEPTH                         = 0X85,
  ZB_NIB_ATTRIBUTE_MAX_ROUTERS                       = 0X86,
  ZB_NIB_ATTRIBUTE_NEIGHBOR_TABLE                    = 0X87,
  ZB_NIB_ATTRIBUTE_BROADCAST_DELIVERY_TIME           = 0X88,
  ZB_NIB_ATTRIBUTE_REPORT_CONSTANT_COST              = 0X89,
  ZB_NIB_ATTRIBUTE_ROUTE_DISCOVERY_RETRIES_PERMITTED = 0X8A,
  ZB_NIB_ATTRIBUTE_ROUTE_TABLE                       = 0X8B,
  ZB_NIB_ATTRIBUTE_SYM_LINK                          = 0X8E,
  ZB_NIB_ATTRIBUTE_CAPABILITY_INFORMATION            = 0X8F,
  ZB_NIB_ATTRIBUTE_ADDR_ALLOC                        = 0X90,
  ZB_NIB_ATTRIBUTE_USE_TREE_ROUTING                  = 0X91,
  ZB_NIB_ATTRIBUTE_MANAGER_ADDR                      = 0X92,
  ZB_NIB_ATTRIBUTE_MAX_SOURCE_ROUTE                  = 0X93,
  ZB_NIB_ATTRIBUTE_UPDATE_ID                         = 0X94,
  ZB_NIB_ATTRIBUTE_TRANSACTION_PERSISTENCE_TIME      = 0X95,
  ZB_NIB_ATTRIBUTE_NETWORK_ADDRESS                   = 0X96,
  ZB_NIB_ATTRIBUTE_STACK_PROFILE                     = 0X97,
  ZB_NIB_ATTRIBUTE_BROADCAST_TRANSACTION_TABLE       = 0X98,
  ZB_NIB_ATTRIBUTE_GROUP_ID_TABLE                    = 0X99,
  ZB_NIB_ATTRIBUTE_EXTENDED_PANID                    = 0X9A,
  ZB_NIB_ATTRIBUTE_USE_MULTICAST                     = 0X9B,
  ZB_NIB_ATTRIBUTE_ROUTE_RECORD_TABLE                = 0X9C,
  ZB_NIB_ATTRIBUTE_IS_CONCENTRATOR                   = 0X9D,
  ZB_NIB_ATTRIBUTE_CONCENTRATOR_RADIUS               = 0X9E,
  ZB_NIB_ATTRIBUTE_CONCENTRATOR_DESCOVERY_TIME       = 0X9F,
  ZB_NIB_ATTRIBUTE_SECURITY_LEVEL                    = 0XA0,
  ZB_NIB_ATTRIBUTE_SECURITY_MATERIAL_SET             = 0XA1,
  ZB_NIB_ATTRIBUTE_ACTIVE_KEY_SEQ_NUMBER             = 0XA2,
  ZB_NIB_ATTRIBUTE_ALL_FRESH                         = 0XA3,
  ZB_NIB_ATTRIBUTE_SECURE_ALL_FRAMES                 = 0XA5,
  ZB_NIB_ATTRIBUTE_LINK_STATUS_PERIOD                = 0XA6,
  ZB_NIB_ATTRIBUTE_ROUTER_AGE_LIMIT                  = 0XA7,
  ZB_NIB_ATTRIBUTE_UNIQUE_ADDR                       = 0XA8,
  ZB_NIB_ATTRIBUTE_ADDRESS_MAP                       = 0XA9,
  ZB_NIB_ATTRIBUTE_TIME_STAMP                        = 0X8C,
  ZB_NIB_ATTRIBUTE_PAN_ID                            = 0X80,
  ZB_NIB_ATTRIBUTE_TX_TOTAL                          = 0X8D
}
zb_nib_attribute_t;

#define ZB_NIB_SEQUENCE_NUMBER() ZG->nwk.nib.sequence_number
#define ZB_NIB_SEQUENCE_NUMBER_INC() (ZG->nwk.nib.sequence_number++)

#define ZB_NIB_MAX_DEPTH() ZG->nwk.nib.max_depth
#define ZB_NIB_DEPTH() ZG->nwk.nib.depth
#define ZB_NIB_DEVICE_TYPE() ZG->nwk.nib.device_type

/* nwk network address should be equal to mac short address, save it in one place */
#define ZB_NIB_NETWORK_ADDRESS() ZG->mac.pib.mac_short_address
/* nwk pan id should be equal to mac pan id, save it in one place */
#define ZB_NIB_PAN_ID() ZG->mac.pib.mac_pan_id
#define ZB_NIB_EXT_PAN_ID() ZG->nwk.nib.extended_pan_id
#define ZB_NIB_UPDATE_ID() ZG->nwk.nib.update_id

#ifdef ZB_SECURITY
#define ZB_NIB_SECURITY_LEVEL() ZG->nwk.nib.security_level
#else
#define ZB_NIB_SECURITY_LEVEL() 0
#endif

/* use tree routing? */
#define ZB_NIB_GET_USE_TREE_ROUTING() ZG->nwk.nib.use_tree_routing
#define ZB_NIB_SET_USE_TREE_ROUTING(v) (ZG->nwk.nib.use_tree_routing = (v))

#define ZB_NIB_SECURITY_MATERIAL() ZG->nwk.nib.secur_material_set

#define ZB_NIB_NWK_MANAGER_ADDR() ZG->nwk.nib.nwk_manager_addr /* TODO: init it correctly */

#define ZB_NIB_NWK_TX_TOTAL() ZG->nwk.nib.nwk_tx_total
#define ZB_NIB_NWK_TX_FAIL()  ZG->nwk.nib.nwk_tx_fail

/*! @} */

#endif /* ZB_NWK_MIB_H */
