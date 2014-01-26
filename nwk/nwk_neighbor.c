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
PURPOSE: Neighbor table
*/

#include "zb_common.h"
#include "zb_nwk.h"
#include "zb_magic_macros.h"


/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_4.h"

#ifndef ZB_ED_ROLE
static void base_neighbor_cleanup();
#endif

static zb_ret_t alloc_new_extneiboard(zb_address_pan_id_ref_t panid_ref, zb_uint16_t short_addr, zb_ext_neighbor_tbl_ent_t **enbt)  ZB_SDCC_REENTRANT;

void zb_nwk_neighbor_init() ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_NWK1, "nb_init", (FMT__0));
  ZB_MEMSET(&ZG->nwk.neighbor.addr_to_neighbor[0], -1, sizeof(ZG->nwk.neighbor.addr_to_neighbor));
  ZG->nwk.neighbor.base_neighbor_size = ZB_NEIGHBOR_TABLE_SIZE;
#ifdef ZB_ED_ROLE
  ZG->nwk.neighbor.ext_neighbor_size = ZB_EXT_NEIGHBOR_TABLE_SIZE;
#endif
}

#ifndef ZB_ED_ROLE
void zb_nwk_exneighbor_start() ZB_SDCC_REENTRANT
{
  zb_uint8_t cut;

  TRACE_MSG(TRACE_NWK1, ">>exnb_start", (FMT__0));

  if (ZG->nwk.neighbor.ext_neighbor_size)
  {
    /* already cut space for the ext neighbor */
    TRACE_MSG(TRACE_NWK1, "already started", (FMT__0));
    return;
  }

  if (ZG->nwk.neighbor.base_neighbor_used)
  {
    zb_ushort_t used = 0;
    zb_ushort_t hole = 0;
    zb_ushort_t i;

    TRACE_MSG(TRACE_NWK3, "pck base n; size %d, used %d", (FMT__D_D,
              ZG->nwk.neighbor.base_neighbor_size, ZG->nwk.neighbor.base_neighbor_used));

    /* pack base neighbor: move all used fields to the left, update addr_to_neighbor */
    for (i = 0 ;
         i < ZG->nwk.neighbor.base_neighbor_size
           && used < ZG->nwk.neighbor.base_neighbor_used ;
         ++i)
    {
      if (ZG->nwk.neighbor.base_neighbor[i].used)
      {
        used++;
        if (hole)
        {
          /* must use memcpy here: sdcc can't assign structures */
          ZB_MEMCPY(&ZG->nwk.neighbor.base_neighbor[i - hole], &ZG->nwk.neighbor.base_neighbor[i], sizeof(zb_neighbor_tbl_ent_t));
          ZG->nwk.neighbor.addr_to_neighbor[ZG->nwk.neighbor.base_neighbor[i].addr_ref] = i - hole;
          ZG->nwk.neighbor.base_neighbor[i].used = 0;
          hole--;
          TRACE_MSG(TRACE_NWK3, "i %d: new used %d, hole %d", (FMT__D_D_D, i, used, hole));
        }
        else
        {
          TRACE_MSG(TRACE_NWK3, "i %d: new used %d", (FMT__D_D, i, used));
        }
      }
      else
      {
        hole++;
        TRACE_MSG(TRACE_NWK3, "i %d: new hole %d", (FMT__D_D, i, hole));
      }
    }
  }

  /* Try to cut 1/2 of buffer or less if more then 1/2 buffer used already */
  cut = ZG->nwk.neighbor.base_neighbor_size / 2;
  TRACE_MSG(TRACE_NWK3, "cut %d", (FMT__D, cut));
  if (ZG->nwk.neighbor.base_neighbor_used > ZG->nwk.neighbor.base_neighbor_size - cut)
  {
    TRACE_MSG(TRACE_NWK3, "used %d, size %d - try clean", (FMT__D_D,
              ZG->nwk.neighbor.base_neighbor_used, ZG->nwk.neighbor.base_neighbor_size));
    base_neighbor_cleanup();
    TRACE_MSG(TRACE_NWK3, "after clean: used %d, size %d", (FMT__D_D,
              ZG->nwk.neighbor.base_neighbor_used, ZG->nwk.neighbor.base_neighbor_size));
  }
  if (ZG->nwk.neighbor.base_neighbor_used > ZG->nwk.neighbor.base_neighbor_size - cut)
  {
    cut = ZG->nwk.neighbor.base_neighbor_size - ZG->nwk.neighbor.base_neighbor_used;
    TRACE_MSG(TRACE_NWK3, "cut %d", (FMT__D, cut));
  }
  ZG->nwk.neighbor.base_neighbor_size -= cut;
  TRACE_MSG(TRACE_NWK3, "new base n size %d", (FMT__D, (int)ZG->nwk.neighbor.base_neighbor_size));

  {
  /* let's align to 4 */
    zb_uint8_t off = MAGIC_ROUND_TO_4(ZG->nwk.neighbor.base_neighbor_size * sizeof(zb_neighbor_tbl_ent_t));
    cut = cut * sizeof(zb_neighbor_tbl_ent_t) - (off - ZG->nwk.neighbor.base_neighbor_size * sizeof(zb_neighbor_tbl_ent_t));
    ZG->nwk.neighbor.ext_neighbor = (zb_ext_neighbor_tbl_ent_t *)(((char *)(&ZG->nwk.neighbor.base_neighbor[0])) + off);
    ZG->nwk.neighbor.ext_neighbor_size = cut / sizeof(zb_ext_neighbor_tbl_ent_t);
    TRACE_MSG(TRACE_NWK3, "cut %d, off %d, ext n size %d ext n %p", (FMT__D_D_D_P,
              (int)cut, (int)off, (int)ZG->nwk.neighbor.ext_neighbor_size, ZG->nwk.neighbor.ext_neighbor));
  }
  ZB_ASSERT(ZG->nwk.neighbor.ext_neighbor_size > 0);

  TRACE_MSG(TRACE_NWK1, "<<exnb_start", (FMT__0));
}

