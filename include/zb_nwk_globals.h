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
PURPOSE: NWK subsystem globals
*/

#ifndef ZB_NWK_GLOBALS_H
#define ZB_NWK_GLOBALS_H 1

#include "zb_mac.h"
#include "zb_nwk_neighbor.h"
#include "zb_nwk.h"

/*! \cond internals_doc */
/*! \addtogroup ZB_NWK */
/*! @{ */

/**
   Nwk state
*/
typedef enum zb_nlme_state_e
{
  ZB_NLME_STATE_IDLE,
  ZB_NLME_STATE_DISC,
  ZB_NLME_STATE_FORMATION_ED_SCAN,
  ZB_NLME_STATE_FORMATION_ACTIVE_SCAN,
  ZB_NLME_STATE_FORMATION,
  ZB_NLME_STATE_ROUTER,
  ZB_NLME_STATE_ED_SCAN,
  ZB_NLME_STATE_REJOIN,
  ZB_NLME_STATE_ORPHAN_SCAN,
  ZB_NLME_STATE_RESET,
  ZB_NLME_STATE_PANID_CONFLICT_RESOLUTION
} ZB_PACKED_STRUCT
zb_nlme_state_t;

/**
   Network device type
*/
typedef enum zb_nwk_device_type_e
{
  ZB_NWK_DEVICE_TYPE_ED,
  ZB_NWK_DEVICE_TYPE_ROUTER,
  ZB_NWK_DEVICE_TYPE_COORDINATOR,
  ZB_NWK_DEVICE_TYPE_NONE
} ZB_PACKED_STRUCT
zb_nwk_device_type_t;

/* Broadcast transaction record */
typedef struct zb_nwk_btr_s
{
  zb_bitfield_t used:1;
  zb_bitfield_t expiration_time:7;
  zb_uint16_t   source_addr;
  zb_uint8_t    sequence_number;
} ZB_PACKED_STRUCT
zb_nwk_btr_t;

/* Wait for buffer allocation reasons */
typedef enum zb_nwk_wait_reason_e
{
  ZB_NWK_WAIT_REASON_IND_AND_RETRANSMIT, /* waitong for a new buffer to call
                                          * indicate data and retransmit it */
  ZB_NWK_WAIT_NUM
} ZB_PACKED_STRUCT
zb_nwk_wait_reason_t;

/* List of buffers waiting to be duplicated  */
typedef struct zb_nwk_buf_alloc_wait_s
{
  zb_bitfield_t used:1;
  zb_bitfield_t wait:7; /* reason to be dup @see zb_nwk_wait_reason_t */
  zb_uint8_t    buf;    /* buffer to be duplicated */
}
zb_nwk_buf_alloc_wait_t;

/* Passive ack bit array size */
#define ZB_NWK_BRCST_PASSIVE_ACK_ARRAY_SIZE (ZB_NEIGHBOR_TABLE_SIZE / 8 + (ZB_NEIGHBOR_TABLE_SIZE % 8) ? 1 : 0)
/* Broadcast retransmition info */
typedef struct zb_nwk_broadcast_retransmit_s
{
  zb_bitfield_t used:1; /*!< 1 if entry is used, 0 - otherwise */
  zb_bitfield_t wait_conf:1; /*!< Send  */
  zb_bitfield_t retries:6; /*!< Number of send retries */
  zb_uint8_t buf; /* buffer to be sent broadcast */
  zb_ushort_t neighbor_table_iterator; /*!< Next child to unicat broadcast
                                        * frame to */
  zb_time_t next_retransmit; /* next time when broadcast should be retransmitted
                              * */
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
  zb_uint8_t seq_num;
  zb_uint8_t passive_ack[ZB_NWK_BRCST_PASSIVE_ACK_ARRAY_SIZE]; /* passive ack
                                                                * bit array */
}
zb_nwk_broadcast_retransmit_t;


