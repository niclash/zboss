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
PURPOSE: Zigbee addresses routine
*/

/*! \addtogroup ZB_NWK_ADDR */
/*! @{ */

#include "zb_common.h"
#include "zb_hash.h"
#include "zb_address.h"

#include "zb_bank_a.h"

static zb_bool_t ieee_search(zb_ieee_addr_compressed_t *ieee_compressed, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT;

static zb_bool_t short_search(zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT;

static zb_ret_t addr_add(zb_ieee_addr_compressed_t *ieee_compressed,
                         zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT;

static void del_short_sorted(zb_address_ieee_ref_t ref) ZB_SDCC_REENTRANT;

static void add_short_sorted(zb_address_ieee_ref_t ref, zb_uint16_t short_addr) ZB_SDCC_REENTRANT;

static void clock_tick() ZB_SDCC_REENTRANT;


static zb_ushort_t zb_check_bit_in_bit_vector(zb_uint8_t *v, zb_ushort_t b) ZB_SDCC_REENTRANT
{
  return ZB_CHECK_BIT_IN_BIT_VECTOR(v, b);
}

static void zb_set_bit_in_bit_vector(zb_uint8_t *v, zb_ushort_t b) ZB_SDCC_REENTRANT
{
  ZB_SET_BIT_IN_BIT_VECTOR(v, b);
}

#define ZB_ADDRESS_PAN_ID_HASH(pan_id) ( ZB_4INT_HASH_FUNC((int)pan_id[0], (int)pan_id[2], (int)pan_id[4], (int)pan_id[6]) % ZB_PANID_TABLE_SIZE )


#ifndef ZB_LIMITED_FEATURES2
zb_ret_t zb_address_set_pan_id(zb_uint16_t short_pan_id, zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t *ref) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_address_pan_id_ref_t h_i = ZB_ADDRESS_PAN_ID_HASH(pan_id);
  zb_address_pan_id_ref_t i;
  zb_address_pan_id_ref_t free_i = (zb_address_pan_id_ref_t)-1;
  zb_bool_t found = ZB_FALSE;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_add_pan_id short_pan_id %d pan_id " TRACE_FORMAT_64 " ref %p", (FMT__D_A_P,
           (int) short_pan_id, TRACE_ARG_64(pan_id), ref));

  /* try to find pan id first */
  TRACE_MSG(TRACE_COMMON3, "start search pan_id reference from h_i %hd", (FMT__H, h_i));
  i = h_i;
  do
  {
    if ( zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, i))
    {
      if (ZB_EXTPANID_CMP(ZG->addr.pan_map[i].long_panid, pan_id) )
      {
        TRACE_MSG(TRACE_COMMON1, "pan_id reference found %d", (FMT__D, (int)i));
        found = ZB_TRUE;
        break;
      }
    }
    else
    {
      free_i = i;
    }
    /* ZB_PANID_TABLE_SIZE is power of 2, so it is cheap to use division */
    /* i = ( i + 1 ) % ZB_PANID_TABLE_SIZE;
       if use sdcc, then code size increases by ~45 bytes  :)
    */
    i = i + 1;
    i = i % ZB_PANID_TABLE_SIZE;
  } while ( i != h_i );

  /* if found return ref, otherwise add new pan_id */
  if ( found )
  {
    *ref = i;
    ret = RET_ALREADY_EXISTS;
  }
  else
  {
    /* try to find free place */
    if (free_i != (zb_address_pan_id_ref_t)(-1))
    {
      TRACE_MSG(TRACE_COMMON1, "pan_id ref after find free place %d", (FMT__D, (int)free_i));
      ZB_EXTPANID_COPY(ZG->addr.pan_map[free_i].long_panid, pan_id);
      ZG->addr.pan_map[free_i].short_panid = short_pan_id;
      zb_set_bit_in_bit_vector(ZG->addr.used_pan_addr, free_i);
      *ref = free_i;
    }
    else
    {
      /* no free place */
      TRACE_MSG(TRACE_COMMON1, "pan address array is full", (FMT__0));
      ret = RET_OVERFLOW;
    }
  }

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_add_pan_id %d", (FMT__D, ret));
  return ret;
}