void zb_nwk_exneighbor_stop(zb_uint16_t parent_short_addr) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_ushort_t i;

  TRACE_MSG(TRACE_NWK1, ">>exneighbor_stop", (FMT__0));

  ZVUNUSED(parent_short_addr);

  if (ZG->nwk.handle.joined)
  {
    /* If joined, move all records from my PAN to the base neighbor table */
    zb_address_pan_id_ref_t my_panid_ref;
    zb_address_ieee_ref_t addr_ref;
    zb_uint8_t free_size = 0;

    if ( zb_address_get_pan_id_ref(ZB_NIB_EXT_PAN_ID(), &my_panid_ref) != RET_OK )
    {
      /* Must extsts already!! */
      TRACE_MSG(TRACE_NWK1, "PAN id " TRACE_FORMAT_64 " not in panids arr - ?", (FMT__A,
                TRACE_ARG_64(ZB_NIB_EXT_PAN_ID())));
    }

    TRACE_MSG(TRACE_NWK3, "ext n used %d", (FMT__D, ZG->nwk.neighbor.ext_neighbor_used));
    for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
    {
      zb_neighbor_tbl_ent_t *ent = NULL;

      if (ZG->nwk.neighbor.ext_neighbor[i].panid_ref == my_panid_ref)
      {
        zb_ieee_addr_t long_address;

        TRACE_MSG(TRACE_NWK3, "i %d - my pan", (FMT__D, (int)i));

        /* search in the base neighbor: maybe, already exists? */
        if (ZG->nwk.neighbor.ext_neighbor[i].short_addr != (zb_uint16_t)~0)
        {
          if (!ZB_ADDRESS_COMPRESED_IS_ZERO(ZG->nwk.neighbor.ext_neighbor[i].long_addr))
          {
            /* both addresses are known */
            zb_ieee_addr_decompress(long_address, &ZG->nwk.neighbor.ext_neighbor[i].long_addr);
            ret = zb_address_update(long_address, ZG->nwk.neighbor.ext_neighbor[i].short_addr,
                                    ZB_FALSE, &addr_ref); /* don't lock address: keep locked */
            TRACE_MSG(TRACE_NWK3, "both addr, ret %d, ref %d", (FMT__D_D, ret, addr_ref));
          }
          else
          {
            /* have only short */
            ret = zb_address_by_short(ZG->nwk.neighbor.ext_neighbor[i].short_addr,
                                      ZB_TRUE, ZB_FALSE, &addr_ref);
            TRACE_MSG(TRACE_NWK3, "short addr, ret %d, ref %d", (FMT__D_D, ret, addr_ref));
          }
        }
        else
        {
          /* have only long */
          zb_ieee_addr_decompress(long_address, &ZG->nwk.neighbor.ext_neighbor[i].long_addr);
          ret = zb_address_by_ieee(long_address, ZB_TRUE, ZB_FALSE, &addr_ref);
          TRACE_MSG(TRACE_NWK3, "long addr, ret %d, ref %d", (FMT__D_D, ret, addr_ref));
        }

        if (ret == RET_OK)
        {
          if (ZG->nwk.neighbor.addr_to_neighbor[addr_ref] != (zb_uint8_t)~0)
          {
            /* entry exists - update */
            ent = &ZG->nwk.neighbor.base_neighbor[ZG->nwk.neighbor.addr_to_neighbor[addr_ref]];
            TRACE_MSG(TRACE_NWK3, "upd ent %p for a_ref %d", (FMT__P_D, ent, addr_ref));
            ZB_ASSERT(ent->used);
            if (ent->device_type == ZB_NWK_DEVICE_TYPE_NONE)
            {
              ent->device_type = ZG->nwk.neighbor.ext_neighbor[i].device_type;
              TRACE_MSG(TRACE_NWK3, "set dev_type %d", (FMT__D, ent->device_type));
            }
          }
          else
          {
            if (ZG->nwk.neighbor.base_neighbor_used == ZG->nwk.neighbor.base_neighbor_size)
            {
              TRACE_MSG(TRACE_NWK3, "try to clean", (FMT__0));
              base_neighbor_cleanup();
              TRACE_MSG(TRACE_NWK3, "after clean sz %d, used %d", (FMT__D_D,
                        ZG->nwk.neighbor.base_neighbor_size, ZG->nwk.neighbor.base_neighbor_used));
            }
            if (ZG->nwk.neighbor.base_neighbor_used < ZG->nwk.neighbor.base_neighbor_size)
            {
              ent = &ZG->nwk.neighbor.base_neighbor[ZG->nwk.neighbor.base_neighbor_used];
              ZG->nwk.neighbor.addr_to_neighbor[addr_ref] = ZG->nwk.neighbor.base_neighbor_used;
              ZG->nwk.neighbor.base_neighbor_used++;
              ZB_BZERO(ent, sizeof(*ent));
              /* Not sure about ZE but other devices are always ON. No more
               * info at this point. */
              ent->device_type = ZG->nwk.neighbor.ext_neighbor[i].device_type;
              ent->rx_on_when_idle = (ent->device_type != ZB_NWK_DEVICE_TYPE_ED);
              ent->depth = ZG->nwk.neighbor.ext_neighbor[i].depth;
              ent->permit_joining = ZG->nwk.neighbor.ext_neighbor[i].permit_joining;
              TRACE_MSG(TRACE_NWK3, "new ent %p, used %d, r.w.i %d, dev_t %d", (FMT__P_D_D_D,
                        ent, ZG->nwk.neighbor.base_neighbor_used, ent->rx_on_when_idle, ent->device_type));
              /* relationship is unknown. */
              ent->relationship = ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE;
            }
            else
            {
              /* no space available! */
              TRACE_MSG(TRACE_ERROR, "No spc for base ent!", (FMT__0));
            }
          }
          if (ent)
          {
            ent->used = 1;
            ent->addr_ref = addr_ref;
            ent->lqi = ZG->nwk.neighbor.ext_neighbor[i].lqi;
          } /* if ent */
        } /* if ok */

      } /* if my pan */

      /* Increase base neighbor table size to have space for ext neighbor
       * elements move. */
      free_size += sizeof(zb_ext_neighbor_tbl_ent_t);
      ZG->nwk.neighbor.base_neighbor_size += free_size / sizeof(zb_neighbor_tbl_ent_t);
      free_size -= free_size / sizeof(zb_neighbor_tbl_ent_t) * sizeof(zb_neighbor_tbl_ent_t);
      TRACE_MSG(TRACE_NWK3, "new base n size %d, rest %d", (FMT__D_D,
                ZG->nwk.neighbor.base_neighbor_size, free_size));
      ZB_ASSERT(ZG->nwk.neighbor.base_neighbor_size <= ZB_NEIGHBOR_TABLE_SIZE);

    }   /* for i */
  }     /* if joined */


  /* Ignore free_size calculations: be simpler and set initial neighbor table
   * size */
  ZG->nwk.neighbor.base_neighbor_size = ZB_NEIGHBOR_TABLE_SIZE;
  ZG->nwk.neighbor.ext_neighbor_used = 0;
  ZG->nwk.neighbor.ext_neighbor_size = 0;
  ZG->nwk.neighbor.ext_neighbor = NULL;
  TRACE_MSG(TRACE_NWK3, "restore base n size %d", (FMT__D, (int)ZG->nwk.neighbor.base_neighbor_size));
  TRACE_MSG(TRACE_NWK3, "base_neighbor_used %d", (FMT__D, (int)ZG->nwk.neighbor.base_neighbor_used));

  TRACE_MSG(TRACE_NWK1, "<<exneighbor_stop", (FMT__0));
}
#endif /* ZB_ED_ROLE */

