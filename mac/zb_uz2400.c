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

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_ubec24xx.h"
#include "zb_types.h"
#include "zb_config.h"
#include "zb_secur.h"
#include "mac_internal.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

#ifdef ZB_UZ2400

#ifdef ZB_NS_BUILD
#error "NS build can't be defined in real transiver build"
#endif

#include "zb_bank_2.h"

void zb_ubec_check_int_status()
{
/*
  SREG ISRSTS
  Bit 7 SLPIF: Sleep alert interrupt bit
  Bit 6 WAKEIF: Wake-up alert interrupt bit
  Bit 5 HSYMTMRIF: Half symbol timer interrupt bit
  Bit 4 SECIF: Security key request interrupt bit
  Bit 3 RXIF: RX receive interrupt bit
  Bit 2 TXG2IF: TX GTS2 FIFO transmission interrupt bit
  Bit 1 TXG1IF: TX GTS1 FIFO transmission interrupt bit
  Bit 0 TXNIF: TX Normal FIFO transmission interrupt bit
*/

  TRACE_MSG(TRACE_MAC1, "Ints: %d" , (FMT__D_D, ZB_IOCTX().int_counter));
  ZB_CLEAR_TRANS_INT();
  ZB_READ_SHORT_REG(ZB_SREG_ISRSTS);
  TRACE_MSG(TRACE_MAC1, "interrupt ISRSTS: 0x%hx" , (FMT__H, ZB_MAC_GET_BYTE_VALUE()));
  TRANS_CTX().int_status = ZB_MAC_GET_BYTE_VALUE();

/* if uz2400 is sleeping, it still can produce an interrupt, and we can even handle it,
   but we can't read/write fifo or some long regs */
  if (ZB_MAC_GET_TRANS_SPLEEPING())
  {
    TRACE_MSG(TRACE_COMMON3, "transceiver, WAKE UP", (FMT__0));
    ZB_MAC_CLEAR_TRANS_SPLEEPING();
    zb_uz2400_register_wakeup();
  }

#ifdef ZB_MAC_SECURITY
  zb_uz_secur_handle_rx();
#endif  /* ZB_MAC_SECURITY */

  /* TODO: Fixme after autoack pending bit problem resolved */
  /* check for a pending frame bit in received ACK, but only after DATA_REQUEST */
  if (ZB_MAC_GET_INDIRECT_DATA_REQUEST())
  {
    ZB_READ_SHORT_REG(ZB_SREG_TXNTRIG);
    TRACE_MSG(TRACE_COMMON2, "TXNTRIG read: 0x%hx", (FMT__H, ZB_MAC_GET_BYTE_VALUE()));
#if 0                           /* this check does not work at 2400 (but works
                                 * at 2410) */
    if (ZB_MAC_GET_BYTE_VALUE()&0x10)
#endif
    {
      TRACE_MSG(TRACE_COMMON2, "Pending data set!", (FMT__0));
      ZB_MAC_SET_PENDING_DATA();
    }
  }

  /**/

  if (ZB_UBEC_GET_TX_DATA_STATUS())
  {
    MAC_CTX().tx_cnt++;
    TRACE_MSG(TRACE_MAC1, "TX counter: %hd", (FMT__H, MAC_CTX().tx_cnt));
    ZB_READ_SHORT_REG(ZB_SREG_TXSR);
    TRACE_MSG(TRACE_COMMON3, "tx status: 0x%hx" , (FMT__H, ZB_MAC_GET_BYTE_VALUE()));
    TRANS_CTX().tx_status = ZB_MAC_GET_BYTE_VALUE();
  }
}


/* Access via spidev differs to another methods: it has registers and fifo
 * access implemented in the transport. */
#ifndef ZB_TRANSPORT_LINUX_SPIDEV

static void zb_uz_send_op_buf()
{
  zb_mac_transport_put_data( ZG->mac.mac_ctx.operation_buf );
  /* mac sync i/o mode is used, wait for operation to complete here */
  ZB_SYNC_WAIT_FOR_SEND_COMPLETION();
}


static void zb_uz_complete_send()
{
  zb_uz_send_op_buf();
  ZB_MAC_STOP_IO();
}