void zb_address_get_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK
{
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id pan_id_reference %d pan_id %p", (FMT__D_P,(int) pan_id_ref, pan_id));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) );
  ZB_EXTPANID_COPY(pan_id, ZG->addr.pan_map[pan_id_ref].long_panid);

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id", (FMT__0));
}

zb_ret_t zb_address_get_pan_id_ref(zb_ext_pan_id_t pan_id , zb_address_pan_id_ref_t *ref) ZB_CALLBACK
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_address_pan_id_ref_t h_i = ZB_ADDRESS_PAN_ID_HASH(pan_id);
  zb_address_pan_id_ref_t i;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id_ref pan_id " TRACE_FORMAT_64 " ref %p", (FMT__A_P, TRACE_ARG_64(pan_id), ref));

  /* try to find pan id */
  TRACE_MSG(TRACE_COMMON3, "start search pan_id reference from h_i %hd", (FMT__H, h_i));
  i = h_i;
  do
  {
    if ( zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, i)
         && ZB_EXTPANID_CMP(ZG->addr.pan_map[i].long_panid, pan_id) )
    {
      TRACE_MSG(TRACE_COMMON1, "pan_id reference found %d", (FMT__D, (int)i));
      ret = RET_OK;
      *ref = i;
      break;
    }
    /* ZB_PANID_TABLE_SIZE is power of 2, so it is cheap to use division */
    /* i = ( i + 1 ) % ZB_PANID_TABLE_SIZE;
       if use sdcc, then code size increases by ~45 bytes  :)
    */
    i = i + 1;
    i = i % ZB_PANID_TABLE_SIZE;
  } while ( i != h_i );

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id_ref %d", (FMT__D, ret));
  return ret;
}


void zb_address_get_short_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_uint16_t *pan_id_p) ZB_CALLBACK
{
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id pan_id_reference %d pan_id %p", (FMT__D_P, (int)pan_id_ref, pan_id_p));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) );
  *pan_id_p = ZG->addr.pan_map[pan_id_ref].short_panid;

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id", (FMT__0));
}


zb_bool_t zb_address_cmp_pan_id_by_ref(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK
{
  zb_bool_t ret;
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_cmp_pan_id_by_ref pan_id_reference %d pan_id %p", (FMT__D_P,(int) pan_id_ref, pan_id));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) );
  ret = ZB_EXTPANID_CMP(pan_id, ZG->addr.pan_map[pan_id_ref].long_panid);

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_cmp_pan_id_by_ref %d", (FMT__D, ret));
  return ret;
}

#else  /* ZB_LIMITED_FEATURES2 */

zb_ret_t zb_address_set_pan_id(zb_uint16_t short_pan_id, zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t *ref) ZB_CALLBACK
{
  *ref = 0;
  ZB_EXTPANID_COPY(ZG->addr.pan_map[0].long_panid, pan_id);
  ZG->addr.pan_map[0].short_panid = short_pan_id;

  return RET_OK;
}


void zb_address_get_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK
{
  ZB_EXTPANID_COPY(pan_id, ZG->addr.pan_map[0].long_panid);
}

zb_ret_t zb_address_get_pan_id_ref(zb_ext_pan_id_t pan_id , zb_address_pan_id_ref_t *ref) ZB_CALLBACK
{
  *ref = 0;
  return RET_OK;
}


void zb_address_get_short_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_uint16_t *pan_id_p) ZB_CALLBACK
{
  *pan_id_p = ZG->addr.pan_map[0].short_panid;
}


zb_bool_t zb_address_cmp_pan_id_by_ref(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id) ZB_CALLBACK
{
  return ZB_TRUE;
}

#endif  /* ZB_LIMITED_FEATURES2 */


void zb_ieee_addr_compress(zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address) ZB_CALLBACK
{
  /* try to find this manufacturer */
  zb_ushort_t i;
  zb_ushort_t free_i = ZB_DEV_MANUFACTORER_TABLE_SIZE;
  for ( i = 0; i < ZB_DEV_MANUFACTORER_TABLE_SIZE; i++)
  {
    if ( zb_check_bit_in_bit_vector(ZG->addr.used_manufacturer, i))
    {
      if (!ZB_MEMCMP(&ZG->addr.dev_manufacturer[i].device_manufacturer[0], &address[0], 3))
      {
        break;
      }
    }
    else
    {
      free_i = i;
    }
  }

  /* found ? */
  if ( i == ZB_DEV_MANUFACTORER_TABLE_SIZE )
  {
    /* add new manufacturer */
    i = free_i;
  }

  /* compress */
  if ( i != ZB_DEV_MANUFACTORER_TABLE_SIZE )
  {
    zb_set_bit_in_bit_vector(ZG->addr.used_manufacturer, i);
    ZB_MEMCPY(&ZG->addr.dev_manufacturer[i].device_manufacturer[0], &address[0], 3);
    compressed_address->dev_manufacturer = i;
    ZB_MEMCPY(&compressed_address->device_id[0], &address[3], 5);
  }
  else
  {
    /* TODO: clear unused manufacturers and add new one */
    ZB_ASSERT(0);
  }
}


