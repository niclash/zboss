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
PURPOSE: NWK layer internals
*/

#ifndef NWK_INTERNAL_H
#define NWK_INTERNAL_H 1

/*! \addtogroup ZB_NWK */
/*! @{ */

/* this parameter shows a acceptable energy level on channel */
#define ZB_NWK_CHANNEL_ACCEPT_LEVEL 60

#define CHECK_PARAM_RET_ON_ERROR(param)             \
do                                                  \
{                                                   \
  if ( !param )                                     \
  {                                                 \
    TRACE_MSG(TRACE_NWK1, "<< ret invalid param", (FMT__0));  \
    ZB_ASSERT(0);                                   \
    return;                                         \
  }                                                 \
} while (0)

#define NWK_FORMATION_FAILURE_CONFIRM(buf, s)                           \
do                                                                      \
{                                                                       \
  (buf)->u.hdr.status = (s);                                            \
  ZB_SCHEDULE_CALLBACK(zb_nlme_network_formation_confirm, ZB_REF_FROM_BUF((buf)));\
}                                                                       \
while(0)

/**
   Common confirm status routine
 */
#define NWK_CONFIRM_STATUS(buf, s, func)                              \
do                                                                    \
{                                                                     \
  (buf)->u.hdr.status = (s);                                          \
  ZB_SCHEDULE_CALLBACK((func), ZB_REF_FROM_BUF((buf)));        \
}                                                                     \
while(0)

/**
   Common confirm status routine
 */
#define NWK_ROUTE_DISCOVERY_CONFIRM(buf, s, ns)                         \
do                                                                      \
{                                                                       \
  zb_nlme_route_discovery_confirm_t *ptr;                               \
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_route_discovery_confirm_t), ptr); \
  ZB_ASSERT(ptr);                                                       \
  (buf)->u.hdr.status = (zb_ret_t)(s);                                  \
  ptr->status = ns;                                                     \
  ZB_SCHEDULE_CALLBACK((zb_nlme_route_discovery_confirm), ZB_REF_FROM_BUF((buf))); \
}                                                                       \
while(0)

/* Return pointer to free entry from array and increment used elements counter */
#define NWK_ARRAY_GET_ENT(array, ent, cnt) do                           \
{                                                                       \
  zb_ushort_t i;                                                        \
  for (i = 0, ent = NULL; i < sizeof(array) / sizeof(array[0]); i++)    \
  {                                                                     \
    if ( !array[i].used )                                               \
    {                                                                   \
      ent = &array[i];                                                  \
      cnt++;                                                            \
      (ent)->used = 1;                                                  \
      break;                                                            \
    }                                                                   \
  }                                                                     \
} while (0)

/* Mark array entry as free and decrements used element counter */
#define NWK_ARRAY_PUT_ENT(array, ent, cnt) do     \
{                                                 \
  if ( ent )                                      \
  {                                               \
    (ent)->used = 0;                              \
    (cnt)--;                                      \
  }                                               \
} while(0)

/* find used enttry with secified parametrs withing array */
#define NWK_ARRAY_FIND_ENT(array, ent, condition) do                    \
{                                                                       \
  zb_ushort_t i;                                                           \
  for (i = 0, ent = &(array)[0]; i < sizeof(array) / sizeof((array)[0]); i++, ent = &(array)[i]) \
  {                                                                     \
    if ( ent->used                                                      \
         && (condition) )                                               \
    {                                                                   \
      break;                                                            \
    }                                                                   \
  }                                                                     \
  if ( i >= sizeof(array) / sizeof(array[0]) )                          \
  {                                                                     \
    ent = NULL;                                                         \
  }                                                                     \
} while (0)


/* Free all array entries */
#define NWK_ARRAY_CLEAR(array, type, cnt) do                            \
{                                                                       \
  type *ent;                                                            \
  zb_ushort_t i;                                                           \
  for (i = 0, ent = &array[0]; i < sizeof(array) / sizeof(array[0]); i++, ent = &array[i]) \
  {                                                                     \
    ent->used = 0;                                                      \
  }                                                                     \
  cnt = 0;                                                              \
} while(0)

