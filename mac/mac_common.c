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
PURPOSE: MAC functions to be put into common bank
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_ubec24xx.h"
#include "zb_mac_globals.h"


/*! \addtogroup ZB_MAC */
/*! @{ */


/* Both MAC and NWK placed to the bank1, so all MAC calls are not banked */
#include "zb_bank_2.h"


/**
   Parses packed mhr header, fills mhr structure
   @param  mhr - out pointer to mhr structure
   @param  ptr - pointer to packed mhr header buffer
   return packed mhr buffer length
*/
zb_uint8_t zb_parse_mhr(zb_mac_mhr_t *mhr, zb_uint8_t *ptr)
{
  zb_uint8_t *ptr_init = ptr;
  zb_uint8_t val;

  TRACE_MSG(TRACE_MAC2, ">>zb_parse_mhr mhr %p, ptr %p", (FMT__P_P, mhr, ptr));
  ZB_BZERO(mhr, sizeof(zb_mac_mhr_t));
  ZB_MEMCPY(mhr->frame_control, ptr, 2);
  ptr += sizeof(zb_uint16_t);
  mhr->seq_number = *ptr;
  ptr += sizeof(zb_uint8_t);


  /* mac spec 7.2.1.1.6 Destination Addressing Mode subfield */
  val = ZB_FCF_GET_DST_ADDRESSING_MODE(mhr->frame_control);
  TRACE_MSG(TRACE_MAC2, "dst addr mode: %hd", (FMT__H, val));
  if (val)
  {
    zb_get_next_letoh16(&mhr->dst_pan_id, &ptr);

    /* dst addr mode: ZB_ADDR_NO_ADDR, ZB_ADDR_16BIT_DEV_OR_BROADCAST or ZB_ADDR_64BIT_DEV */
    if (val == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
    {
      zb_get_next_letoh16(&mhr->dst_addr.addr_short, &ptr);
    }
    else
    {
      ZB_LETOH64(&mhr->dst_addr.addr_long, ptr);
      ptr += sizeof(zb_ieee_addr_t);
    }
  }

  /* mac spec 7.2.1.1.8 Source Addressing Mode subfield */
  val = ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control);
  TRACE_MSG(TRACE_MAC2, "src addr mode: %hd", (FMT__H, val));
  if (val)
  {
    if (!ZB_FCF_GET_PANID_COMPRESSION_BIT(mhr->frame_control))
    {
      zb_get_next_letoh16(&mhr->src_pan_id, &ptr);
    }
    else
    {
      mhr->src_pan_id = mhr->dst_pan_id;
    }

    /* dst addr mode: ZB_ADDR_NO_ADDR, ZB_ADDR_16BIT_DEV_OR_BROADCAST or ZB_ADDR_64BIT_DEV */
    if (val == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
    {
      zb_get_next_letoh16(&mhr->src_addr.addr_short, &ptr);
    }
    else
    {
      ZB_LETOH64(&mhr->src_addr.addr_long, ptr);
      ptr += sizeof(zb_ieee_addr_t);
    }
  }

#ifdef ZB_MAC_SECURITY
  val = ZB_FCF_GET_SECURITY_BIT(mhr->frame_control);
  TRACE_MSG(TRACE_MAC2, "security: %hd", (FMT__H, val));
  if (val)
  {
    /*
      Aux security header is here.
      Its format (see 7.6.2 Auxiliary security header):
      1 - security control (bits 0-2 Security Level bits 3-4 Key Identifier  Mode)
      4 - Frame Counter
      0/1/5/9 Key Identifier
    */

    /* security control */
    mhr->security_level = (*ptr) & 7;
    mhr->key_id_mode = ((*ptr) >> 3) & 7;
    ptr++;
    /* Frame Counter */
    ZB_LETOH32((zb_uint8_t *)&mhr->frame_counter, ptr);
    ptr += 4;
    /* Key Identifier */
    switch (mhr->key_id_mode)
    {
      case 2:
        ZB_MEMCPY(mhr->key_source, ptr, 4);
        ptr += 4;
        break;
      case 3:
        ZB_MEMCPY(mhr->key_source, ptr, 8);
        ptr += 8;
        break;
    }
    if (mhr->key_id_mode)
    {
      mhr->key_index = *ptr;
      ptr++;
    }
  }
#endif

  val = ptr - ptr_init;
  TRACE_MSG(TRACE_MAC2, "<< zb_parse_mhr, ret val %i", (FMT__D, (int)val));
  return val;
}

#include "zb_zdo.h"
void zb_tx_total_inc()
{
  ZB_NIB_NWK_TX_TOTAL()++;
  TRACE_MSG(TRACE_MAC3, "tx_total %d", (FMT__D, ZB_NIB_NWK_TX_TOTAL()));
  if (ZB_NIB_NWK_TX_TOTAL() == 0)
  {
    /* counter overflow, clear fail counter */
    ZB_NIB_NWK_TX_FAIL() = 0;
  }
}

void zb_tx_fail_inc()
{
  ZB_NIB_NWK_TX_FAIL()++;
  TRACE_MSG(TRACE_MAC3, "nwk_tx_fail %hd", (FMT__H, ZB_NIB_NWK_TX_FAIL()));
#ifndef ZB_LIMITED_FEATURES
  if (!ZB_ZDO_GET_CHECK_FAILS())
  {
    ZB_SCHEDULE_CALLBACK(zb_zdo_check_fails, 0);
  }
#endif
}


/*! @} */