void zb_ieee_addr_decompress(zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address) ZB_CALLBACK
{
  ZB_ADDRESS_DECOMPRESS(address, *compressed_address);
}


/**
   Calculate hash function for the compressed long address
 */
#define ZB_ADDRESS_COMPRESSED_IEEE_HASH_MACRO(addr) ( ZB_4INT_HASH_FUNC(  \
  (zb_uint16_t)(addr)->device_id[0],                                \
  (zb_uint16_t)(addr)->device_id[2],                                \
  (zb_uint16_t)(addr)->device_id[3],                                \
  (zb_uint16_t)(addr)->device_id[4]) % ZB_IEEE_ADDR_TABLE_SIZE )


static zb_address_ieee_ref_t zb_address_compressed_ieee_hash(zb_ieee_addr_compressed_t *ieee_compressed) ZB_SDCC_REENTRANT
{
  return ZB_ADDRESS_COMPRESSED_IEEE_HASH_MACRO(ieee_compressed);
}

zb_ret_t zb_address_by_ieee(zb_ieee_addr_t ieee, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_ieee_addr_compressed_t ieee_compressed;
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_ieee ieee %p", (FMT__P, ieee));

  ZB_DUMP_IEEE_ADDR(ieee);
  /* compress address */
  zb_ieee_addr_compress(ieee, &ieee_compressed);
  /* search */
  if (ieee_search(&ieee_compressed, ref_p))
  {
    /* mark address as updated - prevent its immediate reuse */
    ZG->addr.addr_map[*ref_p].clock = 1;
  }
  else
  {
    ret = ( create ) ? addr_add(&ieee_compressed, (zb_uint16_t)~0, ref_p) : RET_NOT_FOUND;
  }

  if (lock && ret == RET_OK)
  {
    zb_address_lock(*ref_p);
  }
  TRACE_MSG(TRACE_COMMON1, "<< zb_address_ieee, ret %i", (FMT__D, ret));
  return ret;
}


zb_ret_t zb_address_by_short(zb_uint16_t short_address, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  if (short_search(short_address, ref_p))
  {
    /* mark address as updated - prevent its immediate reuse */
    ZG->addr.addr_map[*ref_p].clock = 1;
  }
  else
  {
    ret = ( create ) ? addr_add(NULL, short_address, ref_p) : RET_NOT_FOUND;
  }
  if (lock && ret == RET_OK)
  {
    zb_address_lock(*ref_p);
  }
  TRACE_MSG(TRACE_NWK1, "zb_address_by_short %d ret %hd ref %hd", (FMT__D_H, short_address, ret, *ref_p));
  return ret;
}


