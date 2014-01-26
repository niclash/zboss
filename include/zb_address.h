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
PURPOSE: Zigbee address management
*/

#ifndef ZB_ADDRESS_H
#define ZB_ADDRESS_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_NWK_ADDR */
/*! @{ */

#define ZB_UNKNOWN_SHORT_ADDR (zb_uint16_t)(-1)


/**
   Compressed ieee address. One byte  device manufacturer - reference to
   \see zb_dev_manufacturer_t array.
*/
typedef struct zb_ieee_addr_compressed_s
{
  zb_uint8_t dev_manufacturer; /*!< Index from dev manufacturer array */
  zb_uint8_t device_id[5]; /*!< Device id */
} zb_ieee_addr_compressed_t;


/**
   PANID reference

   Should be used inside protocol tables instead of 64 bit PANID
*/
typedef zb_ushort_t zb_address_pan_id_ref_t;


/**
   IEEE address reference

   Should be used inside protocol tables instead of 64/16 bit ieee.
*/
typedef zb_ushort_t zb_address_ieee_ref_t;



#define ZB_SET_BIT_IN_BIT_VECTOR(vector, nbit) ( (vector)[ (nbit) / 8 ] |= ( 1 << ( (nbit) % 8 )) )
#define ZB_CLR_BIT_IN_BIT_VECTOR(vector, nbit) ( (vector)[ (nbit) / 8 ] &= ~( 1 << ( (nbit) % 8 )) )
#define ZB_CHECK_BIT_IN_BIT_VECTOR(vector, nbit) ( (vector)[ (nbit) / 8 ] & ( 1 << ( (nbit) % 8 )) )

/**
   Calculate none zero bits in 8-bit digit
*/
zb_uint8_t magic_bitcount8(zb_uint8_t b) ZB_CALLBACK;

#define ZB_CALC_NONE_ZERO_BITS_IN_BIT_VECTOR(vector, size, result)  \
do                                                                  \
{                                                                   \
  int i;                                                            \
  (result) = 0;                                                     \
  for ( i = 0; i < size; i++ )                                      \
  {                                                                 \
    (result) += magic_bitcount8(vector[i]);                         \
  }                                                                 \
}                                                                   \
while (0)

#ifndef ZB_LIMITED_FEATURES
#define ZB_DUMP_IEEE_ADDR(iee_addr) TRACE_MSG(TRACE_COMMON3, "ieeeaddr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(iee_addr)))
#else
#define ZB_DUMP_IEEE_ADDR(iee_addr)
#endif

/**
   \par work with compresed addresses
 */

#define ZB_ADDRESS_DECOMPRESS(address, copressed_address)               \
do                                                                      \
{                                                                       \
  ZB_MEMCPY(&((address)[0]),                                            \
            &(ZG->addr.dev_manufacturer[(copressed_address).dev_manufacturer].device_manufacturer[0]), \
            (sizeof((address)[0]) * 3));                                \
  ZB_MEMCPY(&((address)[3]), &((copressed_address).device_id[0]), (sizeof((address)[0]) * 5)); \
}                                                                       \
while (0)

/*
#define ZB_ADDRESS_COMPRESSED_CMP(one, two) (             \
    (one).dev_manufacturer == (two).dev_manufacturer      \
  && (one).device_id[0] == (two).device_id[0]             \
  && (one).device_id[1] == (two).device_id[1]             \
  && (one).device_id[2] == (two).device_id[2]             \
  && (one).device_id[3] == (two).device_id[3]             \
  && (one).device_id[4] == (two).device_id[4] )
*/
zb_bool_t zb_address_compressed_cmp(zb_ieee_addr_compressed_t *one, zb_ieee_addr_compressed_t *two) ZB_CALLBACK;
#define ZB_ADDRESS_COMPRESSED_CMP(one, two) zb_address_compressed_cmp(&one, &two)

#define ZB_ADDRESS_COMPRESSED_COPY(dest, src)                       \
  ZB_MEMCPY(&(dest).dev_manufacturer, &(src).dev_manufacturer, sizeof(zb_ieee_addr_compressed_t))

#define ZB_ADDRESS_COMPRESED_IS_ZERO(dest)      \
  (!ZB_MEMCMP(&(dest).dev_manufacturer, g_zero_addr, sizeof(zb_ieee_addr_compressed_t)))


/**
   Add PANID to address storage and return reference

   @param pan_id - 64 bit PANID identifier
   @param ref - (output) reference to PANID.

   @return RET_OK - when success, error code otherwise.
 */
zb_ret_t zb_address_set_pan_id(zb_uint16_t short_pan_id, zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t *ref) ZB_CALLBACK;


/**
   Get extended pan id by reference.

   @param pan_id_ref - reference to panid
   @param pan_id - (output) PANID.

   @return RET_OK - when success, error code otherwise.
 */
void zb_address_get_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK;

/**
   Get pan id reference by extended pan id.

   @param pan_id -  PANID
   @param ref - (output) reference to panid

   @return RET_OK - when success, error code otherwise.
 */