static void zb_uz_wait_recv_compl()
{
  /* Now i/o is synchronous */
  ZB_SYNC_WAIT_FOR_RECV_COMPLETION();
  /* send ZB_MAC_STATE_IO_RECV_FINISHED?? */
  ZB_MAC_STOP_IO();
}


static void zb_uz_compl_reg_read()
{
  zb_uz_wait_recv_compl();
  ZG->mac.mac_ctx.rw_reg.value.byte_value = *( ( zb_uint8_t* ) ZB_BUF_BEGIN( MAC_CTX().operation_recv_buf ) );
  ZB_DUMP_INCOMING_DATA(MAC_CTX().operation_recv_buf);
}


/*
  reads short register into byte_value variable
*/
void zb_mac_short_read_reg(zb_uint8_t short_addr)
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_short_read_reg", (FMT__0 ));
  ZB_MAC_START_IO();
  {
    zb_uint8_t *data_ptr = NULL;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 1, data_ptr );
    ZB_ASSERT( data_ptr );
    *data_ptr = ( short_addr << 1 );
  }

#ifdef ZB_TRANSPORT_8051_DATA_UART
  /* if UART is used as transport for ZB data, set recv buffer
   * before starting to send, because IN data comes asynchronously
   * and if no in-buffer is specified, data received will be
   * lost. SPI data is received synchronously with transmit, so
   * recv buffer should be specifed after actual data send is finished. */
  zb_mac_stransport_start_recv(ZG->mac.mac_ctx.operation_recv_buf, 1);
#endif

  zb_uz_send_op_buf();

#ifndef ZB_TRANSPORT_8051_DATA_UART
  zb_mac_transport_start_recv(MAC_CTX().operation_recv_buf, 1);
#endif

  zb_uz_compl_reg_read();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_short_read_reg", (FMT__0));
}


/*
  reads long register into byte_value variable
*/
void zb_mac_long_read_reg(zb_uint16_t long_addr)
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_long_read_reg", (FMT__0 ));
  ZB_MAC_START_IO();
  {
    zb_uint8_t *data_ptr;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 2, data_ptr );
    ZB_ASSERT( data_ptr );
    long_addr = (long_addr << 5) | 0x8000;
    ZB_HTOBE16_VAL(data_ptr, long_addr);
  }

#ifdef ZB_TRANSPORT_8051_DATA_UART
  /* if UART is used as transport for ZB data, set recv buffer
   * before starting to send, because IN data comes asynchronously
   * and if no in-buffer is specified, data received will be
   * lost. SPI data is received synchronously with transmit, so
   * recv buffer should be specifed after actual data send is finished. */
  zb_mac_transport_start_recv(ZG->mac.mac_ctx.operation_recv_buf, 1);
#endif

  zb_uz_send_op_buf();

#ifndef ZB_TRANSPORT_8051_DATA_UART
  zb_mac_transport_start_recv(ZG->mac.mac_ctx.operation_recv_buf, 1);
#endif

  zb_uz_compl_reg_read();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_long_read_reg", (FMT__0));
}


/*
  writes short register, value byte_value variable; register addr is
  stored in short_addr
  @return RET_BLOCKED or RET_OK
*/
void zb_mac_short_write_reg(zb_uint8_t short_addr, zb_uint8_t byte_value)
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_short_write_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
    zb_uint8_t *data_ptr = NULL;
    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 2, data_ptr );
    ZB_ASSERT( data_ptr );
    *data_ptr++ = ( short_addr << 1 ) | 0x01;
    *data_ptr = byte_value;
  }
  zb_uz_complete_send();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_short_write_reg", (FMT__0));
}


/*
  writes long register
*/
void zb_mac_long_write_reg(zb_uint16_t long_addr, zb_uint8_t byte_value)
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_long_write_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
    zb_uint8_t *data_ptr;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 3, data_ptr );
    ZB_ASSERT( data_ptr );

    long_addr = (long_addr << 5) | 0x8010;
    ZB_HTOBE16_VAL(data_ptr, long_addr);
    data_ptr[2] = byte_value;
  }
  zb_uz_complete_send();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_long_write_reg", (FMT__0));
}


