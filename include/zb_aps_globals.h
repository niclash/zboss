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
PURPOSE: Globals data definitionfor APS subsystem
*/

#ifndef ZB_APS_GLOBALS_H
#define ZB_APS_GLOBALS_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_APS */
/*! @{ */

/**
   Global binding table
*/
typedef struct zb_aps_bind_src_table_s
{
  zb_address_ieee_ref_t src_addr;   /*!< source address as ref from nwkAddressMap */
  zb_uint8_t            src_end;    /*!< source endpoint */
  zb_uint16_t           cluster_id; /*!< cluster id */
} ZB_PACKED_STRUCT zb_aps_bind_src_table_t;

typedef struct zb_aps_bind_long_dst_addr_s
{
  zb_address_ieee_ref_t dst_addr;        /*!< destination address as ref from nwkAddressMap */
  zb_uint8_t            dst_end;         /*!< destination endpoint */
} ZB_PACKED_STRUCT zb_aps_bind_long_dst_addr_t;

#define ZB_APS_BIND_DST_ADDR_GROUP 0
#define ZB_APS_BIND_DST_ADDR_LONG  1
typedef struct zb_aps_bind_dst_table_s
{
  zb_uint8_t            dst_addr_mode;   /*!< destination address mode flag, 0
                                          * - group address, otherwise long
                                          * address plus dest endpoint */
  union
  {
    zb_uint16_t group_addr;                /*!< group address */
    zb_aps_bind_long_dst_addr_t long_addr; /*!< @see zb_asp_long_dst_addr_t */
  } u;
  zb_uint8_t            src_table_index; /*!< index from zb_asp_src_table_t */
} ZB_PACKED_STRUCT zb_aps_bind_dst_table_t;

typedef struct zb_aps_binding_table_s
{
  zb_aps_bind_src_table_t src_table[ZB_APS_SRC_BINDING_TABLE_SIZE];
  zb_aps_bind_dst_table_t dst_table[ZB_APS_DST_BINDING_TABLE_SIZE];
  zb_uint8_t              src_n_elements;
  zb_uint8_t              dst_n_elements;
} ZB_PACKED_STRUCT zb_aps_binding_table_t;

/**
  Group table entry
 */
typedef struct zb_aps_group_table_ent_s
{
  zb_uint16_t    group_addr;
  zb_uint8_t     endpoints[ZB_APS_ENDPOINTS_IN_GROUP_TABLE];
  zb_ushort_t    n_endpoints;
} ZB_PACKED_STRUCT zb_aps_group_table_ent_t;


ZB_RING_BUFFER_DECLARE(zb_aps_grp_up_q, zb_uint8_t, ZB_APS_GROUP_UP_Q_SIZE);

/**
   Group addressing data structure
 */
typedef struct zb_aps_group_table_s
{
  zb_aps_group_table_ent_t groups[ZB_APS_GROUP_TABLE_SIZE]; /*!< APS group table */
  zb_ushort_t              n_groups;                        /*!< # of entries in APS group table */
  zb_aps_grp_up_q_t        pass_up_q; /*!< queue to be used to pass
                                       * incoming group addresses packets up  */
  zb_aps_grp_up_q_t        local_dup_q; /*!< queue to be used to pass sending
                                          group addresses packets to myself */
  zb_buf_t *active_pass_up_buf; /*!< current buffer we are passing up */
} zb_aps_group_table_t;

/**
   APS Informational Base memory-resident data structure
 */
typedef struct zb_apsib_s
{
  zb_uint8_t  aps_counter;

/**
   Start (field name) of the APSIB section to be saved in NVRAM
 */
#define APSIB_SAVE_START aps_designated_coordinator

  /* table 2.138 - Startup parameters */
  zb_uint8_t  aps_designated_coordinator; /*!< This boolean flag indicates whether the
                                            device should assume on startup that it must
                                            become a ZigBee coordinator.  */
  zb_uint8_t  aps_insecure_join; /*!< A boolean flag, which defaults to TRUE and
                                   indicates whether it is OK to use insecure
                                   join on startup.  */
  zb_uint32_t aps_channel_mask; /*!< This is the mask containing allowable
                                  channels on which the device may attempt
                                  to form or join a network at startup time.  */
  zb_ext_pan_id_t aps_use_extended_pan_id; /*!< The 64-bit identifier of the network to join
                                             or form.  */

  zb_ieee_addr_t trust_center_address;
} zb_apsib_t;


