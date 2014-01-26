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
PURPOSE: APS subsystem. Dups detection.
*/

#include "zb_common.h"
#include "zb_aps.h"
#include "aps_internal.h"
#include "zb_scheduler.h"
#include "zb_hash.h"

#include "zb_bank_6.h"

/*! \addtogroup ZB_APS */
/*! @{ */

#define APS_DUPS_DEBUG

void zb_aps_check_timer_cb(zb_uint8_t param) ZB_CALLBACK;


zb_short_t aps_check_dups(zb_uint16_t src_addr, zb_uint8_t aps_counter) ZB_SDCC_REENTRANT
{
#if defined ZB_LIMITED_FEATURES
  (void)src_addr;
  (void)aps_counter;
  return 0;
#else
  zb_address_ieee_ref_t addr_ref;
  zb_short_t is_dup = 0;

  if (zb_address_by_short(src_addr,
                          ZB_TRUE, /* create if absent (is it ever possible?) */
                          ZB_FALSE, /* do not lock. If entry will be expired,
                                     * that is its fatum. */
                          &addr_ref) == RET_OK)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[addr_ref];
    is_dup = (ent->aps_dup_clock != 0
              && ent->aps_dup_counter == aps_counter);

#ifdef APS_DUPS_DEBUG
    TRACE_MSG(TRACE_APS2, "dup detection: is_dup %hd addr 0x%x ref %hd aps_counter %hd clock %hd",
              (FMT__H_D_H_H_H,
               is_dup, src_addr, addr_ref,
               ent->aps_dup_counter,
               ent->aps_dup_clock));
#endif

    ent->aps_dup_counter = aps_counter;
    ent->aps_dup_clock = 3;

    if (!ZG->aps.dups_alarm_running)
    {
      ZG->aps.dups_alarm_running = 1;
      ZB_SCHEDULE_ALARM(zb_aps_check_timer_cb, 0, ZB_APS_DUP_CHECK_TIMEOUT);
    }
  }

  TRACE_MSG(TRACE_APS2, "-aps_check_dups ret %hd", (FMT__H, is_dup));
  return is_dup;
#endif
}


#ifndef ZB_LIMITED_FEATURES
/**
   Alarm callback for the APS dups detection.
 */
void zb_aps_check_timer_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_ushort_t n;
  ZVUNUSED(param);

  ZG->aps.dups_alarm_running = 0;
  for (i = n = 0 ; n < ZG->addr.n_elements ; ++i)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[i];
    if (ent->used)
    {
      n++;
      if (ent->aps_dup_clock)
      {
        ent->aps_dup_clock--;
        ZG->aps.dups_alarm_running |= (ent->aps_dup_clock != 0);
      }
    }
  }

  /* schedule myself again */
  if (ZG->aps.dups_alarm_running)
  {
    ZB_SCHEDULE_ALARM(zb_aps_check_timer_cb, 0, ZB_APS_DUP_CHECK_TIMEOUT);
  }
}
#endif

/*! @} */
