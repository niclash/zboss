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
PURPOSE: ubec uz2410 specific
*/
#include "zb_common.h"

#ifdef ZB_UZ2410

#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_secur.h"
#include "zb_ubec24xx.h"
#include "zb_uz2410.h"
#ifdef ZB_SECURITY
#include "zb_secur.h"
#endif

#include "zb_bank_1.h"

void zb_ext_int_init()
{
  /* enable mac interrupt */
  EI6 = 0x01;
}

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

  ZB_CLEAR_TRANS_INT();
#ifndef ZB_USE_RX_QUEUE
  ZB_READ_SHORT_REG(ZB_SREG_ISRSTS);
  TRACE_MSG(TRACE_MAC1, "ISRSTS: 0x%hx" , (FMT__H, ZB_MAC_GET_BYTE_VALUE()));
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
    if (ZB_MAC_GET_BYTE_VALUE() & 0x10)
    {
      TRACE_MSG(TRACE_COMMON2, "Pending data set!", (FMT__0));
      ZB_MAC_SET_PENDING_DATA();
    }
    else
    {
      ZB_MAC_CLEAR_PENDING_DATA();
      TRACE_MSG(TRACE_MAC2, "Pending data clear", (FMT__0));
    }
    TRACE_MSG( TRACE_MAC1, "pending data %hd", (FMT__H, ZB_MAC_GET_PENDING_DATA()));
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
#endif
}


void zb_mac_short_read_reg(zb_uint8_t short_addr) ZB_SDCC_REENTRANT
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_short_read_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
#ifdef ZB_TRAFFIC_DUMP_ON
    zb_uint8_t *data_ptr = NULL;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 1, data_ptr );
    ZB_ASSERT( data_ptr );
    *data_ptr = (short_addr << 1);
    ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);
#endif
    /* This is actual i/o */
    ZG->mac.mac_ctx.rw_reg.value.byte_value = *((zb_uint8_t ZB_XDATA *)(short_addr|ZB_SHORT_REGS_BASE));

#ifdef ZB_TRAFFIC_DUMP_ON
    *data_ptr = ZG->mac.mac_ctx.rw_reg.value.byte_value;
    ZB_DUMP_INCOMING_DATA(MAC_CTX().operation_buf);
#endif
  }
  ZB_MAC_STOP_IO();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_short_read_reg", (FMT__0));
}


void zb_mac_short_write_reg(zb_uint8_t short_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_short_write_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
#ifdef ZB_TRAFFIC_DUMP_ON
    zb_uint8_t *data_ptr;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 2, data_ptr );
    ZB_ASSERT( data_ptr );
    data_ptr[0] = (short_addr << 1) | 0x01;
    data_ptr[1] = byte_value;
    ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);
#endif

    /* actual i/o */
    *((zb_uint8_t ZB_XDATA *)(short_addr|ZB_SHORT_REGS_BASE)) = byte_value;
  }
  ZB_MAC_STOP_IO();
  TRACE_MSG( TRACE_MAC2, "-zb_mac_short_write_reg", (FMT__0 ));
}


void zb_mac_long_read_reg(zb_uint16_t long_addr) ZB_SDCC_REENTRANT
{

  TRACE_MSG( TRACE_MAC2, "+zb_mac_long_read_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
#ifdef ZB_TRAFFIC_DUMP_ON
    zb_uint8_t *data_ptr;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 2, data_ptr );
    ZB_ASSERT( data_ptr );
    ZB_HTOBE16_VAL(data_ptr, ( long_addr << 5 ) | 0x8000);
    ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 1, data_ptr );
#endif
    /* actual i/o here */
    ZG->mac.mac_ctx.rw_reg.value.byte_value = *((zb_uint8_t ZB_XDATA *)(long_addr|ZB_LONG_REGS_BASE));

#ifdef ZB_TRAFFIC_DUMP_ON
    *data_ptr = ZG->mac.mac_ctx.rw_reg.value.byte_value;
    ZB_DUMP_INCOMING_DATA(MAC_CTX().operation_buf);
#endif
  }
  ZB_MAC_STOP_IO();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_long_read_reg", (FMT__0));
}


