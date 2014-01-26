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
PURPOSE: Network layer API
*/

#ifndef ZB_NWK_H
#define ZB_NWK_H 1

#include "zb_mac.h"
#include "zb_nwk_nib.h"

/*! \addtogroup nwk_api */
/*! @{ */

/**
   Network broadcast addresses types
 */
typedef enum zb_nwk_broadcast_address_e
{
  ZB_NWK_BROADCAST_ALL_DEVICES        = 0xFFFF, /*!< All devices in PAN */
  ZB_NWK_BROADCAST_RESERVED           = 0xFFFE,
  ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE    = 0xFFFD, /*!< macRxOnWhenIdle = TRUE */
  ZB_NWK_BROADCAST_ROUTER_COORDINATOR = 0xFFFC, /*!< All routers and coordinator */
  ZB_NWK_BROADCAST_LOW_POWER_ROUTER   = 0xFFFB /*!< Low power routers only */
} ZB_PACKED_STRUCT
zb_nwk_broadcast_address_t;

/**
   Check that address is broadcast

   @param addr - 16-bit address
   @return TRUE if address is broadcast, FALSE otherwhise
 */
#define ZB_NWK_IS_ADDRESS_BROADCAST(addr) ( ((addr) & 0xFFF0) == 0xFFF0 )

/**
   NWK layer status values

   Got from 3.7
 */
typedef enum zb_nwk_status_e
{
  ZB_NWK_STATUS_SUCCESS                = 0x00, /*!< A request has been executed
                                                * successfully. */
  ZB_NWK_STATUS_INVALID_PARAMETER      = 0xC1, /*!< An invalid or out-of-range
                                                * parameter has been passed to a
                                                * primitive from the next higher
                                                * layer. */
  ZB_NWK_STATUS_INVALID_REQUEST        = 0xC2, /*!< The next higher layer has
                                                * issued a request that is
                                                * invalid or cannot be executed
                                                * given  the current state of
                                                * the  NWK layer. */
  ZB_NWK_STATUS_NOT_PERMITTED          = 0xC3, /*!< An NLME-JOIN.request has
                                                * been  disallowed. */
  ZB_NWK_STATUS_STARTUP_FAILURE        = 0xC4, /*!< An
                                                * NLME-NETWORK-FORMATION.request
                                                * has failed to start a
                                                * network. */
  ZB_NWK_STATUS_ALREADY_PRESENT        = 0xC5, /*!< A device with the address
                                                * supplied to the
                                                * NLMEDIRECT-JOIN.request is
                                                * already  present in the
                                                * neighbor table of the device
                                                * on  which the
                                                * NLMEDIRECT-JOIN.request was
                                                * issued. */
  ZB_NWK_STATUS_SYNC_FAILURE           = 0xC6, /*!< Used to indicate that an
                                                * NLME-SYNC.request has failed
                                                * at the MAC layer. */
  ZB_NWK_STATUS_NEIGHBOR_TABLE_FULL    = 0xC7, /*!< An
                                                * NLME-JOIN-DIRECTLY.request has
                                                * failed because there is no
                                                * more room in the neighbor
                                                * table. */
  ZB_NWK_STATUS_UNKNOWN_DEVICE         = 0xC8, /*!< An NLME-LEAVE.request has
                                                * failed because the device
                                                * addressed in the parameter
                                                * list is not in the neighbor
                                                * table of the issuing device. */
  ZB_NWK_STATUS_UNSUPPORTED_ATTRIBUTE  = 0xC9, /*!< An NLME-GET.request or
                                                * NLME-SET.request has been
                                                * issued with an unknown
                                                * attribute identifier. */
  ZB_NWK_STATUS_NO_NETWORKS            = 0xCA, /*!< An NLME-JOIN.request has
                                                * been issued in an environment
                                                * where no networks are
                                                * detectable. */
  ZB_NWK_STATUS_MAX_FRM_COUNTER        = 0xCC, /*!< Security processing has been
                                                * attempted on an outgoing
                                                * frame, and has failed because
                                                * the frame counter has reached
                                                * its maximum value. */
  ZB_NWK_STATUS_NO_KEY                 = 0xCD, /*!< Security processing has been
                                                * attempted on an outgoing
                                                * frame, and has failed because
                                                * no key was available with
                                                * which to process it. */
  ZB_NWK_STATUS_BAD_CCM_OUTPUT         = 0xCE, /*!< Security processing has been
                                                * attempted on an outgoing
                                                * frame, and has failed because
                                                * the security engine produced
                                                * erroneous output. */
  ZB_NWK_STATUS_NO_ROUTING_CAPACITY    = 0xCF, /*!< An attempt to discover a
                                                * route has failed due to a lack
                                                * of routing table or discovery
                                                * table capacity. */
  ZB_NWK_STATUS_ROUTE_DISCOVERY_FAILED = 0xD0, /*!< An attempt to discover a
                                                * route has failed due to a
                                                * reason other than a lack of
                                                * routing capacity. */
  ZB_NWK_STATUS_ROUTE_ERROR            = 0xD1, /*!< An NLDE-DATA.request has
                                                * failed due to a routing
                                                * failure on the sending
                                                * device. */
  ZB_NWK_STATUS_BT_TABLE_FULL          = 0xD2, /*!< An attempt to send a
                                                * broadcast frame or member mode
                                                * multicast has failed due to
                                                * the fact that there is no room
                                                * in the BTT. */
  ZB_NWK_STATUS_FRAME_NOT_BUFFERED     = 0xD3  /*!< An NLDE-DATA.request has
                                                * failed due to insufficient
                                                * buffering available. A
                                                * non-member mode multicast
                                                * frame was discarded pending
                                                * route discovery. */
}
zb_nwk_status_t;