#ifdef ZB_ED_ROLE
void zb_nwk_exneighbor_stop(zb_uint16_t parent_short_addr) ZB_SDCC_REENTRANT
{
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_NWK1, ">>exneighbor_stop", (FMT__0));

  if ( ZG->nwk.handle.joined
       && parent_short_addr != (zb_uint16_t)(-1)
       && zb_address_by_short(parent_short_addr, ZB_TRUE, ZB_FALSE, &addr_ref) == RET_OK )
  {
    /* If joined, move parent record to the base neighbor table */
    zb_address_pan_id_ref_t my_panid_ref;
    zb_ushort_t i;

    if ( zb_address_get_pan_id_ref(ZB_NIB_EXT_PAN_ID(), &my_panid_ref) != RET_OK )
    {
      /* Must extsts already!! */
      TRACE_MSG(TRACE_NWK1, "PAN id " TRACE_FORMAT_64 " not in panids arr - ?", (FMT__A, TRACE_ARG_64(ZB_NIB_EXT_PAN_ID())));
    }

    TRACE_MSG(TRACE_NWK3, "ext n used %d", (FMT__D, ZG->nwk.neighbor.ext_neighbor_used));
    for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
    {
      zb_neighbor_tbl_ent_t *ent = NULL;
      zb_ext_neighbor_tbl_ent_t *eent = &ZG->nwk.neighbor.ext_neighbor[i];

      if ( eent->short_addr == parent_short_addr
           && eent->panid_ref == my_panid_ref)
      {
        ent = &ZG->nwk.neighbor.base_neighbor[0];
        ZG->nwk.neighbor.addr_to_neighbor[addr_ref] = 0;
        ZG->nwk.neighbor.base_neighbor_used = 1;
        ZB_BZERO(ent, sizeof(*ent));
        /* Not sure about ZE but other devices are always ON. No more
         * info at this point. */

        ent->depth = eent->depth;
        ent->permit_joining = eent->permit_joining;
        ent->device_type = eent->device_type;
        ent->lqi = eent->lqi;
        ent->rx_on_when_idle = (ent->device_type != ZB_NWK_DEVICE_TYPE_ED);
        ent->used = 1;
        /* relationship is unknown. */
        ent->relationship = ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE;
        ent->addr_ref = addr_ref;

        TRACE_MSG(TRACE_NWK3, "new ent %p, used %d, r.w.i %d, dev_t %d", (FMT__P_D_D_D,
                        ent, ZG->nwk.neighbor.base_neighbor_used, ent->rx_on_when_idle, ent->device_type));

        break;
      }
    }
  }
  ZG->nwk.neighbor.ext_neighbor_used = 0;

  TRACE_MSG(TRACE_NWK1, "<<exneighbor_stop", (FMT__0));
}
#endif


