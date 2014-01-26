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
PURPOSE: APS layer API
*/

#ifndef ZB_APS_H
#define ZB_APS_H 1


/*! \addtogroup aps_api */
/*! @{ */

															
/**
   APS addressing mode constants
 */
enum zb_aps_addr_mode_e
{
  ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT = 0, /*!< 0x00 = DstAddress and
                                                   * DstEndpoint not present  */
  ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT = 1, /*!< 0x01 = 16-bit group
                                                   * address for DstAddress;
                                                   * DstEndpoint not present */
  ZB_APS_ADDR_MODE_16_ENDP_PRESENT = 2, /*!< 0x02 = 16-bit address for DstAddress and
                                          DstEndpoint present  */
  ZB_APS_ADDR_MODE_64_ENDP_PRESENT = 3 /*!< 0x03 = 64-bit extended address for
                                         DstAddress and DstEndpoint present  */
};

#define ZB_MIN_ENDPOINT_NUMBER 1
#define ZB_MAX_ENDPOINT_NUMBER 240


typedef enum zb_aps_status_e
{
  ZB_APS_STATUS_SUCCESS               = 0x00, /*!< A request has been executed
                                                * successfully. */
  ZB_APS_STATUS_INVALID_BINDING       = 0xa4, /*!< An APSME-UNBIND.request failed due to
                                               the requested binding link not existing in the
                                               binding table. */
  ZB_APS_STATUS_INVALID_GROUP         = 0xa5, /*!< An APSME-REMOVE-GROUP.request has
                                               been issued with a group identifier that does
                                               not appear in the group table. */
  ZB_APS_STATUS_INVALID_PARAMETER     = 0xa6, /*!< A parameter value was invalid or out of
                                               range.
                                               ZB_APS_STATUS_NO_ACK 0xa7 An APSDE-DATA.request requesting
                                               acknowledged transmission failed due to no
                                               acknowledgement being received. */
  ZB_APS_STATUS_NO_BOUND_DEVICE       = 0xa8, /*!< An APSDE-DATA.request with a destination
                                                addressing mode set to 0x00 failed due to
                                                there being no devices bound to
                                                this device. */
  ZB_APS_STATUS_NO_SHORT_ADDRESS      = 0xa9, /*!< An APSDE-DATA.request with a destination
                                                addressing mode set to 0x03 failed due to no
                                                corresponding short address found in the
                                                address map table. */
  ZB_APS_STATUS_NOT_SUPPORTED         = 0xaa, /*!< An APSDE-DATA.request with a destination
                                                addressing mode set to 0x00 failed due to a
                                                binding table not being supported on the
                                                device. */
  ZB_APS_STATUS_SECURED_LINK_KEY      = 0xab, /*!< An ASDU was received that was secured
                                                using a link key. */
  ZB_APS_STATUS_SECURED_NWK_KEY       = 0xac, /*!< An ASDU was received that was secured
                                                using a network key. */
  ZB_APS_STATUS_SECURITY_FAIL         = 0xad, /*!< An APSDE-DATA.request requesting
                                                security has resulted in an error during the
                                                corresponding security
                                                processing. */
  ZB_APS_STATUS_TABLE_FULL            = 0xae, /*!< An APSME-BIND.request or APSME.ADD-
                                                GROUP.request issued when the binding or
                                                group tables, respectively, were
                                                full. */
  ZB_APS_STATUS_UNSECURED             = 0xaf, /*!< An ASDU was received without
                                               * any security */
  ZB_APS_STATUS_UNSUPPORTED_ATTRIBUTE = 0xb0  /*!< An APSME-GET.request or APSME-
                                                SET.request has been issued with an
                                                unknown attribute identifier. */
} zb_aps_status_t;



/**
   APSDE data request structure.

   This data structure passed to zb_apsde_data_request() in the packet buffer
   (at its tail).
 */
