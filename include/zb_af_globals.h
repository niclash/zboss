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
PURPOSE: AF globals definition
*/

#ifndef ZB_AF_GLOBALS_H
#define ZB_AF_GLOBALS_H 1

#include "zb_mac.h"
#include "zb_test_profile.h"

/*! \addtogroup ZB_AF */
/*! @{ */

/**
   ZigBee descriptors
 */

#define ZB_MANUFACTORER_CODE 0
#define ZB_NWKC_MAC_FRAME_OVERHEAD 0xB
#define ZB_NWKC_MIN_HEADER_OVERHEAD 0x8
/* max nsdulength = aMaxPHYFrameSize -(nwkcMACFrameOverhead + nwkcMinHeaderOverhead) (D.4 aMaxMACFrameSize) */
#define ZB_NSDU_MAX_LEN (MAX_PHY_FRM_SIZE - (ZB_NWKC_MAC_FRAME_OVERHEAD + ZB_NWKC_MIN_HEADER_OVERHEAD))

#define ZB_APSC_MIN_HEADER_OVERHEAD 0x0C
/* max asdu length 256*(NsduLength - apscMinHeaderOverhead); currently fragmentation
 * is not supported  */
#define ZB_ASDU_MAX_LEN (ZB_NSDU_MAX_LEN - ZB_APSC_MIN_HEADER_OVERHEAD)

#define ZB_APS_HEADER_MAX_LEN                                           \
  (2 +                          /* fc + aps counter */                  \
   /* Packet either has dest and src endpoints (1+1 byte) if not group  \
    * addressing or Group address elsewhere - so 2 anyway */            \
   2 +                                                                  \
   /* cluster id, profile id */                                         \
   4 +                                                                  \
                                                                        \
   /* TODO: handle fragmentation and Extended header. Now suppose no Extended header */ \
   0)

#define ZB_APS_PAYLOAD_MAX_LEN ZB_ASDU_MAX_LEN - ZB_APS_HEADER_MAX_LEN

/**
   Node descriptors
 */
typedef struct zb_af_node_desc_s
{
#if 0
  union
  {
    struct
    {
      zb_bitfield_t        logical_type:3;             /* Logical type */
      zb_bitfield_t        complex_desc_available:1;   /* Complex descriptor available */
      zb_bitfield_t        user_desc_available:1;      /* User descriptor available */
      zb_bitfield_t        reserved:3;                 /* Reserved */
      zb_bitfield_t        aps_flags:3;                /* APS flags */
      zb_bitfield_t        frequence_band:5;           /* Frequency band */
    }
    flags;
    zb_uint16_t bulk; /* is used to simplify letoh convertation */
  }
  u;
#endif
  zb_uint16_t          node_desc_flags;
  zb_mac_capability_info_t mac_capability_flags;   /* mac capability */
  zb_uint16_t          manufacturer_code;          /* Manufacturer code */
  zb_uint8_t           max_buf_size;               /* Maximum buffer size */
  zb_uint16_t          max_incoming_transfer_size; /* Maximum incoming transfer size */
  zb_uint16_t          server_mask;                /* Server mask */
  zb_uint16_t          max_outgoing_transfer_size; /* Maximum outgoing transfer size */
  zb_uint8_t           desc_capability_field;      /* Descriptor capability field */
}
ZB_PACKED_STRUCT
zb_af_node_desc_t;

#define ZB_NODE_DESC_LOGICAL_TYPE_MASK 0x7       /* 0000.0000 0000.0111 */
#define ZB_NODE_DESC_COMPLEX_DESC_AVAIL_MASK 0x8 /* 0000.0000 0000.1000 */
#define ZB_NODE_DESC_USER_DESC_AVAIL_MASK 0x10   /* 0000.0000 0001.0000 */
#define ZB_NODE_DESC_APS_FLAGS_MASK    0x700     /* 0000.0111 0000.0000 */
#define ZB_NODE_DESC_FREQ_BAND_MASK    0xF800    /* 1111.1000 0000.0000 */

#define ZB_SET_NODE_DESC_LOGICAL_TYPE(desc, value)              \
  ( (desc)->node_desc_flags &= (zb_uint16_t)~ZB_NODE_DESC_LOGICAL_TYPE_MASK, \
    (desc)->node_desc_flags |= (zb_uint16_t)(value) )

#define ZB_GET_NODE_DESC_LOGICAL_TYPE(desc) \
  ( (desc)->node_desc_flags & ZB_NODE_DESC_LOGICAL_TYPE_MASK )

#define ZB_SET_NODE_DESC_COMPLEX_DESC_AVAIL(desc, value)         \
  ( (desc)->node_desc_flags &= (zb_uint16_t)~ZB_NODE_DESC_COMPLEX_DESC_AVAIL_MASK, \
    (desc)->node_desc_flags |= (zb_uint16_t)((value) << 3) )

