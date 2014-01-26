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
PURPOSE: Serial transport for 8051
*/

#include "zb_common.h"

#ifndef  ZB_SNIFFER
#include "zb_mac_transport.h"
#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_g_context.h"

#else
#include "zb_sniffer_tools.h"
extern ringBuf_t rbTxBuf; 
#endif



/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#include "zb_bank_common.h"
#ifdef ZB_TRANSPORT_8051_UART

#ifdef ZB_IAR
#pragma register_bank=REGISTER_BANK_1
#pragma vector=SERIAL_INTER_NUMBER
#endif
ZB_INTERRUPT serial_inter_handler(void) INTERRUPT_DECLARATION(SERIAL_INTER_NUMBER, REGISTER_BANK_1)
{  
#if defined ZB_TRANSPORT_8051_DATA_UART && !defined ZB_SNIFFER
  if (ZB_SERIAL_RECV_FLAG)
  {
    if (!ZB_RING_BUFFER_IS_FULL(&SER_CTX().rx_buf))
    {
      ZB_CLEAR_SERIAL_RECV_FLAG();
      ZB_RING_BUFFER_PUT(&SER_CTX().rx_buf, SBUF);
    }
  }
#endif
  if (ZB_SERIAL_TRANSMIT_FLAG)
  {
#ifdef ZB_CC25XX
    ZB_CLEAR_SERIAL_TRANSMIT_FLAG();
    ZB_DISABLE_SERIAL_INTER();
#endif
#ifndef ZB_SNIFFER    
    zb_uint8_t volatile *p = ZB_RING_BUFFER_PEEK(&SER_CTX().tx_buf);
    if (p)
#else
    zb_int8_t nbytes = 0;
    zb_uint8_t *p = NULL;
    nbytes = bufGet(&rbTxBuf, p, 1);
    if (nbytes)
#endif /* ZB_SNIFFER */
    {
      SBUF = *p;
      SER_CTX().tx_in_progress = 1;
#ifndef ZB_SNIFFER
      ZB_RING_BUFFER_FLUSH_GET(&SER_CTX().tx_buf);
#endif
    }
    else
    {
      SER_CTX().tx_in_progress = 0;
    }
#ifdef ZB_CC25XX
    ZB_ENABLE_SERIAL_INTER();
#else    
    ZB_CLEAR_SERIAL_TRANSMIT_FLAG();
#endif    
  }
}

#ifdef ZB_SNIFFER
#ifdef ZB_IAR
#pragma vector=SERIAL_RX_INTER_NUMBER
#endif
ZB_INTERRUPT serial_rx_inter_handler(void) INTERRUPT_DECLARATION(SERIAL_RX_INTER_NUMBER, REGISTER_BANK_1)
{
  /* Receives by UART and sets the transciever's channel for sniffing */
  zb_uint8_t ch;
  
  ch = U0DBUF;
  /* Flush all the contexts */
  zb_clear_sniffer();
  zb_start_sniffer(ch);
}

#endif /* ZB_SNIFFER */

#ifndef ZB_SNIFFER

void zb_8051_serial_put_bytes(zb_uint8_t *buf, zb_short_t len)
{
  while (len)
  {
    ZB_DISABLE_SERIAL_INTER();
    if (ZB_RING_BUFFER_IS_FULL(&SER_CTX().tx_buf))
    {
      ZB_ENABLE_SERIAL_INTER();
      ZB_GO_IDLE();
      ZB_DISABLE_SERIAL_INTER();
    }
    else
    {
      ZB_RING_BUFFER_PUT(&SER_CTX().tx_buf, *buf);
      buf++;
      len--;
    }
    if (!SER_CTX().tx_in_progress)
    {
      ZB_SET_SERIAL_TRANSMIT_FLAG();
    }
    ZB_ENABLE_SERIAL_INTER();
  }
  while(!ZB_RING_BUFFER_IS_EMPTY(&SER_CTX().tx_buf))
  {
     ZB_GO_IDLE();
  }
        
}

#ifdef ZB_TRANSPORT_8051_DATA_UART
zb_ushort_t zb_8051_serial_get_bytes(zb_uint8_t *buf, zb_ushort_t size)
{
  zb_uint8_t *p;
  zb_ushort_t ret = 0;

//  ZB_ENABLE_ALL_INTER();
  ZB_DISABLE_SERIAL_INTER();

  while (size
         && (p = ZB_RING_BUFFER_PEEK(&SER_CTX().rx_buf)))
  {
    *buf++ = *p;
    size--;
    ret++;
    ZB_RING_BUFFER_FLUSH_GET(&SER_CTX().rx_buf);
  }

  ZB_ENABLE_SERIAL_INTER();

  return ret;
}
#endif

#ifdef ZB_TRANSPORT_8051_DATA_SPI
/* SPI transport means work on device - needs a signature to synchronize with serial data flow */
#define NEED_SIGNATURE
#endif

#define NEED_SIGNATURE