typedef struct zb_leave_pending_list_s
{
  zb_uint8_t             used;
  zb_uint8_t             buf_ref;
  zb_uint8_t             tsn;
  zb_uint16_t            src_addr;
} ZB_PACKED_STRUCT zb_leave_pending_list_t;


typedef struct zb_leave_ind_prnt_s
{
  zb_address_ieee_ref_t addr_ref;
  zb_uint8_t rejoin;
} zb_leave_ind_prnt_t;

/**
  leave context
*/
typedef struct zb_leave_context_s
{
  zb_leave_pending_list_t pending_list[ZB_ZDO_PENDING_LEAVE_SIZE];
  zb_uint8_t pending_list_size;
  zb_uint8_t rejoin_after_leave;
  zb_uint8_t leave_after_mgmt_leave_rsp_conf;
  zb_leave_ind_prnt_t leave_ind_prnt;
} zb_leave_context_t;


/**
   Place to store values between multiply operations
*/
typedef struct zb_nwk_handle_s
{
  zb_nlme_state_t state; /*!< Current network subsystem state */

  zb_address_ieee_ref_t parent; /*!< parent address (valid if we ar not ZC and joined)  */

  union saved_req_u
  {
    zb_nlme_network_formation_request_t formation;
    zb_nlme_join_request_t join;
  } saved_req;

  union tmp_u
  {
    struct join_tmp_s
    {
      zb_ext_neighbor_tbl_ent_t *parent;
    } join;
    struct rejoin_tmp_s
    {
      zb_ext_pan_id_t extended_pan_id;
      zb_mac_capability_info_t capability_information;
      zb_ext_neighbor_tbl_ent_t *parent;
    } rejoin;
    struct formation_s
    {
      zb_uint8_t energy_detect[ ZB_MAC_SUPPORTED_CHANNELS ]; /*!< Channel energy scan
                                                              * result, used during formation */
      zb_uint8_t channel_pan_count[ZB_MAC_SUPPORTED_CHANNELS];
      zb_uint8_t panid_handled_bm[ZB_PANID_TABLE_SIZE / 8 + 1];
    } formation;
  } tmp;

  zb_uint8_t  status_ind; /* Used to report NWK status indication */
  zb_uint16_t status_addr; /* Used to report NWK status indication */
  zb_uint16_t status_ind_addr; /* Used to report NWK status indication */

  zb_uint8_t permit_join; /*!< True if permit join is in progress */
  zb_uint8_t joined;      /*!< Non-zero if we are joined into the network */

  zb_uint8_t router_started; /*!< True if we are a router and we are
                              * started */
  zb_uint8_t is_tc;             /*!< true if we are Trust Center  */

  /* FIXME: next values are unnecessary for ED with rx_on_when_idle == 0 */
  zb_nwk_btr_t btt[ZB_NWK_BTR_TABLE_SIZE]; /*!< Broadcast transaction
                                            * table see 3.6.5 */
  zb_uint8_t btt_cnt;
  /* end FIXME: */

#ifdef ZB_ROUTER_ROLE
  zb_nwk_broadcast_retransmit_t brrt[ZB_NWK_BRR_TABLE_SIZE]; /* Broadcast
                                                              * retransmition
                                                              table */
  zb_uint8_t brrt_cnt;
#endif

  zb_nwk_buf_alloc_wait_t wait_alloc[ZB_NWK_WAIT_ALLOC_TABLE_SIZE]; /* list of
                                                                     * buffers
                                                                     * waitng to
                                                                     * be
                                                                     * duplicated */
  zb_uint8_t wait_alloc_cnt;

  zb_uint16_t rejoin_req_table[ZB_NWK_REJOIN_REQUEST_TABLE_SIZE];
  zb_uint8_t  rejoin_req_table_cnt;


  /* For PANID conflict resolution */
  zb_uint16_t known_panids[ZB_PANID_TABLE_SIZE];
  zb_uint16_t new_panid;
  zb_uint8_t  panid_conflict;
  /* For indicating we're joined to PRO as end device */
  zb_uint8_t joined_pro;
} ZB_PACKED_STRUCT zb_nwk_handle_t;