/**
 * Writes packet to fifo buffer. Packet should completely formated and
 * written to MAC_CTX().operation_buf. To send send packet via radio use
 * macro ZB_START_FIFO_TX(). To find out if TX finish, wait interrupt
 * UBEC_2400_INTER_NUMBER and check interrupt status ZB_SREG_ISRSTS
 * DS-2400 4.3. Typical TX Operations
 */
void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_write", (FMT__0));

  ZB_MAC_START_IO();
  {
    zb_uint16_t *data_ptr = NULL;

    ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_uint16_t), data_ptr);
    ZB_ASSERT( data_ptr );
    ZB_HTOBE16_VAL(data_ptr, (long_addr << 5) | 0x8010);
  }
  zb_mac_transport_put_data(buf);
  /* mac sync i/o mode is used, wait for operation to complete here */
  ZB_SYNC_WAIT_FOR_SEND_COMPLETION();
  ZB_MAC_STOP_IO();

  TRACE_MSG( TRACE_MAC2, "<< zb_uz2400_fifo_write", (FMT__0));
}

/*
  Read from fifo;
*/
void zb_uz2400_fifo_read(zb_uint8_t tx_fifo, zb_buf_t *buf, zb_uint8_t len) ZB_SDCC_REENTRANT
{

/*
  RXFIFO address
  |   bit[11]    |       bit [10:1]      |        bit[0]    |
  | Long Addr = 1| address 0x300 - 0x38f | read/write = 0/1 |
*/
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_read %hd", (FMT__H, len));

  ZB_MAC_START_IO();
  {
    zb_uint16_t *data_ptr = NULL;

    ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, sizeof(zb_uint16_t), data_ptr);
    ZB_ASSERT(data_ptr);

    /* Only normal RXFIFO is supported now */
    ZB_HTOBE16_VAL(data_ptr, ((tx_fifo ? ZB_NORMAL_TX_FIFO : ZB_RX_FIFO) << 5) | 0x8000 );
  }

#ifdef ZB_TRANSPORT_8051_DATA_UART
  /* if UART is used as transport for ZB data, set recv buffer
   * before starting to send, because IN data comes asynchronously
   * and if no in-buffer is specified, data received will be
   * lost. SPI data is received synchronously with transmit, so
   * recv buffer should be specifed after actual data send is finished. */
  zb_mac_transport_start_recv(buf, 0);
#endif

  zb_uz_send_op_buf();

#ifndef ZB_TRANSPORT_8051_DATA_UART
  zb_mac_transport_start_recv(buf, len);
#endif

  zb_uz_wait_recv_compl();

  TRACE_MSG(TRACE_MAC2, "<< zb_uz2400_fifo_read", (FMT__0));
}

#elif defined ZB_TRANSPORT_LINUX_SPIDEV


/* spidev transport has its own registers read-write and fifo read-write */


/**
 * Writes packet to fifo buffer.
 *
 * spidev transport is syncronous, not need to wait for interrupt here.
 * @return RET_OK
 */
void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_write buf %hd len %hd", (FMT__H_H, ZB_REF_FROM_BUF(buf), ZB_BUF_LEN(buf)));

  zb_spidev_long_write_fifo(long_addr, ZB_BUF_BEGIN(buf), ZB_BUF_LEN(buf));
  zb_buf_cut_left(buf, sizeof(zb_ubec_fifo_header_t));
  ZB_MAC_STOP_IO();
/*  MAC_SET_STATE_FOR_LAYER(ZB_MAC_IO_LAYER_TX, ZB_MAC_STATE_IDLE);*/

  TRACE_MSG( TRACE_MAC2, "<< zb_uz2400_fifo_write, l %hd", (FMT__H_D, ZB_BUF_LEN(buf)));
}