/* translate MAC status to NWK command status */
#define MAC_STATUS_2_NWK_COMMAND_STATUS(s) do                 \
{                                                             \
  switch ( (zb_uint8_t)s )                                    \
  {                                                           \
    case MAC_NO_ACK:                                          \
    case MAC_CHANNEL_ACCESS_FAILURE:                          \
      s = ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE;          \
      break;                                                  \
    case MAC_TRANSACTION_EXPIRED:                             \
      s = ZB_NWK_COMMAND_STATUS_INDIRECT_TRANSACTION_EXPIRY;  \
      break;                                                  \
    default:                                                  \
      break;                                                  \
  }                                                           \
} while (0)

/* Calculate path cost */
#define NWK_CALC_PATH_COST(src_addr, path_cost) do                      \
{                                                                       \
  if ( !ZB_NWK().nwk_report_constant_cost )                             \
  {                                                                     \
    zb_neighbor_tbl_ent_t *nbt;                                         \
    if ( zb_nwk_neighbor_get_by_short(src_addr, &nbt) == RET_OK )       \
    {                                                                   \
      zb_uint8_t i = 0;                                                 \
      while ( 32*(i+1) < nbt->lqi )                                     \
      {                                                                 \
        i++;                                                            \
      }                                                                 \
      path_cost = (7 - i) ? (7 - i) : 1;                                \
    }                                                                   \
    else if ( (src_addr) == ZB_NIB_NETWORK_ADDRESS() )                  \
    {                                                                   \
      path_cost = 0;                                                    \
    }                                                                   \
    else                                                                \
    {                                                                   \
      path_cost = ZB_NWK_STATIC_PATH_COST;                              \
    }                                                                   \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    path_cost = ZB_NWK_STATIC_PATH_COST;                                \
  }                                                                     \
} while (0)

/**
   Called from the discovery confirm with ED scan results.
   Initiate active scan or reports formation failure

   @param buf - buffer containing results - @see
   zb_mac_scan_confirm_t
   @return nothing
 */
void nwk_formation_ed_scan_confirm(zb_buf_t *buf) ZB_SDCC_REENTRANT;

/**
   Called from the discovery confirm with active scan results.
   Initiate new network start routine or reports formation failure.

   @param buf - buffer containing results - @see
   zb_mac_scan_confirm_t
   @return nothing
 */
void nwk_formation_select_channel(zb_buf_t *buf) ZB_SDCC_REENTRANT;


#ifdef ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN

/**
   zb_nwk_daa_calc_cskip

   See 3.6.1.6
   Calculate Cskip value based on the node depth. This value wiil be used to
   determine the address of any joined device and also determining whether to
   route up or down in tree based routing. This function called once after
   nwk_formation or nwk_join

   @param depth - depth of the node inside tree
   @return Cskip value
 */
zb_uint16_t zb_nwk_daa_calc_cskip(zb_uint8_t depth ) ZB_SDCC_REENTRANT;

/**
   Calculate address for child router. See 3.6.1.6
 */
#define ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN_ROUTER_ADDRESS() ( ZG->mac.pib.mac_short_address + ZG->nwk.nib.router_child_num*ZG->nwk.nib.cskip + 1 )

/**
   Calculate address for child ed. See 3.6.1.6
 */
#define ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN_ED_ADDRESS() ( ZG->mac.pib.mac_short_address + ZG->nwk.nib.max_routers*ZG->nwk.nib.cskip + ZG->nwk.nib.ed_child_num + 1 )

#else
  #error Implement Stochastic address assign mechanism
#endif /* ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN */


#ifndef MAC_CERT_TEST_HACKS