zb_ret_t zb_nwk_exneighbor_by_short(zb_address_pan_id_ref_t panid_ref, zb_uint16_t short_addr,
                                    zb_ext_neighbor_tbl_ent_t **enbt) ZB_SDCC_REENTRANT
{
  zb_ret_t ret;
  zb_ushort_t i;

  TRACE_MSG(TRACE_NWK1, ">>exnb_by_short pan %d, addr %d, enbt %p", (FMT__D_D_P,
            (int)panid_ref, short_addr, enbt));

  for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
  {
    zb_ext_neighbor_tbl_ent_t *en = &ZG->nwk.neighbor.ext_neighbor[i];
    if (en->short_addr == short_addr
        && en->panid_ref == panid_ref)
    {
      *enbt = en;
      TRACE_MSG(TRACE_NWK3, "found ent # %d", (FMT__D, i));
      ret = RET_OK;
      goto fin;
    }
  }
  TRACE_MSG(TRACE_NWK3, "try alloc", (FMT__0));
  ret = alloc_new_extneiboard(panid_ref, short_addr, enbt);
fin:
  TRACE_MSG(TRACE_NWK1, "<<exnb_by_short %d", (FMT__D, ret));

  return ret;
}


#ifndef ZB_LIMITED_FEATURES
zb_ret_t zb_nwk_exneighbor_by_ieee(zb_address_pan_id_ref_t panid_ref, zb_ieee_addr_t long_addr, zb_ext_neighbor_tbl_ent_t **enbt) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_ushort_t i;
  zb_ieee_addr_compressed_t compressed_addr;

  TRACE_MSG(TRACE_NWK1, ">>exnb_by_ieee pan %d addr %p enbt %p", (FMT__D_P_P,
            panid_ref, long_addr, enbt));
  zb_ieee_addr_compress(long_addr, &compressed_addr);

  for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
  {
    if (ZG->nwk.neighbor.ext_neighbor[i].panid_ref == panid_ref
        && ZB_ADDRESS_COMPRESSED_CMP(ZG->nwk.neighbor.ext_neighbor[i].long_addr, compressed_addr))
    {
      TRACE_MSG(TRACE_NWK3, "found ent # %d", (FMT__D, i));
      *enbt = &ZG->nwk.neighbor.ext_neighbor[i];
      ret = RET_OK;
    }
  }
  if (ret == RET_NOT_FOUND)
  {
    TRACE_MSG(TRACE_NWK3, "try alloc new ent", (FMT__0));
    ret = alloc_new_extneiboard(panid_ref, (zb_uint16_t)~0, enbt);
    if (ret == RET_OK)
    {
      zb_ieee_addr_compress(long_addr, &(*enbt)->long_addr);
    }
  }

  TRACE_MSG(TRACE_NWK1, "<<exnb_by_ieee %d", (FMT__D, ret));

  return ret;
}
#endif