enum zb_aps_retrans_ent_state_e
{
  ZB_APS_RETRANS_ENT_FREE,
  ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_ALRM_RUNNING,
  ZB_APS_RETRANS_ENT_SENT_MAC_NOT_CONFIRMED_APS_ACKED_ALRM_RUNNING,
  ZB_APS_RETRANS_ENT_SENT_MAC_CONFIRMED_ALRM_RUNNING
};

/**
   APS retransmissions
 */
typedef struct zb_aps_retrans_ent_s
{
  zb_uint16_t  addr;
  zb_uint16_t  clusterid;
  zb_uint8_t   aps_counter;
  zb_uint8_t   src_endpoint;
  zb_uint8_t   dst_endpoint;
  zb_uint8_t   buf;

  zb_bitfield_t aps_retries:5;
  zb_bitfield_t nwk_insecure:1;
  zb_bitfield_t state:2;        /* \see zb_aps_retrans_ent_state_e */
} ZB_PACKED_STRUCT zb_aps_retrans_ent_t;


ZB_RING_BUFFER_DECLARE(zb_ack_q, zb_uint8_t, ZB_APS_RETRANS_ACK_Q_SIZE);


typedef struct zb_aps_retrans_s
{
  zb_aps_retrans_ent_t hash[ZB_N_APS_RETRANS_ENTRIES];
  zb_uint8_t           ack_buf;
  zb_ack_q_t           ack_q;
#ifdef APS_RETRANSMIT_TEST
  zb_uint8_t   counter;
#endif
} zb_aps_retrans_t;


typedef struct zb_aps_tmp_s
{
  zb_ushort_t            neighbor_table_iterator;
  zb_uint8_t             key_seq_number;
} zb_aps_tmp_t;


#define ZB_APS_ZDO_ED_SCAN_MASK              1
#define ZB_APS_CHANNEL_MANAGER_ED_SCAN_MASK (1 << 1)

#define ZB_APS_SET_ZDO_ED_SCAN_FLAG() ( ZG->aps.flags |= ZB_APS_ZDO_ED_SCAN_MASK )
#define ZB_APS_CLEAR_ZDO_ED_SCAN_FLAG() ( ZG->aps.flags &= ~ZB_APS_ZDO_ED_SCAN_MASK )
#define ZB_APS_GET_ZDO_ED_SCAN_FLAG() ( ZG->aps.flags & ZB_APS_ZDO_ED_SCAN_MASK )

#define ZB_APS_SET_CHANNEL_MANAGER_ED_SCAN_FLAG() ( ZG->aps.flags |= ZB_APS_CHANNEL_MANAGER_ED_SCAN_MASK )
#define ZB_APS_CLEAR_CHANNEL_MANAGER_ED_SCAN_FLAG() ( ZG->aps.flags &= ~ZB_APS_CHANNEL_MANAGER_ED_SCAN_MASK )
#define ZB_APS_GET_CHANNEL_MANAGER_ED_SCAN_FLAG() ( ZG->aps.flags & ZB_APS_CHANNEL_MANAGER_ED_SCAN_MASK )


/**
   APS subsystem globals
 */
typedef struct zb_aps_globals_s
{
  zb_apsib_t             aib;
  zb_aps_binding_table_t binding;
  zb_aps_retrans_t       retrans;
  zb_aps_group_table_t   group;
  zb_aps_tmp_t           tmp;

  zb_uint8_t             authenticated; /* if 1, we are authenticated in the network */
  zb_uint8_t             flags;
  zb_uint8_t             dups_alarm_running;
} zb_aps_globals_t;

/**
   Get APS counter field from AIB.

   @return APS counter
 */
#define ZB_AIB_APS_COUNTER() ZG->aps.aib.aps_counter

/**
   Increment APS counter AIB field.
*/
#define ZB_AIB_APS_COUNTER_INC() ZG->aps.aib.aps_counter++


#define ZB_AIB() ZG->aps.aib


/*! @} */
/*! \endcond */

#endif /* ZB_APS_GLOBALS_H */
