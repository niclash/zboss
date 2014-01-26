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
PURPOSE: Zigbee scheduler.
*/

/*! \addtogroup sched */
/*! @{ */

#include "zb_common.h"
#include "zb_list_macros.h"

#include "zb_bank_common.h"

/**
   \par Scheduler and timer management.

   The idea is to be able to stop timer when nobody waits for it.
   That means, current time can't be checked by the application: it does not
   goes forward always.
   The only API is to run callback after some timeout.
   If scheduler have some delayed callback, we must run a timer, so scheduler
   can keep current time which is actual only when we have some delayed
   callbacks.

   Time management has platform-independent and platform-dependent parts.
   Scheduler interface and macros to operate with time are platform-independent.

   Time stored in platform-dependent units (ticks). It can overflow, so operate
   with time only using macros: it handles overflow. Overflow frequency depends
   on time type size and timer resolution and is platform-dependent.

   Timer start and examine are platform-dependent.

   Platform-dependent timer part provides macros to convert between
   platform-dependent time type ('raw' time) and milliseconds.
*/

#ifdef SDCC
static void sdcc_callf(zb_callback_t funcp, zb_uint8_t param)
{
  funcp(param);
}
#endif

void zb_sched_loop_iteration() ZB_SDCC_REENTRANT/* ZB_KEIL_REENTRANT */
{
  zb_uint8_t did_something;
  do
  {
    zb_time_t t = 0;
    did_something = 0;

    /* checking interrupt and processing all mac related routines */
    CHECK_INT_N_TIMER(); 
    zb_mac_main_loop();

    /* First execute regular (immediate) callbacks */
    {
      zb_cb_q_ent_t *ent;
      while ((ent = ZB_RING_BUFFER_PEEK(&ZG->sched.cb_q)) != NULL)
      {
        TRACE_MSG(TRACE_COMMON3, "cb_q written %hd, read_i %hd, write_i %hd", (FMT__H_H_H, ZG->sched.cb_q.written, ZG->sched.cb_q.read_i,ZG->sched.cb_q.write_i ));
        did_something = 1;
        if ((ZB_GET_TRANS_INT())&&ZB_IN_BUF_AVAILABLE())
        {
          break;
        }
        TRACE_MSG(TRACE_COMMON2, "%p calling cb %p param %hd in_b %hd len %hd",
                  (FMT__P_P_H_H, ent, ent->func, ent->param,
                   ent->param ? ZB_BUF_FROM_REF(ent->param)->u.hdr.is_in_buf: (zb_uint8_t)-1,
                   ent->param ? ZB_BUF_FROM_REF(ent->param)->u.hdr.len : -1
                    ));

#ifndef SDCC
        (*ent->func)(ent->param);
#else
        /* SDCC wants reentrant functions when called via pointer here, but not
         * need it when called using third function! */
        sdcc_callf(ent->func, ent->param);
#endif
        ZB_RING_BUFFER_FLUSH_GET(&ZG->sched.cb_q);
      }
    }

    /* Handle delayed execution */
    if (ZG->sched.tm_queue!=0)
    {
      zb_tm_q_ent_t *ent;
      t = ZB_TIMER_GET();

      /* execute all callbacks scheduled to run before current time */
      while ((ent = ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next)) != NULL
             && ZB_TIME_GE(t, ent->run_time))
      {
        did_something = 1;
        if ((ZB_GET_TRANS_INT())&&ZB_IN_BUF_AVAILABLE())
        {
          break;
        }
        ZB_LIST_CUT_HEAD(ZG->sched.tm_queue, next, ent);
        /* call the callback */
        TRACE_MSG(TRACE_COMMON2, "%p calling alarm cb %p param %hd", (FMT__P_P_H, ent, ent->func, ent->param));
#ifndef SDCC
        ent->func(ent->param);
#else
        sdcc_callf(ent->func, ent->param);
#endif
        /* put to the freelist */
        ZB_STK_PUSH(ZG->sched.tm_freelist, next, ent);

      }

      /*
       * If have something to run later, restart hw
       * timer.
       */
    }
    if (ZG->sched.tm_queue)
    {
      ZB_TIMER_START(ZB_TIME_SUBTRACT(ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next)->run_time,
                                      t));
    }
    else
    {
      /* 11/21/11 CR:1855: Have no sync timer now, so can rename to
         'zb_timer_stop' */
      zb_timer_stop_async();
    }
  }
  while (did_something);
