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
PURPOSE: Packet buffers pool
*/

#ifndef ZB_BUFPOOL_H
#define ZB_BUFPOOL_H 1



/*! \addtogroup buf */
/*! @{ */

#define ZB_UNDEFINED_BUFFER (zb_uint8_t)(-1)

/**
   Packet buffer header.
 */
typedef struct zb_buf_hdr_s
{
  zb_uint8_t len;              /*!< current layer buffer length  */
  zb_uint8_t data_offset;      /*!< data offset in buffer buf*/
  zb_uint8_t handle;           /*!< The handle associated with the NSDU to be
                                * transmitted by the NWK layer entity.  */
  zb_uint8_t mac_hdr_offset;  /* mac hdr offset, used to access to mac data
                               * from upper layers, used at least in route
                               * discovery */
  zb_int16_t status;            /*!< some status to be passed with packet  */
  zb_bitfield_t is_in_buf:1;    /*!< if 1, this is input buffer */

  zb_bitfield_t encrypt_type:2; /*!< payload must be encrypted before send, if
                                 * !0. \see zb_secur_buf_encr_type_e.
                                 */
  zb_bitfield_t use_same_key:1;    /*!< if 1, use same nwk key# packet was
                                    * encrypted by */
  zb_bitfield_t zdo_cmd_no_resp:1; /*!< if 1, this is ZDO command with no
                                    * responce - call cqallback at confirm  */
  zb_bitfield_t reserved:3;
#if 0
#ifdef ZB_NEED_ALIGN
  zb_uint8_t    align;
#endif
#endif 
  zb_uint8_t    mhr_len;          /* used for TRANS_SEND_COMMAND */
} ZB_PACKED_STRUCT zb_buf_hdr_t;


/**
   Packet buffer
 */
typedef struct zb_buf_s
{
  union
  {
    zb_buf_hdr_t hdr;
    struct zb_buf_s *next;
  } u;
  zb_uint8_t   buf[ZB_IO_BUF_SIZE];
} zb_buf_s_t;

#define zb_buf_t zb_buf_s_t

/* check if input(output) buffer available */
#define ZB_IN_BUF_AVAILABLE() (ZG->bpool.bufs_allocated[1] < ZB_IOBUF_POOL_SIZE/2)
#define ZB_OUT_BUF_AVAILABLE() (ZG->bpool.bufs_allocated[0] < ZB_IOBUF_POOL_SIZE/2)

/**
   Return current buffer pointer
 */
#define ZB_BUF_BEGIN(zbbuf) ((zbbuf)->buf + (zbbuf)->u.hdr.data_offset)

/**
   Return current buffer length
 */
#define ZB_BUF_LEN(zbbuf) ((zbbuf)->u.hdr.len)

/**
   Return current buffer offset
 */
#define ZB_BUF_OFFSET(zbbuf) ((zbbuf)->u.hdr.data_offset)

/**
   Initial allocate space in buffer.

   @param zbbuf - buffer
   @param size  - size to allocate

   @return pointer to the allocated space
 */
zb_void_t *zb_buf_initial_alloc(zb_buf_t *zbbuf, zb_uint8_t size);
#define ZB_BUF_INITIAL_ALLOC(zbbuf, size, ptr) (ptr) = zb_buf_initial_alloc((zbbuf), (size))

/**
   Allocate space at buffer begin

   @param zbbuf - buffer
   @param size  - size to allocate
   @param ptr   - (out) pointer to the new buffer begin
 */
#define ZB_BUF_ALLOC_LEFT(zbbuf, size, ptr) (ptr) = zb_buf_smart_alloc_left((zbbuf), (size))
zb_void_t *zb_buf_smart_alloc_left(zb_buf_t *zbbuf, zb_uint8_t size) ZB_SDCC_REENTRANT;

/**
   Allocate space at buffer end

   @param zbbuf - buffer
   @param size  - size to allocate
   @param ptr   - (out) pointer to the space allocated
 */
#define ZB_BUF_ALLOC_RIGHT(zbbuf, size, ptr) (ptr) = zb_buf_smart_alloc_right((zbbuf), (size))
zb_void_t *zb_buf_smart_alloc_right(zb_buf_t *zbbuf, zb_uint8_t size) ZB_SDCC_REENTRANT;

/**
   Cut space at buffer begin

   Note: removed assert from here because it can be called from SPI int handler

   @param zbbuf - buffer
   @param size  - size to cut
   @param ptr   - (out) pointer to the new buffer begin
 */
#define ZB_BUF_CUT_LEFT(zbbuf, size, ptr)  (ptr) = zb_buf_cut_left((zbbuf), (size)) 
void *zb_buf_cut_left(zb_buf_t *zbbuf, zb_uint8_t size);


