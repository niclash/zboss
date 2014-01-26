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
PURPOSE: Zigbee packet buffers pool
*/

#include <string.h>
#include "zb_common.h"
#include "zb_bufpool.h"


/*! \addtogroup buf */
/*! @{ */

#include "zb_bank_common.h"

#define ZB_BUFS_LIMIT (ZB_IOBUF_POOL_SIZE / 2)

#if (defined ZB_NS_BUILD && !defined SDCC) || defined ZB_PLATFORM_LINUX_ARM_2400
#define ZB_DEBUG_BUFFERS
#endif

#ifdef KEIL

/*
 * In Keil can't use assert here because assert calls logger, logger calls
 * zb_putchar, zb_putchar allocates the buffer.
 * In the future will rewrite trace to use fixed-size buffer and do not use
 * packet buffers.
 */
#ifdef ZB_ASSERT
#undef ZB_ASSERT
#define ZB_ASSERT(x)
#endif

#endif

#if 0
#define VERIFY_BUFS() trace_bufs()
#define VERIFY_BUF(b) verify_buf(b)
void trace_bufs()
{
  zb_buf_t *p;
  TRACE_MSG(TRACE_MAC1, "buffers verify : head %p", (FMT__P, ZG->bpool.head));
  for (p = ZG->bpool.head ; p ; p = p->u.next)
  {
    TRACE_MSG(TRACE_MAC1, "buf %p %hd next %p", (FMT__P_H_P, p, ZB_REF_FROM_BUF(p), p->u.next));
  }
}

void verify_buf(zb_buf_t *b)
{
  zb_buf_t *p;
  for (p = ZG->bpool.head ; p ; p = p->u.next)
  {
    if (p == b)
    {
      TRACE_MSG(TRACE_MAC1, "ALREADY FREE buf %p %hd next %p", (FMT__P_H_P, p, ZB_REF_FROM_BUF(p), p->u.next));
    }
  }
}

#else
#define VERIFY_BUFS()
#define VERIFY_BUF(b)
#endif

static zb_buf_t *zb_get_buf(zb_uint8_t is_in)
{
  zb_buf_t *buf = NULL;

  /* check that zb_init_buffers() was called */
  ZB_ASSERT(ZG->bpool.head || ZG->bpool.bufs_allocated[0] || ZG->bpool.bufs_allocated[1]);

  /*
    Logically pool divided into 2 parts: input or output.
    Do not allow one part to eat entire another part to exclude deadlock.
   */
  if (ZG->bpool.bufs_allocated[is_in] < ZB_BUFS_LIMIT)
  {
    buf = ZG->bpool.head;
    if (buf)
    {
      VERIFY_BUFS();
      ZG->bpool.head = buf->u.next;
      VERIFY_BUFS();
      ZB_BZERO(&buf->u, sizeof(buf->u));
      ZG->bpool.bufs_allocated[is_in]++;
      ZB_ASSERT(ZG->bpool.bufs_allocated[is_in] <= ZB_BUFS_LIMIT);
      buf->u.hdr.is_in_buf = is_in;
    }
  }
#ifdef ZB_DEBUG_BUFFERS
  TRACE_MSG( TRACE_MAC1, "zb_get_buf %hd: buffer %p, ref %hd, head %p, allocated %hd / %hd",
             (FMT__H_P_H_P_H, is_in, buf, ZB_REF_FROM_BUF(buf), ZG->bpool.head, ZG->bpool.bufs_allocated[0], ZG->bpool.bufs_allocated[1]));
#endif

  return buf;
}


zb_buf_t *zb_get_in_buf()
{
  return zb_get_buf(1);
}

zb_buf_t *zb_get_out_buf()
{
  return zb_get_buf(0);
}


static zb_ret_t zb_get_buf_delayed(zb_callback_t callback, zb_uint8_t is_in)
{
  zb_buf_t *buf = zb_get_buf(is_in);
  if( buf )
  {
    return ZB_SCHEDULE_CALLBACK( callback,  ZB_REF_FROM_BUF(buf));
  }
   else
  {
    zb_buf_q_ent_t *ent;

    VERIFY_BUFS();
    ZB_STK_POP(ZG->sched.buf_freelist, next, ent);
    if (ent)
    {
      VERIFY_BUFS();
      ent->func = callback;
      if (is_in)
      {
        ZB_SL_LIST_INSERT_TAIL(ZG->sched.inbuf_queue, next, ent);
      }
      else
      {
        ZB_SL_LIST_INSERT_TAIL(ZG->sched.outbuf_queue, next, ent);
      }
      VERIFY_BUFS();
    }
    else
    {
      return RET_ERROR;
    }
  }

  return RET_OK;
}