/**
   Allocate new extended neighbor table entry

   @return RET_OK if ok, RET_NOT_FOUND if no more free entries.
 */
static zb_ret_t alloc_new_extneiboard(zb_address_pan_id_ref_t panid_ref, zb_uint16_t short_addr, zb_ext_neighbor_tbl_ent_t **enbt)  ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_NWK1, ">>alloc_new_extnb pan %d addr %d enbt %p", (FMT__D_D_P,
            panid_ref, short_addr, enbt));

  if (ZG->nwk.neighbor.ext_neighbor_used == ZG->nwk.neighbor.ext_neighbor_size)
  {
    TRACE_MSG(TRACE_ERROR, "No space in ext neighbor table", (FMT__0));
    ret = RET_NO_MEMORY;
  }
  else
  {
    zb_ext_neighbor_tbl_ent_t *en;
    en = &ZG->nwk.neighbor.ext_neighbor[ZG->nwk.neighbor.ext_neighbor_used];
    ZG->nwk.neighbor.ext_neighbor_used++;
    TRACE_MSG(TRACE_NWK3, "allocated ext ent %p new used %hd", (FMT__P_D, en, ZG->nwk.neighbor.ext_neighbor_used));
    ZB_BZERO(en, sizeof(zb_ext_neighbor_tbl_ent_t));
    en->short_addr = short_addr;
    en->panid_ref = panid_ref;
    *enbt = en;
  }

  TRACE_MSG(TRACE_NWK1, "<<alloc_new_extnb %d", (FMT__D, ret));
  return ret;
}