zb_ret_t zb_address_update(zb_ieee_addr_t ieee_address, zb_uint16_t short_address, zb_bool_t lock, zb_address_ieee_ref_t *ref_p) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  zb_bool_t found_short;
  zb_bool_t found_long;
  zb_address_map_t *ent = NULL;
  zb_address_ieee_ref_t ref;
  zb_address_ieee_ref_t ref_long;
  zb_ieee_addr_compressed_t ieee_compressed;

  /* compress address */
  zb_ieee_addr_compress(ieee_address, &ieee_compressed);
  found_short = short_search(short_address, &ref);
  found_long = ieee_search(&ieee_compressed, &ref_long);
  if (found_short && found_long)
  {
    if (ref != ref_long)
    {
      if ( !ZG->addr.addr_map[ref].lock_cnt )
      {
        zb_address_delete(ref);
        ref = ref_long;
      }
      else if ( !ZG->addr.addr_map[ref_long].lock_cnt )
      {
        zb_address_delete(ref_long);
      }
      else
      {
        /*
          Resolve conflict: 2 locked entries with long/short address mismatch.
          Not sure how to do it. Scan for _every_ data structure which use
          address and remove it??
        */
        ZB_ASSERT(0);
      }
    }
  }
  else if (found_long)
  {
    ref = ref_long;
  }
  if (found_short || found_long)
  {
    ent = &ZG->addr.addr_map[ref];
    /* TODO: if address has changed and not locked, try to relocate it for better
     * long address hash performance */
    ZB_MEMCPY(&ent->ieee_addr, &ieee_compressed, sizeof(ieee_compressed));
    if (ent->addr != short_address)
    {
      if (ent->addr != (zb_uint16_t)~0)
      {
        del_short_sorted(ref);
      }
      ent->addr = short_address;
      add_short_sorted(ref, short_address);
    }
    ent->clock = 1;
  }
  else
  {
    ret = addr_add(&ieee_compressed, short_address, &ref);
  }
  if (ret == RET_OK)
  {
    *ref_p = ref;

    if (lock)
    {
      zb_address_lock(ref);
    }
  }
  TRACE_MSG(TRACE_COMMON2, "zb_address_update short 0x%x long " TRACE_FORMAT_64 " ref %hd ret %d",
            (FMT__D_A, short_address, TRACE_ARG_64(ieee_address), ref, ret));
  return ret;
}


void zb_address_by_ref(zb_ieee_addr_t ieee_address, zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref) ZB_CALLBACK
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];
  *short_address_p = ent->addr;
  ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);
}


void zb_address_ieee_by_ref(zb_ieee_addr_t ieee_address, zb_address_ieee_ref_t ref) ZB_CALLBACK
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];
  ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);
}


void zb_address_short_by_ref(zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref) ZB_CALLBACK
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];
  *short_address_p = ent->addr;
}


zb_uint16_t zb_address_short_by_ieee(zb_ieee_addr_t ieee_address) ZB_CALLBACK
{
  zb_address_ieee_ref_t ref;
  if (zb_address_by_ieee(ieee_address, ZB_FALSE, ZB_FALSE, &ref) == RET_OK)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[ref];
    return ent->addr;
  }
  return (zb_uint16_t)-1;
}


zb_ret_t zb_address_ieee_by_short(zb_uint16_t short_addr, zb_ieee_addr_t ieee_address) ZB_CALLBACK
{
  zb_address_ieee_ref_t ref;
  if (zb_address_by_short(short_addr, ZB_FALSE, ZB_FALSE, &ref) == RET_OK)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[ref];
    ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);
    return RET_OK;
  }
  return RET_NOT_FOUND;
}


void zb_address_lock(zb_address_ieee_ref_t ref) ZB_CALLBACK
{
#ifndef ZB_LIMITED_FEATURES
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_ieee_lock ref %d", (FMT__D,(int) ref));

  ZG->addr.addr_map[ref].lock_cnt++;
  ZB_ASSERT(ZG->addr.addr_map[ref].lock_cnt != 0); /* check for overflow */
  ZG->addr.addr_map[ref].clock = 1;

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_ieee_lock", (FMT__0));
#endif
}


void zb_address_unlock(zb_address_ieee_ref_t ref) ZB_CALLBACK
{
#ifndef ZB_LIMITED_FEATURES
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_ieee_unlock ref %d", (FMT__D, (int)ref));

  ZB_ASSERT(ZG->addr.addr_map[ref].lock_cnt);
  ZG->addr.addr_map[ref].lock_cnt--;

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_ieee_unlock", (FMT__0));
#endif
}


/**
   Search address by 64-bit address

   Use open hash search. In the worst case it works like linear search.

   @param ieee_compressed - compressed long address
   @param ref_p - (out) address reference

   @return ZB_TRUE if found, else ZB_FALSE
 */