typedef struct zb_apsde_data_req_s
{
  union zb_addr_u dst_addr; /*!< Destination address */
  zb_uint16_t  profileid;       /*!< The identifier of the profile for which this
                                  frame is intended.  */
  zb_uint16_t  clusterid;       /*!< The identifier of the object for which this
                                  frame is intended.  */
  zb_uint8_t   dst_endpoint;    /*!< either the number of the individual endpoint
                                  of the entity to which the ASDU is being
                                  transferred or the broadcast endpoint (0xff).  */
  zb_uint8_t   src_endpoint;    /*!< The individual endpoint of the entity from
                                  which the ASDU is being transferred. */
  zb_uint8_t   radius;          /*!< The distance, in hops, that a frame will be
                                 * allowed to travel through the network. */
  zb_uint8_t addr_mode;    /*!< The type of destination address supplied by
                                  the DstAddr parameter - @see zb_aps_addr_mode_e  */
  zb_uint8_t tx_options;   /*!< The transmission options for the ASDU to be
                                  transferred. These are a bitwise OR of one or
                                  more of the following:
                                  0x01 = Security enabled transmission
                                  0x02 = Use NWK key
                                  0x04 = Acknowledged transmission
                                  0x08 = Fragmentation permitted.
                                  \see zb_apsde_tx_opt_e

                           */
} zb_apsde_data_req_t;


/**
   APSME binding structure.

   This data structure passed to zb_apsme_bind_request()
 */
typedef struct zb_apsme_binding_req_s
{
  zb_ieee_addr_t  src_addr;       /*!< The source IEEE address for the binding entry. */
  zb_uint8_t      src_endpoint;       /*!< The source endpoint for the binding entry. */
  zb_uint16_t     clusterid;      /*!< The identifier of the cluster on the source
                                        device that is to be bound to the destination device.*/
  zb_uint8_t      addr_mode;      /*!< The type of destination address supplied by
                                       the DstAddr parameter - @see zb_aps_addr_mode_e  */
  union zb_addr_u dst_addr;       /*!< The destination address for the binding entry. */
  zb_uint8_t      dst_endpoint;   /*!< This parameter will be present only if
                                       the DstAddrMode parameter has a value of
                                       0x03 and, if present, will be the
                                       destination endpoint for the binding entry.*/
} zb_apsme_binding_req_t;

/**
   Parsed APS header
   This data structure passed to zb_aps_hdr_parse()
 */
typedef struct  zb_aps_hdr_s
{
  zb_uint8_t  fc;
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
  zb_uint16_t group_addr;
  zb_uint8_t  dst_endpoint;
  zb_uint8_t  src_endpoint;
  zb_uint16_t clusterid;
  zb_uint16_t profileid;
  zb_uint8_t  aps_counter;
} ZB_PACKED_STRUCT zb_aps_hdr_t;


/**
   Parameters of the APSDE-DATA.indication primitive.

 */
typedef zb_aps_hdr_t zb_apsde_data_indication_t;


/**
   The transmission options for the ASDU to be
   transferred. These are a bitwise OR of one or
   more.
 */
enum zb_apsde_tx_opt_e
{
  ZB_APSDE_TX_OPT_SECURITY_ENABLED = 1, /*!< 0x01 = Security enabled transmission  */
  ZB_APSDE_TX_OPT_USE_NWK_KEY = 2,      /*!< 0x02 = Use NWK key  */
  ZB_APSDE_TX_OPT_ACK_TX = 4,           /*!< 0x04 = Acknowledged transmission  */
  ZB_APSDE_TX_OPT_FRAG_PERMITTED = 8    /*!< 0x08 = Fragmentation permitted  */
};


/**
   NLDE-DATA.request primitive

   This function can be called via scheduler, returns immediatly.
   Later zb_nlde_data_confirm will be called to pass NLDE-DATA.request result up.

   @param apsdu - packet to send (@see zb_buf_t) and parameters at buffer tail
          \see zb_nlde_data_req_t

   @b Example:
@code
  {
    zb_apsde_data_req_t *req;
    zb_ushort_t i;

    buf = ZB_BUF_FROM_REF(param);
    ZB_BUF_INITIAL_ALLOC(buf, 10, ptr);
    for (i = 0 ; i < 10 ; ++i)
    {
      ptr[i] = i % 32 + '0';
    }

    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    req->dst_addr.addr_short = 0; // ZC
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 5;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;
    buf->u.hdr.handle = 0x11;
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
  }
@endcode
 */
void zb_apsde_data_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Remove APS header from the packet

   @param packet - APS packet
   @param ptr - (out) pointer to the APS data begin

   @b Example:
@code
void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  ptr = ZB_APS_HDR_CUT_P(asdu);

  TRACE_MSG(TRACE_APS3, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS3, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
  }

  zb_free_buf(asdu);
}
@endcode
 */
#define ZB_APS_HDR_CUT_P(packet, ptr)                   \
  ZB_BUF_CUT_LEFT(packet, zb_aps_full_hdr_size(ZB_BUF_BEGIN(packet)), ptr)