void zb_mac_long_write_reg(zb_uint16_t long_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT
{
  TRACE_MSG( TRACE_MAC2, "+zb_mac_long_write_reg", (FMT__0 ));

  ZB_MAC_START_IO();
  {
#ifdef ZB_TRAFFIC_DUMP_ON
    zb_uint8_t *data_ptr = NULL;

    ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 3, data_ptr );
    ZB_ASSERT( data_ptr );
    ZB_HTOBE16_VAL(data_ptr, ( long_addr << 5 ) | 0x8010);
    data_ptr[2] = byte_value;
    ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);
#endif
    /* actual i/o here */
    *((zb_uint8_t ZB_XDATA *)(long_addr|ZB_LONG_REGS_BASE)) = byte_value;
  }
  ZB_MAC_STOP_IO();

  TRACE_MSG( TRACE_MAC2, "-zb_mac_long_write_reg", (FMT__0 ));
}


void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  TRACE_MSG(TRACE_MAC2, ">> zb_uz2400_fifo_write", (FMT__0));
  ZVUNUSED(long_addr);

  ZB_ASSERT(MAC_STATE_FOR_LAYER(ZB_MAC_IO_LAYER_TX) == ZB_MAC_STATE_IDLE);
  ZB_MAC_START_IO();

  /* actual i/o here */
  ZB_MEMCPY((zb_uint8_t ZB_XDATA *)ZB_NORMAL_FIFO_ADDR, ZB_BUF_BEGIN(buf), ZB_BUF_LEN(buf));

#ifdef ZB_TRAFFIC_DUMP_ON
  {
    zb_uint16_t *data_ptr = NULL;

    ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_uint16_t), data_ptr);
    ZB_ASSERT( data_ptr );
    ZB_HTOBE16_VAL(data_ptr, (long_addr << 5) | 0x8010);
  }
  ZB_DUMP_OUTGOING_DATA(buf);

  /* remove headers */
  ZB_BUF_CUT_LEFT2((buf), sizeof(zb_ubec_fifo_header_t)+sizeof(zb_uint16_t));
#else
  ZB_BUF_CUT_LEFT2((buf), sizeof(zb_ubec_fifo_header_t));
#endif
  ZB_MAC_STOP_IO();
  TRACE_MSG( TRACE_MAC2, "<< zb_uz2400_fifo_write", (FMT__0));
}


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
    ZB_HTOBE16_VAL(data_ptr, ((tx_fifo ? ZB_NORMAL_TX_FIFO : ZB_RX_FIFO)));
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
  //CHIPCFG |= 0x88; /* taken from the sample source code */
  CHIPCFG |= 0x9a; /* taken from the sample source code */
  ZB_WRITE_SHORT_REG(ZB_SREG_SOFTRST, 0x02);
  ZB_UPDATE_LONGMAC();


#if 0
  /* LREG 0x23C RXFRMTYPE
     Now, it's disabled, according to DS-2400-51_v0_6_RN.pdf, p.158
     RXFTYPE[7:0]: RX Frame Type Filter
     00001011: (default - Do Not Change)
     bit 7-4 reserver
     bit3 command
     bit2 ack
     bit1 data
     bit0 beacon */

  ZB_WRITE_LONG_REG(ZB_LREG_RXFRMTYPE, 0x0B); /* we accept all frames */

/*
  SREG FIFOEN
  Bit 7 FIFOEN: TXFIFO and RXFIFO output enable manual control
  1: TXFIFO and RXFIFO are always output enabled.
  Note: Setting Bit7 value to 0 is forbidden, or fatal error will occur.
  Bit 5-2 TXONTS: The last symbol number before TX. The minimum value is 1.
  Bit 1-0 TXONT: The period that rfmode1 active before TX.

  fifoen = 1
  txonts = 2
  txont  = 0
  1000.1000 == 0x88

  NOTE: in DS-2400 it is written to set value 0x98
*/
  ZB_WRITE_SHORT_REG(ZB_SREG_FIFOEN, 0x88);


/*
  SREG BBREG6
  Bit 7 RSSIMODE1: RSSI mode 1 enable
  Bit 6 RSSIMODE2: RSSI mode 2 enable
  Bit 0 RSSIRDY: RSSI ready signal for RSSIMODE1 use

  Append RSSI value in Rx packets
  RSSIMODE2 = 1
  0100.0000 == 0x40
*/
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG6, 0x40);

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
  LREG RFCTL7
  Bit 7-6 SLPCLK: Sleep clock source selection
  10: internal ring oscillator
  01: external crystal

  1000.0000 == 0x80
*/
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTL7, 0x80);

/*
  LREG RFCTL2
  Bit 7 PLLCTL: RF Phase Lock Loop (PLL) control
  Bit 6-5 RSSIDC: RSSI DC level shift
  Bit 4-3 RSSISLOPE: RSSI range control

  pllctl = 1
  rssidc = 0
  rssislope = 0
  1000.0000 == 0x80
*/
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTL2, 0x80);

