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
PURPOSE: Zigbee cluster library
*/

#ifndef ZB_ZCL_H
#define ZB_ZCL_H 1

/*! \addtogroup ZB_ZCL */
/*! @{ */

/**
   Used to define the end of the list
 */
#define ZB_ZCL_END_MARKER 0xffff

/**
   ZCL common command IDs.
 */
typedef enum zb_zcl_cmd_e
{
  ZB_ZCL_CMD_READ_ATTRIB             = 0x00, /*!< Read attributes command */
  ZB_ZCL_CMD_READ_ATTRIB_RESP        = 0x01, /*!< Read attributes response command */
  ZB_ZCL_CMD_WRITE_ATTRIB            = 0x02, /*!< Write attributes foundation command */
  ZB_ZCL_CMD_WRITE_ATTRIB_RESP       = 0x03, /*!< Write attributes response command */
  ZB_ZCL_CMD_WRITE_ATTRIB_UNDIV      = 0x04, /*!< Write attributes undivided command */
  ZB_ZCL_CMD_WRITE_ATTRIB_NO_RESP    = 0x05, /*!< Write attributes no response command */
  ZB_ZCL_CMD_CONFIG_REPORT           = 0x06, /*!< Configure reporting command */
  ZB_ZCL_CMD_CONFIG_REPORT_RESP      = 0x07, /*!< Configure reporting response command */
  ZB_ZCL_CMD_READ_REPORT_CFG         = 0x08, /*!< Read reporting config command */
  ZB_ZCL_CMD_READ_REPORT_CFG_RESP    = 0x09, /*!< Read reporting config response command */
  ZB_ZCL_CMD_REPORT_ATTRIB           = 0x0a, /*!< Report attribute command */
  ZB_ZCL_CMD_DEFAULT_RESP            = 0x0b, /*!< Default response command */
  ZB_ZCL_CMD_DISC_ATTRIB             = 0x0c, /*!< Discover attributes command */
  ZB_ZCL_CMD_DISC_ATTRIB_RESP        = 0x0d  /*!< Discover attributes response command */
} zb_zcl_cmd_t;

/**
  ZCL attribute data type values
*/
typedef enum zb_zcl_attr_type_e
{
  ZB_ZCL_ATTR_TYPE_NULL           = 0x00, /*!< Null data type */
  ZB_ZCL_ATTR_TYPE_8BIT           = 0x08, /*!< 8-bit value data type */
  ZB_ZCL_ATTR_TYPE_16BIT          = 0x09, /*!< 16-bit value data type */
  ZB_ZCL_ATTR_TYPE_32BIT          = 0x0b, /*!< 32-bit value data type */
  ZB_ZCL_ATTR_TYPE_BOOL           = 0x10, /*!< Boolean data type */
  ZB_ZCL_ATTR_TYPE_8BITMAP        = 0x18, /*!< 8-bit bitmap data type */
  ZB_ZCL_ATTR_TYPE_16BITMAP       = 0x19, /*!< 16-bit bitmap data type */
  ZB_ZCL_ATTR_TYPE_32BITMAP       = 0x1b, /*!< 32-bit bitmap data type */
  ZB_ZCL_ATTR_TYPE_U8             = 0x20, /*!< Unsigned 8-bit value data type */
  ZB_ZCL_ATTR_TYPE_U16            = 0x21, /*!< Unsigned 16-bit value data type */
  ZB_ZCL_ATTR_TYPE_U32            = 0x23, /*!< Unsigned 32-bit value data type */
  ZB_ZCL_ATTR_TYPE_BYTE_ARRAY     = 0x41, /*!< Byte array data type */
  ZB_ZCL_ATTR_TYPE_CHAR_STRING    = 0x42, /*!< Charactery string (array) data type */
  ZB_ZCL_ATTR_TYPE_IEEE_ADDR      = 0xf0, /*!< IEEE address (U64) type */
  ZB_ZCL_ATTR_TYPE_INVALID        = 0xff  /*!< Invalid data type */
} zb_zcl_attr_type_t;

