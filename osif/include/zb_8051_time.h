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
PURPOSE: 8051-specific timer implementation
*/

#ifndef ZB_8051_TIME_H
#define ZB_8051_TIME_H 1

/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#ifdef ZB_8051_TIMER
/**
   \par Time implementation based on HW timer in 8051.

   The idea is to use timer interrupt.

   All that functions are internal timer implementation.
   See zb_scheduler.h for the public timer API.
 */

/*
  1 operation time = 1/ZB_XTAL_FREQ
  ticks count (per 1 msec) = 1 msec / operation_time (msec)

*/

/* for SDCC: handler prototype MUST be defined in the same file with main() function */
#define DECLARE_TIMER0_INTER_HANDLER() \
  void timer0_inter_handler(void) INTERRUPT_DEFINITION(TIMER0_INTER_NUMBER, REGISTER_BANK_3);
  

/*
It seems all timeouts in MAC are multiples for beacon interval. Maximal value of
timeout in MAC is (2^14 + 1) = 16385 beacon intervals. 1 beacon interval = 15.36 ms
and it is calculated as follows:
1 beacon interval = aBaseSuperframeDuration * symbol duration
aBaseSuperframeDuration = aBaseSlotDuration * aNumSuperframeSlots
aBaseSlotDuration = 60
aNumSuperframeSlots = 16
1 symbol = 16e-6 sec
8051 with 24.5 MHz oscilator with 16-bit timer maximal time interval is 128 ms
(with system clock division by 48) it is nearly equal to 8 beacon intervals,
and 32 ms with system clock division by 12.

So, the idea is to use 1 beacon interval to measure timeout interval. Timer is set
to fire timer interrupt handler every 15.36 ms and internal counter is used to measure
time interval in "beacon intervals".
*/

/* timer value = (15360 * Oscillator MHz) / (clock divider) */
#ifdef C8051F120
#define ZB_SYSTEM_OSCILLATOR_DIVIDER 48
#elif defined ZB_CC25XX
#define ZB_SYSTEM_OSCILLATOR_DIVIDER 128
#else
#define ZB_SYSTEM_OSCILLATOR_DIVIDER 2
#endif
/* TODO: check me for TI_CC2530 */
#ifdef ZB_CC25XX
#define ZB_8051_TIMER_VALUE (zb_uint16_t)(((zb_uint16_t)ZB_BEACON_INTERVAL_USEC / ZB_SYSTEM_OSCILLATOR_DIVIDER) * ZB_SHORT_XTAL_FREQ)
#else
#define ZB_8051_TIMER_VALUE (ZB_MAX_TIME_VAL - (zb_uint16_t)(((zb_uint16_t)ZB_BEACON_INTERVAL_USEC / ZB_SYSTEM_OSCILLATOR_DIVIDER) * ZB_SHORT_XTAL_FREQ))
#endif

#define ZB_TIMER_LOW_BYTE ZB_GET_LOW_BYTE(ZB_8051_TIMER_VALUE)
#define ZB_TIMER_HI_BYTE  ZB_GET_HI_BYTE(ZB_8051_TIMER_VALUE)

#define ZB_TIMER_INIT() zb_8051_init_timer()

#define ZB_CHECK_TIMER_IS_ON() ((TR0) == 1) /* TRUE if timer is ON */

/* start timer 0 */
#ifndef ZB_CC25XX
#define ZB_START_8051_TIMER()                                 \
  (TL0 = ZB_TIMER_LOW_BYTE, TH0 = ZB_TIMER_HI_BYTE, TR0 = 1)
#define ZB_STOP_8051_TIMER() (TR0 = 0)
#else 
#define ZB_START_8051_TIMER()                                 \
  (  T1CC0L  = ZB_TIMER_LOW_BYTE,  T1CC0H  = ZB_TIMER_HI_BYTE, T1CTL |=(0x02))
#define ZB_STOP_8051_TIMER() (T1CTL &=~(0x02))
#endif 

#define ZB_START_HW_TIMER() ZB_START_8051_TIMER()


void zb_8051_init_timer();


#endif  /* ZB_8051_TIMER */

/*! @} */

#endif /* ZB_8051_TIME_H */
