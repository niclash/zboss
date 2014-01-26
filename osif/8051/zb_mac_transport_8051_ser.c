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
PURPOSE: 8051 serial  transport for mac data and dump. Use syncronous
put and non-blocked get from the int handler.
*/


#include "zb_common.h"

#ifdef ZB_TRANSPORT_8051_DATA_UART

#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_scheduler.h"
#include "zb_mac_transport.h"

/*! \addtogroup ZB_MAC_TRANSPORT */
/*! @{ */

#include "zb_bank_common.h"


void zb_mac_transport_init()
  ZB_SDCC_REENTRANT                  /* __reentrant for sdcc, to save DSEG space */
{
  TRACE_MSG(TRACE_MAC3, "nothing to do", (FMT__0));
}


/**
   Initiates receive operation: sets buffer for receive and required
   bytes number to read
   @param buf - pointer on receive buffer
   @param bytes_to_recv - number of bytes to read or 0
   @return void
 */
void zb_mac_transport_start_recv(zb_buf_t *buf, zb_short_t bytes_to_recv)
{
  ZB_ASSERT(buf); /* check input buffer pointer */
  ZB_ASSERT(!ZG->ioctx.recv_data_buf);

  ZG->ioctx.bytes_to_recv = bytes_to_recv;
  ZG->ioctx.recv_data_buf = buf;
  TRACE_MSG(TRACE_MAC3, "set recv_data_buf %p", (FMT__P, ZG->ioctx.recv_data_buf));

  (void)zb_8051_serial_try_recv();
}


zb_ret_t zb_8051_serial_try_recv()
{
  zb_ret_t ret = RET_OK;

  if (!ZG->ioctx.recv_data_buf)
  {
    ZB_OSIF_GLOBAL_LOCK();
    if (!ZB_RING_BUFFER_IS_EMPTY(&SER_CTX().rx_buf))
    {
      MAC_SET_STATE_FOR_LAYER(ZB_MAC_IO_LAYER_RX, ZB_MAC_STATE_IO_RECV_PENDING);
      TRACE_MSG(TRACE_MAC3, "set mac rx state %i", (FMT__D, ZB_MAC_STATE_IO_RECV_PENDING));
    }
    ZB_OSIF_GLOBAL_UNLOCK();

    return RET_OK;
  }

  if (!ZG->ioctx.bytes_to_recv)
  {
    if (zb_8051_serial_get_bytes(&ZG->ioctx.bytes_to_recv, 1) != 0)
    {
      zb_uint8_t *p;
      ZB_BUF_INITIAL_ALLOC(ZG->ioctx.recv_data_buf, ZG->ioctx.bytes_to_recv, p);
      *p = ZG->ioctx.bytes_to_recv;
      TRACE_MSG( TRACE_MAC3, "will recv %d", (FMT__D, (int)ZG->ioctx.bytes_to_recv));
      ZB_BUF_LEN(ZG->ioctx.recv_data_buf) = 1;
    }
    ret = RET_BLOCKED;
  }
  else
  {
    zb_ushort_t n = ZG->ioctx.bytes_to_recv - ZB_BUF_LEN(ZG->ioctx.recv_data_buf);

    if (n)
    {
      /* this is nonblocked read: get all we can */
      n = zb_8051_serial_get_bytes(
        ZB_BUF_BEGIN(ZG->ioctx.recv_data_buf) + ZB_BUF_LEN(ZG->ioctx.recv_data_buf),
        n);

      ZB_BUF_LEN(ZG->ioctx.recv_data_buf) += n;
      TRACE_MSG( TRACE_MAC3, "n %d", (FMT__D, (int)n));

      if (ZG->ioctx.bytes_to_recv == ZB_BUF_LEN(ZG->ioctx.recv_data_buf))
      {
        TRACE_MSG( TRACE_MAC3, "received buf %p %d bytes: %x %x %x %x", (FMT__P_D_D_D_D_D,
                   ZG->ioctx.recv_data_buf, (int)ZB_BUF_LEN(ZG->ioctx.recv_data_buf),
                   (int)ZB_BUF_BEGIN(ZG->ioctx.recv_data_buf)[0],
                   (int)ZB_BUF_BEGIN(ZG->ioctx.recv_data_buf)[1],
                   (int)ZB_BUF_BEGIN(ZG->ioctx.recv_data_buf)[2],
                   (int)ZB_BUF_BEGIN(ZG->ioctx.recv_data_buf)[3]));
        /* we are done */
        ZB_SET_RECV_STATUS(ZB_RECV_FINISHED);
        ZG->ioctx.recv_data_buf = NULL;
        TRACE_MSG(TRACE_MAC3, "set recv_data_buf %p", (FMT__P, ZG->ioctx.recv_data_buf));
        ZB_MAC_STOP_IO();
      }
      else
      {
        ret = RET_BLOCKED;
      }
    }
    else
    {
      TRACE_MSG( TRACE_MAC1, "n == 0 - why are we here?", (FMT__0));
    }
  }
  return ret;
}


void zb_mac_transport_put_data(zb_buf_t *buf)
{
  zb_mac_transport_hdr_t *t_hdr = NULL;

  {
    zb_uint8_t *ptr;
    ZB_BUF_ALLOC_LEFT(buf, 1, ptr);
    *ptr = ZB_BUF_LEN(buf);
  }

  ZB_DUMP_OUTGOING_DATA(buf);

  /* if UART trasport is used for zigbee data transfer, put zb_mac_transport_hdr_t in it
     this header is used by pipe_data_router to find out type of received packet, it can be
     zb data, trace and dump */

  ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_mac_transport_hdr_t), t_hdr);
  t_hdr->len = buf->u.hdr.len;
  t_hdr->type = ZB_MAC_TRANSPORT_TYPE_DATA;
  /* prefix data by length for ns-3 plugin module */

  TRACE_MSG( TRACE_MAC3, "put buf %p %d bytes", (FMT__P_D, buf, (int)ZB_BUF_LEN(buf)));
  /* synchronus write! */
  zb_8051_serial_put_bytes(ZB_BUF_BEGIN(buf), ZB_BUF_LEN(buf));

  /* Buffer can be used for resend. Put it into initial state: no header, no
   * length byte. */
  ZB_BUF_CUT_LEFT(buf, sizeof(zb_mac_transport_hdr_t) + 1, t_hdr);

  ZB_SET_SEND_STATUS(ZB_NO_IO);
}


#endif  /* ZB_TRANSPORT_8051_DATA_UART */

/*! @} */
