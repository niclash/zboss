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
PURPOSE: ubec uz2400 specific
*/
#ifdef ZB_CC25XX
#include "zb_common.h"
#include "zb_cc25xx.h"
#include "ioCC2530.h"

#ifndef ZB_SNIFFER
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_secur.h"
#endif /* ZB_SNIFFER */

void init_clock()
{
    if (CLOCK_SOURCE == CLOCK_XTAL)
    {
      CLKCONCMD &= ~0x80;  /*mask slow 32k clock oscillator */
    }
    else if (CLOCK_SOURCE == CLOCK_RC)
    {
      CLKCONCMD |= 0x40;  /* bit mask, for the system clock oscillator */
    }
    /* possibly, we need to wait here for clock update */
}


void clock_set_src()
{
    register zb_uint8_t osc32k_bm = CLKCONCMD & 0x80;

    SLEEPCMD &= ~0x04;       /* power up oscillators */
    /* wait until ready */
    while (!(SLEEPSTA & 0x20) || (SLEEPSTA & 0x04));

    if (CLOCK_SRC == CLOCK_RC_SRC)
    {
        CLKCONCMD = (osc32k_bm | 0x40 | (0x01 << 3) | 0x01);
    }
    else
    {
        CLKCONCMD = (osc32k_bm | (0x00 << 3));
    }
    /* possibly, we need to wait here for clock update */
    SLEEPCMD |= 0x04;        /* power down the unused oscillator*/
}


void init_zu2400()
{
  /* setup 32mhz clock */
  /* optimize me! Increasing call nesting isn't welcome. */
#if 0
  init_clock(); /* transceiver needs clock for carrying frequency modulation */
  clock_set_src();
#endif
  /* RF INIT */
#ifndef ZB_SNIFFER
  /* Enable auto ack, CRC */
  FRMCTRL0 |= (0x20 | 0x40);
  FRMCTRL1 |=0x02;
#else
  /* Enable auto CRC only */
  FRMCTRL0 |= 0x40;
  FRMCTRL1 &= ~0x02;
#endif /* ZB_SNIFFER */

  /* Recommended RX settings, copy/paste */
  TXFILTCFG = 0x09;
  AGCCTRL1  = 0x15;
  FSCAL1    = 0x00;

  /* Enable RX interrupt*/
  /* RXPKTDONE interrupt */
  RFIRQM0 |= 1<<6;
  /* general RF interrupts */
  IEN2 |= 0x01;
#ifndef ZB_SNIFFER
  FRMFILT0 = 0x0D; /* filter Enabled */
#else /* set transciever to promiscuous mode */
  FRMFILT0 = 0;
#endif /* ZB_SNIFFER */
  FRMFILT1 = 0x78;
  SRCMATCH = 0;
  EA = 1; /* enabling all interrupts */

}

void zb_ubec_check_int_status()
{
  ZB_CLEAR_TRANS_INT();
}

void zb_transceiver_set_channel(zb_uint8_t channel_number)
{
  /* 5 is a frequency step */
#ifndef ZB_SNIFFER
  MAC_CTX().current_channel = channel_number;
#endif
  FREQCTRL = (ZB_TRANSCEIVER_START_CHANNEL_NUMBER + (channel_number - ZB_TRANSCEIVER_START_CHANNEL_NUMBER) * 5);
}

#ifndef ZB_SNIFFER

void zb_uz2400_fifo_read(zb_uint8_t tx_fifo, zb_buf_t *buf, zb_uint8_t len) ZB_SDCC_REENTRANT
{
/*
  RXFIFO address
  |   bit[11]    |       bit [10:1]      |        bit[0]    |
  | Long Addr = 1| address 0x300 - 0x38f | read/write = 0/1 |
*/
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_read", (FMT__0));

  ZB_MAC_START_IO();
#ifdef ZB_TRAFFIC_DUMP_ON
  {
    zb_uint16_t *data_ptr = NULL;

    ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, sizeof(zb_uint16_t), data_ptr);
    ZB_ASSERT(data_ptr);

    /* Produce same traffic dump as 2400 */
    ZB_HTOBE16_VAL(data_ptr, ((tx_fifo ? ZB_NORMAL_TX_FIFO : ZB_RX_FIFO) << 5) | 0x8000 );
  }
  ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);