zb_ret_t zb_nwk_neighbor_delete(zb_address_ieee_ref_t ieee_ref)
{
#ifndef ZB_LIMITED_FEATURES2
  zb_ret_t ret = RET_NOT_FOUND;
  zb_uint8_t n = ZG->nwk.neighbor.addr_to_neighbor[ieee_ref];
  TRACE_MSG(TRACE_NWK1, ">>nb_del ieee %d", (FMT__D, ieee_ref));

  if ( n != (zb_uint8_t)-1 )
  {
    ZG->nwk.neighbor.addr_to_neighbor[ieee_ref] = (zb_uint8_t)-1;
    ZG->nwk.neighbor.base_neighbor[n].used = 0;
    ret = RET_OK;
  }

  TRACE_MSG(TRACE_NWK1, "<<nb_del %d", (FMT__D, ret));
  return ret;
#else
  return RET_OK;
#endif
}


/**
   Cleanup base heighbor table: remove not useful entries
 */
#ifndef ZB_ED_ROLE
static void base_neighbor_cleanup()
{
  TRACE_MSG(TRACE_NWK1, "base_nb_cl", (FMT__0));
}
#endif

zb_ret_t zb_nwk_neighbor_get(zb_address_ieee_ref_t addr, zb_bool_t create_if_absent, zb_neighbor_tbl_ent_t **nbt) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_uint8_t n = ZG->nwk.neighbor.addr_to_neighbor[addr];

  TRACE_MSG(TRACE_NWK1, ">>zb_nwk_neighbor_get addr_ref %hd cr %hd nbt %p", (FMT__H_H_P,
            addr, create_if_absent, nbt));

  if (n == (zb_uint8_t)-1)
  {
    if (create_if_absent)
    {
      if (ZG->nwk.neighbor.base_neighbor_used == ZG->nwk.neighbor.base_neighbor_size - 1)
      {
        ret = RET_NO_MEMORY;
      }
      else
      {
        zb_address_lock(addr);
        n = ZG->nwk.neighbor.base_neighbor_used;
        ZG->nwk.neighbor.addr_to_neighbor[addr] = n;
        ZG->nwk.neighbor.base_neighbor_used++;
        {
          zb_neighbor_tbl_ent_t *bn = &ZG->nwk.neighbor.base_neighbor[n];
          ZB_BZERO(bn, sizeof(zb_neighbor_tbl_ent_t));
          bn->used = 1;
          bn->addr_ref = addr;
          bn->relationship = ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE;
          /* unknown - see table 2.126*/
          bn->permit_joining = 2;
        /* set our depth by default (not sure it is always right) */
#if defined ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN && defined ZB_ROUTER_ROLE
          bn->depth = ZG->nwk.nib.depth;
#endif
          *nbt = bn;
        }
      }
    }
    else
    {
      ret = RET_NOT_FOUND;
    }
  }
  else
  {
    *nbt = &ZG->nwk.neighbor.base_neighbor[n];
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nwk_neighbor_get %d", (FMT__D, ret));
  return ret;
}


#ifndef ZB_LIMITED_FEATURES2
zb_ret_t zb_nwk_neighbor_ext_to_base_tmp(zb_ext_neighbor_tbl_ent_t *ext_ent) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_NWK1, ">>nb_ext_to_base_tmp %p", (FMT__P, ext_ent));

  /* TODO: check if it's ok */