/**
   Remove APS header from the packet

   @param packet - APS packet
   @param ptr - (out) pointer to the APS data begin

   @b Example:
@code
void data_indication(zb_uint8_t param)
{
  zb_ushort_t i;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  ZB_APS_HDR_CUT(asdu);

  TRACE_MSG(TRACE_APS3, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));

  zb_free_buf(asdu);
}
@endcode
 */
#define ZB_APS_HDR_CUT(packet)                                 \
  zb_buf_cut_left(packet, zb_aps_full_hdr_size(ZB_BUF_BEGIN(packet)))


/**
   APSME-ADD-GROUP.request primitive parameters
*/
typedef struct zb_apsme_add_group_req_s
{
  zb_uint16_t group_address;    /*!< The 16-bit address of the group being added.  */
  zb_uint8_t  endpoint;         /*!< The endpoint to which the given group is being added.  */
} zb_apsme_add_group_req_t;


/**
   APSME-ADD-GROUP.confirm primitive parameters
*/
typedef struct zb_apsme_add_group_conf_s
{
  zb_uint16_t group_address;    /*!< The 16-bit address of the group being added.  */
  zb_uint8_t  endpoint;         /*!< The endpoint to which the given group is being added.  */
  zb_uint8_t  status;
} zb_apsme_add_group_conf_t;


/*! @} */



/*! \addtogroup aps_ib */
/*! @{ */

/**
   APS Information Base constants
 */
typedef enum zb_aps_aib_attr_id_e
{
  ZB_APS_AIB_BINDING                   = 0xc1, /*!< The current set of binding table entries in the device
                                                 (see subclause 2.2.8.2.1). */
  ZB_APS_AIB_DESIGNATED_COORD          = 0xc2, /*!< TRUE if the device should become the ZigBee Coordinator on
                                                 startup, FALSE if otherwise. */
  ZB_APS_AIB_CHANNEL_MASK              = 0xc3, /*!< The mask of allowable channels for this device
                                                 to use for network operations. */
  ZB_APS_AIB_USE_EXT_PANID             = 0xc4, /*!< The 64-bit address of a network to form or to join. */
  ZB_APS_AIB_GROUP_TABLE               = 0xc5, /*!< The current set of group table entries (see Table 2.25). */
  ZB_APS_AIB_NONMEMBER_RADIUS          = 0xc6, /*!< The value to be used for the NonmemberRadius
                                                 parameter when using NWK layer multicast. */
  ZB_APS_AIB_PERMISSION_CONFIG         = 0xc7, /*!< The current set of permission configuration items. */
  ZB_APS_AIB_USE_INSECURE_JOIN         = 0xc8, /*!< A flag controlling the use of insecure join at startup. */
  ZB_APS_AIB_INTERFRAME_DELAY          = 0xc9, /*!< Fragmentation parameter - the standard delay, in
                                                 milliseconds, between sending two blocks of a
                                                 fragmented transmission (see subclause 2.2.8.4.5). */
  ZB_APS_AIB_LAST_CHANNEL_ENERGY       = 0xca, /*!< The energy measurement for the channel energy
                                                 scan performed on the previous channel just
                                                 before a channel change (in accordance with [B1]). */
  ZB_APS_AIB_LAST_CHANNEL_FAILURE_RATE = 0xcb, /*!< The latest percentage of transmission network
                                                 transmission failures for the previous channel just before
                                                 a channel change (in percentage of failed transmissions
                                                 to the total number of transmissions attempted) */
  ZB_APS_AIB_CHANNEL_TIMER             = 0xcc  /*!< A countdown timer (in hours) indicating the time to the next
                                                 permitted frequency agility channel change. A value of NULL
                                                 indicates the channel has not been changed previously. */
} zb_aps_aib_attr_id_t;

/**
  APSME GET request structure
 */
typedef struct zb_apsme_get_request_s
{
  zb_aps_aib_attr_id_t aib_attr;       /*!< The identifier of the AIB attribute to read. */
} zb_apsme_get_request_t;

/**
  APSME GET confirm structure
 */
typedef struct zb_apsme_get_confirm_s
{
  zb_aps_status_t status;         /*!< The results of the request to read an AIB attribute value. */
  zb_aps_aib_attr_id_t aib_attr;  /*!< The identifier of the AIB attribute that was read.*/
  zb_uint8_t aib_length;          /*!< The length, in octets, of the attribute value being returned.*/
  /* Various */                   /*!< The value of the AIB attribute that was read.*/
} ZB_PACKED_STRUCT zb_apsme_get_confirm_t;

/**
  APSME SET request structure
 */