/*
  LREG RFCTRL6
  Bit 7 TXFIL: TX filter control
  Bit 5-4 20MRECVR: 20MHz clock recovery time (recovery from sleep) control
  Bit 3 BATEN: Battery monitor enable

  txfil = 1
  20mrecvr = 1
  1001.0000 == 0x90
*/
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTL6, 0x90);

/*
  LREG RFCTRL8
  Bit 4 RFVCO: VCO control. Recommend value is 1.
  0001.0000 == 0x10
*/
  ZB_WRITE_LONG_REG(ZB_LREG_RFCTL8, 0x10);

/*
  SREG RFCTL
  Bit 4 -3 WAKECNTEXT: 20MHz clock recovery time extension bits
  Bit 2 RFRST: RF state reset.
  Reset RF state. RF state must be reset in order to change the RF channels.
  Bit 1 RFTXMODE: RF is forced into TX mode.
  Bit 0 RFRXMODE: RF is forced into RX mode

  wakecntext = 1
  0000.1000 == 0x08
  Set delay time to wait for 20Mhz oscillator stability
*/
  ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, 0x08);

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
#else
  {
    static zb_uz_reg_wr_str_t s_wregs[] =
      {
        {ZB_LREG_RXFRMTYPE, 0x0B},
        {ZB_SREG_FIFOEN, 0x88},
        {ZB_SREG_BBREG6, 0x40},
        {ZB_LREG_SCLKDIV, 0x01},
        {ZB_LREG_RFCTL7, 0x80},
        {ZB_LREG_RFCTL2, 0x73},
        {ZB_LREG_RFCTL6, 0x90},
        {ZB_LREG_RFCTL8, 0x10},
        {ZB_SREG_RFCTL, 0x08}
      };
    /* zb_uz_write_regs_array(s_wregs, sizeof(s_wregs) / sizeof(s_wregs[0]));*/
    /* there's only one call, so, due to no inline implemented in C51, we're using macro */
    ZB_UZ_WRITE_REGS_ARRAY(s_wregs, (sizeof(s_wregs) / sizeof(s_wregs[0])));

  }
#endif

  zb_uz24_mask_short_reg(ZB_SREG_INTMSK, 0xBF);

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
#ifndef ZB_USE_RX_QUEUE
    ZB_READ_SHORT_REG(ZB_SREG_ISRSTS);
#else
    ZB_MAC_GET_BYTE_VALUE() = TRANS_CTX().int_status;
#endif
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

/*
  LREG TSTMODE
  Bit 4-3 RSSIWAIT: RSSI state machine parameter.
  Bit 2-0 TESTMODE: UZ2400 test mode using GPIO

  Set register value to 0x0f to enable PA.
  4.2.6. External Power Amplifier Configuration
*/
  ZB_READ_LONG_REG(ZB_LREG_TESTMODE);
  ZB_MAC_GET_BYTE_VALUE() |= 0x0F;
  ZB_WRITE_LONG_REG(ZB_LREG_TESTMODE, ZB_MAC_GET_BYTE_VALUE());

/*
  LREG RFCTL3
  Bit 7-6 TXPOWL: Large scale control for TX power in dB
  Bit 5-3 TXPOWF: Fine scale control for TX power in dB
  DS-2400: Appendix B. TX Power Configuration
*/

  ZB_WRITE_LONG_REG(ZB_LREG_RFCTL3, 0x40);

/*
  SREG TXBCNMSK
  Bit 7 TXBCNMSK: TX beacon FIFO interrupt mask

  DS-2400 4.5.1. Beacon Mode Setting
  Enable the beacon interrupt mask by setting SREG0x25[7] to 1
  1000.0000 == 0x80
*/

  zb_uz24_or_mask_short_reg(ZB_SREG_TXBCNMSK, 0x80);

  /* TODO: check, in ubec stack interrupts are enabled here */
  ZB_ENABLE_TRANSIVER_INT();
  TRACE_MSG( TRACE_MAC1, "<< init_zu2400", (FMT__0));
}

zb_ret_t zb_tansceiver_set_auto_ack()
{
  /* NOACKRSP: Disable automatic acknowledge ACK response */
  /* TODO: set NOACKRSP bit to 0*/
  return RET_OK;
}


zb_ret_t zb_transceiver_init()
{
  zb_ret_t ret = RET_OK;

  MAC_CTX().current_channel = 0xff;
  return ret;
}


#endif /* ZB_UZ2410 */