/**
   Network command status codes
*/
typedef enum zb_nwk_command_status_e
{
  ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE           = 0x00, /*!< No route available */
  ZB_NWK_COMMAND_STATUS_TREE_LINK_FAILURE            = 0x01, /*!< Tree link failure */
  ZB_NWK_COMMAND_STATUS_NONE_TREE_LINK_FAILURE       = 0x02, /*!< None-tree link failure */
  ZB_NWK_COMMAND_STATUS_LOW_BATTERY_LEVEL            = 0x03, /*!< Low battery level */
  ZB_NWK_COMMAND_STATUS_NO_ROUTING_CAPACITY          = 0x04, /*!< No routing capacity */
  ZB_NWK_COMMAND_STATUS_NO_INDIRECT_CAPACITY         = 0x05, /*!< No indirect capacity */
  ZB_NWK_COMMAND_STATUS_INDIRECT_TRANSACTION_EXPIRY  = 0x06, /*!< Indirect transaction expiry */
  ZB_NWK_COMMAND_STATUS_TARGET_DEVICE_UNAVAILABLE    = 0x07, /*!< Target device unavailable */
  ZB_NWK_COMMAND_STATUS_TARGET_ADDRESS_UNALLOCATED   = 0x08, /*!< Target address unallocated */
  ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE          = 0x09, /*!< Parent link failure */
  ZB_NWK_COMMAND_STATUS_VALIDATE_ROUTE               = 0x0a, /*!< Validate route */
  ZB_NWK_COMMAND_STATUS_SOURCE_ROUTE_FAILURE         = 0x0b, /*!< Source route failure */
  ZB_NWK_COMMAND_STATUS_MANY_TO_ONE_ROUTE_FAILURE    = 0x0c, /*!< Many-to-one route failure */
  ZB_NWK_COMMAND_STATUS_ADDRESS_CONFLICT             = 0x0d, /*!< Address conflict */
  ZB_NWK_COMMAND_STATUS_VERIFY_ADDRESS               = 0x0e, /*!< Verify address */
  ZB_NWK_COMMAND_STATUS_PAN_IDENTIFIER_UPDATE        = 0x0f, /*!< Pan identifier update */
  ZB_NWK_COMMAND_STATUS_NETWORK_ADDRESS_UPDATE       = 0x10, /*!< Network address update */
  ZB_NWK_COMMAND_STATUS_BAD_FRAME_COUNTER            = 0x11, /*!< Bad frame counter  */
  ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER      = 0x12 /*!< Bad key sequence number */
}
zb_nwk_command_status_t;


/**
   'frame security failed' status mentioned in 4.3.1.2  Security Processing of
   Incoming Frames but not defined in the table 3.42   Status Codes for Network
   Status Command Frame

   Really need this status for for intra-pan portrability procedure
   (AZD601,602). Let's use other security status code.
 */
#define ZB_NWK_COMMAND_STATUS_FRAME_SECURITY_FAILED  ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER


/**
   Check that NWK command status is security-related

   @param st - status code
   @return 1 if NWK command status is security-related
 */
#define ZB_NWK_COMMAND_STATUS_IS_SECURE(st) \
  ((st) == ZB_NWK_COMMAND_STATUS_BAD_FRAME_COUNTER || (st) == ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER)


/**
   Parameters for NLDE-DATA.request primitive
 */
typedef struct zb_nlde_data_req_s
{
  zb_uint16_t  dst_addr;        /*!< Destination address.  */
  zb_uint8_t   radius;          /*!< The distance, in hops, that a frame will be
                                 * allowed to travel through the network. */
  zb_uint8_t addr_mode;    /*!< The type of destination address supplied by
                                  the DstAddr parameter - @see zb_addr_mode_e  */
  zb_uint8_t nonmember_radius; /*!< The distance, in hops, that a multicast
                                      frame will be relayed by nodes not a
                                      member of the group. A value of 0x07 is
                                      treated as infinity.  */
  zb_uint8_t discovery_route; /*!< The DiscoverRoute parameter may be
                                     used to control route discovery
                                     operations for the transit of this frame
                                     (see sub-clause3.6.3.5):
                                     0x00 = suppress route discovery
                                     0x01 = enable route discovery  */
  zb_uint8_t security_enable; /*!< The SecurityEnable parameter may be
                                     used to enable NWK layer security
                                     processing for the current frame. If the
                                     nwkSecurityLevel attribute of the NIB
                                     has a value of 0, meaning no security,
                                     then this parameter will be ignored.
                                     Otherwise, a value of TRUE denotes
                                     that the security processing specified
                                     by the security level will be applied,
                                     and a value of FALSE denotes that no
                                     security processing will be applied.  */

  zb_uint8_t ndsu_handle; /*!< The handle associated with the NSDU to be
                           * transmitted by the NWK layer entity. */

} zb_nlde_data_req_t;


/**
   NLDE-DATA.request primitive

   This function return immediatly.
   Later zb_nlde_data_confirm will be called to pass NLDE-DATA.request result up.

   @param nldereq - parameters structure - @see zb_nlde_data_req_t
                    This variable does not pass to other levels, so it can be
                    local variable in the caller.

   @b Example:
@code
  {
    zb_nlde_data_req_t *req;
    zb_uint16_t dst_addr;

    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_nlde_data_req_t));
    // send to parent
    zb_address_short_by_ref(&dst_addr, ZG->nwk.handle.parent);
    TRACE_MSG(TRACE_APS3, "parent %hd parent_addr %d", (FMT__H_D, ZG->nwk.handle.parent, dst_addr));

    req->dst_addr = dst_addr;
    req->radius = 0; // use default
    req->addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    req->discovery_route = 0;
    req->security_enable = 0;
    req->ndsu_handle = 10;

    TRACE_MSG(TRACE_APS3, "Sending nlde_data.request", (FMT__0));
    ZB_SCHEDULE_CALLBACK(zb_nlde_data_request, ZB_REF_FROM_BUF(buf));
  }
@endcode
 */
void zb_nlde_data_request(zb_uint8_t param) ZB_CALLBACK;

void call_status_indication(zb_uint8_t param) ZB_CALLBACK;

/**
   Arguments of the NLME-STATUS.request routine.
*/
typedef struct zb_nlme_status_indication_s
{
  zb_nwk_command_status_t status; /*!< Error code associated with the failure */
  zb_uint16_t network_addr;  /*!< The network device address associated with the
                              * status information */
} ZB_PACKED_STRUCT
zb_nlme_status_indication_t;


/**
   Arguments of the NLME-SEND-STATUS.confirm routine.
*/
typedef struct zb_nlme_send_status_s
{
  zb_uint16_t dest_addr; /*!< address to send status information to */
  zb_nlme_status_indication_t status; /*!< status information @see
                                       * zb_nlme_status_indication_t */
  zb_uint8_t ndsu_handle; /*!< The handle associated with the NSDU to be
                           * transmitted by the NWK layer entity. */
} ZB_PACKED_STRUCT
zb_nlme_send_status_t;