/* temprally disabled */
#if 0
  /* 
     Q: can it work as busy-wait loop waiting for transiever interrupt?

     A: Sure, but then RF circle of transceiver should not
     be disabled, and I'm not sure, that it's a good idea in terms of power
     saving
  */
  if (((ZB_RING_BUFFER_IS_EMPTY(&MAC_CTX().mac_rx_queue))&&!MAC_CTX().recv_buf_full)&&!MAC_CTX().tx_wait_cb)
  {
    ZB_TRANS_GO_IDLE();
    ZB_GO_IDLE();
  }
#endif
#ifdef ZB_NS_BUILD
  {
    ZB_TRY_IO();
  }
#endif
}


zb_ret_t zb_schedule_tx_cb(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mac_cb_ent_t *ent = ZB_RING_BUFFER_PUT_RESERVE(&ZG->sched.mac_tx_q);
  if (ent)
  {
    ent->func = func;
    ent->param = param;
    ZB_RING_BUFFER_FLUSH_PUT(&ZG->sched.mac_tx_q);
    TRACE_MSG(TRACE_COMMON2, "%p scheduled mac cb %p param %hd (in_b %hd)",
              (FMT__P_P_H_H, ent, ent->func, ent->param, (!param ? (zb_uint8_t)-1 : ZB_BUF_FROM_REF(param)->u.hdr.is_in_buf)));
    ZB_ASSERT(param <= ZB_IOBUF_POOL_SIZE);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "MAC callbacks rb overflow! param %hd", (FMT__H, param));
    ret = RET_OVERFLOW;
  }
  return ret;
}



#if 0 /* re-enable this, if planning to use mac layer queue, but better to make one scheduling routine for all cb's */
zb_ret_t zb_schedule_mac_cb(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mac_cb_ent_t *ent = ZB_RING_BUFFER_PUT_RESERVE(&ZG->sched.mac_cb_q);
  if (ent)
  {
    ent->func = func;
    ent->param = param;
    ZB_RING_BUFFER_FLUSH_PUT(&ZG->sched.mac_cb_q);
    TRACE_MSG(TRACE_COMMON2, "%p scheduled mac cb %p param %hd (in_b %hd)",
              (FMT__P_P_H_H, ent, ent->func, ent->param, (!param ? (zb_uint8_t)-1 : ZB_BUF_FROM_REF(param)->u.hdr.is_in_buf)));
    ZB_ASSERT(param <= ZB_IOBUF_POOL_SIZE);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Callbacks rb overflow! param %hd", (FMT__H, param));
    ret = RET_OVERFLOW;
  }
  return ret;
}
#endif

zb_ret_t zb_schedule_callback(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_cb_q_ent_t *ent = ZB_RING_BUFFER_PUT_RESERVE(&ZG->sched.cb_q);

  if (ent)
  {
    ent->func = func;
    ent->param = param;
    ZB_RING_BUFFER_FLUSH_PUT(&ZG->sched.cb_q);
    TRACE_MSG(TRACE_COMMON2, "%p scheduled cb %p param %hd (in_b %hd)",
              (FMT__P_P_H_H, ent, ent->func, ent->param, (!param ? (zb_uint8_t)-1 : ZB_BUF_FROM_REF(param)->u.hdr.is_in_buf)));
    ZB_ASSERT(param <= ZB_IOBUF_POOL_SIZE);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Callbacks rb overflow! param %hd", (FMT__H, param));
    ret = RET_OVERFLOW;
  }
  return ret;
}


static void insert_tmq_head(zb_tm_q_ent_t *nent)
{
  ZB_LIST_INSERT_HEAD(ZG->sched.tm_queue, next, nent);
}

/* run_after time is specified in beacon intervals, convert it to
 * internal time representation: milliseconds for */