typedef struct zb_apsme_set_request_s
{
  zb_aps_aib_attr_id_t aib_attr;  /*!< The identifier of the AIB attribute to be written. */
  zb_uint8_t aib_length;          /*!< The length, in octets, of the attribute value being set. */
  /* Various */                   /*!< The value of the AIB attribute that should be written. */
} ZB_PACKED_STRUCT zb_apsme_set_request_t;

/**
  APSME SET confirm structure
 */
typedef struct zb_apsme_set_confirm_s
{
  zb_aps_status_t   status;       /*!< The result of the request to write the AIB Attribute. */
  zb_aps_aib_attr_id_t aib_attr;  /*!< The identifier of the AIB attribute that was written. */
} ZB_PACKED_STRUCT zb_apsme_set_confirm_t;


/**
  APSME GET request primitive
 */
void zb_apsme_get_request(zb_uint8_t param) ZB_CALLBACK;

/**
  APSME GET confirm primitive
 */
void zb_apsme_get_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
  APSME SET request primitive
 */
void zb_apsme_set_request(zb_uint8_t param) ZB_CALLBACK;

/**
  APSME SET confirm primitive
 */
void zb_apsme_set_confirm(zb_uint8_t param) ZB_CALLBACK;


/*! @} */


/*! \cond internals_doc */
/*! \addtogroup ZB_APS */
/*! @{ */

/**
   Initialize APS subsystem
 */
void zb_aps_init();


/**
   APSDE-DATA.confirm primitive

   The primitive reports the results of a request to transfer a data PDU (ASDU) from

   Note that zb_apsde_data_confirm must be defined in the APS layer!
   APS layer just calls this function.

   @param nsdu - sent packet - @see zb_buf_t. APS must free or reuse it. Following packet fields
          are used:
          * status - The status of the corresponding request.
                     INVALID_REQUEST,
                     MAX_FRM_COUNTER,
                     NO_KEY,
                     BAD_CCM_OUTPUT,
                     ROUTE_ERROR,
                     BT_TABLE_FULL,
                     FRAME_NOT_BUFFERED
                     or any status values returned
                     from security suite or the
                     MCPS-DATA.confirm
                     primitive.
          * handle - The handle associated with the NSDU being confirmed.
 */
void zb_apsde_data_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   APS command codes constants
 */
enum zb_aps_commands_e
{
  APS_CMD_SKKE_1 = 1,
  APS_CMD_SKKE_2,
  APS_CMD_SKKE_3,
  APS_CMD_SKKE_4,
  APS_CMD_TRANSPORT_KEY,
  APS_CMD_UPDATE_DEVICE,
  APS_CMD_REMOVE_DEVICE,
  APS_CMD_REQUEST_KEY,
  APS_CMD_SWITCH_KEY,
  APS_CMD_EA_INIT_CHLNG,
  APS_CMD_EA_RSP_CHLNG,
  APS_CMD_EA_INIT_MAC_DATA,
  APS_CMD_EA_RSP_MAC_DATA,
  APS_CMD_TUNNEL
};


/**
   NLDE-DATA.indication primitive

   This function called via scheduler by the NWK layer to pass
   incoming data packet to the APS layer.
   Note that zb_nlde_data_indication() must be defined in the APS layer!
   NWK layer just calls this function.

   @param nsdu - The set of octets comprising the NSDU to be
                 transferred (with length)

   Other fields got from MAC nsdu by macros

 */
void zb_apsde_data_indication(zb_uint8_t param) ZB_CALLBACK;

zb_uint8_t aps_find_src_ref(zb_address_ieee_ref_t src_addr_ref, zb_uint8_t src_end, zb_uint16_t cluster_id);

/**
   APSME-BIND.request primitive.
 */
void zb_apsme_bind_request(zb_uint8_t param) ZB_CALLBACK;

/**
   APSME-BIND.confirm primitive
*/
void zb_apsme_bind_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   APSME-UNBIND.request primitive.
 */
void zb_apsme_unbind_request(zb_uint8_t param) ZB_CALLBACK;

/**
   APSME-UNBIND.confirm primitive
*/
void zb_apsme_unbind_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   Signals user that data is sent and acknowledged
 */
void zb_apsde_data_acknowledged(zb_uint8_t param) ZB_CALLBACK;

/**
   APSME-UPDATE-DEVICE.indication primitive
 */
void zb_apsme_update_device_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Send APS command.

   Internal routine to be used to send APS command frame.
   See 2.2.5.2.2  APS Command Frame Format

   @param param - buffer with command body
   @param dest_addr - address of device to send to
   @param command - command id
   @param secure  - if true, secure transfer at nwk level
 */