/**
   Read from FIFO using spidev Linux/ARM transport.

   Read is synchronous.

   @param len - length to read from fifo. If len == 0, read from rx fifo, first
   byte is a packet length. If len > 0, this is read of encryption result from tx fifo.

   @return RET_OK
*/
void zb_uz2400_fifo_read(zb_uint8_t tx_fifo, zb_buf_t *buf, zb_uint8_t len)
{
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_read", (FMT__0));

  if (len)
  {
    /* if len != 0, this is read of encryption result from tx fifo */
    zb_buf_initial_alloc(buf, len);
    zb_spidev_long_read_fifo(tx_fifo ? ZB_NORMAL_TX_FIFO : ZB_RX_FIFO,
                             ZB_BUF_BEGIN(buf), len);
  }
  else
  {
    /* if len == 0, this is read from rx fifo. Must first read packet length
     * then packet body */
    zb_spidev_long_read(ZB_RX_FIFO, &len);
    len += ZB_MAC_PACKET_LENGTH_SIZE + ZB_MAC_EXTRA_DATA_SIZE;
    zb_buf_initial_alloc(buf, len);
    /* really read packet length twice to simplify traffic analyzed logic */
    zb_spidev_long_read_fifo(ZB_RX_FIFO, ZB_BUF_BEGIN(buf), len);
  }

  /* spidev i/o is syncronous, so set states to proceed immediatly */
  ZB_MAC_STOP_IO();
  TRACE_MSG(TRACE_MAC2, "<< zb_uz2400_fifo_read %i", (FMT__0));
}

#else

#error "PORT ME!!!!"

#endif  /* transports choice */


#if 0
void zb_uz24_write_long_reg_array(zb_uint16_t start_reg, zb_uint8_t *array, zb_uint8_t cnt)
{
  zb_ushort_t i;
  for (i = 0 ; i < cnt ; i++)
  {
    ZB_WRITE_LONG_REG(start_reg + i, array[i]);
  }
}
#endif


void init_zu2400()
{
  TRACE_MSG( TRACE_MAC1, ">> init_zu2400", (FMT__0));
  ZB_DISABLE_TRANSIVER_INT();
/*
  Perform software reset,
  DS-2400
  4.2.4. Initialization

  short register SOFTRST
  Bit 2 RSTPWR: Power management reset = 0
  Bit 1 RSTBB: Baseband reset          = 1
  Bit 0 RSTMAC: MAC reset              = 1

  in DS-2400 it is written to set rstpwr bit, but ubec stack src doesn't do it
  0000.0011 == 0x03
*/

  ZB_WRITE_SHORT_REG(ZB_SREG_SOFTRST, 0x03);
  ZB_8051_DELAY();

  ZB_UPDATE_LONGMAC();


  /* TODO: optimize by size - implement data structure with reg/value pairs and
   * routine interpreting it. Note that can easiely distingush between
   * short/long regs: long regs are over 0x100 */

  /* Initialization procedure described at DS-2400-51 <Rev. 0.6> p.54-55/180 */
#if 0
  ZB_WRITE_SHORT_REG(ZB_SREG_GATECLK, 0x20);
  ZB_WRITE_SHORT_REG(ZB_SREG_PACON1, 0x08);
  ZB_WRITE_SHORT_REG(ZB_SREG_FIFOEN, 0x94);
  ZB_WRITE_SHORT_REG(ZB_SREG_TXPEMISP, 0x95);
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG3, 0x50);
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG5, 0x07);
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG6, 0x40);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL0, 0x03);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL1, 0x02);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL2, 0x66);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL4, 0x09);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL6, 0x30);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL7, 0xEC);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL8, 0x8C);
  ZB_WRITE_LONG_REG(ZB_LREG_GPIODIR, 0x00);
  ZB_WRITE_LONG_REG(ZB_LREG_SECCTRL, 0x20);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL50, 0x05);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL51, 0xC0);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL52, 0x01);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL59, 0x00);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL73, 0x40);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL74, 0xC5);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL75, 0x13);
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL76, 0x07);
  ZB_WRITE_SHORT_REG(ZB_SREG_INTMSK, 0x00);
  ZB_WRITE_SHORT_REG(ZB_SREG_SOFTRST, 0x02);
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x04);
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x00);
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x02);