#define ZB_NWK_ROUTER_ADDRESS_ASSIGN ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN_ROUTER_ADDRESS
#define ZB_NWK_ED_ADDRESS_ASSIGN ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN_ED_ADDRESS

#else

#define ZB_NWK_ROUTER_ADDRESS_ASSIGN() ZB_PREDEFINED_ROUTER_ADDR
#define ZB_NWK_ED_ADDRESS_ASSIGN()     ZB_PREDEFINED_ED_ADDR


#endif


#ifdef ZB_NWK_TREE_ROUTING
/**
   zb_nwk_tree_routing_init

   Initialize tree route subsystem.

   @return nothing
 */
void zb_nwk_tree_routing_init() ZB_SDCC_REENTRANT;

/**
   zb_nwk_tree_routing_route

   Find next hop to destination

   @param dest_address - address of the destination device
   @return Neighbor table entry to route packet to, NULL on error
 */
zb_neighbor_tbl_ent_t *zb_nwk_tree_routing_route(zb_uint16_t dest_address) ZB_SDCC_REENTRANT;
#endif /* ZB_NWK_TREE_ROUTING */

#ifdef ZB_NWK_MESH_ROUTING
//#if 1
/**
   zb_nwk_mesh_routing_init

   Initialize mesh route subsystem.

   @return nothing
 */
void zb_nwk_mesh_routing_init() ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_routing_deinit

   Deinitialize mesh route subsystem.

   @return nothing
 */
void zb_nwk_mesh_routing_deinit() ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_route_discovery

   Sets up new route discoveru operation. This device will be route request
   originator.

   @param cbuf - output buffer
   @param dest_address - address of the destination device
   @param radius - route discovery radius, 0 - use default radius
   @return nothing
 */
void zb_nwk_mesh_route_discovery(zb_buf_t *cbuf, zb_uint16_t dest_addr, zb_uint8_t radius) ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_rreq_handler

   Process an incoming route request and decide if it needs to be forwarded
   or a route reply needs to be generated

   @param buf - buffer with incoming route request packet
   @param nwk_hdr - pointer to the network header
   @param nwk_cmd_rreq - pointer to the network route request header
   @return nothing
 */
void zb_nwk_mesh_rreq_handler(zb_buf_t *buf, zb_nwk_hdr_t *nwk_hdr, zb_nwk_cmd_rreq_t *nwk_cmd_rreq) ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_rrep_handler

   Process an incoming route reply

   @param buf - buffer with incoming route request packet
   @param nwk_hdr - pointer to the network header
   @param nwk_cmd_rrep - pointer to the network route reply header
   @return nothing
 */
void zb_nwk_mesh_rrep_handler(zb_buf_t *buf, zb_nwk_hdr_t *nwk_hdr, zb_nwk_cmd_rrep_t *nwk_cmd_rrep) ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_find_route

   Search for the route

   @param dest_addr - packet destination address
   @return routing table entry if route exists, NULL - otherwise
 */
zb_nwk_routing_t *zb_nwk_mesh_find_route(zb_uint16_t dest_addr) ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_find_route_discovery_entry

   Search for the route discovery entry

   @param dest_addr - packet destination address
   @return route discovery table entry if exists, NULL - otherwise
 */
zb_nwk_route_discovery_t *zb_nwk_mesh_find_route_discovery_entry(zb_uint16_t dest_addr) ZB_SDCC_REENTRANT;

/**
   zb_nwk_mesh_add_buf_to_pending

   Add buffer to the pending list, to send it after route will be found

   @param buf - packet buffer
   @param handle - nsdu packet handle
   @return RET_OK on success, error code otherwise
 */
zb_ret_t zb_nwk_mesh_add_buf_to_pending(zb_buf_t *buf, zb_uint8_t handle) ZB_SDCC_REENTRANT;
#endif /* ZB_NWK_MESH_ROUTING */

void zb_nwk_forward(zb_uint8_t param) ZB_CALLBACK;