#define ZB_GET_NODE_DESC_COMPLEX_DESC_AVAIL(desc)                \
  ( ((desc)->node_desc_flags & ZB_NODE_DESC_COMPLEX_DESC_AVAIL_MASK) >> 3 )

#define ZB_SET_NODE_DESC_USER_DESC_AVAIL(desc, value)                 \
  ( (desc)->node_desc_flags &= (zb_uint16_t)~ZB_NODE_DESC_USER_DESC_AVAIL_MASK,    \
    (desc)->node_desc_flags |= (zb_uint16_t)((value) << 4) )

#define ZB_GET_NODE_DESC_USER_DESC_AVAIL(desc)                    \
  ( ((desc)->node_desc_flags & ZB_NODE_DESC_USER_DESC_AVAIL_MASK) >> 4 )

#define ZB_SET_NODE_DESC_APS_FLAGS(desc, value)                    \
  ( (desc)->node_desc_flags &= (zb_uint16_t)~ZB_NODE_DESC_APS_FLAGS_MASK,       \
    (desc)->node_desc_flags |= (zb_uint16_t)((value) << 8) )

#define ZB_GET_NODE_DESC_APS_FLAGS(desc)                    \
  ( ((desc)->node_desc_flags & ZB_NODE_DESC_APS_FLAGS_MASK) >> 8 )

#define ZB_SET_NODE_DESC_FREQ_BAND(desc, value)               \
  ( (desc)->node_desc_flags &= (zb_uint16_t)~ZB_NODE_DESC_FREQ_BAND_MASK,       \
    (desc)->node_desc_flags |= (zb_uint16_t)((value) << 11) )

#define ZB_GET_NODE_DESC_FREQ_BAND(desc)                      \
  ( ((desc)->node_desc_flags & ZB_NODE_DESC_FREQ_BAND_MASK) >> 11 )


/* Node descriptor types */

typedef enum zb_logical_type_e
{
  ZB_COORDINATOR = 0,
  ZB_ROUTER = 1,
  ZB_END_DEVICE = 2
}
zb_logical_type_t;

enum zb_freq_band_e
{
  ZB_FREQ_BAND_868 = 1,
  ZB_FREQ_BAND_902 = 1 << 2,
  ZB_FREQ_BAND_2400 = 1 << 3
};

enum zb_server_mask_bit_e
{
  ZB_PRIMARY_TRUST_CENTER = 1,
  ZB_BACKUP_TRUST_CENTER = 1 << 1,
  ZB_PRIMARY_BINDING_TABLE_CENTER = 1 << 2,
  ZB_BACKUP_BINDING_TABLE_CENTER = 1 << 3,
  ZB_PRIMARY_DISCOVERY_CACHE = 1 << 4,
  ZB_BACKUP_DISCOVERY_CACHE = 1 << 5,
  ZB_NETWORK_MANAGER = 1 << 6
};

enum zb_desc_capability_e
{
  ZB_EXT_ACTIVE_EP_LIST = 1,
  ZB_EXT_SIMPLE_DESC_LIST = 1 <<1
};

/* Power descriptor types */

typedef enum zb_current_power_mode_e
{
  ZB_POWER_MODE_SYNC_ON_WHEN_IDLE = 0,
  ZB_POWER_MODE_COME_ON_PERIODICALLY = 1,
  ZB_POWER_MODE_COME_ON_WHEN_STIMULATED = 2
} ZB_PACKED_STRUCT
zb_current_power_mode_t;

typedef enum zb_power_src_e
{
  ZB_POWER_SRC_CONSTATNT = 1,
  ZB_POWER_SRC_RECHARGEABLE_BATTERY = 1 << 1,
  ZB_POWER_SRC_DISPOSABLE_BATTERY = 1 << 2
} ZB_PACKED_STRUCT
zb_power_src_t;

typedef enum zb_power_source_level_e
{
  ZB_POWER_LEVEL_CRITICAL = 0, /* 0000 */
  ZB_POWER_LEVEL_33 = 4,       /* 0100 */
  ZB_POWER_LEVEL_66 = 8,       /* 1000 */
  ZB_POWER_LEVEL_100 = 12      /* 1100 */
} ZB_PACKED_STRUCT
zb_power_source_level_t;



/**
   Node power descriptors
 */
typedef struct zb_af_node_power_desc_s
{
#if 0
  union
  {
    struct zb_power_desc_flags_s
    {
      zb_bitfield_t  current_power_mode:4;         /* Current power mode */
      zb_bitfield_t  available_power_sources:4;    /* Available power sources */
      zb_bitfield_t  current_power_source:4;       /* Current power source */
      zb_bitfield_t  current_power_source_level:4; /* Current power source level */
    }
    flags;
    zb_uint16_t bulk;
  }
  u;
#endif
  zb_uint16_t power_desc_flags;
} ZB_PACKED_STRUCT zb_af_node_power_desc_t;

