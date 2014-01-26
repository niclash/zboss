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
PURPOSE: direct uz2410 stransport for mac data and dump.
*/

#include "zb_common.h"
#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_scheduler.h"
#include "zb_mac_transport.h"

#include "zb_bank_common.h"

#if 0                           /* not used! */

#ifdef ZB_UZ2410
void zb_mac_transport_init()                  /* __reentrant for sdcc, to save DSEG space */
 /* just for compativility  */
{
}

void zb_mac_transport_start_recv(zb_buf_t *buf, zb_short_t bytes_to_recv)
{
  ZB_ASSERT(buf); /* check input buffer pointer */
  ZB_ASSERT(!ZG->ioctx.recv_data_buf); /* check status of previous receive operation - if pointer is not
                                        * NULL, recv is not finished*/
  /* place data at buffer begin, reserving space for the traffic dump */
  
  ZB_SCHED_GLOBAL_LOCK();
  if (ZG->ioctx.recv_data_buf != NULL)
  {
    /* Error! Previous receive operation is not finished */
    ZB_ASSERT(0);
  }
  ZG->ioctx.recv_data_buf = buf;
  ZG->ioctx.bytes_to_recv = bytes_to_recv;
  ZB_SCHED_GLOBAL_UNLOCK();
}


/* stores buffer pointer to out buffer pointer and signals transport
 * to start transmit; if previous data is not sent, error appears */
void zb_mac_transport_put_data(zb_buf_t *buf)
{
  ZB_SCHED_GLOBAL_LOCK();
  if (ZG->ioctx.send_data_buf)
  {
    /* Error! previous data is not sent yet!!! */
    ZB_ASSERT(0);
  }
  else
  {
    ZG->ioctx.send_data_buf = buf;
  }
  ZB_DUMP_OUTGOING_DATA(buf);
  /* remove headers */
  ZB_BUF_CUT_LEFT2((buf), sizeof(zb_ubec_fifo_header_t)+sizeof(zb_uint16_t));

  ZB_SET_SEND_STATUS(ZB_NO_IO);
  ZB_SCHED_GLOBAL_UNLOCK();
}

#endif /* ZB_UZ2410 */

#endif  /* 0 */