#else
  {
    static ZB_CODE zb_uz_reg_wr_str_t s_wregs[] =
      {
        {ZB_SREG_GATECLK, 0x20},
        {ZB_SREG_PACON1, 0x08},
        {ZB_SREG_FIFOEN, 0x94},
        {ZB_SREG_TXPEMISP, 0x95},
        {ZB_SREG_BBREG3, 0x50},
        {ZB_SREG_BBREG5, 0x07},
        {ZB_SREG_BBREG6, 0x40},
        {ZB_LREG_RFCTRL0, 0x03},
        {ZB_LREG_RFCTRL1, 0x02},
        {ZB_LREG_RFCTRL2, 0x66},
        {ZB_LREG_RFCTRL4, 0x09},
        {ZB_LREG_RFCTRL6, 0x30},
        {ZB_LREG_RFCTRL7, 0xEC},
        {ZB_LREG_RFCTRL8, 0x8C},
        {ZB_LREG_GPIODIR, 0x00},
        {ZB_LREG_SECCTRL, 0x20},
        {ZB_LREG_RFCTRL50, 0x05},
        {ZB_LREG_RFCTRL51, 0xC0},
        {ZB_LREG_RFCTRL52, 0x01},
        {ZB_LREG_RFCTRL59, 0x00},
        {ZB_LREG_RFCTRL73, 0x40},
        {ZB_LREG_RFCTRL74, 0xC5},
        {ZB_LREG_RFCTRL75, 0x13},
        {ZB_LREG_RFCTRL76, 0x07},
        {ZB_SREG_INTMSK, 0x00},
        {ZB_SREG_SOFTRST, 0x02},
        {ZB_SREG_RFCTL, 0x04},
        {ZB_SREG_RFCTL, 0x00},
        {ZB_SREG_RFCTL, 0x02}
      };
    ZB_UZ_WRITE_REGS_ARRAY(s_wregs, (sizeof(s_wregs) / sizeof(s_wregs[0])));
  }
#endif

  /* TODO: check, is it enough for "192us waiting" */
  ZB_8051_DELAY();
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x01);
  /* TODO: check, is it enough for "192us waiting" */
  ZB_8051_DELAY();
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x00);

  /* LREG 0x23C RXFRMTYPE
     Now, it's disabled, according to DS-2400-51_v0_6_RN.pdf, p.158
     RXFTYPE[7:0]: RX Frame Type Filter
     00001011: (default - Do Not Change)
     bit 7-4 reserver
     bit3 command
     bit2 ack                                                                                          ?
     bit1 data
     bit0 beacon */

  ZB_WRITE_LONG_REG(ZB_LREG_RXFRMTYPE, 0x0B); /* we accept all frames */

/*
  LREG SCLKDIV
  Bit 7 I2CWDTEN: I2C watchdog timer enable
  Bit 4-0 SCLKDIV: sleep clock division selection.
  n: the sleep clock is divided by 2^n before being fed to logic circuit.

  SCLKDIV = 1
  I2CWDTEN = 0
  0000.0001 == 0x1
*/
  ZB_WRITE_LONG_REG(ZB_LREG_SCLKDIV, 0x01);

/*
  SREG INTMSK
  Bit 7 SLPMSK: Sleep alert interrupt mask
  Bit 6 WAKEMSK: Wake-up alert interrupt mask
  Bit 5 HSYMTMRMSK: Half symbol timer interrupt mask
  Bit 4 SECMSK: security interrupt mask
  Bit 3 RXMSK: RX receive interrupt mask
  Bit 2 TXG2MSK: TX GTS2 FIFO transmission interrupt mask
  Bit 1 TXG1MSK: TX GTS1 FIFO transmission interrupt mask
  Bit 0 TXNMSK: TX Normal FIFO transmission interrupt mask

  Read original value and turn on wakeup interrupt
  wakemsk = 0
  1011.1111 == 0xbf
*/
  zb_uz24_mask_short_reg(ZB_SREG_INTMSK, 0xBF);

/*
  SREG SLPACK
  Bit 7 SLPACK: Sleep acknowledge. Set this bit to 1 will cause UZ2400 enter the sleep mode
  immediately. This bit will be automatically cleared to 0.
  Bit 3-0 WAKECNT: System clock (20MHz) recovery time

  Read original value and set bit to enter sleep mode
  slpack = 1
  1000.0000 == 0x80
*/
#if 0
  if (ret == RET_OK)
  {
    ret = ZB_READ_SHORT_REG(ZB_SREG_SLPACK);
  }
  if (ret == RET_OK)
  {
    ZB_MAC_GET_BYTE_VALUE() |= 0x80;
    ret = ZB_WRITE_SHORT_REG(ZB_SREG_SLPACK, ZB_MAC_GET_BYTE_VALUE());
  }
