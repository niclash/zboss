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
PURPOSE: Neighbor table (base and additional)
*/

#ifndef ZB_NWK_NEIGHBOR_H
#define ZB_NWK_NEIGHBOR_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_NWK */
/*! @{ */


/**
   Extended neighbor table entry: entry used at discovery time.
 */
typedef struct zb_ext_neighbor_tbl_ent_s
{
  zb_uint16_t               short_addr; /*!< 16 bit network address of the
                                         neighboring device */
  /* 2 */

  zb_ieee_addr_compressed_t long_addr; /*!< 64 bit address (packed) */
  /* 8 */
  zb_uint8_t                lqi;  /*!< Link quality */
  /* 9 */
  zb_uint8_t                update_id; /*!< This field reflects the value of nwkUpdateId from the NIB.  */
  /* 10 */

  /* additional fields - table 3.49 */
  zb_bitfield_t             logical_channel:5; /*!< The current logical channel
                                                occupied by the network.  */

  zb_bitfield_t             panid_ref:3; /*!< ref to the extended pan id  */
  /* 11 */

  zb_bitfield_t             beacon_order:4; /*!< This specifies how often the MAC
                                              sub-layer beacon is to be
                                              transmitted by a given device on the
                                              network.   */

  zb_bitfield_t             depth:4; /*!< The network depth of this
                                       device. A value of 0x00
                                       indicates that this device is th
                                       ZigBee coordinator for the
                                       network.  */
  /* 12 */
  /* fields for the Network Descriptor - table 3.8 */

  zb_bitfield_t             stack_profile:4; /*!< A ZigBee stack profile identifier.   */
  zb_bitfield_t             superframe_order:4; /*!< For beacon-oriented networks, that
                                                  is, beacon order < 15, this specifies
                                                  the length of the active period of the
                                                  superframe. */
  /* 13 */
  zb_bitfield_t             permit_joining:1; /*!< A value of TRUE indicates that at
                                             least one ZigBee router on the
                                             network currently permits joining,
                                             i.e. its NWK has been issued an
                                             NLME-PERMIT-JOINING
                                             primitive and, the time limit if
                                             given, has not yet expired.  */
  zb_bitfield_t             device_type:2; /*!< Neighbor device type - \see
                                            * zb_nwk_device_type_t */

  zb_bitfield_t             router_capacity:1; /*!< This value is set to TRUE if this
                                                 device is capable of accepting
                                                 join requests from router-
                                                 capable devices and is set to
                                                 FALSE otherwise.   */
  zb_bitfield_t             end_device_capacity:1; /*!< This value is set to TRUE if the
                                                     device is capable of accepting
                                                     join requests from end devices
                                                     seeking to join the network and
                                                     is set to FALSE otherwise.  */
  zb_bitfield_t             potential_parent:1; /*!< This field usage - see
                                                  3.6.1.4.1.1  Child Procedure:
                                                  If the Status parameter indicates a refusal to permit
                                                  joining on the part of the neighboring device (that is, PAN at capacity or PAN
                                                  access denied), then the device attempting to join should set the Potential parent
                                                  bit to 0 in the corresponding neighbor table entry to indicate a failed join attempt.
                                                */

  /* 29 bit */
  zb_bitfield_t             reserved:2;
  /* 14 */
  /* 16 bytes total */
} ZB_PACKED_STRUCT
zb_ext_neighbor_tbl_ent_t;


