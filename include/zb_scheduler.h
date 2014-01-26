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
PURPOSE: Zigbee scheduler: cooperative multitasking.
*/

#ifndef ZB_SCHEDULER_H
#define ZB_SCHEDULER_H 1

#include "zb_osif.h"
#include "zb_list_macros.h"

/*! \addtogroup sched */
/*! @{ */

/**
   \par scheduler

Use cooperative multitasking.
Trivial scheduler: do all in callbacks.
No 'task' primitive.
Base primitive - callback call. Callback will be called indirectly, via scheduler.
Callback call can be treated as event send.
Callbacks schedule done via scheduler in the main scheduler loop.
Can pass 1 parameter (void*) to the callback.
Callback initiated using call schedule_callback(func, param).
Scheduling callback does not block currently running callback.
More then one callback can be scheduled. It will be called later, when current function will
return to the scheduler.

Before main loop call application-dependent initialization functions.
It can schedule some callbacks.
Callbacks will be called later, in the main loop.

Data structure for callbacks support - fixed-size ring buffer of callbacks control structure.
Callbacks served in FIFO order, no priorities.

When no callbacks to call, scheduler put device asleep (stop CPU for 8051, wait
inside select() for Linux); it can be waked by interrupt (8051) or data arrive
or timeout (Linux).

There are 2 possible kinds of routines: callbacks running in the main loop and interrupt handlers.
Interrupt handlers works with SPI, UART, timer, transiver interrupt (what else?).
Interrupt handler can't schedule callback call.

To work with data shared between interrupt handler and main loop introduced "global lock" operation.
It means interrupts disable when running not in the interrupt context.
In Linux it means either mutex lock or nothing (depending on i/o implementation).

 */

#include "zb_time.h"
#include "zb_ringbuffer.h"

/**
   Callback function typedef.
   Callback is function planned to execute by another function.
   Note that callback must be declared as reentrant for dscc.

   @param param - callback parameter - usually, but not always, ref to packet buf

   @return none.
 */
typedef void (ZB_CODE * zb_callback_t)(zb_uint8_t param) ZB_CALLBACK;



/**
   Immediate pending callbacks queue entry
 */
typedef struct zb_cb_q_ent_s
{
  zb_callback_t func;           /*!< function to call  */
  zb_uint8_t param;             /*!< parameter to pass to 'func'  */
} zb_cb_q_ent_t;

typedef struct zb_mac_cb_ent_s
{
 zb_callback_t func;   /* currently, it's the same as common queue, */
 zb_uint8_t param;     /* but, possibly, it's better to remove param from it */ 
}zb_mac_cb_ent_t;

/**
   Delayed (scheduled to run after timeout) callbacks queue entry.
 */
typedef struct zb_tm_q_ent_s
{
  zb_callback_t func;           /*!< function to call  */
  zb_uint8_t param;             /*!< parameter to pass to 'func'  */
  zb_time_t run_time;           /*!< time to run at  */
  /* TODO: implement list macros for indexed lists and use byte instead pointer here! */
  ZB_LIST_FIELD(struct zb_tm_q_ent_s *, next);
} zb_tm_q_ent_t;

typedef struct zb_buf_q_ent_s
{
  zb_callback_t func;           /*!< function to call  */
  ZB_SL_LIST_FIELD(struct zb_buf_q_ent_s *, next);
} zb_buf_q_ent_t;

/**
   Immediate pending callbacks queue (ring buffer)
 */
ZB_RING_BUFFER_DECLARE(zb_cb_q, zb_cb_q_ent_t, ZB_SCHEDULER_Q_SIZE);
/* Mac layer high priority queue */
/* ZB_RING_BUFFER_DECLARE(zb_mac_cb_q, zb_mac_cb_ent_t, ZB_MAC_QUEUE_SIZE);*/
ZB_RING_BUFFER_DECLARE(zb_mac_tx_q, zb_mac_cb_ent_t, ZB_MAC_QUEUE_SIZE);


/**
   Data structures for the delayed execution.
 */
typedef struct zb_sched_globals_s
{
  zb_cb_q_t cb_q;           /*!< immediate callbacks queue  */
#ifdef ZB_NS_BUILD 
  zb_uint8_t    mac_receive_pending;
#endif
  /* zb_mac_cb_q_t mac_cb_q; */ /* re-enable this, if planning to use high priority mac layer queue */
  zb_mac_tx_q_t mac_tx_q;	/* queue of callback's waiting for tx */
  zb_tm_q_ent_t tm_buffer[ZB_SCHEDULER_Q_SIZE]; /*!< buffer for the timer queue entries  */
  zb_buf_q_ent_t delayed_buf[ZB_BUF_Q_SIZE];
  /* TODO: implement list macros for indexed lists and use byte instead pointer here! */
  ZB_LIST_DEFINE(zb_tm_q_ent_t   *, tm_queue);    /*!< delayed callbacks queue  */
  ZB_STK_DEFINE(zb_tm_q_ent_t   *, tm_freelist); /*!< freelist of the timer queue entries  */
  ZB_SL_LIST_DEFINE(zb_buf_q_ent_t  *, inbuf_queue);
  ZB_SL_LIST_DEFINE(zb_buf_q_ent_t  *, outbuf_queue);
  ZB_STK_DEFINE(zb_buf_q_ent_t  *, buf_freelist);
} zb_sched_globals_t;