#ifdef NEED_SIGNATURE
#define SIGNATURE_SIZE 2
#else
#define SIGNATURE_SIZE 0
#endif

#ifdef ZB_TRACE_LEVEL

#define INITIAL_OFFSET (sizeof(zb_mac_transport_hdr_t) + SIGNATURE_SIZE)
#define PUTCHAR_BUF_SIZE 127
static ZB_SDCC_XDATA zb_uint8_t s_putchar_s[PUTCHAR_BUF_SIZE] = {0xde, 0xad, 0, ZB_MAC_TRANSPORT_TYPE_TRACE};
static zb_ushort_t s_putchar_len = INITIAL_OFFSET;    /* for hdr+signature */


void zb_flush_putchar()
{
  if (s_putchar_len > INITIAL_OFFSET)
  {
    {
      zb_mac_transport_hdr_t *hdr = (zb_mac_transport_hdr_t *)(s_putchar_s + SIGNATURE_SIZE);
      hdr->time = ZB_TIMER_GET();
      hdr->len = s_putchar_len - SIGNATURE_SIZE;
    }
    zb_8051_serial_put_bytes(s_putchar_s, s_putchar_len);
    s_putchar_len = INITIAL_OFFSET;
  }
}

void zb_putchar(char c)
{
  s_putchar_s[s_putchar_len++] = c;
}

#if 0
void zb_putchar(char c)
{
  s_putchar_s[s_putchar_len] = c;
  s_putchar_len++;
  if (s_putchar_len == PUTCHAR_BUF_SIZE
      || c == '\n' )
  {
#ifndef ZB_TEXT_ONLY_TRACE
    zb_mac_transport_hdr_t *hdr = (zb_mac_transport_hdr_t *)(s_putchar_s + SIGNATURE_SIZE);
    hdr->len = s_putchar_len - SIGNATURE_SIZE;
    hdr->time = ZB_TIMER_GET();
    zb_8051_serial_put_bytes(s_putchar_s, s_putchar_len);
#else
    zb_8051_serial_put_bytes(s_putchar_s + INITIAL_OFFSET, s_putchar_len - INITIAL_OFFSET);
#endif
    s_putchar_len = INITIAL_OFFSET;
  }
}
#endif

#endif /* ZB_TRACE_LEVEL */


#ifdef ZB_TRAFFIC_DUMP_ON

/**
   Dump traffic to the dump file.

   @param buf    - output buffer.
   @param is_w   - if 1, this is output, else input

   @return nothing.
 */
void zb_mac_traffic_dump(zb_buf_t *buf, zb_bool_t is_w) ZB_SDCC_REENTRANT
{
  static zb_uint16_t pkt_counter = 0;
  struct trace_hdr_s
  {
#ifdef NEED_SIGNATURE
    zb_char_t sig[2];
#endif
    zb_mac_transport_hdr_t h;
  } ZB_PACKED_STRUCT;

  pkt_counter++;

  TRACE_MSG(TRACE_MAC1, ">>zb_mac_traffic_dump is_w %hd #%u buf %p [%hd] %hx %hx %hx %hx",
            (FMT__H_D_P_H_H_H_H_H,
             (zb_uint8_t)is_w,
             (zb_uint16_t)pkt_counter,
             buf,
             (zb_uint8_t)ZB_BUF_LEN(buf),
             (zb_uint8_t)(ZB_BUF_BEGIN(buf)[0]),
             (zb_uint8_t)(ZB_BUF_BEGIN(buf)[1]),
             (zb_uint8_t)(ZB_BUF_BEGIN(buf)[2]),
             (zb_uint8_t)(ZB_BUF_BEGIN(buf)[3])));

  /* If UART trasport is used for zigbee data transfer, put zb_mac_transport_hdr_t in it.
     This header is used by pipe_data_router to find out type of received packet, it can be
     zb data, trace and dump */
  {
    struct trace_hdr_s hdr;

#ifdef NEED_SIGNATURE
    hdr.sig[0] = 0xde;
    hdr.sig[1] = 0xad;
#endif
    hdr.h.len = ZB_BUF_LEN(buf) + sizeof(zb_mac_transport_hdr_t);
    if (is_w)
    {
      hdr.h.type = 0x80 | ZB_MAC_TRANSPORT_TYPE_DUMP;
    }
    else
    {
      hdr.h.type = ZB_MAC_TRANSPORT_TYPE_DUMP;
    }
    ZB_HTOLE16(&hdr.h.time, &ZB_TIMER_GET());
    /* synchronus write here */
    zb_8051_serial_put_bytes((zb_uint8_t *)&hdr, sizeof(hdr));
  }
  zb_8051_serial_put_bytes(ZB_BUF_BEGIN(buf), ZB_BUF_LEN(buf));

  TRACE_MSG(TRACE_MAC1, "<<zb_mac_traffic_dump", (FMT__0));
}

#endif /* ZB_SNIFFER */

#endif


#endif /* ZB_TRANSPORT_8051_UART */

/*! @} */