/**
   ZCL attribute access values
*/
typedef enum zb_zcl_attr_access_e
{
  ZB_ZCL_ATTR_ACCESS_READ_ONLY    = 0x00, /*!< Attribute is read only */
  ZB_ZCL_ATTR_ACCESS_READ_WRITE   = 0x01  /*!< Attribute is read/write */
} zb_zcl_attr_access_t;

/**
   ZCL status values.
*/
typedef enum zb_zcl_status_e
{
  ZB_ZCL_STATUS_SUCCESS                  = 0x00, /*!< ZCL Success */
  ZB_ZCL_STATUS_FAIL                     = 0x01, /*!< ZCL Fail */
  ZB_ZCL_STATUS_MALFORMED_CMD            = 0x80, /*!< Malformed command */
  ZB_ZCL_STATUS_UNSUP_CLUST_CMD          = 0x81, /*!< Unsupported cluster
                                                   command */
  ZB_ZCL_STATUS_UNSUP_GEN_CMD            = 0x82, /*!< Unsupported general
                                                   command */
  ZB_ZCL_STATUS_UNSUP_MANUF_CLUST_CMD    = 0x83, /*!< Unsupported manuf-specific
                                                   clust command */
  ZB_ZCL_STATUS_UNSUP_MANUF_GEN_CMD      = 0x84, /*!< Unsupported manuf-specific
                                                   general command */
  ZB_ZCL_STATUS_INVALID_FIELD            = 0x85, /*!< Invalid field */
  ZB_ZCL_STATUS_UNSUP_ATTRIB             = 0x86, /*!< Unsupported attribute */
  ZB_ZCL_STATUS_INVALID_VALUE            = 0x87, /*!< Invalid value */
  ZB_ZCL_STATUS_READ_ONLY                = 0x88, /*!< Read only */
  ZB_ZCL_STATUS_INSUFF_SPACE             = 0x89, /*!< Insufficient space */
  ZB_ZCL_STATUS_DUPE_EXISTS              = 0x8a, /*!< Duplicate exists */
  ZB_ZCL_STATUS_NOT_FOUND                = 0x8b, /*!< Not found */
  ZB_ZCL_STATUS_UNREPORTABLE_ATTRIB      = 0x8c, /*!< Unreportable attribute */
  ZB_ZCL_STATUS_INVALID_TYPE             = 0x8d, /*!< Invalid type */
  ZB_ZCL_STATUS_HW_FAIL                  = 0xc0, /*!< Hardware failure */
  ZB_ZCL_STATUS_SW_FAIL                  = 0xc1, /*!< Software failure */
  ZB_ZCL_STATUS_CALIB_ERR                = 0xc2, /*!< Calibration error */
  ZB_ZCL_STATUS_DISC_COMPLETE            = 0x01, /*!< Discovery complete */
  ZB_ZCL_STATUS_DISC_INCOMPLETE          = 0x00  /*!< Discovery incomplete */
} zb_zcl_status_t;


/**
   ZCL frame type
*/
typedef enum zb_zcl_frame_type_e
{
  ZB_ZCL_FRAME_TYPE_COMMON            = 0x00, /*!< Command acts across the
                                               * entire profile  */
  ZB_ZCL_FRAME_TYPE_CLUSTER_SPECIFIED = 0x01  /*!< Command is specific to a
                                               * cluster */
} zb_zcl_frame_type_t;

/**
   ZCL frame direction
*/
typedef enum zb_zcl_frame_direction_e
{
  ZB_ZCL_FRAME_DIRECTION_TO_SRV = 0x00,
  ZB_ZCL_FRAME_DIRECTION_TO_CLI = 0x01
} zb_zcl_frame_direction_t;


/**
   ZCL attribute structure
*/
typedef struct zb_zcl_attr_s
{
  zb_uint16_t id; /*!< Attribute id */
  zb_zcl_attr_type_t type; /*!< Attribute type @see zb_zcl_attr_type_t */
  zb_zcl_attr_access_t access; /*!< Attribute access options @ see
                                * zb_zcl_attr_access_t */
  zb_voidp_t data_p; /*!< Pointer to data */
} zb_zcl_attr_t;