zb_ret_t zb_address_get_pan_id_ref(zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t *ref) ZB_CALLBACK;

/**
   Get short pan id by reference.

   @param pan_id_ref - reference to panid
   @param pan_id - (output) PANID.

   @return RET_OK - when success, error code otherwise.
 */
void zb_address_get_short_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_uint16_t *pan_id_p) ZB_CALLBACK;


/**
   Compare pan id in the source form with pan id ref

   @param pan_id_ref - pan id ref
   @param pan_id     - pan id (64-bit)

   @return ZB_TRUE if addresses are equal, ZB_FALSE otherwhise
 */
zb_bool_t zb_address_cmp_pan_id_by_ref(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK;

/**
   Update long/short address pair, create if not exist, optionally lock.
   Reaction on device annonce etc. We have both long and short address. Must
   syncronize with this information address translation table.

   Note: never call zb_address_update() with empty (zero) ieee_address or empty
   (-1) short_address.

   @param ieee_address - long address
   @param short_address - short address
   @param lock - if TRUE, lock address entry
   @param ref - (out) address reference

   @return RET_OK or error code
 */
zb_ret_t zb_address_update(zb_ieee_addr_t ieee_address, zb_uint16_t short_address, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK;


/**
   Get address by address ref
   Get existing ieee and short addresses by address ref. Update address alive
   time if it not locked.

   @param ieee_address  - (out) long address
   @param short_address_p - (out) short address
   @param ref - address reference
 */
void zb_address_by_ref(zb_ieee_addr_t ieee_address, zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref) ZB_CALLBACK;

/**
   Get ieee address by address ref
   Get existing ieee by address ref. Update address alive time if it not locked.

   @param ieee_address  - (out) long address
   @param ref - address reference
 */
void zb_address_ieee_by_ref(zb_ieee_addr_t ieee_address, zb_address_ieee_ref_t ref) ZB_CALLBACK;


/**
   Get short address by address ref
   Get existing short address by address ref. Update address alive time if it not locked.

   @param short_address_p  - (out) short address
   @param ref - address reference
 */
void zb_address_short_by_ref(zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref) ZB_CALLBACK;

/**
   Get address ref by long address, optionaly create if not exist, optionally lock.
   Update address alive time if not locked.
   @param ieee - ieee device address
   @param create - if TRUE, create address entry if it's not exists
   @param lock - if TRUE, lock address entry
   @param ref - (out) address reference

   Note: never call zb_address_by_ieee() with empty (zero) ieee_address


   @return RET_OK or error code
 */
zb_ret_t zb_address_by_ieee(zb_ieee_addr_t ieee, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK;


/**
   Get short address by long (ieee) address

   @param ieee_address - long address

   @return short address if ok, -1 otherwhise.
 */
zb_uint16_t zb_address_short_by_ieee(zb_ieee_addr_t ieee_address) ZB_CALLBACK;


/**
   Get short address by long (ieee) address

   @param short_addr - short address
   @param ieee_address - (out)long address

   @return RET_OK or RET_NOT_FOUND
 */
zb_ret_t zb_address_ieee_by_short(zb_uint16_t short_addr, zb_ieee_addr_t ieee_address) ZB_CALLBACK;



/**
   Get address ref by long address, create if not exist, optionally lock.
   Update address alive time if not locked.
   @param ieee - ieee device address
   @param create - if TRUE, create address entry if it's not exists
   @param lock - if TRUE, lock address entry
   @param ref - (out) address reference

   Note: never call zb_address_by_short() with empty (-1) short_address


   @return RET_OK or error code
 */
zb_ret_t zb_address_by_short(zb_uint16_t short_address, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK;


/**
   Increase address lock counter, when it used in some table.
   Address must be already locked.

   @param ref - ieee/network address pair reference

 */
void zb_address_lock(zb_address_ieee_ref_t ref) ZB_CALLBACK;


/**
   UnLock address counter. Decrease lock counter.
   Move address to lru head if lock counter is zero.

   @param ref - ieee/network address pair reference

 */
void zb_address_unlock(zb_address_ieee_ref_t ref) ZB_CALLBACK;

/**
   Delete address.

   @param ref - ieee/network address pair reference
 */
void zb_address_delete(zb_address_ieee_ref_t ref) ZB_CALLBACK;


/**
   Deinitialize ZigBee address management

   Function doesn't return values
 */
void zb_address_deinit() ZB_CALLBACK;


/**
   Compress long address: store manufacturer address part elsewhere

   This routine packs 8 bytes address to 6 bytes

   @param address - uncompressed address
   @param compressed_address - (out) compressed address
 */
void zb_ieee_addr_compress(zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address) ZB_CALLBACK;


/**
   Decompress compressed long address.

   This routine unpacks 6 bytes address to 8 bytes

   @param address - (out) uncompressed address
   @param compressed_address - compressed address
 */
void zb_ieee_addr_decompress(zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address) ZB_CALLBACK;

/*! @} */
/*! \endcond */

#endif /* ZB_ADDRESS_H */
