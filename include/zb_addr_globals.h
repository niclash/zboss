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
PURPOSE: Address compression, hash and etc.
*/

#ifndef ZB_ADDR_GLOBALS_H
#define ZB_ADDR_GLOBALS_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_NWK */
/*! @{ */

/**
   \par Define 'global address context' - common place to store and manage
   various ZigBee addresses. It contains array of PANID's, IEEE(64 bit) and 16
   bit address pairs. Protocol tables contains only 1 byte reference to the
   corresponding array. These arrays are hashes indeed, this speed up address
   search. To minimize memory usage open address hash is used.

   Address context consists of:
    - PANID's array - open addressing hash contains known PANID's. Seems MAC or NWK
   should clear it before new scan operation.
    - IEEE/16 bits address pair hash. Array of ieee/16bit address pair. ieee
   address is the key in this hash.
    - 16 bits address hash. The key of this hash is 16 bit address, used for 16
   bits address and corresponding ieee address search. This array contains
   reference to ieee array.

   Clear and reuse elements in ieee/16bit address map:
   Each IEEE/16 bits address pair has lock counter. Lock counter is zero when
   this address is not used in any table, those not used addresses organized in
   lru list. Head of lru list contains address elements that's not used for a
   long time.
*/

#include "zb_pooled_list.h"
#include "zb_address.h"

#define ZB_DEV_MANUFACTORER_TABLE_SIZE 16


/**
   Manufacturer part of the ieee address
*/
typedef struct zb_dev_manufacturer_s
{
  zb_uint8_t device_manufacturer[3]; /*!< Manufactureer identifier - 3 octets */
} zb_dev_manufacturer_t;


/**
   64 bit / 16 bit address map
*/
typedef struct zb_address_map_s
{
  zb_ieee_addr_compressed_t  ieee_addr; /*!< Comressed ieee address */
  zb_uint16_t                addr; /*!< 16 bit device address */
  zb_uint8_t                 aps_dup_counter;
  zb_bitfield_t              used:1; /*!< if 0, this entry is free (never used)  */
  zb_bitfield_t              lock_cnt:4; /*!< lock counter. not locked if 0  */
  zb_bitfield_t              clock:1;    /*!< clock value for the clock usage algorithm  */
  zb_bitfield_t              aps_dup_clock:2;
} ZB_PACKED_STRUCT zb_address_map_t;


/**
   ext pan id and short pan id -> pan ref map
 */
typedef struct zb_pan_id_map_s
{
  zb_ext_pan_id_t long_panid;
  zb_uint16_t     short_panid;
} ZB_PACKED_STRUCT zb_pan_id_map_t;

/**
   Global address context
*/
typedef struct zb_addr_globals_s
{
  zb_uint8_t used_manufacturer[ZB_DEV_MANUFACTORER_TABLE_SIZE / 8 + 1];   /*!<
                                                                           * dev_manufacturer
                                                                           * usage mask  */
  zb_dev_manufacturer_t dev_manufacturer[ZB_DEV_MANUFACTORER_TABLE_SIZE]; /*!<
                                                                           * Manufacturers
                                                                           * array */

  zb_pan_id_map_t pan_map[ZB_PANID_TABLE_SIZE]; /*!< PAN ID's hash */
  zb_uint8_t used_pan_addr[ZB_PANID_TABLE_SIZE / 8 + 1]; /*!< pan_addr usage mask  */

  zb_address_map_t addr_map[ZB_IEEE_ADDR_TABLE_SIZE]; /*!< Address map - open
                                                       * hash by 64-bit address */
  zb_uint8_t short_sorted[ZB_IEEE_ADDR_TABLE_SIZE]; /*!< 16 bits address sort array */
  zb_ushort_t n_elements;                            /*!< # of elements in the
                                                     * address translation table  */
  zb_ushort_t n_sorted_elements; /*!< # of elements in the short_sorted[] */
  zb_ushort_t clock_i;
} zb_addr_globals_t;


/*! @} */
/*! \endcond */

#endif /* ZB_ADDR_GLOBALS_H */