zb_ret_t zb_get_in_buf_delayed(zb_callback_t callback)
{
  return zb_get_buf_delayed(callback, 1);
}


zb_ret_t zb_get_out_buf_delayed(zb_callback_t callback)
{
  return zb_get_buf_delayed(callback, 0);
}


void zb_free_buf(zb_buf_t *buf)
{
  /* do trace this function, because it can cause lack of out buffers */
  zb_buf_q_ent_t *ent = NULL;

  /* check that zb_init_buffers() was called */
  ZB_ASSERT(ZG->bpool.head || ZG->bpool.bufs_allocated[0] || ZG->bpool.bufs_allocated[1]);

  ZB_ASSERT(ZG->bpool.bufs_allocated[buf->u.hdr.is_in_buf] > 0);
  ZG->bpool.bufs_allocated[buf->u.hdr.is_in_buf]--;

#ifdef ZB_DEBUG_BUFFERS
  TRACE_MSG(TRACE_NWK3, "zb_free_buf %p, ref %hd, in buf %hi allocated in %hd out %hd",
            (FMT__P_H_H_H_H, buf, ZB_REF_FROM_BUF(buf), buf->u.hdr, buf->u.hdr.is_in_buf,
             ZG->bpool.bufs_allocated[1], ZG->bpool.bufs_allocated[0]));
#endif

  VERIFY_BUF(buf);
  buf->u.next = ZG->bpool.head;
  ZG->bpool.head = buf;
  VERIFY_BUFS();

  if (buf->u.hdr.is_in_buf)
  {
    /* if we need a buffer for rx packet, we should not pass it to some */
  /* other callback */
  if (!MAC_CTX().rx_need_buf)
      ZB_SL_LIST_CUT_HEAD(ZG->sched.inbuf_queue, next, ent);
  }
  else
  {
    ZB_SL_LIST_CUT_HEAD(ZG->sched.outbuf_queue, next, ent);
  }
  if (ent)
  {
    ZB_SCHEDULE_CALLBACK(ent->func, ZB_REF_FROM_BUF(zb_get_buf(buf->u.hdr.is_in_buf)));
    ZB_STK_PUSH(ZG->sched.buf_freelist, next, ent);
  }

#ifdef ZB_DEBUG_BUFFERS
  TRACE_MSG( TRACE_MAC1, "free_buf: %hd/%hd buf %p, next %p, head %p",
             (FMT__H_H_P_P_P,
              ZG->bpool.bufs_allocated[1], ZG->bpool.bufs_allocated[0],
              buf, buf->u.next, ZG->bpool.head));
#endif

}

/**
   Initial allocate space in buffer.

   @param zbbuf - buffer
   @param size  - size to allocate
   @param ptr   - (out) pointer to the buffer begin
 */
zb_void_t *zb_buf_initial_alloc(zb_buf_t *zbbuf, zb_uint8_t size)
{
  zb_uint8_t is_in_buf = zbbuf->u.hdr.is_in_buf;
  ZB_ASSERT((size) < ZB_IO_BUF_SIZE);
  ZB_BZERO(&zbbuf->u, sizeof(zbbuf->u));
  zbbuf->u.hdr.is_in_buf = is_in_buf;
  (zbbuf)->u.hdr.len = (size);
  (zbbuf)->u.hdr.data_offset = (ZB_IO_BUF_SIZE - (size)) / 2;
  return (void *)ZB_BUF_BEGIN(zbbuf);
}


/**
   Initial allocate space in buffer.

   @param zbbuf - buffer
   @param size  - size to allocate
   @param ptr   - (out) pointer to the buffer begin
 */
zb_void_t *zb_buf_smart_alloc_left(zb_buf_t *zbbuf, zb_uint8_t size) ZB_SDCC_REENTRANT
{
  ZB_ASSERT(size + ZB_BUF_LEN(zbbuf) < ZB_IO_BUF_SIZE);
  if (zbbuf->u.hdr.data_offset < size)
  {
    /* not sure: try to align to 2 or 4? Or it can cause more problems? */
    zb_uint8_t *p = ZB_BUF_BEGIN(zbbuf);
    ZB_MEMMOVE(p + (size - zbbuf->u.hdr.data_offset), p, zbbuf->u.hdr.len);
    zbbuf->u.hdr.data_offset = size;
  }
  zbbuf->u.hdr.len += (size);
  (zbbuf)->u.hdr.data_offset -= (size);
  return ZB_BUF_BEGIN(zbbuf);
}