typedef struct zb_neighbor_tbl_ent_s
{
  zb_address_ieee_ref_t     addr_ref;
  /* 1 */
  zb_bitfield_t             permit_joining:2; /*!< copy from extended neighbor
                                               * (that is, from the beacon
                                               * frame), but not sure it is
                                               * always actual */
  zb_bitfield_t             device_type:2; /*!< Neighbor device type - \see
                                            * zb_nwk_device_type_t */

  zb_bitfield_t             depth:4; /*!< The network depth of this
                                       device. A value of 0x00
                                       indicates that this device is th
                                       ZigBee coordinator for the
                                       network.  */
  /* 2 */
  zb_bitfield_t             used:1;


  zb_bitfield_t             rx_on_when_idle:1; /*!< Indicates if neighboris receiver
                                                 enabled during idle periods:
                                                 TRUE = Receiver is on
                                                 FALSE = Receiver is off
                                                 This field should be present for
                                                 entries that record the parent or
                                                 children of a ZigBee router or
                                                 ZigBee coordinator.  */

  zb_bitfield_t             relationship:3; /*!< The relationship between the
                                              neighbor and the current device:
                                              0x00=neighbor is the parent
                                              0x01=neighbor is a child
                                              0x02=neighbor is a sibling
                                              0x03=none of the above
                                              0x04=previous child
                                              0x05=unauthenticated child
                                              This field shall be present in every
                                              neighbor table entry.
                                              \see zb_nwk_relationship_e
                                            */


  zb_bitfield_t             reserved:3;
  /* 3 */
  zb_uint8_t                transmit_failure;  /*!< Previous transmission result */
  /* 4 */
  zb_uint8_t                lqi;  /*!< Link quality */
  /* 5 */
#if 0                                       /* not used in ZB 2007 */
  /* Following fields present only if nwkSymLink = TRUE - this is PRO, not 2007 */
  zb_uint8_t                outgoing_cost;  /*!< The cost of an outgoing link */
  zb_uint8_t                age;  /*!< The number of nwkLinkStatusPeriod intervals since a
                                   * link status command was received */
#endif

#ifdef ZB_SECURITY
  zb_uint32_t               incoming_frame_counter; /*!< incoming frame counter
                                                     * for this device after
                                                     * key change */
  /* 9 */
  zb_uint8_t                key_seq_number; /*!< key number for which
                                             * incoming_frame_counter is valid  */
  /* 10 */
#endif
  /* 5 bytes total without security, 10 bytes with security */

} ZB_PACKED_STRUCT
zb_neighbor_tbl_ent_t;


typedef struct zb_neighbor_tbl_s

{
  /* base (run-time) neighbor table */
  zb_neighbor_tbl_ent_t     base_neighbor[ZB_NEIGHBOR_TABLE_SIZE];
  /* array for addressing neighbor table by network address ref */
  zb_uint8_t                addr_to_neighbor[ZB_IEEE_ADDR_TABLE_SIZE];
  /* current neighbor table size, taking into account possible table cut for
   * ext neighbor table */
  zb_ushort_t               base_neighbor_size;
  /* number of used neighbor table entries */
  zb_ushort_t               base_neighbor_used;
  /* ext neighbor table */
#ifdef ZB_ED_ROLE
  zb_ext_neighbor_tbl_ent_t ext_neighbor[ZB_EXT_NEIGHBOR_TABLE_SIZE];
#else
  zb_ext_neighbor_tbl_ent_t *ext_neighbor;
#endif
  /* ext neighbor table size */
  zb_ushort_t               ext_neighbor_size;
  /* number of used ext neighbor table entries */
  zb_ushort_t               ext_neighbor_used;

#ifdef ZB_SECURITY
  /* clock pointer for incoming_frame_counter expire */
  zb_ushort_t               incoming_frame_counter_clock;
#endif
}
zb_neighbor_tbl_t;

#define ZB_NWK_GET_PASSIVE_ACK_DEV_NUM(result)                          \
do                                                                      \
{                                                                       \
  zb_uint8_t _i;                                                          \
  (result) = 0;                                                         \
  for (_i = 0; _i < ZG->nwk.neighbor.base_neighbor_size; _i++)          \
  {                                                                     \
    if ( ZG->nwk.neighbor.base_neighbor[_i].used                        \
         && (ZG->nwk.neighbor.base_neighbor[_i].device_type == ZB_NWK_DEVICE_TYPE_ROUTER \
             || ZG->nwk.neighbor.base_neighbor[_i].device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR) ) \
    {                                                                   \
      (result)++;                                                       \
    }                                                                   \
  }                                                                     \
} while (0)

#define ZB_NWK_NEIGHBOR_GET_INDEX_BY_ADDRESS(addr) ( addr - &ZG->nwk.neighbor.base_neighbor[0] )

/**
   Neighbor table subsystem init.
 */
void zb_nwk_neighbor_init() ZB_SDCC_REENTRANT;

#ifndef ZB_ED_ROLE
/**
   Start work with extended neighbor table: cut space from the base neighbor table
 */
void zb_nwk_exneighbor_start() ZB_SDCC_REENTRANT;
#endif

/**
   Stop work with extended neighbor table: move some entries to the base table,
   give all memory to the base
 */
void zb_nwk_exneighbor_stop(zb_uint16_t parent_short_addr) ZB_SDCC_REENTRANT;

/**
   Get extended neighbor table entry by short address

   @param panid_ref  - ext PAN id ref
   @param short_addr - 16-bit device address
   @param enbt       - (out) allocated or found ext neighbor table entry

   @return RET_OK if success, error code if error
 */