/**
   NWK Neighbor relationship between neighbors
*/
enum zb_nwk_relationship_e
{
  ZB_NWK_RELATIONSHIP_PARENT                = 0x00,
  ZB_NWK_RELATIONSHIP_CHILD                 = 0x01,
  ZB_NWK_RELATIONSHIP_SIBLING               = 0x02,
  ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE     = 0x03,
  ZB_NWK_RELATIONSHIP_PREVIOUS_CHILD        = 0x04,
  ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD = 0x05
};


/**
   NWK route state
*/
typedef enum zb_nwk_route_state_e
{
  ZB_NWK_ROUTE_STATE_ACTIVE,
  ZB_NWK_ROUTE_STATE_DISCOVERY_UNDERWAY,
  ZB_NWK_ROUTE_STATE_DISCOVERY_FAILED,
  ZB_NWK_ROUTE_STATE_DISCOVERY_INACTIVE,
  ZB_NWK_ROUTE_STATE_VALIDATION_UNDERWAY,
  ZB_NWK_ROUTE_STATE_NUM
} zb_nwk_route_state_t;


/**
   NWK routing
*/
typedef struct zb_nwk_routing_s
{
  zb_bitfield_t used:1; /*!< 1 if entry is used, 0 - otherwise */
  zb_bitfield_t status:3; /*!< The status of the route, @see zb_nwk_route_state_t */
  zb_bitfield_t no_route_cache:1; /*!< Dest does not store source routes */
  zb_bitfield_t many_to_one:1; /*!< Dest is the concentrator and many-to-one
                                * request was used  */
  zb_bitfield_t route_record_required:1; /*!< Route record command frame should
                                          * be sent to the dest prior to the
                                          * next data packet */
  zb_bitfield_t group_id_flag:1; /*!< Indicates that dest_addr is a Group ID */
  zb_uint16_t dest_addr; /*!< 16-bit network address or Group ID of this route */
  zb_uint16_t next_hop_addr; /*!< The 16 bit network address of the next
                                 * hop on the way to the destination */
} ZB_PACKED_STRUCT
zb_nwk_routing_t;

/**
   NWK route discovery
*/
typedef struct zb_nwk_route_discovery_s
{
  zb_bitfield_t used:1; /*!< 1 if entry is used, 0 - otherwise   */
  zb_uint8_t request_id; /*!< Sequence number for a route request */
  zb_uint16_t source_addr; /*!< 16-bit network address of the route
                            * requests initiator */
  zb_uint16_t sender_addr; /*!< 16-bit network address of the device that
                            * has sent the most recent lowest cost route
                            * request */
  zb_uint16_t dest_addr; /*!< 16-bit network destination address of this
                          * request */
  zb_uint8_t forward_cost; /*!< Path cost from the source of the route request
                            * to the current device */
  zb_uint8_t residual_cost; /*!< Path cost from the current to the destination
                             * device */
  zb_uint16_t expiration_time; /*!< Countdown timer indicating when route
                                * discovery expires */
} ZB_PACKED_STRUCT
zb_nwk_route_discovery_t;

/**
   NWK pending list element
*/
typedef struct zb_nwk_pend_s
{
  zb_bitfield_t used:1; /*!< 1 if entry is used, 0 - otherwise */
  zb_buf_t *buf; /*!< buffer waiting for route discovery */
  zb_uint8_t handle; /*!< nsdu handle */
  zb_uint16_t dest_addr; /*!< 16-bit network destination address of this
                          * request */
  zb_bitfield_t expiry:6; /*!< expiration time */
  zb_bitfield_t waiting_buf:1; /*!< if pending buffer waits new buffer to
                                 * start route discovery */

} ZB_PACKED_STRUCT
zb_nwk_pend_t;