#define ZB_ZCL_SET_ATTRIBUTE(attr, id, type, access, _data_p) \
{                                                          \
  attr->id = id;                                           \
  attr->type = type;                                       \
  attr->access = access;                                   \
  attr->data_p = _data_p;                                       \
}

/**
   ZCL cluster structure
*/
typedef struct zb_zcl_cluster_s zb_zcl_cluster_t;

struct zb_zcl_cluster_s
{
  zb_uint8_t ep; /*!< Endpoint that cluster belongs to */
  zb_uint16_t cluster_id; /*!< Cluster ID */
  zb_zcl_attr_t *attr_list; /*!< Cluster attribute list */

  zb_void_t (*handle)(zb_uint16_t, zb_uint8_t, zb_uint16_t, zb_uint8_t, zb_zcl_cluster_t *) ZB_SDCC_REENTRANT; /*!< Function to handle frames addressed to that cluster */
  zb_void_t (*action)(zb_uint16_t, zb_uint8_t, zb_uint16_t, zb_uint8_t, zb_zcl_cluster_t *); /*!< Cluster action handler */
};

/**
   ZCL frame control field
*/
typedef struct zb_zcl_frame_ctrl_s
{
  zb_bitfield_t frame_type:2;       /*!< Frame type @see zb_zcl_frame_type_t */
  zb_bitfield_t manufacturer:1;     /*!< Manufacturer specific frame */
  zb_bitfield_t direction:1;        /*!< Direction */
  zb_bitfield_t disable_def_resp:1; /*!< Disable default response */
  zb_bitfield_t reserved:3;
} ZB_PACKED_STRUCT
zb_zcl_frame_ctrl_t;

/**
   ZCL frame header with manufacturer code
*/
typedef struct zb_zcl_frame_header_full_s
{
  zb_zcl_frame_ctrl_t frame_ctrl; /*!< Frame control filed @see zb_zcl_frame_ctrl_t */
  zb_uint16_t manufacturer_code; /*!< Manufacturer Code */
  zb_uint8_t seq_number; /*!< Transaction Sequence Number */
  zb_uint8_t command_id; /*!< Command Identifier Field */
} ZB_PACKED_STRUCT
zb_zcl_frame_full_t;

/**
   ZCL frame header without manufacturer code
*/
typedef struct zb_zcl_frame_header_short_s
{
  zb_zcl_frame_ctrl_t frame_ctrl; /*!< Frame control filed @see zb_zcl_frame_ctrl_t */
  zb_uint8_t seq_number; /*!< Transaction Sequence Number */
  zb_uint8_t command_id; /*!< Command Identifier Field */
} ZB_PACKED_STRUCT
zb_zcl_frame_short_t;

/**
   Get ZCL frame type
*/
#define ZB_ZCL_FCTL_GET_TYPE(p) ( (zb_zcl_frame_type_t)(((zb_zcl_frame_ctrl_t *)p)->frame_type) )

/**
   Get true if ZCL frame is manufacturer specific
*/
#define ZB_ZCL_FCTL_GET_MANUFACTURER(p) ( ((zb_zcl_frame_ctrl_t *)p)->manufacturer )

/**
   Get ZCL frame direction
*/
#define ZB_ZCL_FCTL_GET_DIRECTION(p) ( ((zb_zcl_frame_ctrl_t *)p)->direction )

/**
   Get ZCL frame response field
*/
#define ZB_ZCL_FCTL_GET_DISABLE_RESP(p) ( ((zb_zcl_frame_ctrl_t *)p)->disable_def_resp )

/**
   Caclulate ZCL frame header size
*/
#define ZB_ZCL_FRAME_HDR_GET_SIZE(p) ( ZB_ZCL_FCTL_GET_MANUFACTURER(p) ? \
                                       sizeof(zb_zcl_frame_header_full_t) : \
                                       sizeof(zb_zcl_frame_header_short_t) )