#endif
/* TODO: check - ubec stack uses long cycle with nop here */

/*
  SREG WAKECTL
  Bit 7 IMMWAKE: Immediate wake-up mode enables.
  Bit 6 REGWAKE: Register-triggered wake-up signal. It should be
  cleared to 0 by host MCU.

  Read original value and set 2 bits, turn on external wake up mode
  1100.0000 == 0xC0
*/

  zb_uz24_or_mask_short_reg(ZB_SREG_WAKECTL, 0xC0);

/* TODO: check - ubec stack uses long cycle with nop here */

/* TODO: check, in ubec stack interrupts are disabled here */

/*
  SREG WAKECTL
  Bit 7 IMMWAKE: Immediate wake-up mode enables.
  Bit 6 REGWAKE: Register-triggered wake-up signal. It should be
  cleared to 0 by host MCU.

  Read original value and clear regwake to wake up the chip
  1011.1111 == 0xBF
*/
  zb_uz24_mask_short_reg(ZB_SREG_WAKECTL, 0xBF);
/*
  SREG ISRSTS
  Bit 7 SLPIF: Sleep alert interrupt bit
  Bit 6 WAKEIF: Wake-up alert interrupt bit
  Bit 5 HSYMTMRIF: Half symbol timer interrupt bit
  Bit 4 SECIF: Security key request interrupt bit
  Bit 3 RXIF: RX receive interrupt bit
  Bit 2 TXG2IF: TX GTS2 FIFO transmission interrupt bit
  Bit 1 TXG1IF: TX GTS1 FIFO transmission interrupt bit
  Bit 0 TXNIF: TX Normal FIFO transmission interrupt bit

  check wakeif bit
  0100.0000 == 0x40
*/

  /* wait for chip ready */
  do
  {
    ZB_READ_SHORT_REG(ZB_SREG_ISRSTS);
  }
  while (!(ZB_MAC_GET_BYTE_VALUE() & 0x40));

/*
  SREG WAKECTL
  Bit 7 IMMWAKE: Immediate wake-up mode enables.
  Bit 6 REGWAKE: Register-triggered wake-up signal. It should be
  cleared to 0 by host MCU.

  immwake = 0 turn back normal wake up mode
  0111.1111 == 0x7f
*/

  zb_uz24_mask_short_reg(ZB_SREG_WAKECTL, 0x7F);

  /* Enable all interrupts */
  ZB_WRITE_SHORT_REG(ZB_SREG_INTMSK, 0);

  /* TODO: check, in ubec stack interrupts are enabled here */
  ZB_ENABLE_TRANSIVER_INT();

  TRACE_MSG( TRACE_MAC1, "<< init_zu2400", (FMT__0));
}


/*
  Sets transceiver mode to receive only beacon frames, sets promiscuous mode
  @return RET_OK on success RET_ERROR on error
*/
void zb_transceiver_set_beacon_only_mode()
{
/* set promiscuous mode */
  zb_uz24_or_mask_short_reg(ZB_SREG_RXMCR, 0x01);
}

/*
  Sets transceiver to normal mode receives all frames with normal filtering
  @return RET_OK on success RET_ERROR on error
*/
void zb_transceiver_normal_mode()
{
/* set promiscuous mode */
  zb_uz24_mask_short_reg(ZB_SREG_RXMCR, 0xFE);
}


void zb_tansceiver_set_auto_ack()
{
  /* NOACKRSP: Disable automatic acknowledge ACK response */
  /* TODO: set NOACKRSP bit to 0*/
}

zb_ret_t zb_transceiver_init()
{
  zb_ret_t ret = RET_OK;

  MAC_CTX().current_channel = 0xff;

  return ret;
}

#ifdef C8051F120
void zb_ext_int_init()
{
  SFRPAGE = CONFIG_PAGE;
  XBR1 = 0x19; /* enable INT1 in crossbar */
  SFRPAGE = LEGACY_PAGE;
  IT1 = 1; /* INT1 type (level(0), edge(1)) */
  PX1 = 1; /* INT1 high priority */
  EX1 = 1; /* INT1 enable */
}
#endif


#endif  /* ZB_UZ2400 */


/*! @} */