#define ZB_BUF_CUT_LEFT2(zbbuf, size)                 \
do                                                        \
{                                                         \
  (zbbuf)->u.hdr.len -= (size);                           \
  (zbbuf)->u.hdr.data_offset += (size);                   \
} while (0)




/**
   Cut space at buffer end

   @param zbbuf - buffer
   @param size  - size to cut
 */
#define ZB_BUF_CUT_RIGHT(zbbuf, size) zb_buf_cut_right((zbbuf), (size))
void zb_buf_cut_right(zb_buf_t *zbbuf, zb_uint8_t size);


/**
   Get buffer tail of size 'size'

   Macro usually used to place external information (some parameters) to the
   buffer

   @param zbbuf - buffer
   @param size - requested size
   @return pointer to the buffer tail
 */
zb_void_t *zb_get_buf_tail(zb_buf_t *zbbuf, zb_uint8_t size);
#define ZB_GET_BUF_TAIL zb_get_buf_tail

#define ZB_GET_BUF_PARAM(zbbuf, type) ((type *)ZB_GET_BUF_TAIL((zbbuf), sizeof(type)))


/**
   Copy data to the bufefr tail - assign parameter.

   Take care on space on the buffer tail, move data if necessary.

   @param zbbuf - buffer
   @param param - data to copy
   @param size  - data size

 */
void zb_buf_assign_param(zb_buf_t *zbbuf, zb_uint8_t *param, zb_uint8_t size) ZB_SDCC_REENTRANT;

#define ZB_SET_BUF_PARAM(zbbuf, param, type) ( *((type *)ZB_GET_BUF_TAIL(zbbuf, sizeof(type))) = (param) )
#define ZB_SET_BUF_PARAM_PTR(zbbuf, param, type) ( ZB_MEMCPY((type *)ZB_GET_BUF_TAIL(zbbuf, sizeof(type)), (param), sizeof(type)) )

/**
   Copy one buffer to the other

   @param src_buf - source buffer
   @param dst_buf - destination buffer
 */
#define ZB_BUF_COPY(dst_buf, src_buf)                                   \
do                                                                      \
{                                                                       \
  zb_uint8_t is_in = (dst_buf)->u.hdr.is_in_buf;                        \
  ZB_MEMCPY((dst_buf), (src_buf), sizeof(zb_buf_t));                    \
  (dst_buf)->u.hdr.is_in_buf = is_in;                                   \
} while (0)

/**
  Reuse previously used buffer
  @param zbbuf - buffer
 */
zb_void_t zb_buf_reuse(zb_buf_t *zbbuf);
#define ZB_BUF_REUSE zb_buf_reuse

#define ZB_BUF_GET_FREE_SIZE(zbbuf) (unsigned)(ZB_IO_BUF_SIZE - ZB_BUF_LEN(zbbuf))

/**
   Initialize packet buffers pool.

   To be called at start time.

   @return nothing
 */
void zb_init_buffers()ZB_CALLBACK;


/**
   Get IN buffer from the buffers list.

   If no buffers available, does not block.
   To be called from the interrupt handler reading packets.
   If no buffer available, int handler must skip this packet.

   @return pointer to the buffer or NULL if no buffer available.
 */
zb_buf_t *zb_get_in_buf();


/**
   Get OUT buffer from the buffers list.

   If no buffers available, does not block.
   To be called from the main loop routine.

   @return pointer to the buffer.
 */
zb_buf_t *zb_get_out_buf();


/**
   Free packt buffer.
   Put packet buffer into freelist.

   Can be called from the main loop.

   @param buf - packet buffer.
   @return nothing
 */
void zb_free_buf(zb_buf_t *buf);

#define ZB_BUF_FROM_REF(ref) (&ZG->bpool.pool[ref])

#define ZB_REF_FROM_BUF(buf) (buf - &ZG->bpool.pool[0])

/**
   Allocate IN buffer. Call callback when buffer is available.

   If buffer available, schedules callback for execution immediatly.
   If no buffers available now, schedule callback later, when buffer will be available.

   @return RET_OK or error code.
 */
zb_ret_t zb_get_in_buf_delayed(zb_callback_t callback);

#define ZB_GET_IN_BUF_DELAYED zb_get_in_buf_delayed


/**
   Allocate OUT buffer. Call callback when buffer is available.

   If buffer available, schedules callback for execution immediatly.
   If no buffers available now, schedule callback later, when buffer will be available.

   @return RET_OK or error code.
 */
zb_ret_t zb_get_out_buf_delayed(zb_callback_t callback);

#define ZB_GET_OUT_BUF_DELAYED zb_get_out_buf_delayed

/*! @} */

#endif /* ZB_BUFPOOL_H */