void zb_aps_send_command(zb_uint8_t param, zb_uint16_t dest_addr, zb_uint8_t command, zb_bool_t secure);

/**
  \par Macros for APS FC parse-compose
 */

#define ZB_APS_FC_GET_FRAME_TYPE(fc)  ((fc) & 3)
#define ZB_APS_FC_SET_FRAME_TYPE(fc, t)  ((fc) |= (t))

#define ZB_APS_FC_GET_DELIVERY_MODE(fc) (((fc)>>2) & 3)
#define ZB_APS_FC_SET_DELIVERY_MODE(fc, m) ((fc) |= (m) << 2)

#define ZB_APS_FC_GET_ACK_FORMAT(fc) (((fc)>>4) & 1)
#define ZB_APS_FC_SET_ACK_FORMAT(fc, f) ((fc) |= ((f) << 4))

#define ZB_APS_FC_GET_SECURITY(fc) (((fc)>>5) & 1)
#define ZB_APS_FC_SET_SECURITY(fc, f) ((fc) |= ((f) << 5))

#define ZB_APS_FC_GET_ACK_REQUEST(fc) (((fc)>>6) & 1)
#define ZB_APS_FC_SET_ACK_REQUEST(fc, f) ((fc) |= ((f) << 6))

#define ZB_APS_FC_GET_EXT_HDR_PRESEBT(fc) (((fc)>>7) & 1)
#define ZB_APS_FC_SET_EXT_HDR_PRESENT(fc, f) ((fc) |= ((f) << 7))


/**
   Setup FC for APS command: frame type, ack request, ack format
 */
#define ZB_APS_FC_SET_COMMAND(fc, need_ack)                                      \
  (fc) |= (/*frame type*/ZB_APS_FRAME_COMMAND | /*ack req*/(((need_ack)) << 6) | /*ack format*/(1 << 4))


enum zb_aps_frame_type_e
{
  ZB_APS_FRAME_DATA = 0,
  ZB_APS_FRAME_COMMAND = 1,
  ZB_APS_FRAME_ACK = 2
};


enum zb_aps_frame_delivery_mode_e
{
  ZB_APS_DELIVERY_UNICAST = 0,
  ZB_APS_DELIVERY_INDIRECT = 1,
  ZB_APS_DELIVERY_BROADCAST = 2,
  ZB_APS_DELIVERY_GROUP = 3
};


/**
   Ger APS header size (not include AUX security hdr)
 */
#define ZB_APS_HDR_SIZE(fc)                                             \
  (2 +                          /* fc + aps counter */                  \
   ((ZB_APS_FC_GET_FRAME_TYPE(fc) == ZB_APS_FRAME_COMMAND) ? 0 :  (     \
     /* Packet either has dest endpoint (1) or group address (2) */     \
     1 +                                                                \
     (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_GROUP) +       \
     /* cluster id, profile id */                                       \
     2 + 2 +                                                            \
     /* src endpoint */                                                 \
     1 +                                                                \
     /* TODO: handle fragmentation and Extended header. Now suppose no Extended header */ \
     0                                                                  \
    )))

/**
   Return APS header + APS aux hdr size
 */
zb_ushort_t zb_aps_full_hdr_size(zb_uint8_t *pkt);


#define APS_CONFIRM_STATUS(buf, s, func)                \
do                                                      \
{                                                       \
  (buf)->u.hdr.status = (s);                            \
  ZB_SCHEDULE_CALLBACK((func), ZB_REF_FROM_BUF((buf))); \
}                                                       \
while(0)


/**
   APSME-ADD-GROUP.request primitive

   Note that this routine DOES NOT call APSME-ADD-GROUP.confirm primitive.
   Instead, from the application layer use zb_zdo_add_group_req which has
   additional parameter - user's callback to be called as
   APSME-ADD-GROUP.confirm.

   @param param - buffer with parameter. \see zb_apsme_add_group_req_t

*/
void zb_apsme_add_group_request(zb_uint8_t param) ZB_CALLBACK;

#define ZDO_MGMT_APS_LEAVE_RESP_CLID    0x8034 /* it is from zdo for leave callback*/

void zb_aps_hdr_parse(zb_buf_t *packet, zb_aps_hdr_t *aps_hdr, zb_bool_t cut_nwk_hdr) ZB_SDCC_REENTRANT;

/*! @} */
/*! \endcond */

#endif /* ZB_APS_H */
