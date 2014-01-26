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
PURPOSE: SPI transport for 8051: code for the common bank
*/

#include "zb_common.h"
#include "zb_mac_transport.h"
#include "zb_bufpool.h"
#include "zb_ringbuffer.h"

#ifdef ZB_TRANSPORT_8051_DATA_SPI

#include "zb_bank_common.h"

/**
   8051 spi port interrupt handler
 */

#ifdef ZB_IAR
#pragma register_bank=REGISTER_BANK_3
#pragma vector=SPI_INTER_NUMBER
#endif
ZB_INTERRUPT spi_inter_handler(void) INTERRUPT_DECLARATION(SPI_INTER_NUMBER, REGISTER_BANK_3)
{
  zb_buf_t *send_buf = NULL;
  zb_char_t need_tx = 0;

  if (SPIF == 1)
  {
    /* transmit-complete or receive-data-ready */
    SPIF = 0;
    if (SPI_CTX().io_request == ZB_NO_SPI_IO_REQUEST)
    {
      /* Check if some data is requested to read - if recv_data_buf is not NULL, read received spi value */
      if (ZB_IOCTX().recv_data_buf != NULL)
      { 
        ZB_IOCTX().recv_data_buf->buf
          [ZB_IOCTX().recv_data_buf->u.hdr.len + ZB_IOCTX().recv_data_buf->u.hdr.data_offset] = SPI0DAT;

        if (ZB_IOCTX().bytes_to_recv == 0)
        {									   
          /* Read RX fifo packet: the 1st byte contains packet length + 9 bytes of additional data:
             LQI, RSSI, Frame timer, Superframe counter + 1 byte of length itself */
          /* TODO: check ZB_MAC_EXTRA_DATA_SIZE 9 or 2? ds-2400  3.2.3. RXMAC, 4.4.1. Receive Packet in RXFIFO */
          ZB_IOCTX().bytes_to_recv = ZB_MAC_PACKET_LENGTH_SIZE + ZB_MAC_EXTRA_DATA_SIZE +
            ZB_IOCTX().recv_data_buf->buf[ZB_IOCTX().recv_data_buf->u.hdr.len +
                                         ZB_IOCTX().recv_data_buf->u.hdr.data_offset];
        }
        ZB_IOCTX().recv_data_buf->u.hdr.len++;

        if (ZB_IOCTX().recv_data_buf->u.hdr.len >= ZB_IOCTX().bytes_to_recv)
        {
          ZB_IOCTX().recv_data_buf = NULL;
          ZB_SET_RECV_STATUS(ZB_RECV_FINISHED);
          ZB_MAC_STOP_IO();
        }
        else
        {
          /* transmit is needed to receive next byte over spi */
          need_tx = 1;
        }
      }
    }
    else if (SPI_CTX().io_request == ZB_SPI_RX_REQUEST)
    {
      /* force send to receive the 1st byte */
      SPI_CTX().io_request = ZB_NO_SPI_IO_REQUEST;
      need_tx = 1;
    }

    /* Check  if there is something to send */
    SPI_CTX().tx_in_progress = 0;
    send_buf = ZB_IOCTX().send_data_buf;

    if (send_buf != NULL)
    {
      SPI_CTX().tx_in_progress = 1;
      SPI0DAT = send_buf->buf[send_buf->u.hdr.data_offset + SPI_CTX().sent_num];
      need_tx = 0;
      SPI_CTX().sent_num++;
      if (SPI_CTX().sent_num == send_buf->u.hdr.len)
      {
        /* remove headers */
        /* TODO: move it out of int handler */
        /* remove headers */
        ZB_BUF_CUT_LEFT2((send_buf), sizeof(zb_ubec_fifo_header_t)+sizeof(zb_uint16_t));

        /* In MAC commands exchenge 1 fixed buffer is used and
         * reused, do not put into free queue   */
        send_buf = NULL;
        SPI_CTX().sent_num = 0;
        ZB_SET_SEND_STATUS(ZB_SEND_FINISHED);
      }
    }
    ZB_IOCTX().send_data_buf = send_buf;

    if (need_tx)
    {
      /* send byte value 0 just to receive next byte over spi */
      SPI_CTX().tx_in_progress = 1;
      SPI0DAT = 0;
    }
  }
  else if (WCOL == 1)
  {
    /* write collision. TODO: indicate it somehow? Need to reset? */
    WCOL = 0;
  }
  else if (RXOVRN == 1)
  {
    /* rx overrun. TODO: indicate it somehow? Need to reset? */
    RXOVRN = 0;
  }

}

#endif /* ZB_TRANSPORT_8051_DATA_SPI */