/**
   Send status indication primitive

   Send status to the remote device

   @param v_buf - request params - @see
   zb_nlme_send_status_t
   @return nothing

   @b Example:
@code
{
  zb_nlme_send_status_t *request = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_send_status_t);

  request->dest_addr = 0; // send status indication to the coordinator
  request->status.status = ZB_NWK_COMMAND_STATUS_LOW_BATTERY_LEVEL;
  request->status.network_addr = ZB_NIB_NETWORK_ADDRESS();
  request->ndsu_handle = 0;

  ZB_SCHEDULE_CALLBACK(zb_nlme_send_status, param);
}

@endcode
 */
void zb_nlme_send_status(zb_uint8_t param) ZB_CALLBACK;


/*! @} */

/*! \addtogroup nwk_ib */
/*! @{ */



/**
   Arguments of the NLME-GET.request routine.
*/
typedef struct zb_nlme_get_request_s
{
  zb_nib_attribute_t nib_attribute; /*!< Attribute value, @see
                                     * zb_nib_attribute_t */
} ZB_PACKED_STRUCT
zb_nlme_get_request_t;


/**
   NLME-GET.request primitive

   Perform get NIB attribute

   @param v_buf - buffer containing parameters - @see
   zb_nlme_get_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_get_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-GET.confirm routine.
*/
typedef struct zb_nlme_get_confirm_s
{
  zb_nwk_status_t status; /*!< The result of the operation */
  zb_nib_attribute_t nib_attribute;  /*!< Attribute value, @see
                                      * zb_nib_attribute_t */
  zb_uint16_t attribute_length; /*!< Length attribute value */
  /* next is attribute value */
} ZB_PACKED_STRUCT
zb_nlme_get_confirm_t;


/**
   NLME-GET.confirm primitive

   Report the results of reading attribute from NIB.

   @param v_buf - buffer containing results - @see
   zb_nlme_get_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_get_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-SET.request routine.
*/
typedef struct zb_nlme_set_request_s
{
  zb_nib_attribute_t nib_attribute; /*!< Attribute value, @see zb_nib_attribute_t */
  zb_uint16_t attr_length; /*!<  */
} ZB_PACKED_STRUCT
zb_nlme_set_request_t;


/**
   NLME-SET.request primitive

   Perform set NIB attribute

   @param v_buf - buffer containing parameters - @see
   zb_nlme_set_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_set_request(zb_uint8_t param) ZB_CALLBACK;



/**
   Arguments of the NLME-SET.confirm routine.
*/
typedef struct zb_nlme_set_confirm_s
{
  zb_nwk_status_t status;           /*!< The result of the operation */
  zb_nib_attribute_t nib_attribute; /*!< Attribute value, @see
                                     * zb_nib_attribute_t */
} ZB_PACKED_STRUCT
zb_nlme_set_confirm_t;


/**
   NLME-SET.confirm primitive

   Report the results of writing attribute from NIB.

   @param v_buf - buffer containing results - @see
   zb_nlme_set_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_set_confirm(zb_uint8_t param) ZB_CALLBACK;

/*! @} */

/*! \cond internals_doc */
/*! \addtogroup ZB_NWK */
/*! @{ */


/**
   NLDE-DATA.confirm primitive

   This function called via scheduler by the NWK layer to indicate
   NLDE-DATA.request result to the APS layer.
   Note that zb_nlde_data_confirm must be defined in the APS layer!
   NWK layer just calls this function.
   NWK and lower lagers does not free nsdu. APS must do it (or it can reuse it,
   transmit to the another address etc).

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
void zb_nlde_data_confirm(zb_uint8_t param) ZB_CALLBACK;


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
void zb_nlde_data_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Initialize NWK stack layer
 */
void zb_nwk_init();



/**
   \par Macros to compare network update ids taking overflow into account.

  No recommendations about compare in the specification.
  Now suppose overflow if difference between values > 1/2 of the entire values
  diapason. Is it right?
 */
#define NWK_UPDATE_ID_MIDDLE 127

/**
   @return true if id1 >= id2, taking overflow into account
 */
#define ZB_NWK_UPDATE_ID1_GE_ID2(id1, id2) (((zb_uint8_t)(id1) - (zb_uint8_t)(id2)) < NWK_UPDATE_ID_MIDDLE)

/**
   @return true if id1 < id2, taking overflow into account
 */
#define ZB_NWK_UPDATE_ID1_LT_ID2(id1, id2) (((zb_uint8_t)(id1) - (zb_uint8_t)(id2)) > NWK_UPDATE_ID_MIDDLE)


/**
   Check that link quality is good enough to attempt to join

   According to 6.9.8 Link quality indicator (LQI),
   The minimum and maximum LQI values (0x00 and 0xff) should be associated with the lowest and highest
   quality compliant signals detectable by the receiver, and LQI values in between should be uniformly
   distributed between these two limits. At least eight unique values of LQI
   shall be used.


   Also, from 3.6.1.4.1.1    Child Procedure:
   - The link quality for frames received from this device is such that a link cost of
   at most 3 is produced when calculated as described in sub-clause 3.6.3.1.

   @return TRUE if it is ok to join to device with such LQI and FALSE otherwhise
 */
#ifndef ZB_NS_BUILD
#define ZB_LINK_QUALITY_IS_OK_FOR_JOIN(lqi) (((lqi) / (256 / 8)) >= 3)
#else
                                           /* no lqi in NS - always ok */
#define ZB_LINK_QUALITY_IS_OK_FOR_JOIN(lqi) 1
#endif


/**
   Compare link quality

   @return TRUE if lqi1 > lqi2
 */
#define ZB_LINK_QUALITY_1_IS_BETTER(lqi1, lqi2) ((lqi1) > (lqi2))


/**
   \par macros to manipulate with nwk packet header
 */

/**
   NWK packet header.

   Fields up to seq_num always pesents, other - depending on frame_control
   contents.

   See 3.3.1 for the NWK header definition.
 */