/**
   Initialize scheduler subsystem.
 */
void zb_sched_init() ZB_SDCC_REENTRANT;

/**
   Call all callbacks.
   All cooperative multitasking done here.

   Call all callbacks from the queue. Callbacks can schedule other callbacks, so
   potentially stay here infinite.
   In practice at some point callbacks ring buffer became empty.
   Put device into asleep waiting for interrupts (8051) or wait for data from
   other source (Linux).

   This function usually placed into main loop.

   This function MUST be reentrant in Keil: must not share its xdata segment with
   functions called from it by pointers.

   @return none
 */
void zb_sched_loop_iteration() ZB_SDCC_REENTRANT;

/**
   Schedule callback execution.
   Schedule execution of function `func' in the main scheduler loop.

   @param func - function to execute
   @param param - callback parameter - usually, but not always ref to packet buffer

   @return RET_OK or error code.
 */

zb_ret_t zb_schedule_callback(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT;

/** Just the similar to schedule callback function, but used for mac cb queue */
zb_ret_t zb_schedule_mac_cb(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT;

#define ZB_SCHEDULE_CALLBACK zb_schedule_callback

/* Schedule a callback, that should be called right after current tx finished
   Usually sheduled from callback, which directly sends data or command */
#define ZB_SCHEDULE_AFTER_TX_CB(cb) (MAC_CTX().tx_wait_cb = cb)
/* Schedules a high priority callback, all callbacks could be scheduled to 
this queue. Currently unused. */
#define ZB_SCHEDULE_MAC_CB zb_schedule_mac_cb
/* Schedules a callback, that requires NORMAL_FIFO for transfer or security operations,
it will be called after current tx finished or just during next scheduler loop */
#define ZB_SCHEDULE_TX_CB zb_schedule_tx_cb


/**
   Schedule alarm - callback to be executed after timeout.

   Function will be called via scheduler after timeout expired (maybe, plus some
   additional time).
   Timer resolution depends on implementation.
   Same callback can be scheduled for execution more then once.

   @param func - function to call via scheduler
   @param param - parameter to pass to the function
   @param timeout_bi - timeout, in beacon intervals
   @return RET_OK or error code
 */
zb_ret_t zb_schedule_alarm(zb_callback_t func, zb_uint8_t param, zb_time_t timeout_bi) ZB_SDCC_REENTRANT;

#define ZB_SCHEDULE_ALARM zb_schedule_alarm


/**
   Special parameter for zb_schedule_alarm_cancel(): cancel alarm once without
   parameter check

   Cancel only one alarm without check for parameter
 */
#define ZB_ALARM_ANY_PARAM (zb_uint8_t)(-1)
/**
   Special parameter for zb_schedule_alarm_cancel(): cancel alarm for all
   parameters
 */
#define ZB_ALARM_ALL_CB (zb_uint8_t)(-2)
#define ZB_SCHEDULE_ALARM_CANCEL zb_schedule_alarm_cancel

/**
   Cancel scheduled alarm.

   This function cancel previously scheduled alarm. Function is identified by
   the pointer.

   @param func - function to cancel
   @param param - parameter to cancel. \see ZB_ALARM_ANY_PARAM. \see ZB_ALARM_ALL_CB
   @return RET_OK or error code
 */
zb_ret_t zb_schedule_alarm_cancel(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Return true if scheduler has any pending callbacks
 */
#define ZB_SCHED_HAS_PENDING_CALLBACKS() !ZB_RING_BUFFER_IS_EMPTY(&ZG->sched.cb_q)


/**
   Wait (block, go idle) until condition will not be true.

   @param condition - condition to check for
 */
#define ZB_SCHED_WAIT_COND(condition)           \
do                                              \
{                                               \
  ZB_SCHED_GLOBAL_LOCK();                       \
  while ( !(condition) )                        \
  {                                             \
    ZB_SCHED_GLOBAL_UNLOCK();                   \
    ZB_GO_IDLE();                               \
    ZB_SCHED_GLOBAL_LOCK();                     \
  }                                             \
  ZB_SCHED_GLOBAL_UNLOCK();                     \
}                                               \
while(0)


/**
   Global lock operation
   Protect manupulation with queues in the main loop by this macro.
   It disables interrupts on 8051 device and locks mutex in Linux.
 */
#define ZB_SCHED_GLOBAL_LOCK ZB_OSIF_GLOBAL_LOCK


/**
   Global unlock operation
   Protect manupulation with queues by this macro.
   It enables interrupts on 8051 device and unlocks mutex in Linux.
 */
#define ZB_SCHED_GLOBAL_UNLOCK ZB_OSIF_GLOBAL_UNLOCK

/**
   Global lock operation - call from the interrupt handler

   @return RET_OK if success, RET_BUZY if locked by userspace
 */
#define ZB_SCHED_GLOBAL_LOCK_INT() ZB_OSIF_GLOBAL_LOCK_INT

/**
   Global unlock operation - call from the interrupt handler
 */
#define ZB_SCHED_GLOBAL_UNLOCK_INT() ZB_OSIF_GLOBAL_UNLOCK_INT

/* Schedules a callback, that requires NORMAL_FIFO for transfer or security operations,
it will be called after current tx finished or just during next scheduler loop */
zb_ret_t zb_schedule_tx_cb(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Flag to run mac main loop
 */

/*! @} */

#endif /* ZB_SCHEDULER_H */