/**
   NWK address allocation method.
*/
typedef enum zb_nwk_address_alloc_method_e
{
  ZB_NWK_ADDRESS_ALLOC_METHOD_DISTRIBUTED,
  ZB_NWK_ADDRESS_ALLOC_METHOD_RESERVER,
  ZB_NWK_ADDRESS_ALLOC_METHOD_STOCHASTIC
} ZB_PACKED_STRUCT
zb_nwk_address_alloc_method_t;

#if 0
/**
   NWK route request entry
*/
typedef struct zb_nwk_route_request_s
{
  zb_bitfield_t used:1; /*!< 1 if entry is used, 0 - otherwise */
  zb_uint8_t retries; /*!< Number of retries for the route request */
  zb_uint8_t radius; /*!< Radius for the route request */
  zb_uint16_t originator; /*!< Originator of the route request */
  zb_nwk_cmd_rreq_t cmd; /*!< Command data for the route request */
} ZB_PACKED_STRUCT
zb_nwk_rreq_t;
#endif


#ifdef ZB_SECURITY
/**
   Part of the secured material stored in array.

   Other secured material components stored elsewhere:
   OutgoingFrameCounter - NIB
   IncomingFrameCounterSet - neighbor table
 */
typedef struct zb_secur_material_set_t
{
  zb_uint8_t          key[ZB_CCM_KEY_SIZE];
  zb_uint8_t          key_seq_number;
} zb_secur_material_set_t;
#endif