static zb_bool_t ieee_search(zb_ieee_addr_compressed_t *ieee_compressed, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT
{
  zb_address_ieee_ref_t i = zb_address_compressed_ieee_hash(ieee_compressed);
  zb_address_ieee_ref_t cnt;

  for (cnt = 0 ; cnt < ZB_IEEE_ADDR_TABLE_SIZE ; ++cnt, i = (i + 1) % ZB_IEEE_ADDR_TABLE_SIZE)
  {
    if ( ZG->addr.addr_map[i].used
         && ZB_ADDRESS_COMPRESSED_CMP(ZG->addr.addr_map[i].ieee_addr, *ieee_compressed))
    {
      TRACE_MSG(TRACE_COMMON1, "ieee reference found %d", (FMT__D, (int)i));
      *ref_p = i;
      return ZB_TRUE;
    }
  }

  return ZB_FALSE;
}


/**
   Search address by 16-bit address

   Use binary search over short_sorted array.

   @param short_addr - short address
   @param ref_p - (out) address reference

   @return ZB_TRUE if found, else ZB_FALSE
 */
static zb_bool_t short_search(zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT
{
  zb_bool_t found = ZB_FALSE;
  zb_ushort_t l = 0;

  /* when using signed bytes as indexes, table size must be < 127 */
  ZB_ASSERT_COMPILE_DECL(ZB_IEEE_ADDR_TABLE_SIZE <= ZB_SHORT_MAX);

  /* if has exactly 1 element, not need to do binary search */
  if (ZG->addr.n_sorted_elements > 1)
  {
    /* search using binary search */
    zb_ushort_t r = ZG->addr.n_sorted_elements;
    while (l < r)
    {
      zb_short_t i = l + (r - l) / 2;
      if (short_addr > ZG->addr.addr_map[ZG->addr.short_sorted[i]].addr)
      {
        l = i + 1;
      }
      else
      {
        r = i;
      }
    }
  }

  TRACE_MSG(TRACE_NWK3, "short_addr %d l %d ZG->addr.short_sorted[l] %d addr %d",
            (FMT__D_D_D_D, short_addr, l, ZG->addr.short_sorted[l], ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr));

  if (ZG->addr.n_sorted_elements > 0   /* empty? */
      && l < ZG->addr.n_sorted_elements
      && short_addr == ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr)
  {
    *ref_p = ZG->addr.short_sorted[l];
    found = ZB_TRUE;
  }

  return found;
}


/**
   Add ieee/short address pair.
   Allocate space, add an entry.

   Possible problem: entry can't be moved once allocated.
   If entry first creates without long address, its search will be not
   effective!

   @param ieee_compressed - pointer to the compressed long address, NULL if absent
   @param short_addr - short address, 0 if absent

 */
static zb_ret_t addr_add(zb_ieee_addr_compressed_t *ieee_compressed,
                         zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p) ZB_SDCC_REENTRANT
{
  zb_address_ieee_ref_t i = ieee_compressed ? zb_address_compressed_ieee_hash(ieee_compressed) : 0;
  zb_address_ieee_ref_t i_end = i;
  zb_address_map_t *ent;
  zb_uint8_t not_found = 1;

  /* First find a place to insert */
  do
  {
    ent = &ZG->addr.addr_map[i];
    /* FIXME: Correct enries reuse. Seems it should reuse items only when it
     * doesn't have room for new entrie */
#if 0
    if (!ent->used || (ent->lock_cnt == 0 && !ent->clock))
#endif
    if (!ent->used)
    {
      not_found = 0;
      break;
    }
    i = (i + 1) % ZB_IEEE_ADDR_TABLE_SIZE;
  } while (i != i_end);

  if (not_found)
  {
    return RET_OVERFLOW;
  }

  clock_tick();

  ent->addr = short_addr;
  ent->clock = 1;
  if (ieee_compressed)
  {
    ZB_MEMCPY(&ent->ieee_addr, ieee_compressed, sizeof(*ieee_compressed));
  }
  else
  {
    ZB_BZERO(&ent->ieee_addr, sizeof(*ieee_compressed));
  }
  /* Now update short sorted array if short address exists */
  if (short_addr != (zb_uint16_t)~0)
  {
    if (ent->used)
    {
      del_short_sorted(i);
    }
    add_short_sorted(i, short_addr);
  }
  ZG->addr.n_elements += (!ent->used);
  ent->used = 1;
  *ref_p = i;

  return RET_OK;
}


/**
   Delete address from the translation array

   @param ref - address ref
 */
void zb_address_delete(zb_address_ieee_ref_t ref) ZB_CALLBACK
{
  del_short_sorted(ref);
  ZG->addr.addr_map[ref].used = 0;
  ZG->addr.n_elements--;
}


/**
   Delete en entry from the short address sorted search array

   @param ref - address ref
 */
static void del_short_sorted(zb_address_ieee_ref_t ref) ZB_SDCC_REENTRANT
{
  zb_address_ieee_ref_t i;
  zb_ushort_t dec = 0;
  /* Use trivial scan because dups in the short address are possible */
  for (i = 0 ; i < ZG->addr.n_sorted_elements ; ++i)
  {
    if (ZG->addr.short_sorted[i] == ref)
    {
      dec++;
      if (i != ZG->addr.n_sorted_elements - 1)
      {
        TRACE_MSG(TRACE_NWK3, "move to %hd %hd %hd", (FMT__H_H_H, i, i+1, ZG->addr.n_sorted_elements - i));
        ZB_MEMMOVE(&ZG->addr.short_sorted[i], &ZG->addr.short_sorted[i+1], ZG->addr.n_sorted_elements - i);
      }
    }
  }
  ZG->addr.n_sorted_elements -= dec;
}


/**
   Add an entry into short address sorted search array

   @param ref - address ref
   @param short_addr - short address
 */
static void add_short_sorted(zb_address_ieee_ref_t ref, zb_uint16_t short_addr) ZB_SDCC_REENTRANT
{
  zb_ushort_t l = 0;

  if (ZG->addr.n_sorted_elements > 1)
  {
    zb_ushort_t r = ZG->addr.n_sorted_elements;
    /* search place using binary search */
    while (l < r)
    {
      zb_ushort_t i = l + (r - l) / 2;
      if (short_addr >= ZG->addr.addr_map[ZG->addr.short_sorted[i]].addr)
      {
        l = i + 1;
      }
      else
      {
        r = i;
      }
    }
  }
  if (ZG->addr.n_sorted_elements > 0
      && l < ZG->addr.n_sorted_elements
      && short_addr > ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr)
  {
    l++;
  }
  if (l < ZG->addr.n_sorted_elements)
  {
    ZB_MEMMOVE(&ZG->addr.short_sorted[l+1], &ZG->addr.short_sorted[l], ZG->addr.n_sorted_elements - l);
  }
  ZG->addr.short_sorted[l] = ref;
  ZG->addr.n_sorted_elements++;
}


/**
   Do single clock algorithm tick.

   Find first non-locked used element and clear its 'clock' field so it could be
   reused later. Update clock pointer.
 */
static void clock_tick() ZB_SDCC_REENTRANT
{
#ifndef ZB_LIMITED_FEATURES
  zb_address_map_t *ent;
  zb_ushort_t limit = ZG->addr.clock_i;
  zb_ushort_t cnt = 0;
  zb_bool_t found = ZB_FALSE;
  while (ZG->addr.clock_i < ZB_IEEE_ADDR_TABLE_SIZE && cnt < ZG->addr.n_elements)
  {
    ent = &ZG->addr.addr_map[ZG->addr.clock_i];
    ZG->addr.clock_i++;
    if (ent->used)
    {
      cnt++;
      if (!ent->lock_cnt)
      {
        ent->clock = 0;
        found = ZB_TRUE;
        break;
      }
    }
  }
  if (!found && cnt < ZG->addr.n_elements)
  {
    ZG->addr.clock_i = 0;
    while (ZG->addr.clock_i < limit && cnt < ZG->addr.n_elements)
    {
      ent = &ZG->addr.addr_map[ZG->addr.clock_i];
      ZG->addr.clock_i++;
      if (ent->used)
      {
        cnt++;
        if (!ent->lock_cnt)
        {
          ent->clock = 0;
          break;
        }
      }
    } /* while */
  } /* if */
#endif
}

zb_bool_t zb_address_compressed_cmp(zb_ieee_addr_compressed_t *one, zb_ieee_addr_compressed_t *two) ZB_CALLBACK
{
  return (zb_bool_t)((one)->dev_manufacturer == (two)->dev_manufacturer
          && !ZB_MEMCMP(&(one)->device_id[0], &(two)->device_id[0], 5));
}

zb_uint8_t magic_bitcount8(zb_uint8_t b) ZB_CALLBACK
{
  b = (b & 0x55) + ((b >> 1) & 0x55);
  b = (b & 0x33) + ((b >> 2) & 0x33);
  b = (b & 0x0f) + ((b >> 4) & 0x0f);
  return (b);
}
/*! @} */