#ifdef ZB_ED_ROLE
  ZG->nwk.neighbor.base_neighbor_used = 0;
#else
  if ( ZG->nwk.neighbor.base_neighbor_used >= ZG->nwk.neighbor.base_neighbor_size )
  {
    ret = RET_NO_MEMORY;
  }
  else
#endif
  {
    zb_ieee_addr_t long_address;
    zb_address_ieee_ref_t addr_ref;

#ifdef ZB_ED_ROLE
    zb_neighbor_tbl_ent_t *ent = &ZG->nwk.neighbor.base_neighbor[0];
#else
    zb_neighbor_tbl_ent_t *ent = &ZG->nwk.neighbor.base_neighbor[ZG->nwk.neighbor.base_neighbor_used];
#endif

    ZB_ADDRESS_DECOMPRESS(long_address, ext_ent->long_addr);
    zb_address_by_ieee(long_address, ZB_TRUE, ZB_FALSE, &addr_ref);
    ZG->nwk.neighbor.addr_to_neighbor[addr_ref] = ZG->nwk.neighbor.base_neighbor_used;

    ZG->nwk.neighbor.base_neighbor_used++;
    ZB_BZERO(ent, sizeof(*ent));
    ent->device_type = ext_ent->device_type;
    ent->rx_on_when_idle = (ent->device_type != ZB_NWK_DEVICE_TYPE_ED);
    TRACE_MSG(TRACE_NWK3, "new ent %p, used %d, r.w.i %d, dev_t %d", (FMT__P_D_D_D,
                           ent, ZG->nwk.neighbor.base_neighbor_used, ent->rx_on_when_idle, ent->device_type));
    /* relationship is unknown. */
    ent->relationship = ZB_NWK_RELATIONSHIP_NONE_OF_THE_ABOVE;
    ent->used = 1;
    ent->addr_ref = addr_ref;
    ent->lqi = ext_ent->lqi;
  }

  TRACE_MSG(TRACE_NWK1, "<<nb_ext_to_base_tmp %d", (FMT__D, ret));
  return ret;
}
#endif


void zb_nwk_neighbor_clear()
{
  ZB_BZERO(&ZG->nwk.neighbor, sizeof(ZG->nwk.neighbor));

  zb_nwk_neighbor_init();
}