typedef struct zb_nwk_hdr_s
{
  zb_uint8_t     frame_control[2];
  zb_uint16_t    dst_addr;
  zb_uint16_t    src_addr;
  zb_uint8_t     radius;
  zb_uint8_t     seq_num;
  zb_ieee_addr_t dst_ieee_addr;
  zb_ieee_addr_t src_ieee_addr;
  zb_uint8_t     mcast_control;
} ZB_PACKED_STRUCT zb_nwk_hdr_t;


/**
   Frame type value from the NWK FCF field: data
 */
#define ZB_NWK_FRAME_TYPE_DATA     0

/**
   Frame type value from the NWK FCF field: command
 */
#define ZB_NWK_FRAME_TYPE_COMMAND  1


/**
   \par NWK FCF stored in the order it transmitted over network. Bits access
   macros takes care about using right bytes. No endian conversion is necessary.
   See \see ZB_PKT_16B_ZERO_BYTE / \see ZB_PKT_16B_FIRST_BYTE definition for
   details.

   See 3.3.1.1 for the NWK FCF definition.
 */

/**
   Get frame type from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
 */
#define ZB_NWK_FRAMECTL_GET_FRAME_TYPE(fctl) ((fctl)[ZB_PKT_16B_ZERO_BYTE] & 3)

/**
   Set frame type in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param t - frame type.
 */
#define ZB_NWK_FRAMECTL_SET_FRAME_TYPE(fctl, t ) ((fctl)[ZB_PKT_16B_ZERO_BYTE] |= (t))

/**
   Get protocol version from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_PROTOCOL_VERSION(fctl) (((fctl)[ZB_PKT_16B_ZERO_BYTE] >> 2) & 0xf)



/**
   Set frame type in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param v - protocol version
 */
#define ZB_NWK_FRAMECTL_SET_PROTOCOL_VERSION(fctl, v) ((fctl)[ZB_PKT_16B_ZERO_BYTE] |= ((v) << 2))

#define ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(fctl, t, v) ((fctl)[ZB_PKT_16B_ZERO_BYTE] |= ((t) | ((v) << 2)))


/**
   Get 'discover route' from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_DISCOVER_ROUTE(fctl) (((fctl)[ZB_PKT_16B_ZERO_BYTE] >> 5) & 3)

/**
   Set 'discover route' in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param r - 'discover route' value
 */
#define ZB_NWK_FRAMECTL_SET_DISCOVER_ROUTE(fctl, r) ((fctl)[ZB_PKT_16B_ZERO_BYTE] |= ((r) << 6))

/**
   Get multicast flag from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_MULTICAST_FLAG(fctl) (((fctl)[ZB_PKT_16B_FIRST_BYTE]) & 1)

/**
   Set multicast flag in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param m - multicast flag value
 */
#define ZB_NWK_FRAMECTL_SET_MULTICAST_FLAG(fctl, m) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= (m))


/**
   Get 'security' from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_SECURITY(fctl) (((fctl)[ZB_PKT_16B_FIRST_BYTE] >> 1) & 1)

/**
   Set 'security' in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param s - 'security' value
 */
#define ZB_NWK_FRAMECTL_SET_SECURITY(fctl, s) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= ((s) << 1))


/**
   Get 'source route' from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_SOURCE_ROUTE(fctl) (((fctl)[ZB_PKT_16B_FIRST_BYTE] >> 2) & 1)

/**
   Set 'source route' in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param s - 'source route' value
 */
#define ZB_NWK_FRAMECTL_SET_SOURCE_ROUTE(fctl, s) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= ((s) << 2))

/**
   Get 'destination ieee address' from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
*/
#define ZB_NWK_FRAMECTL_GET_DESTINATION_IEEE(fctl) (((fctl)[ZB_PKT_16B_FIRST_BYTE] >> 3) & 1)

/**
   Set 'destination ieee address' in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param s - 'destination ieee address' value
 */
#define ZB_NWK_FRAMECTL_SET_DESTINATION_IEEE(fctl, d) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= ((d) << 3))

/**
   Get 'source ieee address' from the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
 */
#define ZB_NWK_FRAMECTL_GET_SOURCE_IEEE(fctl) (((fctl)[ZB_PKT_16B_FIRST_BYTE] >> 4) & 1)
/**
   Set 'source ieee address' in the NWK header Frame Control field

   @param fctl - Frame Control Field of NWK header
   @param s - 'source ieee address' value
 */
#define ZB_NWK_FRAMECTL_SET_SOURCE_IEEE(fctl, s) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= ((s) << 4))


#define ZB_NWK_FRAMECTL_SET_SRC_DEST_IEEE(fctl, s, d) ((fctl)[ZB_PKT_16B_FIRST_BYTE] |= (((d) << 3) | ((s) << 4)))

/**
   Return size of the header part up to addresses

   @param fctl - Frame Control Field of NWK header
   @return header part size
 */
#define ZB_NWK_SHORT_HDR_SIZE(is_multicast) \
  (ZB_OFFSETOF(zb_nwk_hdr_t, dst_ieee_addr) + ((is_multicast) ? 1 : 0))

/**
   Return hdr size with only one extended address present

   @return header part size
 */
#define ZB_NWK_HALF_HDR_SIZE(is_multicast) (ZB_OFFSETOF(zb_nwk_hdr_t, src_ieee_addr) + ((is_multicast) ? 1 : 0))

/**
   Return full size of the header with extended addresses

   @return header part size
 */
#define ZB_NWK_FULL_HDR_SIZE(is_multicast) (ZB_OFFSETOF(zb_nwk_hdr_t, mcast_control) + ((is_multicast) ? 1 : 0))


/**
   Calculate network header size by NWK FCF

   @param fctl - Frame Control Field of NWK header
   @return header size
 */
zb_ushort_t zb_nwk_hdr_size(zb_uint8_t *fctl);
#define ZB_NWK_HDR_SIZE(fctl) zb_nwk_hdr_size(fctl)


/**
   Remove network header from an NWK packet placed into a packet buffer.

   To be used by APS layer to cut NWK header to get APS packet.

   @param packet - buffer holding packet
   @param ptr    - (out) pointer to the beginning of packet just after header.
 */
#define ZB_NWK_HDR_CUT(packet, ptr)                   \
do                                                    \
{                                                     \
  zb_nwk_hdr_t *_nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(packet); \
  ZB_BUF_CUT_LEFT(packet, ZB_NWK_HDR_SIZE(_nwk_hdr->frame_control), ptr);   \
} while (0)