zb_ret_t zb_schedule_alarm(zb_callback_t func, zb_uint8_t param, zb_time_t run_after) ZB_SDCC_REENTRANT /* __reentrant for sdcc, to save DSEG space */
{
  zb_ret_t ret = RET_OK;
  zb_tm_q_ent_t *nent;
  zb_tm_q_ent_t *ent;

  zb_time_t t;

  /* allocate entry - get from the freelist */
  ZB_STK_POP(ZG->sched.tm_freelist, next, nent);
  TRACE_MSG(TRACE_INFO3, "tm_freelist, get nent %p", (FMT__P, (void*)nent));
  if (nent !=0)
  {
    nent->func = func;
    nent->param = param;

    t = ZB_TIMER_GET();
    nent->run_time = ZB_TIME_ADD(t, run_after);

    TRACE_MSG(TRACE_COMMON2, "%p scheduled alarm %p, run_after %d, at %d, param %hd", (FMT__P_P_D_D_H,
                                                                                       nent, nent->func, (int)run_after,
                                                                                       (int)nent->run_time, nent->param));

    if ((ZG->sched.tm_queue==0)             /* queue is empty */
        || ZB_TIME_GE(
          ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next)->run_time, nent->run_time)) /* new time is before
                                                                                  * queue start */
    {
      insert_tmq_head(nent);
    }
    else
    {
      ent = ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next);
      while ( (ent != 0)
              && ZB_TIME_GE(nent->run_time, ent->run_time) )
      {
        ent = ZB_LIST_NEXT(ent, next);
      }

      if ( ent != 0 )
      {
        ent = ZB_LIST_PREV(ent, next);

        if ( ent != 0)
        {
          /* insert after found entry */
          ZB_LIST_INSERT_AFTER(ZG->sched.tm_queue, next, ent, nent);
        }
        else
        {
          /* insert head */
          insert_tmq_head(nent);
        }
      }
      else
      {
        /* insert tail */
        ZB_LIST_INSERT_TAIL(ZG->sched.tm_queue, next, nent);
      }
    }

#if 0
    TRACE_MSG(TRACE_COMMON3, "current time %d", (FMT__D, t));
    ZB_LIST_ITERATE(ZG->sched.tm_queue, next, ent)
    {
      TRACE_MSG(TRACE_COMMON3, "entry %p cb %p run_time %d", (FMT__P_P_D, (void*)ent, ent->func, ent->run_time));
    }
#endif


    /*
     * If list head must be executed just now, not need to start
     * a timer: will execute it immediatly in zb_sched_loop_iteration()
     */
    ent = ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next);
    t = ZB_TIMER_GET();
    TRACE_MSG(TRACE_COMMON3, "cur t %d, head t %d", (FMT__D_D, t, ent->run_time));
    if (ent->run_time != t
        && ZB_TIME_GE(ent->run_time, t))
    {
      ZB_TIMER_START(ZB_TIME_SUBTRACT(ent->run_time, t));
    }
  }
  else
  {
    ret = RET_OVERFLOW;
  }
  return ret;
}


zb_ret_t zb_schedule_alarm_cancel(zb_callback_t func, zb_uint8_t param) ZB_SDCC_REENTRANT /* __reentrant for sdcc, to save DSEG space */
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_tm_q_ent_t *ent;
  zb_tm_q_ent_t *next;

  /* go through alarms list */
  for (ent = ZB_LIST_GET_HEAD(ZG->sched.tm_queue, next) ; ent ; ent = next)
  {
    next = ZB_LIST_NEXT(ent, next);
    if (ent->func == func && (param == ZB_ALARM_ANY_PARAM || param == ZB_ALARM_ALL_CB || param == ent->param))
    {
      TRACE_MSG(TRACE_COMMON3, "%p alarm cancel func %p, param %hd", (FMT__P_P_H, ent, ent->func, ent->param));
      /* remove from scheduled alarms, add to free list */
      ZB_LIST_REMOVE(ZG->sched.tm_queue, next, ent);
      ZB_STK_PUSH(ZG->sched.tm_freelist, next, ent);
      TRACE_MSG(TRACE_INFO3, "tm_queue -> tm_freelist, ent %p", (FMT__P, (void*)ent));
      ret = RET_OK;
      if (param != ZB_ALARM_ALL_CB)
      {
        break;
      }
    }
  }

  return ret;
}

/*! @} */
