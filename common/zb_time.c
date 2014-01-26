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
PURPOSE: time functions implementation for 8051 for the Common bank
*/


#include "zb_common.h"
#include "zb_osif.h"
#include "zb_time.h"

/*! \addtogroup ZB_BASE */
/*! @{ */

#include "zb_bank_common.h"

void zb_timer_start(zb_time_t timeout)
{
  zb_time_t t_cur = ZB_TIMER_GET();
  zb_time_t t = ZB_TIME_ADD(t_cur, timeout);

  ZB_DISABLE_ALL_INTER();
  if (!ZB_TIMER_CTX().started
#ifdef ZB8051
      || ZB_TIME_GE(t, ZB_TIMER_CTX().timer_stop)
#else
      || !ZB_TIME_GE(t, ZB_TIMER_CTX().timer_stop)
      || !ZB_TIME_GE(ZB_TIMER_CTX().timer_stop, t_cur)
#endif
    )
  {
    ZB_TIMER_CTX().timer_stop = t;
    ZB_TIMER_CTX().started = 1;
  }
  ZB_ENABLE_ALL_INTER();

  if (!ZB_CHECK_TIMER_IS_ON())
  {
    /* timer is stopped - start it */
    ZB_START_HW_TIMER();
  }
#if 0
#ifdef ZB8051
  TRACE_MSG(TRACE_OSIF3, "tmo %d, stop at %d; t 0x%x hi 0x%x lo 0x%x freq %d",
            (FMT__D_D_D_D_D_D, (int)timeout, (int)ZB_TIMER_CTX().timer_stop, (zb_uint16_t)ZB_8051_TIMER_VALUE, (zb_uint16_t)ZB_TIMER_HI_BYTE, (zb_uint16_t)ZB_TIMER_LOW_BYTE, (int)ZB_SHORT_XTAL_FREQ));
#else
  TRACE_MSG(TRACE_OSIF3, "tmo %d, stop at %d", (FMT__D_D, (int)timeout, (int)ZB_TIMER_CTX().timer_stop));
#endif
#endif
}


void zb_timer_stop_async()
{
  ZB_TIMER_CTX().started = 0;
#ifdef ZB_NS_BUILD
  TRACE_MSG(TRACE_MAC3, "stop async timer, started = 0", (FMT__0));
#endif
}

/*! @} */