/**
   Get source address from the NWK header

   @param nwk_hdr - pointer to the network packet header
   @param addr - pointer to the short address
 */
#define ZB_NWK_GET_SRC_ADDR(nwk_hdr, addr) \
ZB_LETOH16(addr, &((zb_nwk_hdr_t *)nwk_hdr)->src_addr)

#define ZB_NWK_GET_DST_ADDR(nwk_hdr, addr) \
ZB_LETOH16(addr, &((zb_nwk_hdr_t *)nwk_hdr)->dst_addr)

#define ZB_NWK_ADDR_TO_H16(addr)                \
{                                               \
  zb_uint16_t tmp = addr;                       \
  ZB_LETOH16(&addr, &tmp);                      \
}

#define ZB_NWK_ADDR_TO_LE16(addr)                \
{                                                \
  zb_uint16_t tmp = addr;                        \
  ZB_HTOLE16(&addr, &tmp);                       \
}

#define ZB_PAN_ID_TO_H16(addr)                  \
{                                               \
  zb_uint16_t tmp = addr;                       \
  ZB_LETOH16(&addr, &tmp);                      \
}

#define ZB_PAN_ID_TO_LE16(addr)                  \
{                                                \
  zb_uint16_t tmp = addr;                        \
  ZB_HTOLE16(&addr, &tmp);                       \
}


/**
   Network command constants
 */
typedef enum zb_nwk_cmd_e
{
  ZB_NWK_CMD_ROUTE_REQUEST   = 0x01,
  ZB_NWK_CMD_ROUTE_REPLY     = 0x02,
  ZB_NWK_CMD_NETWORK_STATUS  = 0x03,
  ZB_NWK_CMD_LEAVE           = 0x04,
  ZB_NWK_CMD_ROUTE_RECORD    = 0x05,
  ZB_NWK_CMD_REJOIN_REQUEST  = 0x06,
  ZB_NWK_CMD_REJOIN_RESPONSE = 0x07,
  ZB_NWK_CMD_LINK_STATUS     = 0x08,
  ZB_NWK_CMD_NETWORK_REPORT  = 0x09,
  ZB_NWK_CMD_NETWORK_UPDATE  = 0x0a
} ZB_PACKED_STRUCT
zb_nwk_cmd_t;

/**
   Get command id from nwk command packet

   @param buf - pointer to the packet buffer
   @param nwk_hdr_size - network packet header
 */
#define ZB_NWK_CMD_FRAME_GET_CMD_ID(buf, nwk_hdr_size) ( *(zb_nwk_cmd_t *)(ZB_BUF_BEGIN(buf) + (nwk_hdr_size)) )

/**
   Get command pointer to the command payload from nwk command packet

   @param buf - pointer to the packet buffer
   @param nwk_hdr_size - network packet header size
 */
#define ZB_NWK_CMD_FRAME_GET_CMD_PAYLOAD(buf, nwk_hdr_size) ( (zb_uint8_t *)(ZB_BUF_BEGIN(buf) + (nwk_hdr_size)) + 1)

/**
   Route request command
*/
typedef struct zb_nwk_cmd_rreq_s
{
  zb_uint8_t opt; /*!< Command options */
  zb_uint8_t rreq_id; /*!< Route request id */
  zb_uint16_t dest_addr; /*!< Final destination address */
  zb_int8_t path_cost; /*!< Route request total path cost */
} ZB_PACKED_STRUCT
zb_nwk_cmd_rreq_t;

/**
   Route response command
*/
typedef struct zb_nwk_cmd_rrep_s
{
  zb_uint8_t opt; /*!< Command options */
  zb_uint8_t rreq_id; /*!< Route request id */
  zb_uint16_t originator; /*!< Originator address */
  zb_uint16_t responder; /*!< Responder address */
  zb_uint8_t path_cost; /*!< Path cost */
} ZB_PACKED_STRUCT
zb_nwk_cmd_rrep_t;

/**
   Rejoin request command
*/
typedef struct zb_nwk_rejoin_request_s
{
  zb_mac_capability_info_t capability_information; /*!< The operating capabilities of the
                                                    * device */
} ZB_PACKED_STRUCT
zb_nwk_rejoin_request_t;

/**
   Rejoin response command
*/
typedef struct zb_nwk_rejoin_response_s
{
  zb_uint16_t network_addr; /*!< network address */
  zb_uint8_t rejoin_status; /*!< rejoin status */
} ZB_PACKED_STRUCT
zb_nwk_rejoin_response_t;

#define ZB_NWK_COMMAND_SIZE(payload_size) (1 + payload_size)
#define ZB_NWK_ALLOC_COMMAND_GET_PAYLOAD_PTR(buf, cmd_id, cmd_size, payload) \
  {                                                                     \
    zb_uint8_t *_ptr;                                                   \
    ZB_BUF_ALLOC_RIGHT(buf, ZB_NWK_COMMAND_SIZE(cmd_size), _ptr);       \
    *_ptr = (cmd_id);                                                   \
    (payload) = (zb_uint8_t *)(_ptr + 1);                               \
  }

/**
   Network discovery request parameters.
   This structure describe parameters passed to
   zb_nlme_network_discovery_request call.
*/
typedef struct zb_nlme_network_discovery_request_s
{
  zb_uint32_t scan_channels; /*!< Bit mask indicates channels to scan */
  zb_uint8_t scan_duration; /*!< Time to spend scanning each channel */
} ZB_PACKED_STRUCT
zb_nlme_network_discovery_request_t;


/**
   NLME-NETWORK-DISCOVERY.request primitive

   This function return immediatly.
   Later zb_nlme_network_discovery_confirm will be called to pass NLME-NETWORK-DISCOVERY.confirm result up.

   @param v_buf - buffer containing parameters - @see
   zb_nlme_network_discovery_request_t
   @return nothing
 */
void zb_nlme_network_discovery_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Network descriptor.
   This is a part of network discovery confirm result.
