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
PURPOSE: Common definitions for time functionality
*/

#ifndef ZB_TIME_H
#define ZB_TIME_H 1

/*! \addtogroup time */
/*! @{ */

/**
   \par Timer functionality.

   The idea is: platform has some timer which can be stopped or run.
   When run, it increments (or decrements - depends on platform) some counter
   until counter overflow (underflow), then issues interrupt - wakeups main loop
   if it sleeping.
   Time stored in ticks; time resolution is platform dependent, its usual value
   is 15.36 usec - 1 beacon interval.
   Note that time type has limited capacity (usually 16 bits) and can overflow.
   Macros which works with time handles overflow. It is supposed that time values will
   not differ to more then 1/2 of the maximum time value.

   All that timer macros will not be used directly by the application code - it
   is scheduler internals. The only API for timer is ZB_SCHEDULE_ALARM() call.
 */


/**
   Timer type.

   16 bits for 8051 - it will be hw timer value.
   Not sure it is right to use 16 bits in Linux.
   But let's do it now to debug owerflow.
   In the future could use 32 bits in Linux.
 */
typedef zb_uint16_t zb_time_t;


/**
   Get current timer value (beacon intervals)
 */
#define ZB_TIMER_GET() (ZB_TIMER_CTX().timer)

/**
   Time subtraction: subtract 'b' from 'a'

   Take overflow into account: change sign (subtraction order) if result >
   values_diapasin/2.
   Suppose a always >= b, so result is never negative.
   This macro will be used to calculate, for example, amount of time to sleep
   - it is positive by definition.
   Do not use it to compare time values! Use ZB_TIME_GE() instead.
   Note that both a and b is of type @ref zb_time_t. Can't decrease time (subtract
   constant from it) using this macro.

   @param a - time to subtract from
   @param b - time to subtract
   @return subtraction result
 */
#define ZB_TIME_SUBTRACT(a, b) ((zb_time_t)((a) - (b)) < ZB_HALF_MAX_TIME_VAL ? (zb_time_t)((a) - (b)) : (zb_time_t)((b) - (a)))

/**
   Time add: add 'a' to 'b'

   Overflow is possible, but this is ok - it handled by subtraction and compare macros.

   @param a - time to add to
   @param b - value to add
   @return addition result
 */
#define ZB_TIME_ADD(a, b) (zb_time_t)((a) + (b))

/**
   Compare times a and b - check that a >= b

   Taking into account overflow and unsigned values arithmetic and supposing
   difference between a and b can't be > 1/2 of the overall time values
   diapason,
   a >= b only if a - b < values_diapason/2

   @param a - first time value to compare
   @param b - second time value to compare
   @return 1 is a >= b, 0 otherwhise
 */
#define ZB_TIME_GE(a, b) ((zb_time_t)((a) - (b)) < ZB_HALF_MAX_TIME_VAL)

/**
   \par Time measurement unit is beacon interval.
   It is both internal representation and value used in API.
   It is still possible to convert it to/from msec.
   1 beacon interval = aBaseSuperframeDuration * symbol duration
   aBaseSuperframeDuration = aBaseSlotDuration * aNumSuperframeSlots
   aBaseSlotDuration = 60
   aNumSuperframeSlots = 16
   1 symbol = 16e-6 sec (mac spec 6.5.3.2 Symbol rate)
*/
#define ZB_BEACON_INTERVAL_USEC 15360 /* in microseconds */

/**
 One second timeout
*/
#define ZB_TIME_ONE_SECOND ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000)
/**
  Convert time from beacon intervals to millisecinds

  Try to not cause overflow in 16-bit arithmetic (with some precision lost...)
*/
#define ZB_TIME_BEACON_INTERVAL_TO_MSEC(t) (ZB_BEACON_INTERVAL_USEC / 100 * (t) / 10)

/**
  Convert time from millisecinds to beacon intervals

  Try to not cause overflow in 16-bit arithmetic (with some precision lost...)
*/
#define ZB_MILLISECONDS_TO_BEACON_INTERVAL(ms) (((10l * (ms) + 3) / (ZB_BEACON_INTERVAL_USEC / 100)))


/**
   Start timer - assign time to sleep

   @param interval - time in internal forrmat to sleep before delayed callback run
 */
#define ZB_TIMER_START(interval) zb_timer_start(interval)

/*! @} */

/*! \cond internals_doc */
/*! \addtogroup ZB_BASE */
/*! @{ */

void zb_timer_start(zb_time_t timeout);
void zb_timer_stop_async();


#define ZB_MAX_TIME_VAL ZB_UINT16_MAX
#define ZB_HALF_MAX_TIME_VAL (ZB_MAX_TIME_VAL / 2)

/**
   Timer internals

   'timer' always ticks (if some timer is started), usually with overflow.
   Timer unit is beacon interval, for every system.
 */
typedef struct zb_timer_s
{
  zb_time_t timer_stop; /* time to stop timer (disable timer interrupts etc) */

  zb_time_t timer; /* current time, measured in beacon intervals */

  zb_uint8_t  started;
}
zb_timer_t;

#ifdef ZB_8051_TIMER
/* 8051, timer interrupts */
#include "zb_8051_time.h"
#endif

/*! @} */
/*! \endcond */

#endif /* ZB_TIME_H */