zb_ret_t zb_nwk_neighbor_get_by_short(zb_uint16_t short_addr, zb_neighbor_tbl_ent_t **nbt) ZB_SDCC_REENTRANT
{
  zb_address_ieee_ref_t addr_ref;

  if (zb_address_by_short(short_addr, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK
#ifndef ZB_ED_ROLE
      && zb_nwk_neighbor_get(addr_ref, ZB_FALSE, nbt) == RET_OK
#else
      && ZG->nwk.neighbor.base_neighbor[0].addr_ref == addr_ref
#endif
    )
  {
#ifdef ZB_ED_ROLE
    *nbt = &ZG->nwk.neighbor.base_neighbor[0];
#endif
    return RET_OK;
  }
  return RET_NOT_FOUND;
}

zb_ret_t zb_nwk_neighbor_get_by_ieee(zb_ieee_addr_t long_addr, zb_neighbor_tbl_ent_t **nbt)
{
  zb_address_ieee_ref_t addr_ref;

  if (zb_address_by_ieee(long_addr, ZB_FALSE, ZB_FALSE, &addr_ref) == RET_OK
#ifndef ZB_ED_ROLE
      && zb_nwk_neighbor_get(addr_ref, ZB_FALSE, nbt) == RET_OK
#else
      && ZG->nwk.neighbor.base_neighbor[0].addr_ref == addr_ref
#endif
    )
  {
#ifdef ZB_ED_ROLE
    *nbt = &ZG->nwk.neighbor.base_neighbor[0];
#endif
    return RET_OK;
  }
  return RET_NOT_FOUND;
}


#ifdef ZB_SECURITY

void zb_nwk_neighbor_incoming_frame_counter_clock(zb_uint8_t key_seq_number)
{
  if (ZG->nwk.neighbor.base_neighbor_size)
  {
#ifndef ZB_ED_ROLE
    zb_short_t i = ZG->nwk.neighbor.incoming_frame_counter_clock =
      (ZG->nwk.neighbor.incoming_frame_counter_clock + 1) % ZG->nwk.neighbor.base_neighbor_size;
#define I i
#else
#define I 0
#endif
    zb_int16_t diff = key_seq_number - ZG->nwk.neighbor.base_neighbor[I].key_seq_number;

    if (diff < 0)
    {
      diff = 256 + diff;
    }
    if (diff > 3)
    {
      ZG->nwk.neighbor.base_neighbor[I].incoming_frame_counter = 0;
      ZG->nwk.neighbor.base_neighbor[I].key_seq_number = key_seq_number;
    }
  }
}

#endif

#ifndef ZB_ED_ROLE
zb_ushort_t zb_nwk_neighbor_next_rx_on_i(zb_ushort_t i)
{
  while (i < ZG->nwk.neighbor.base_neighbor_used
         && !(ZG->nwk.neighbor.base_neighbor[i].used
              && ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle))
  {
    TRACE_MSG(TRACE_SECUR2, "skip nb ent %hd addr_ref %hd used %hd rel %hd dev_t %hd rx_on %hd",
              (FMT__H_P_H_H_H_H,
               (zb_uint8_t)i, &ZG->nwk.neighbor.base_neighbor[i],
               (zb_uint8_t)ZG->nwk.neighbor.base_neighbor[i].addr_ref,
               (zb_uint8_t)ZG->nwk.neighbor.base_neighbor[i].used,
               (zb_uint8_t)ZG->nwk.neighbor.base_neighbor[i].relationship,
               (zb_uint8_t)ZG->nwk.neighbor.base_neighbor[i].device_type,
               (zb_uint8_t)ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle));
    i++;
  }
  if (i >= ZG->nwk.neighbor.base_neighbor_used)
  {
    i = ~0;
  }
  TRACE_MSG(TRACE_SECUR2, "<< zb_nwk_neighbor_next_ze_children_i next ze %hd", (FMT__H, i));
  return i;
}


zb_ushort_t zb_nwk_neighbor_next_ze_children_i(zb_uint16_t addr, zb_ushort_t i)
{
  while (i < ZG->nwk.neighbor.base_neighbor_used
         && !(ZG->nwk.neighbor.base_neighbor[i].used
              && (ZG->nwk.neighbor.base_neighbor[i].relationship == ZB_NWK_RELATIONSHIP_CHILD
                  || ZG->nwk.neighbor.base_neighbor[i].relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD)
              && ZG->nwk.neighbor.base_neighbor[i].device_type == ZB_NWK_DEVICE_TYPE_ED
              && (addr == ZB_NWK_BROADCAST_ALL_DEVICES
                  || (addr == ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE
                      && ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle))) )
  {
    i++;
  }
  if ( i >= ZG->nwk.neighbor.base_neighbor_used )
  {
    i = ~0;
  }
  TRACE_MSG(TRACE_SECUR3, "next ze %hd", (FMT__H, i));
  return i;
}

zb_ushort_t zb_nwk_neighbor_next_ze_children_rx_off_i(zb_ushort_t i)
{
  while (i < ZG->nwk.neighbor.base_neighbor_used
         && !(ZG->nwk.neighbor.base_neighbor[i].used
              && (ZG->nwk.neighbor.base_neighbor[i].relationship == ZB_NWK_RELATIONSHIP_CHILD
                  || ZG->nwk.neighbor.base_neighbor[i].relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD)
              && !ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle) )
  {
    i++;
  }
  if ( i >= ZG->nwk.neighbor.base_neighbor_used )
  {
    i = ~0;
  }
  TRACE_MSG(TRACE_SECUR3, "next ze %hd", (FMT__H, i));
  return i;
}
#endif

/*! @} */