#endif

  /* actual i/o */
  {
    zb_uint8_t ZB_XDATA *xptr;

    xptr = (zb_uint8_t ZB_XDATA *)(tx_fifo ? ZB_NORMAL_FIFO_ADDR : ZB_NORMAL_RXFIFO_ADDR);
    if (len == 0)
    {
      len =  *(xptr) + ZB_MAC_PACKET_LENGTH_SIZE + ZB_MAC_EXTRA_DATA_SIZE;
    }
    ZB_MEMCPY(ZB_BUF_BEGIN(buf), xptr, len);
    buf->u.hdr.len = len;
  }

  ZB_MAC_STOP_IO();

  TRACE_MSG(TRACE_MAC2, "<< zb_uz2400_fifo_read %i", (FMT__0));
}

void zb_uz_short_reg_write_2b(zb_uint8_t reg, zb_uint16_t v)
{
  *(zb_uint16_t ZB_XDATA *)(reg) = v;

}

void zb_transceiver_set_coord_ext_addr(zb_ieee_addr_t coord_addr_long)
{
}

void zb_transceiver_set_coord_short_addr(zb_uint16_t coord_addr_short)
{
}


void zb_mac_short_write_reg(zb_uint8_t short_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT
{
}

void zb_mac_long_write_reg(zb_uint16_t long_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT
{
}

void zb_mac_short_read_reg(zb_uint8_t short_addr) ZB_SDCC_REENTRANT
{
}

void zb_transceiver_update_long_mac()
{
  EXT_ADDR0 = ZB_PIB_EXTENDED_ADDRESS()[0];
  EXT_ADDR1 = ZB_PIB_EXTENDED_ADDRESS()[1];
  EXT_ADDR2 = ZB_PIB_EXTENDED_ADDRESS()[2];
  EXT_ADDR3 = ZB_PIB_EXTENDED_ADDRESS()[3];
  EXT_ADDR4 = ZB_PIB_EXTENDED_ADDRESS()[4];
  EXT_ADDR5 = ZB_PIB_EXTENDED_ADDRESS()[5];
  EXT_ADDR6 = ZB_PIB_EXTENDED_ADDRESS()[6];
  EXT_ADDR7 = ZB_PIB_EXTENDED_ADDRESS()[7];
}

void zb_set_pan_id(zb_uint16_t pan_id)
{
   PAN_ID0 = (pan_id &0xFF);
   PAN_ID1 = ((pan_id>>8)&0xFF);
}

void zb_transceiver_get_rssi(zb_uint8_t *rssi_value)
{
   zb_uint16_t tmp = 1000; /* not sure how long should we wait here */
   FRMCTRL0 =  0x0C;
   ISFLUSHRX(); /* flush rx fifo */
   RFST = 0xE3; /* ISRXON */\
   while (!RSSISTAT);
   FRMCTRL0 |= 0x10; /* energy scan enabled */
   while(tmp--)
    {
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
        asm("NOP");
    }
   *rssi_value = RSSI;
   /* restore */
   RFST = 0xEF;
   ISFLUSHRX();

   FRMCTRL0 =  (0x20 | 0x40);
}

zb_ret_t zb_transceiver_send_fifo_packet(zb_uint8_t header_length, zb_uint16_t fifo_addr,
                                         zb_buf_t *buf, zb_uint8_t need_tx) ZB_SDCC_REENTRANT
{
  zb_uint8_t *fc = ZB_BUF_BEGIN(buf);

  TRACE_MSG(TRACE_MAC1, ">> zb_transceiver_send_fifo_packet, %d, addr %x, buf %p, state %hd", (FMT__D_D_P,
                                                                                               (zb_uint16_t)header_length, fifo_addr, buf));

  ZB_ASSERT(fifo_addr == ZB_NORMAL_TX_FIFO);

/* ds-2400 4.3.1. Transmit Packet in Normal FIFO */

  {
    zb_ubec_fifo_header_t *ubec_fifo_header;
    ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_ubec_fifo_header_t), ubec_fifo_header);
    ZB_ASSERT(ubec_fifo_header);
    /* ZB_CC2530_FIFO_TAIL is crc + rssi */
    ubec_fifo_header->frame_length = ZB_BUF_LEN(buf) - sizeof(zb_ubec_fifo_header_t) + ZB_CC2530_FIFO_TAIL;
#ifdef ZB_TRAFFIC_DUMP_ON
    zb_uint8_t *header_len_tmp = ZB_GET_BUF_PARAM(buf, zb_uint8_t);
    *header_len_tmp = header_length;
#endif    
  }

  ZB_UBEC_CLEAR_NORMAL_FIFO_TX_STATUS();
  ZB_CLEAR_TX_STATUS();

  zb_uz2400_fifo_write(fifo_addr, buf);

  /* TODO: if acknowledgement is required for normal fifo, set ackreq
   * bit (SREG0x1B[2]) */
  if (need_tx)
  {
    /* we need to determine if our frame broadcast or not */

    /* Don't want to parse entire mhr here. All we need is frame control and
     * destination address. Destination address has fixed position in mhr.
     * Fields layout is fc (2b), seq number (1b), dest panid (2b), dest address (2b).
     */
    zb_uint8_t need_ack = (!((ZB_FCF_GET_FRAME_TYPE(fc) == MAC_FRAME_BEACON
                              || (ZB_FCF_GET_DST_ADDRESSING_MODE(fc) == ZB_ADDR_16BIT_DEV_OR_BROADCAST
                                  && fc[5] == 0xff && fc[6] == 0xff)))
                           && ZB_FCF_GET_ACK_REQUEST_BIT(fc));

    /* The same bit is used to start normal and beacon fifio.
       If not joined yet (pac_pan_id is not set), do not request acks.
    */
    TRACE_MSG(TRACE_MAC2, "Need ACK: %hd", (FMT__H, need_ack));
    ZB_START_NORMAL_FIFO_TX(ZB_MAC_PIB_MAX_FRAME_RETRIES, need_ack);
    if (need_ack)
    {
      ZB_MAC_START_ACK_WAITING();
    }
    else
    {
      ZB_MAC_SET_ACK_OK();
    }
  }
  TRACE_MSG(TRACE_MAC1, "<< zb_transceiver_send_fifo_packet", (FMT__0));
  return RET_OK;
}