/**
   Get ZCL frame manufacturer code
*/
#define ZB_ZCL_FRAME_HDR_GET_MANUFACTURER_CODE(p) ( *(zb_uint16_t *)((zb_uint8_t *)p + sizeof(zb_zcl_frame_ctrl_t)) )

/**
   Get ZCL frame sequence number
*/
#define ZB_ZCL_FRAME_HDR_GET_SEQ_NUM(p) ( *((zb_uint8_t *)p             \
                                            + sizeof(zb_zcl_frame_ctrl_t) \
                                            + (ZB_ZCL_FCTL_GET_MANUFACTURER(p) ? sizeof(zb_uint16_t) : 0)) )

/**
   Get ZCL frame command identifier
*/
#define ZB_ZCL_FRAME_HDR_GET_COMMAND_ID(p) ( *((zb_uint8_t *)p          \
                                               + sizeof(zb_zcl_frame_ctrl_t) \
                                               + (ZB_ZCL_FCTL_GET_MANUFACTURER(p) ? sizeof(zb_uint16_t) : 0) \
                                               + sizeof(zb_uint8_t)) )
/**
   Return next sequence number for ZCL frame
*/
#define ZB_ZCL_GET_SEQ_NUM() (ZCL_CTX()->seq_number++)


/**
   Initialize Zigbee cluster library
 */
void zb_zcl_init();

/**
   Register new cluster

   @param ep - Endpoint that cluster belogs to
   @param cluster_id - Cluster ID
   @param attr_list - Pointer to cluster attribute list @see zb_zcl_attr_t
   @param handle - Cluster handle function. All frames for this cluster will go
   here to get processed
   @param action - Cluster action handle.

   @return Pointer to cluster structure on success, NULL otherwise
 */
zb_zcl_cluster_t *zb_zcl_register_cluster(zb_uint8_t ep,
                                          zb_uint16_t cluster_id,
                                          zb_zcl_attr_t *attr_list,
                                          void (*handle)(zb_uint16_t, zb_uint8_t, zb_uint16_t, zb_uint8_t, zb_zcl_cluster_t *),
                                          void (*action)(zb_uint16_t, zb_uint8_t, zb_uint16_t, zb_uint8_t, zb_zcl_cluster_t *));

/**
   Find cluster by cluster_id

   @param cluster_id - Cluster ID to be found

   @return Pointer to cluster structure if cluster found, NULL otherwise
 */
zb_zcl_cluster_t *zb_zcl_find_cluster(zb_uint16_t cluster_id);

/**
   Find attribute by attribute_id

   @param attr_list - List of cluster attributes
   @param attribute_id - Attribute ID to be found

   @return Pointer to attribute structure if attribute found, NULL otherwise
 */
zb_zcl_attr_t *zb_zcl_find_attribute(zb_zcl_attr_t *attr_list, zb_uint16_t attribute_id);

/**
   Calculate attribute size in butes

   @param attr - Attribute to calculate size

   @return Attribute size in bytes
 */
zb_uint8_t zb_zcl_get_attribute_size(zb_zcl_attr_t *attr);

/**
   Main function to handle ZCL commands

   @param src_addr - Addrees of the device that send this frame
   @param src_ep - Endpoint of the source device
   @param profile_id - Profile ID
   @param buf - Incoming buffer with ZCL frame
   @param cluster - Pointer to the cluster that buffer belogs to

   @return nothing
 */
void zb_zcl_handle(zb_uint16_t src_addr, zb_uint8_t src_ep, zb_uint16_t profile_id, zb_buf_t *buf, zb_zcl_cluster_t *cluster);


/**
   Generate defaul response command

   @param buf - Buffer to generate the response
   @param status - Response status
   @param seq_number - Sequence number of the request
   @param command_id - Command ID

   @return nothing
 */
void zb_zcl_gen_default_resp(zb_buf_t *buf, zb_zcl_status_t status, zb_uint8_t seq_number, zb_uint8_t command_id);

/**
   DeInitialize Zigbee cluster library
 */
void zb_zcl_deinit();

/*! @} */

#endif /* ZB_ZCL_H */