/**
   zb_nwk_leave_handler

   @param param - buffer with scan result
   @return nothing
 */
void zb_nlme_rejoin_scan_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nlme_rejoin_response

   @param param - buffer
   @return nothing
 */
void zb_nlme_rejoin_response(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nlme_rejoin_response_timeout

   @param param - buffer
   @return nothing
 */
void zb_nlme_rejoin_response_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nlme_rejoin_request

   @param param - buffer
   @return nothing
 */
void zb_nlme_rejoin_request(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nlme_rejoin_resp_sent

   @param param - buffer
   @return nothing
 */
void zb_nlme_rejoin_resp_sent(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nlme_orphan_scan_confirm

   @param param - buffer
   @return nothing
 */
void zb_nlme_orphan_scan_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   zb_nwk_nib_init

   @return nothing
 */
void zb_nwk_nib_init();

/* This handles is used to transfer service nwk data. No confirm shuld be called
 * under NWK layer */
#define ZB_NWK_INTERNAL_NSDU_HANDLE         0xFF
#define ZB_NWK_INTERNAL_REJOIN_CMD_HANDLE   0xFE
#define ZB_NWK_INTERNAL_REJOIN_CMD_RESPONSE 0xFD
/*!< When got data.confirm, do actual leave, then call leave.indication and, maybe, rejoin  */
#define ZB_NWK_INTERNAL_LEAVE_IND_AT_DATA_CONFIRM_HANDLE    0xFC
/*!< When got data.confirm, send responce to mgmt_leave command. Maybe, leave later  */
#define ZB_NWK_INTERNAL_LEAVE_CONFIRM_AT_DATA_CONFIRM_HANDLE   0xFB

zb_nwk_hdr_t *nwk_alloc_hdr(zb_buf_t *packet, zb_uint8_t *fc);

void remove_parent_from_potential_parents(zb_ext_neighbor_tbl_ent_t *parent);


void zb_panid_conflict_got_network_report(zb_uint8_t param, zb_uint16_t *panids, zb_uint8_t n_panids);
void zb_panid_conflict_remember_panid(zb_uint16_t panid);
void zb_panid_conflict_network_update(zb_uint8_t param) ZB_SDCC_REENTRANT;
zb_uint8_t * zb_nwk_fill_out_command(zb_uint8_t param, zb_uint16_t dest, zb_uint8_t command_id, zb_uint8_t size) ZB_SDCC_REENTRANT;
void zb_panid_conflict_send_network_report(zb_uint8_t param) ZB_CALLBACK;
void zb_panid_conflict_send_nwk_update(zb_uint8_t param) ZB_CALLBACK;
void zb_panid_conflict_send_status_ind(zb_uint8_t param) ZB_CALLBACK;
void zb_panid_conflict_network_update_recv(zb_nwk_update_cmd_t *upd);
void zb_panid_conflict_set_panid_alarm(zb_uint8_t param) ZB_CALLBACK;
void zb_panid_conflict_set_panid(zb_uint8_t param) ZB_CALLBACK;
void zb_panid_conflict_schedule_network_report(zb_uint8_t param, zb_uint16_t panid);

/*
  Alloc and fill nwk hdr, return pointer to the allocated hdr
*/
zb_nwk_hdr_t *nwk_alloc_and_fill_hdr(zb_buf_t *buf,
                                     zb_uint16_t dst_addr,
                                     zb_uint8_t *src_ieee_addr, zb_uint8_t *dst_ieee_addr,
                                     zb_bool_t is_multicast, zb_bool_t is_secured,
                                     zb_bool_t is_cmd_frame) ZB_SDCC_REENTRANT;

/*
  Alloc and fill place for nwk command, return pointer to the command payload
*/
zb_uint8_t *nwk_alloc_and_fill_cmd(zb_buf_t *buf, zb_nwk_cmd_t cmd, zb_uint8_t cmd_size);




/*! @} */

#endif /* NWK_INTERNAL_H */