#define ZB_POWER_DESC_CUR_POWER_MODE_MASK     0x000F /* 0000.0000 0000.1111 */
#define ZB_POWER_DESC_AVAIL_POWER_SOURCES_MASK 0x00F0 /* 0000.0000 1111.0000 */
#define ZB_POWER_DESC_CUR_POWER_SOURCE_MASK   0x0F00 /* 0000.1111 0000.0000 */
#define ZB_POWER_DESC_CUR_POWER_SOURCE_LEVEL_MASK 0xF000 /* 1111.0000 0000.0000 */

#define ZB_SET_POWER_DESC_CUR_POWER_MODE(desc, value)               \
  ( (desc)->power_desc_flags &= (zb_uint16_t)~ZB_POWER_DESC_CUR_POWER_MODE_MASK, \
    (desc)->power_desc_flags |= (zb_uint16_t)(value) )

#define ZB_GET_POWER_DESC_CUR_POWER_MODE(desc)                      \
  ( (desc)->power_desc_flags & ZB_POWER_DESC_CUR_POWER_MODE_MASK )

#define ZB_SET_POWER_DESC_AVAIL_POWER_SOURCES(desc, value)               \
  ( (desc)->power_desc_flags &= (zb_uint16_t)~ZB_POWER_DESC_AVAIL_POWER_SOURCES_MASK, \
    (desc)->power_desc_flags |= (zb_uint16_t)((value) << 4) )

#define ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(desc)                      \
  ( ((desc)->power_desc_flags & ZB_POWER_DESC_AVAIL_POWER_SOURCES_MASK) >> 4 )

#define ZB_SET_POWER_DESC_CUR_POWER_SOURCE(desc, value)               \
  ( (desc)->power_desc_flags &= (zb_uint16_t)~ZB_POWER_DESC_CUR_POWER_SOURCE_MASK, \
    (desc)->power_desc_flags |= (zb_uint16_t)((value) << 8) )

#define ZB_GET_POWER_DESC_CUR_POWER_SOURCE(desc)                      \
  ( ((desc)->power_desc_flags & ZB_POWER_DESC_CUR_POWER_SOURCE_MASK) >> 8 )

#define ZB_SET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(desc, value)               \
  ( (desc)->power_desc_flags &= (zb_uint16_t)~ZB_POWER_DESC_CUR_POWER_SOURCE_LEVEL_MASK, \
    (desc)->power_desc_flags |= (zb_uint16_t)((value) << 12) )

#define ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(desc)                  \
  ( ((desc)->power_desc_flags & ZB_POWER_DESC_CUR_POWER_SOURCE_LEVEL_MASK) >> 12 )


/**
   Simple descriptor
 */
#define ZB_DECLARE_SIMPLE_DESC(in_clusters_count, out_clusters_count)   \
  typedef struct zb_af_simple_desc_ ## in_clusters_count ## _ ## out_clusters_count ## _s \
  {                                                                     \
    zb_uint8_t    endpoint;                 /* Endpoint */              \
    zb_uint16_t   app_profile_id;           /* Application profile identifier */ \
    zb_uint16_t   app_device_id;            /* Application device identifier */ \
    zb_uint8_t app_device_version:4;     /* Application device version */ \
    zb_uint8_t reserved:4;               /* Reserved */              \
    zb_uint8_t    app_input_cluster_count;  /* Application input cluster count */ \
    zb_uint8_t    app_output_cluster_count; /* Application output cluster count */ \
    zb_uint16_t   app_cluster_list[in_clusters_count + out_clusters_count]; /* Application input and output cluster list */ \
  } ZB_PACKED_STRUCT                                                    \
  zb_af_simple_desc_ ## in_clusters_count ## _ ## out_clusters_count ## _t

ZB_DECLARE_SIMPLE_DESC(1,1); /* General descriptor type */
ZB_DECLARE_SIMPLE_DESC(7,8); /* ZDO descriptor type */

#define ZB_MAX_EP_NUMBER 1 /* max supported EP number, increase if needed */