*/
typedef struct zb_nlme_network_descriptor_s
{
  zb_ext_pan_id_t extended_pan_id; /*!< Extended PAN ID
                                  * of the network */
  zb_uint8_t    logical_channel; /*!< The current logical channel ocuppied by
                                  * the network */
  /* use bitfields to fit descriptors array to the single buffer */
  zb_bitfield_t stack_profile:4; /*!< Stack profile identifier  */
  zb_bitfield_t zigbee_version:4; /*!< The version of the ZigBee protocol */
  zb_bitfield_t beacon_order:4; /*!< How often the MAC sub-layer beacon is to
                                 * be transmitted */
  zb_bitfield_t superframe_order:4; /*!< The length of the active period of the
                                     * superframe  */
  zb_bitfield_t permit_joining:1; /*!< Indecates that at least one router on
                                   * the network currently permits joining */
  zb_bitfield_t router_capacity:1; /*!< True if device is capable of accepting
                                    * join requests from roter-capable devices */
  zb_bitfield_t end_device_capacity:1; /*!< True if device is capable of accepting
                                    * join requests from end devices */
  zb_bitfield_t reserved:5;
} ZB_PACKED_STRUCT
zb_nlme_network_descriptor_t;


/**
   Arguments of the NLME-NETWORK-DISCOVERY.confirm routine.
*/
typedef struct zb_nlme_network_discovery_confirm_s
{
  zb_mac_status_t status; /*!< MAC status codes */
  zb_uint8_t network_count; /*!< Number of discovered networks */
  /* next here is an array of zb_nlme_network_descriptor_t */
} ZB_PACKED_STRUCT
zb_nlme_network_discovery_confirm_t;


/**
   NLME-NETWORK-DISCOVERY.confirm primitive

   Report the results of the network discovery operation.

   @param v_buf - buffer containing results - @see
   zb_nlme_network_discovery_confirm_t. Note: this structure passed as data, not
   as parameter! Rationale: it has variable size, so it is not easy to define
   where it should begin in parameter.
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_network_discovery_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-NETWORK-FORMATION.request routine.
*/
typedef struct zb_nlme_network_formation_request_s
{
  zb_uint32_t scan_channels; /*!< Bit mask indicates channels to scan */
  zb_uint8_t scan_duration; /*!< Time to spend scanning each channel */
#if 0                       /* not supported by mac anyway */
  zb_uint8_t beacon_order; /*!< The beacon order */
  zb_uint8_t superframe_order; /*!< The superframe order */
  zb_uint8_t battery_life_extension; /*!< If true - start support battery
                                           * life extension */
#endif
} ZB_PACKED_STRUCT
zb_nlme_network_formation_request_t;


/**
   NLME-NETWORK-FORMATION.request primitive

   Starting new ZigBee network with itself as a coordinator.

   @param v_buf - buffer containing parameters - @see
   zb_nlme_network_formation_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_network_formation_request(zb_uint8_t param) ZB_CALLBACK;



/**
   Arguments of the NLME-NETWORK-FORMATION.confirm routine.
*/
typedef struct zb_nlme_network_formation_confirm_s
{
  zb_nwk_status_t status; /*!< MAC status codes */
} ZB_PACKED_STRUCT
zb_nlme_network_formation_confirm_t;


/**
   NLME-NETWORK-FORMATION.confirm primitive

   Report the results of the network formation request.

   @param v_buf - buffer containing results - @see
   zb_nlme_network_formation_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_network_formation_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-PERMIT_JOINING.request routine.
*/
typedef struct zb_nlme_permit_joining_request_s
{
  zb_uint8_t permit_duration; /*!< Time in seconds during which the coordinator
                               * or router will allow associations */
} ZB_PACKED_STRUCT
zb_nlme_permit_joining_request_t;


/**
   NLME-PERMIT-JOINING.request primitive

   Allow/disallow network joining

   @param v_buf - buffer containing parameters - @see
   zb_nlme_network_formation_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_permit_joining_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-NETWORK-FORMATION.confirm routine.
*/
typedef struct zb_nlme_permit_joining_confirm_s
{
  zb_mac_status_t status; /*!< MAC status codes */
} ZB_PACKED_STRUCT
zb_nlme_permit_joining_confirm_t;


/**
   NLME-PERMIT-JOINING.confirm primitive

   Report the results of the permit joining request.

   @param v_buf - buffer containing results - @see
   zb_nlme_permit_joining_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_permit_joining_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-START-ROUTER.request routine.
*/
typedef struct zb_nlme_start_router_request_s
{
  zb_uint8_t beacon_order; /*!< The beacon order */
  zb_uint8_t superframe_order; /*!< The superframe order */
  zb_uint8_t battery_life_extension; /*!< If true - start support battery
                                           * life extension */
#ifdef SDCC
  zb_uint8_t align_to_4;
#endif
} ZB_PACKED_STRUCT
zb_nlme_start_router_request_t;


/**
   NLME-START-ROUTER.request primitive

   Start router activity

   @param v_buf - buffer containing parameters - @see
   zb_nlme_start_router_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_start_router_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-START-ROUTER.confirm routine.
*/
typedef struct zb_nlme_start_router_confirm_s
{
  zb_mac_status_t status; /*!< MAC status codes */
} ZB_PACKED_STRUCT
zb_nlme_start_router_confirm_t;


/**
   NLME-START-ROUTER.confirm primitive

   Report the results of the start router request.

   @param v_buf - buffer containing results - @see
   zb_nlme_start_router_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_start_router_confirm(zb_uint8_t param) ZB_CALLBACK;



/**
   Arguments of the NLME-ED-SCAN.request routine.
*/
typedef struct zb_nlme_ed_scan_request_s
{
  zb_uint32_t scan_channels; /*!< Bit mask indicates channels to scan */
  zb_uint8_t scan_duration; /*!< Time to spend scanning each channel */
} ZB_PACKED_STRUCT
zb_nlme_ed_scan_request_t;


/**
   NLME-ED_SCAN.request primitive

   Start energy scan

   @param v_buf - buffer containing parameters - @see
   zb_nlme_ed_scan_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_ed_scan(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-ED-SCAN.confirm routine.
*/
typedef struct zb_nlme_ed_scan_confirm_s
{
  zb_mac_status_t status; /*!< MAC status codes */
  zb_uint32_t unscanned_channels; /*!< Indicate not scanned channels */
  /* next is the list of zb_uint8_t describes energy measurements */
} ZB_PACKED_STRUCT
zb_nlme_ed_scan_confirm_t;