zb_void_t *zb_buf_smart_alloc_right(zb_buf_t *zbbuf, zb_uint8_t size) ZB_SDCC_REENTRANT
{
  void *ptr;
  ZB_ASSERT((size) + ZB_BUF_LEN(zbbuf) < ZB_IO_BUF_SIZE);
  (ptr) = (void *)(ZB_BUF_BEGIN(zbbuf) + ZB_BUF_LEN(zbbuf));
  (zbbuf)->u.hdr.len += (size);
  return ptr;
}

void zb_buf_assign_param(zb_buf_t *zbbuf, zb_uint8_t *param, zb_uint8_t size) ZB_SDCC_REENTRANT
{
  ZB_ASSERT(zbbuf && (zbbuf->u.hdr.len + size <= ZB_IO_BUF_SIZE));
  if (ZB_IO_BUF_SIZE - (zbbuf->u.hdr.data_offset + zbbuf->u.hdr.len) < size)
  {
    zb_ushort_t delta = size - (ZB_IO_BUF_SIZE - (zbbuf->u.hdr.data_offset + zbbuf->u.hdr.len));
    zb_uint8_t *p = ZB_BUF_BEGIN(zbbuf);
    ZB_MEMMOVE(p - delta, p, zbbuf->u.hdr.len);
    zbbuf->u.hdr.data_offset -= delta;
  }
  ZB_MEMCPY(zb_get_buf_tail(zbbuf, size), param, size);
}

void *zb_buf_cut_left(zb_buf_t *zbbuf, zb_uint8_t size)
{
  ZB_ASSERT(ZB_BUF_LEN(zbbuf) >= (size));               /* ++VS */
  (zbbuf)->u.hdr.len -= (size);
  (zbbuf)->u.hdr.data_offset += (size);
  return (void *)ZB_BUF_BEGIN(zbbuf);
}



void zb_buf_cut_right(zb_buf_t *zbbuf, zb_uint8_t size)
{
  ZB_ASSERT(ZB_BUF_LEN(zbbuf) >= (size));
  (zbbuf)->u.hdr.len -= (size);
}


/**
   Get buffer tail of size 'size'

   Macro usually used to place external information (some parameters) to the
   buffer

   @param zbbuf - buffer
   @param size - requested size
   @return pointer to the buffer tail
 */
zb_void_t *zb_get_buf_tail(zb_buf_t *zbbuf, zb_uint8_t size)
{
  ZB_ASSERT((zbbuf) && ZB_BUF_LEN(zbbuf) + (zbbuf)->u.hdr.data_offset + (size) <= ZB_IO_BUF_SIZE);
  return (void *)((zbbuf)->buf + (ZB_IO_BUF_SIZE - (size)));
}


zb_void_t zb_buf_reuse(zb_buf_t *zbbuf)
{
  zb_uint8_t is_in_buf = zbbuf->u.hdr.is_in_buf;
#ifdef ZB_DEBUG_BUFFERS
  TRACE_MSG(TRACE_NWK3, "zb_reuse_buf %p, ref %hd, in buf %hi allocated in %hd out %hd",
            (FMT__P_H_H_H_H, zbbuf, ZB_REF_FROM_BUF(zbbuf), zbbuf->u.hdr, zbbuf->u.hdr.is_in_buf,
             ZG->bpool.bufs_allocated[1], ZG->bpool.bufs_allocated[0]));
#endif
  ZB_BZERO(&zbbuf->u, sizeof(zbbuf->u));
  zbbuf->u.hdr.is_in_buf = is_in_buf;
}

#ifdef __DBG
zb_void_t temp_debug_dump_buf_hdr(zb_buf_t *buf)
{
  TRACE_MSG(TRACE_MAC2, "!!! buf %p, len %hd, data_offset %hd, handle %hd, mac_hdr_offset %hd, status %d",
    (FMT__P_H_H_H_H_D, (buf), (buf)->u.hdr.len, (buf)->u.hdr.data_offset,
     (buf)->u.hdr.handle, (buf)->u.hdr.mac_hdr_offset, (buf)->u.hdr.status));
}
#endif

/*! @} */