/* Macro to set node descriptor, 2.3.2.3 Node Descriptor  */
#define ZB_SET_NODE_DESCRIPTOR(logical_type_p, frequence_band_p, mac_capability_flags_p, manufacturer_code_p, \
                                max_buf_size_p, max_incoming_transfer_size_p, server_mask_p, \
                                max_outgoing_transfer_size_p, desc_capability_field_p) \
  ( ZB_SET_NODE_DESC_LOGICAL_TYPE(ZB_ZDO_NODE_DESC(), (logical_type_p)), \
    ZB_SET_NODE_DESC_COMPLEX_DESC_AVAIL(ZB_ZDO_NODE_DESC(), 0),  /* complex desc is not supported */ \
    ZB_SET_NODE_DESC_USER_DESC_AVAIL(ZB_ZDO_NODE_DESC(), 0),     /* usr desc is not supported */ \
    ZB_SET_NODE_DESC_APS_FLAGS(ZB_ZDO_NODE_DESC(), 0), /* not supported by spec */ \
    ZB_SET_NODE_DESC_FREQ_BAND(ZB_ZDO_NODE_DESC(), (frequence_band_p)), \
    ZB_ZDO_NODE_DESC()->mac_capability_flags = (mac_capability_flags_p),          \
    ZB_ZDO_NODE_DESC()->manufacturer_code = (zb_uint16_t)(manufacturer_code_p), \
    ZB_ZDO_NODE_DESC()->max_buf_size = (zb_uint8_t)(max_buf_size_p),              \
    ZB_ZDO_NODE_DESC()->max_incoming_transfer_size = (zb_uint16_t)(max_incoming_transfer_size_p), \
    ZB_ZDO_NODE_DESC()->server_mask = (zb_uint16_t)(server_mask_p),                \
    ZB_ZDO_NODE_DESC()->max_outgoing_transfer_size = (zb_uint16_t)(max_outgoing_transfer_size_p), \
    ZB_ZDO_NODE_DESC()->desc_capability_field = (zb_uint8_t)(desc_capability_field_p) )


/**
  Set node descriptor for FFD
  @param device_type - FFD device type ZB_COORDINATOR or ZB_ROUTER
*/
void zb_set_ffd_node_descriptor(zb_logical_type_t device_type) ZB_SDCC_REENTRANT;

/**
  Set node descriptor for end device
  @param power_src - 1 if the current power source is mains power, 0 otherwise
  @param rx_on_when_idle - receiver on when idle sub-field
  @param alloc_addr - allocate address sub-field
*/
void zb_set_ed_node_descriptor(zb_int8_t power_src, zb_int8_t rx_on_when_idle, zb_int8_t alloc_addr) ZB_SDCC_REENTRANT;

/**
  Set node power descriptor
  @param current_power_mode - current power mode
  @param available_power_sources - available power sources
  @param current_power_source - current power source
  @param current_power_source_level - current power source level
*/
void zb_set_node_power_descriptor(zb_current_power_mode_t current_power_mode, zb_uint8_t available_power_sources,
                                  zb_uint8_t current_power_source, zb_power_source_level_t current_power_source_level) ZB_SDCC_REENTRANT;

/**
  Set simple descriptor parameters
  @param simple_desc - pointer to simple descriptor
  @param endpoint - Endpoint
  @param app_profile_id - Application profile identifier
  @param app_device_id - Application device identifier
  @param app_device_version - Application device version
  @param app_input_cluster_count - Application input cluster count
  @param app_output_cluster_count - Application output cluster count
*/
void zb_set_simple_descriptor(zb_af_simple_desc_1_1_t *simple_desc,
                              zb_uint8_t  endpoint, zb_uint16_t app_profile_id,
                              zb_uint16_t app_device_id, zb_bitfield_t app_device_version,
                              zb_uint8_t app_input_cluster_count, zb_uint8_t app_output_cluster_count) ZB_SDCC_REENTRANT;

/**
  Set input cluster item
  @param simple_desc - pointer to simple descriptor
  @param cluster_number - cluster item number
  @param cluster_id - cluster id
*/
void zb_set_input_cluster_id(zb_af_simple_desc_1_1_t *simple_desc, zb_uint8_t cluster_number, zb_uint16_t cluster_id) ZB_SDCC_REENTRANT;

/**
  Set output cluster item
  @param simple_desc - pointer to simple descriptor
  @param cluster_number - cluster item number
  @param cluster_id - cluster id
*/
void zb_set_output_cluster_id(zb_af_simple_desc_1_1_t *simple_desc, zb_uint8_t cluster_number, zb_uint16_t cluster_id) ZB_SDCC_REENTRANT;

/**
  Set default descriptors values for FFD.
  @param device_type - device type ZB_COORDINATOR or ZB_ROUTER
 */
void zb_set_default_ffd_descriptor_values(zb_logical_type_t device_type) ZB_SDCC_REENTRANT;

/**
  Set default descriptors values for end device.
 */
void zb_set_default_ed_descriptor_values() ZB_SDCC_REENTRANT;

/**
  Adds simple descriptor.
  @param simple_desc - pointer to simple descriptor to add
  @return RET_OK, RET_OVERFLOW
 */
zb_ret_t zb_add_simple_descriptor(zb_af_simple_desc_1_1_t *simple_desc) ZB_SDCC_REENTRANT;


/*! @} */

#endif /* ZB_AF_GLOBALS_H */