/**
   Network join method.
*/
typedef enum zb_nlme_rejoin_method_e
{
  ZB_NLME_REJOIN_METHOD_ASSOCIATION    = 0x00, /*!< Throught association */
  ZB_NLME_REJOIN_METHOD_DIRECT         = 0x01, /*!< Join directly or rejoining
                                                * using the orphaning */
  ZB_NLME_REJOIN_METHOD_REJOIN         = 0x02, /*!< Using NWK rejoing
                                                * procedure */
  ZB_NLME_REJOIN_METHOD_CHANGE_CHANNEL = 0x03  /*!< Changing the network
                                                * channel  */
} ZB_PACKED_STRUCT
zb_nlme_rejoin_method_t;



/**
   NLME-ED-SCAN.confirm primitive

   Report the results of the ed scan request.

   @param v_buf - buffer containing results - @see
   zb_nlme_ed_scan_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_ed_scan_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   Arguments of the NLME-JOIN.request routine.
*/
typedef struct zb_nlme_join_request_s
{
  zb_ext_pan_id_t extended_pan_id; /*!< Extended PAN ID
                                                        * of the network */
  zb_uint32_t scan_channels; /*!< Bit mask indicates channels to scan */
  zb_mac_capability_info_t capability_information; /*!< The operating capabilities of the
                                      * device */
  zb_nlme_rejoin_method_t rejoin_network; /*!< Join network method @see zb_nlme_rejoin_method_t */
  zb_uint8_t scan_duration; /*!< Time to spend scanning each channel */
  zb_uint8_t security_enabled; /*!< Use security rejoing */
} ZB_PACKED_STRUCT
zb_nlme_join_request_t;


/**
   NLME-JOIN.request primitive

   Join to the network

   @param v_buf - buffer containing parameters - @see
   zb_nlme_join_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_join_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-JOIN.indication routine.
*/
typedef struct zb_nlme_join_indication_s
{
  zb_uint16_t network_address; /*!< The network address of an entity */
  zb_ieee_addr_t extended_address; /*!< 64 bit IEEE address of an entity */
  zb_mac_capability_info_t capability_information; /*!< The operating capabilities of the
                                                    * device */
  zb_nlme_rejoin_method_t rejoin_network; /*!< Join network method @see
                                           * zb_nlme_rejoin_method_t */
  zb_uint8_t secure_rejoin; /*!< Secure joining */
} ZB_PACKED_STRUCT
zb_nlme_join_indication_t;


/**
   NLME-JOIN.indication primitive

   Notify about new joined device.

   @param v_buf - buffer containing parameters - @see
   zb_nlme_join_indication_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_join_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-JOIN.confirm routine.
*/
typedef struct zb_nlme_join_confirm_s
{
  zb_mac_status_t status; /*!< MAC status codes */
  zb_uint16_t network_address; /*!< Allocated network address */
  zb_ext_pan_id_t extended_pan_id; /*!< Extended PAN ID
                                                        * of the network */
  zb_uint8_t active_channel; /*!< Current network channel */
} ZB_PACKED_STRUCT
zb_nlme_join_confirm_t;


/**
   NLME-JOIN.confirm primitive

   Report the results of the join request.

   @param v_buf - buffer containing results - @see
   zb_nlme_join_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_join_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-DIRECT-JOIN.request routine.
*/
typedef struct zb_nlme_direct_join_request_s
{
  zb_ieee_addr_t device_address; /*!< 64 bit IEEE address of the device to be directly joined */
  zb_mac_capability_info_t capability_information; /*!< The operating capabilities of the
                                      * device */
} ZB_PACKED_STRUCT
zb_nlme_direct_join_request_t;


/**
   NLME-DIRECT-JOIN.request primitive

   Directly Join anoter device to the network

   @param v_buf - buffer containing parameters - @see
   zb_nlme_direct_join_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_direct_join_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-DIRECT-JOIN.confirm routine.
*/
typedef struct zb_nlme_direct_join_confirm_s
{
  zb_nwk_status_t status; /*!< MAC status codes */
  zb_ieee_addr_t device_address; /*!< 64 bit IEEE address */
} ZB_PACKED_STRUCT
zb_nlme_direct_join_confirm_t;


/**
   NLME-DIRECT-JOIN.confirm primitive

   Report the results of the direct join request.

   @param v_buf - buffer containing results - @see
   zb_nlme_direct_join_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_direct_join_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   NWK Command Payload Field, see 3.4.4.3
*/
typedef struct zb_nwk_leave_payload_s
{
  zb_bitfield_t remove_children:1; /* Remove children */
  zb_bitfield_t request:1;         /* Request */
  zb_bitfield_t rejoin:1;          /* Rejoin */
  zb_bitfield_t reserved:5;        /* Reserved */
} ZB_PACKED_STRUCT
zb_nwk_leave_payload_priv_t;

#define ZB_LEAVE_PL_SET_REQUEST(pl)  (pl) |= (1 << 6)
#define ZB_LEAVE_PL_SET_REJOIN(pl, v) (pl) |= ((!!(v)) << 5)
#define ZB_LEAVE_PL_SET_REMOVE_CHILDREN(pl, v)  (pl) |= ((!!(v)) << 7)

#define ZB_LEAVE_PL_GET_REQUEST(pl)  (((pl) >> 6) & 1)
#define ZB_LEAVE_PL_GET_REJOIN(pl)   (((pl) >> 5) & 1)
#define ZB_LEAVE_PL_GET_REMOVE_CHILDREN(pl)  (((pl) >> 7) & 1)

/**
   Arguments of the NLME-LEAVE.request routine.
*/
typedef struct zb_nlme_leave_request_s
{
  zb_ieee_addr_t device_address; /*!< 64 bit IEEE address of the device to
                                  * remove, zero fill if device itself */
  zb_uint8_t remove_children; /*!< If true - remove child devices from the
                                    * network */
  zb_uint8_t rejoin; /*!< If true - Join after leave */
} ZB_PACKED_STRUCT
zb_nlme_leave_request_t;


/**
   NLME-LEAVE.request primitive

   Leave the network

   @param v_buf - buffer containing parameters - @see
   zb_nlme_leave_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_leave_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-LEAVE.indication routine.
*/
typedef struct zb_nlme_leave_indication_s
{
  zb_ieee_addr_t device_address; /*!< 64 bit IEEE address of the device to romove, zero fill if device
                                  * itself */
  zb_uint8_t rejoin; /*!< Join after leave */
} ZB_PACKED_STRUCT
zb_nlme_leave_indication_t;