/**
   This is NWK NIB residental in memory.
   It is not clear yet when it will be save to nvram and when read.
*/
typedef struct zb_nib_s
{
  zb_uint8_t     sequence_number;   /*!< A sequence number used to identify
                                     * outgoing frames */
  zb_uint16_t    passive_ack_timeout; /*!< Maximum time duration allowed for
                                       * the parent and all child to retransmit
                                       * a broadcast message */

  zb_uint8_t     max_broadcast_retries; /*!< The maximum number of retries allowed after a
                                          broadcast transmission failure. */
  zb_ext_pan_id_t  extended_pan_id; /*!< The extended PAN identifier for the PAN
                                   * of which the device is a member */
  zb_uint16_t      short_pan_id; /*!< short PAN id  */

  zb_uint8_t     device_type;   /*!< Current device role, @see zb_nwk_device_type_t */
  zb_uint8_t     update_id;     /*!< nwkUpdateId - The value identifying a
                                  snapshot of the network
                                  settings with which this
                                  node is operating with. */
#if defined ZB_NWK_MESH_ROUTING && defined ZB_ROUTER_ROLE
//#if 1
  zb_nwk_routing_t routing_table[ZB_NWK_ROUTING_TABLE_SIZE]; /*!< Routing
                                                                    * table */
  zb_uint8_t routing_table_cnt; /*!< Routing table used elements */

  zb_nwk_route_discovery_t route_disc_table[ZB_NWK_ROUTE_DISCOVERY_TABLE_SIZE]; /*!< Route discovery table */
  zb_uint8_t route_disc_table_cnt; /*!< Discovery table used elements */

  zb_uint8_t rreq_id; /*!< ID, increments each new route discovery procedure */
  zb_nwk_pend_t pending_table[ZB_NWK_PENDING_TABLE_SIZE]; /*!< store pending
                                                           * buffers while
                                                           * route discovery is
                                                           * in progress */
  zb_uint8_t pending_table_cnt; /*!< number of used elements inside pending
                                 * buffer */
  zb_uint16_t aps_rreq_addr; /*!< APS layer call us to find path to this
                              * address */
#endif /* ZB_NWK_MESH_ROUTING && ZB_ROUTER_ROLE */

  zb_uint8_t     max_depth; /*!< The depth a device can have */
  zb_uint8_t     max_children; /*!< The number of children a device is allowed
                                * to have */
  zb_uint8_t     router_child_num; /*!< Number of child devices with router
                                    * capability */
  zb_uint8_t     ed_child_num;  /*!< Number of child ed devices */

#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
  zb_uint8_t     max_routers; /*!< The number of routers any one device is
                               * allowed to have as children. */
  zb_uint16_t    cskip; /*!< Cskip value - size of the address sub-block beeing
                         * distributed */
  zb_uint8_t     depth; /*!< current node depth */
#endif

  zb_bitfield_t use_tree_routing:1; /*!< if device is able to use tree routing */

  zb_bitfield_t addr_alloc:3; /*!< Address assign method @see
                               * zb_nwk_address_alloc_method_t */
  zb_bitfield_t reserved:5;

#ifdef ZB_SECURITY
  zb_bitfield_t           security_level:4; /*!< The security level for
                                            outgoing and incoming
                                            NWK frames; the
                                            allowable security level
                                            identifiers are presented
                                            in Table 4.38.
                                            For ZB 2007 (Standard security only)
                                            only values 0 and 5 are possible.
                                            Or, seems, only value 5 is possible?
                                            */
  /* all_fresh is always 0 for Standard security */
  zb_bitfield_t           secure_all_frames:1;

  zb_bitfield_t           active_secur_material_i:2; /*!< index in
                                                      * secur_material_set for
                                                      * keys with
                                                      * key_seq_number == active_key_seq_number  */

  zb_secur_material_set_t secur_material_set[ZB_SECUR_N_SECUR_MATERIAL]; /*!< Set of network security
                                                                  material descriptors
                                                                  capable of maintaining
                                                                  an active and alternate
                                                                  network key.  */
  zb_uint8_t              active_key_seq_number; /*!< The sequence number of
                                                   the active network key in
                                                   nwkSecurityMaterialSet.  */
  zb_uint32_t             outgoing_frame_counter; /*!< OutgoingFrameCounter
                                                   * stored here (not in the
                                                   * secured material).
                                                   * Rationale: will never use
                                                   * "old" key - why store more
                                                   * then 1 counter?
                                                   */
  zb_uint32_t             prev_outgoing_frame_counter;
#endif
  zb_uint16_t nwk_manager_addr; /*!< The address of the designated
                                 * network channel manager function. */

  zb_uint16_t nwk_tx_total; /*!< A count of unicast transmissions made
                             * by the NWK layer on this device */
  zb_uint16_t nwk_tx_fail; /*!< A count of failed transmissions */

  zb_bitfield_t nwk_report_constant_cost:1; /*!< If this is set to 0, the NWK
                                             * layer shall calculate link cost
                                             * from all neighbor nodes using the
                                             * LQI values reported by the MAC
                                             * layer; otherwise, it shall report
                                             * a constant value. */
} zb_nib_t;

/* NWK Broadcast delivery time See 3.5.2.1 */
#if 0
/* according to this formula we get to large value for
 * BROADCAST_DELIVERY_TIME - 192 sec, lets decrease it down to 30 */
#define ZB_NWK_BROADCAST_DELIVERY_TIME() ( 2*ZG->nwk.nib.max_depth*(/*0.05 +*/ ZB_NWK_MAX_BROADCAST_JITTER/2 \
                                                                    + ZG->nwk.nib.passive_ack_timeout*ZB_MWK_INITIAL_RREQ_RETRIES/1000) )
#endif
#define ZB_NWK_BROADCAST_DELIVERY_TIME() 9

/**
   NWK global context
*/
typedef struct zb_nwk_globals_s
{
  zb_nib_t           nib;
  zb_nwk_handle_t    handle;
  zb_neighbor_tbl_t  neighbor;
  zb_leave_context_t leave_context; /* leave context */
} zb_nwk_globals_t;

zb_void_t zb_nwk_set_device_type(zb_nwk_device_type_t device_type);

#define ZB_NWK() ZG->nwk.nib

#define ZB_NWK_GET_RREQ_ID() (++ZB_NWK().rreq_id)

/*! @} */
/*! \endcond */

#endif /* ZB_NWK_GLOBALS_H */