void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_write", (FMT__0));
  ZVUNUSED(long_addr);

  ZB_MAC_START_IO();


  /* prepare transceiver */
  /* disable rx interrupt */
  RFIRQM0 &= ~(1<<6);
  /* disable general RF interrupts */
  IEN2 &= ~(0x01);
  ISFLUSHTX();
  ISTXON();
  RFIRQF1 = ~0x02; /* tx done interrupt cleared */
  {
    zb_uint8_t i;
    for (i = 0;i<ZB_BUF_LEN(buf);i++)
    {
      RFD = *(zb_uint8_t *)(ZB_BUF_BEGIN(buf)+i);
    }
  }
  IEN2 |= 0x01; /* enable rf general interrupt back */
  RFIRQM1 |= 0x03;
  RFIRQM0 |= (1<<6);
  ISFLUSHRX(); /* flush rx */
  RFST = 0xE3; /* rx on    */



#ifdef ZB_TRAFFIC_DUMP_ON
  {
    zb_uint16_t *data_ptr = NULL;

    ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_uint16_t)+sizeof(zb_uint8_t), data_ptr);
    ZB_ASSERT( data_ptr );
    /* change for TI? */
    ZB_HTOBE16_VAL(data_ptr, (long_addr << 5) | 0x8010);
    /* wireshark plugin compatibility */
    *(zb_uint8_t *)((zb_uint8_t *)data_ptr+sizeof(zb_uint16_t)) = *(zb_uint8_t *)(ZB_GET_BUF_PARAM(buf, zb_uint8_t));
    *(zb_uint8_t *)((zb_uint8_t *)data_ptr+sizeof(zb_uint16_t)+sizeof(zb_uint8_t))-=ZB_CC2530_FIFO_TAIL;
  }
  ZB_DUMP_OUTGOING_DATA(buf);

  /* remove headers */
  ZB_BUF_CUT_LEFT2((buf), sizeof(zb_ubec_fifo_header_t)+sizeof(zb_uint16_t)+sizeof(zb_uint8_t)/*header length, for wireshark compatibility*/);
#else
  ZB_BUF_CUT_LEFT2((buf), sizeof(zb_ubec_fifo_header_t));
#endif
  ZB_MAC_STOP_IO();
  TRACE_MSG( TRACE_MAC2, "<< zb_uz2400_fifo_write", (FMT__0));
}

#endif /* ZB_SNIFFER */


#endif