/**
   NLME-LEAVE.indication primitive

   Notify about leave device

   @param v_buf - buffer containing parameters - @see
   zb_nlme_leave_indication_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_leave_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-LEAVE.confirm routine.
*/
typedef struct zb_nlme_leave_confirm_s
{
  zb_nwk_status_t status; /*!< MAC status codes */
  zb_ieee_addr_t device_address; /*!< 64 bit IEEE address */
} ZB_PACKED_STRUCT
zb_nlme_leave_confirm_t;


/**
   NLME-LEAVE.confirm primitive

   Report the results of the direct join request.

   @param v_buf - buffer containing results - @see
   zb_nlme_leave_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_leave_confirm(zb_uint8_t param) ZB_CALLBACK;


void zb_nwk_do_leave(zb_uint8_t param, zb_uint8_t rejoin) ZB_SDCC_REENTRANT;


void zb_nwk_forget_device(zb_address_ieee_ref_t addr_ref);

/**
   Arguments of the NLME-RESET.request routine.
*/
typedef struct zb_nlme_reset_request_s
{
  zb_uint8_t warm_start; /*!< if false - reset all stack values */
} ZB_PACKED_STRUCT
zb_nlme_reset_request_t;


/**
   NLME-RESET.request primitive

   Perform reset operation

   @param v_buf - buffer containing parameters - @see
   zb_nlme_reset_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_reset_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-RESET.confirm routine.
*/
typedef struct zb_nlme_reset_confirm_s
{
  zb_nwk_status_t status; /*!< The result of the operation */
} ZB_PACKED_STRUCT
zb_nlme_reset_confirm_t;


/**
   NLME-RESET.confirm primitive

   Report the results of the reset request.

   @param v_buf - buffer containing results - @see
   zb_nlme_reset_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_reset_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-SYNC.request routine.
*/
typedef struct zb_nlme_sync_request_s
{
  zb_uint8_t track; /*!< Whether ot not the sync should be maintained for
                     * future beacons */
} ZB_PACKED_STRUCT
zb_nlme_sync_request_t;


/**
   NLME-SYNC.request primitive

   Perform sync operation

   @param v_buf - buffer containing parameters - @see
   zb_nlme_sync_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_sync_request(zb_uint8_t param) ZB_CALLBACK;


/**
   NLME-SYNC-LOSS.indication primitive

   Notify the loss of synchronisation

   @param v_buf - empty
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_sync_loss_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-SYNC.confirm routine.
*/
typedef struct zb_nlme_sync_confirm_s
{
  zb_nwk_status_t status; /*!< The result of the operation */
} ZB_PACKED_STRUCT
zb_nlme_sync_confirm_t;


/**
   NLME-SYNC.confirm primitive

   Report the results of the sync request.

   @param v_buf - buffer containing results - @see
   zb_nlme_sync_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_sync_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   NLME-STATUS.indication primitive

   Notifies about network failes.

   @param v_buf - buffer containing parameters - @see
   zb_nlme_status_indication_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_status_indication(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-ROUTE-DISCOVERY.request routine.
*/
typedef struct zb_nlme_route_discovery_request_s
{
  zb_addr_mode_t address_mode;  /*!< Kind of destination address provided */
  zb_uint16_t network_addr;     /*!< The destination of the route discovery */
  zb_uint8_t radius;            /*!< Number of hopes */
  zb_bitfield_t no_route_cache; /*!< True - no route table should be
                                 * established */
  zb_bitfield_t reserved:7;
} ZB_PACKED_STRUCT
zb_nlme_route_discovery_request_t;


/**
   NLME-ROUTE-DISCOVERY.request primitive

   Initiate a route discovery

   @param v_buf - buffer containing parameters - @see
   zb_nlme_route_discovery_request_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_route_discovery_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Arguments of the NLME-ROUTE-DISCOVERY.confirm routine.
*/
typedef struct zb_nlme_route_discovery_confirm_s
{
  zb_uint8_t status;            /*!< @see zb_nwk_status_t indeed  */
} ZB_PACKED_STRUCT
zb_nlme_route_discovery_confirm_t;


/**
   NLME-ROUTE-DISCOVERY.confirm primitive

   Report the results of the route discovery

   @param v_buf - buffer containing results - @see
   zb_nlme_route_discovery_confirm_t
   @return RET_OK on success, error code otherwise.
 */
void zb_nlme_route_discovery_confirm(zb_uint8_t param) ZB_CALLBACK;


/**
   Update beacon payload in the PIB.

   To be called after any network configuration change: formation, join etc.
   As a side effect increments NIB Update id.
 */
void zb_nwk_update_beacon_payload();

void zb_mlme_associate_indication(zb_uint8_t param) ZB_CALLBACK;
void zb_mlme_associate_confirm(zb_uint8_t param) ZB_CALLBACK;
void zb_mlme_comm_status_indication(zb_uint8_t param) ZB_CALLBACK;
void zb_mlme_orphan_indication(zb_uint8_t param) ZB_CALLBACK;
void zb_nlme_ed_scan_request(zb_uint8_t param) ZB_CALLBACK;


/**
   3.4.9 Network Report Command
 */
typedef struct zb_nwk_report_cmd_s
{
  zb_uint8_t command_options;
  zb_ext_pan_id_t epid;
  zb_uint16_t panids[1];
} ZB_PACKED_STRUCT zb_nwk_report_cmd_t;

#define ZB_NWK_REPORT_INFO_COUNT(options) ((options) & 0xf)
#define ZB_NWK_REPORT_COMMAND_ID(options) (((options >> 4)) & 0xf)
#define ZB_NWK_REPORT_IS_PANID_CONFLICT(options) (ZB_NWK_REPORT_COMMAND_ID(options) == 0)


/**
   3.4.10 Network Update Comman
 */
typedef struct zb_nwk_update_cmd_s
{
  zb_uint8_t command_options;
  zb_ext_pan_id_t epid;
  zb_uint8_t update_id;
  zb_uint16_t new_panid;
} ZB_PACKED_STRUCT zb_nwk_update_cmd_t;

/*! @} */
/*! \endcond */



#endif /* ZB_NWK_H */