zb_ret_t zb_nwk_exneighbor_by_short(zb_address_pan_id_ref_t panid_ref, zb_uint16_t short_addr, zb_ext_neighbor_tbl_ent_t **enbt) ZB_SDCC_REENTRANT;

/**
   Get extended neighbor table entry by long address

   @param panid_ref  - ext PAN id ref
   @param long_addr  - 64-bit device address
   @param enbt       - (out) allocated or found ext neighbor table entry

   @return RET_OK if success, error code if error
 */
zb_ret_t zb_nwk_exneighbor_by_ieee(zb_address_pan_id_ref_t panid_ref, zb_ieee_addr_t long_addr, zb_ext_neighbor_tbl_ent_t **enbt) ZB_SDCC_REENTRANT;


/**
   Get neighbor table entry by address, create if absent

   @param addr_ref - address
   @param create_if_absent - create new entry if absent
   @param nbt - (out) neighbor table entry

   @return RET_OK if just created RET_ALREADY_EXISTS if entry exists, error code
   if error
 */
zb_ret_t zb_nwk_neighbor_get(zb_address_ieee_ref_t addr_ref, zb_bool_t create_if_absent, zb_neighbor_tbl_ent_t **nbt) ZB_SDCC_REENTRANT;


#define ZB_NWK_NEIGHBOR_GET_EXISTING(ieee_ref)                          \
  ((ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] == (zb_uint8_t)-1)      \
                                   ?                                    \
                                   NULL                                 \
                                   : &ZG->nwk.neighbor.base_neighbor[ ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] ])

/**
   Get neighbor table entry by short address, does not create if absent

   @param short_addr - address
   @param nbt - (out) neighbor table entry

   @return RET_OK if forund, else RET_NOT_FOUND
   if error
 */
zb_ret_t zb_nwk_neighbor_get_by_short(zb_uint16_t short_addr, zb_neighbor_tbl_ent_t **nbt) ZB_SDCC_REENTRANT;

/**
   Get neighbor table entry by long address, does not create if absent

   @param long_addr - address
   @param nbt - (out) neighbor table entry

   @return RET_OK if forund, else RET_NOT_FOUND
   if error
 */
zb_ret_t zb_nwk_neighbor_get_by_ieee(zb_ieee_addr_t long_addr, zb_neighbor_tbl_ent_t **nbt);



/**
   Remove entity by ieee address

   @param ieee_ref - ieee address reference

   @return RET_OK if success, error code if error
 */
zb_ret_t zb_nwk_neighbor_delete(zb_address_ieee_ref_t ieee_ref);

/**
   Remove all entities

   @return nothing
 */
void zb_nwk_neighbor_clear();


/**
   Copy ext entry to base, to be able to send packet to it.

   @param ext_ent - pointer to external table entry

   @return RET_OK if success, error code if error
 */
zb_ret_t zb_nwk_neighbor_ext_to_base_tmp(zb_ext_neighbor_tbl_ent_t *ext_ent) ZB_SDCC_REENTRANT;


/**
   Run one iteration of the incoming frame counter expiration clock algorithm

   The goal is to clear (drop to 0) counters for too old entries (with distance
   between it and key_seq_number > 3).
   Do it one entry at a time to exclude unpredictable delays.

   @param key_seq_number - current key sequence number
 */
void zb_nwk_neighbor_incoming_frame_counter_clock(zb_uint8_t key_seq_number);


/* FIXME: Implement one function instead next three, but logic will be quite complex */
/**
   Get next device with rx_on_when_idle == 1

   @param i - current index in the neighbor table. Use 0 to start iteration.
   @return index in the neighbor table (0 is valid index) or ~0 if no more such entry
 */
zb_ushort_t zb_nwk_neighbor_next_rx_on_i(zb_ushort_t i);

/**
   Get next child to retransmit broadcast farame to

   @param addr - broadcast address
   @param i - current index in the neighbor table. Use 0 to start iteration.
   @return index in the neighbor table or ~0 if no more such children
 */
zb_ushort_t zb_nwk_neighbor_next_ze_children_i(zb_uint16_t addr, zb_ushort_t i);

/**
   Get next child with rx_on_when_idle == 1

   @param addr - broadcast address
   @param i - current index in the neighbor table. Use 0 to start iteration.
   @return index in the neighbor table or ~0 if no more such children
 */
zb_ushort_t zb_nwk_neighbor_next_ze_children_rx_off_i(zb_ushort_t i);

/*! @} */
/*! \endcond */

#endif /* ZB_NWK_NEIGHBOR_H */
