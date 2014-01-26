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
PURPOSE: ubec uz2400 specific code
*/
#ifndef ZB_UBEC24XX_H
#define ZB_UBEC24XX_H 1

#ifndef ZB_CC25XX
#ifndef UBEC_SPECIFIC_INCLUDED
#define UBEC_SPECIFIC_INCLUDED

#ifndef ZB_NS_BUILD


#define ZB_RXFLUSH() \
(ZB_READ_SHORT_REG(ZB_SREG_RXFLUSH), \
 ZB_MAC_GET_BYTE_VALUE()|=0x01,      \
 ZB_WRITE_SHORT_REG(ZB_SREG_RXFLUSH, ZB_MAC_GET_BYTE_VALUE()))


/**
   Min channel # of UZ2400
 */
#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
/**
   Max channel # of UZ2400
 */
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26

/**
   Send command/data/beacon to the transiver FIFO

   @param header_length - mhr length to write to UZ transiver
   @param buf - buffer to send
 */

#define ZB_TRANS_SEND_COMMAND(header_length, buf)          \
  zb_transceiver_send_fifo_packet((header_length), ZB_NORMAL_TX_FIFO, (buf), 1)
#define ZB_TRANS_FILL_FIFO(header_length, buf)          \
  zb_transceiver_send_fifo_packet((header_length), ZB_NORMAL_TX_FIFO, (buf), 0)


/**
   Receive packet from the transiver
 */
#define ZB_TRANS_RECV_PACKET(buf) zb_uz2400_fifo_read(0, buf, 0)
#define ZB_TRANS_READ_TXNFIFO(buf, a)            \
  zb_uz2400_fifo_read(1, buf, a)


#ifndef ZB_TRANSPORT_LINUX_SPIDEV
/**
   Write long UZ2400 register

   @param addr - register address
   @param reg_value - value to write
 */
#define ZB_WRITE_LONG_REG(addr, reg_value)                 \
  zb_mac_long_write_reg(addr, reg_value)

/**
   Write short UZ2400 register

   @param addr - register address
   @param reg_value - value to write
 */
#define ZB_WRITE_SHORT_REG(addr, reg_value)                \
  zb_mac_short_write_reg(addr, reg_value)

/**
   Read short UZ2400 register

   @param addr - register address
 */
#define ZB_READ_SHORT_REG(addr)                            \
    zb_mac_short_read_reg(addr)

/**
   Read long UZ2400 register

   @param addr - register address
 */
#define ZB_READ_LONG_REG(addr)                             \
   zb_mac_long_read_reg(addr)

#else  /* linux/spidev transport */

#define ZB_ENABLE_TRANSIVER_INT()
#define ZB_DISABLE_TRANSIVER_INT()

/**
   Write long UZ2400 register

   @param addr - register address
   @param reg_value - value to write
 */
#define ZB_WRITE_LONG_REG(addr, reg_value) zb_spidev_long_write(addr, reg_value)

/**
   Write short UZ2400 register

   @param addr - register address
   @param reg_value - value to write
 */
#define ZB_WRITE_SHORT_REG(addr, reg_value)  zb_spidev_short_write(addr, reg_value)

/**
   Read short UZ2400 register

   @param addr - register address
 */
#define ZB_READ_SHORT_REG(addr) zb_spidev_short_read(addr, &ZG->mac.mac_ctx.rw_reg.value.byte_value)

/**
   Read long UZ2400 register

   @param addr - register address
 */
#define ZB_READ_LONG_REG(addr) zb_spidev_long_read(addr, &ZG->mac.mac_ctx.rw_reg.value.byte_value)


#endif  /* 8051 / linux */

/*
  DS-2400  4.3.1. Transmit Packet in Normal FIFO
  SREG0x1B, TXNTRIG
  Bit 4 PENDACK: Indication of incoming ACK packet with pending-bit set to 1.
  Bit 3 INDIRECT: Activate indirect transmission - coordinator only.
  Bit 2 TXNACKREQ: Transmit a packet with ACK packet expected. If ACK is not received, UZ2400 retransmits.
  Bit 1 TXNSECEN: Transmit a packet which needs to be encrypted
  Bit 0 TXNTRIG: Trigger TXMAC to transmit the packet inside TX normal FIFO.

Set TXNTRIG = 1
0000.0001 = 0x01
*/
#ifdef ZB_TRANSPORT_8051_DATA_SPI
#ifdef KEIL
#include <intrins.h>
#endif

/**
SPI FLAG
 */
#ifdef C8051F120
#define ZB_SET_NSS_UP() (NSSMD0 = 0)
#define ZB_SET_NSS_DOWN() (NSSMD0 = 1)
#endif
/**
  8051 delay if device sleep switched off
 */
#ifdef SDCC
#define ZB_8051_DELAY() \
 __asm \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  nop; \
  __endasm

#elif defined ZB_IAR

#define ZB_8051_DELAY() asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n")

#elif defined KEIL

#define ZB_8051_DELAY() \
  ( _nop_(),            \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_(), \
   _nop_()) \

#else

#error Port me!

#endif  /* compilers switch */

#else
#define ZB_SET_NSS_UP()
#define ZB_SET_NSS_DOWN()
#define ZB_8051_DELAY()
#endif



/**
   Trigger TX in normal UZ2400 FIFO

   @param need_ack - if 1, wait for ACK and retransmit automatically
 */
#define ZB_START_NORMAL_FIFO_TX(retries, need_ack)  \
  (MAC_CTX().tx_cnt = 0,\
  (ZB_WRITE_SHORT_REG(ZB_SREG_TXNTRIG, ((retries << 5) & 0xE0) | ((need_ack)? 0x65 : 0x01))))



 /* DS-2400 4.2.1. Memory Space */
#ifndef ZB_UZ2410_256
#define ZB_SHORT_REGS_BASE 0xF800
#define ZB_LONG_REGS_BASE 0xFC00
#else 
#define ZB_SHORT_REGS_BASE 0x7800
#define ZB_LONG_REGS_BASE 0x7C00
#endif
#define ZB_NORMAL_RXFIFO_ADDR (ZB_NORMAL_FIFO_ADDR+0x300)








/* DS-2400 4.2.1. Memory Space */
#define ZB_NORMAL_TX_FIFO          0x00
#define ZB_BEACON_TX_FIFO          0x80
#define ZB_GTS1_TX_FIFO            0x100
#define ZB_GTS2_TX_FIFO            0x180
#define ZB_RX_FIFO                 0x300
/* Mac address regs 			*/

#define         SADRL           0x03
#define         SADRH           0x04
#define         EADR0           0x05
#define         EADR1           0x06
#define         EADR2           0x07
#define         EADR3           0x08
#define         EADR4           0x09
#define         EADR5           0x0a
#define         EADR6           0x0b
#define         EADR7           0x0c
/*					`			*/
#define ZB_SREG_RXMCR              0x00
#define ZB_SREG_PANIDL             0x01
#define ZB_SREG_PANIDH             0x02
#define ZB_SREG_RXFLUSH            0x0D
#define ZB_SREG_ORDER              0x10
#define ZB_SREG_TXMCR              0x11
#define ZB_SREG_ACKTMOUT           0x12
#define ZB_SREG_FIFOEN             0x18
#define ZB_SREG_TXNTRIG            0x1B
#define ZB_SREG_TXPEND             0x21
#define ZB_SREG_WAKECTL            0x22
#define ZB_SREG_TXSR               0x24
#define ZB_SREG_RXSR               0x30
#define ZB_SREG_TXBCNMSK           0x25
#define ZB_SREG_SOFTRST            0x2A
#define ZB_SREG_SECCR0             0x2C
#define ZB_SREG_SECCR2             0x37
#define ZB_SREG_ISRSTS             0x31
#define ZB_SREG_INTMSK             0x32
#define ZB_SREG_SLPACK             0x35
#define ZB_SREG_RFCTL              0x36
#define ZB_SREG_BBREG2             0x3A
#define ZB_SREG_BBREG3             0x3B
#define ZB_SREG_BBREG5             0x3D
#define ZB_SREG_BBREG6             0x3E
#define ZB_SREG_BBREG7             0x3F
#define ZB_SREG_GATECLK            0x26
#define ZB_SREG_PACON1             0x17
#define ZB_SREG_TXPEMISP           0x2E



#define ZB_LREG_GPIODIR            0x23D
#define ZB_LREG_RFCTRL0            0x200
#define ZB_LREG_RFCTRL1            0x201
#define ZB_LREG_RFCTRL2            0x202
#define ZB_LREG_RFCTRL4            0x204
#define ZB_LREG_RFCTRL6            0x206
#define ZB_LREG_RFCTRL7            0x207
#define ZB_LREG_RFCTRL8            0x208
#define ZB_LREG_RFCTL2             0x202
#define ZB_LREG_RFCTL3             0x203
#define ZB_LREG_RFCTL6             0x206
#define ZB_LREG_RFCTL7             0x207
#define ZB_LREG_RFCTL8             0x208
#define ZB_LREG_RFCTRL77           0x277
#define ZB_LREG_RFCTRL50           0x250
#define ZB_LREG_RFCTRL51           0x251
#define ZB_LREG_RFCTRL52           0x252
#define ZB_LREG_RFCTRL59           0x259
#define ZB_LREG_RFCTRL73           0x273
#define ZB_LREG_RFCTRL74           0x274
#define ZB_LREG_RFCTRL75           0x275
#define ZB_LREG_RFCTRL76           0x276
#define ZB_LREG_RSSI               0x210

#define ZB_LREG_SADRCTRL           0x212
#define ZB_LREG_SRCADR_0           0x213
#define ZB_LREG_SRCADR_7           0x21A
#define ZB_LREG_HLEN               0x21E

#define ZB_LREG_SCLKDIV            0x220
#define ZB_LREG_TESTMODE           0x22F
#define ZB_LREG_ASSOEADR0          0x230
#define ZB_LREG_ASSOEADR1          0x231
#define ZB_LREG_ASSOEADR2          0x232
#define ZB_LREG_ASSOEADR3          0x233
#define ZB_LREG_ASSOEADR4          0x234
#define ZB_LREG_ASSOEADR5          0x235
#define ZB_LREG_ASSOEADR6          0x236
#define ZB_LREG_ASSOEADR7          0x237
#define ZB_LREG_ASSOSADR0          0x238
#define ZB_LREG_ASSOSADR1          0x239
#define ZB_LREG_RXFRMTYPE          0x23C
#define ZB_LREG_SECCTRL            0x24D

#define ZB_LREG_UPNONCE_0          0x240
#define ZB_LREG_UPNONCE_12         0x24C

#define ZB_LREG_KEY_0              0x280


#define TXMCR_SLOTTED_MASK         0x20




#define ZB_SREG_ORDER_FIRST_START_VAL  0xFF

/**
   Header to be written into normal UZ2400 fifo before packet
 */

typedef struct zb_ubec_fifo_header_s
{
  zb_uint8_t header_length;
  zb_uint8_t frame_length;
} ZB_PACKED_STRUCT
zb_ubec_fifo_header_t;

/*
  DS-2400 SREG0x31 ISRSTS (interrupt status)
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

#define ZB_UBEC_GET_RX_DATA_STATUS() (TRANS_CTX().int_status & 0x08)  /* get bit 3 */
#define ZB_UBEC_GET_TX_DATA_STATUS() (TRANS_CTX().int_status & 0x01)  /* get bit 0 */
#define ZB_UBEC_GET_WAKE_STATUS()    (TRANS_CTX().int_status & 0x40)  /* get bit 6 */
#define ZB_UBEC_GET_SECUR_STATUS()   (TRANS_CTX().int_status & 0x10)  /* get bit 4 */

#define ZB_UBEC_CLEAR_SECUR_STATUS() (TRANS_CTX().int_status &= (~0x10))
#define ZB_UBEC_CLEAR_RX_DATA_STATUS() (TRANS_CTX().int_status &= (~0x08))  /* clear bit 3 */

/* DS-2400 SREG0x24 TXSR */
typedef struct zb_ubec_tx_status_s
{
  zb_bitfield_t txns:1;  /* Normal FIFO release status, 1: Fail (retry count exceed), 0: Ok */
  zb_bitfield_t txg1s:1; /* GTS1 FIFO release status, 1: Fail (retry count exceed), 0: OK */
  zb_bitfield_t txg2s:1; /* GTS2 FIFO release status, 1: Fail (retry count exceed), 0: OK */
  zb_bitfield_t txg1fnt:1; /* GTS1 FIFO transmission fails due to not enough time before end of GTS */
  zb_bitfield_t txg2fnt:1; /* GTS2 FIFO transmission fails due to not enough time before end of GTS */
  zb_bitfield_t ccafail:1; /* Channel busy causes CSMA-CA fails */
  zb_bitfield_t TXRETRY:2; /* Retry times of the most recent TXNFIFO transmission */
} ZB_PACKED_STRUCT
zb_ubec_tx_status_t;


/**
   Transiver context specific for UZ2400
 */

typedef struct zb_transceiver_ctx_s
{
  zb_uint8_t int_status;
  zb_uint8_t tx_status;
  zb_uint8_t interrupt_flag;
}
zb_transceiver_ctx_t;

#define ZB_MAC_GET_BYTE_VALUE() ZG->mac.mac_ctx.rw_reg.value.byte_value
#define ZB_CLEAR_NORMAL_FIFO_BUSY() (TRANS_CTX().normal_fifo_busy = 0)
#define ZB_UBEC_GET_NORMAL_FIFO_TX_STATUS() (TRANS_CTX().int_status & 0x01) /* check bit 0 */
#define ZB_UBEC_CLEAR_NORMAL_FIFO_TX_STATUS() (TRANS_CTX().int_status &= 0xFE) /* clear bit 0 */

#define ZB_SET_TRANS_INT() (TRANS_CTX().interrupt_flag = 1)
#define ZB_CLEAR_TRANS_INT() (TRANS_CTX().interrupt_flag = 0)
#define ZB_GET_TRANS_INT() (TRANS_CTX().interrupt_flag)



/*
  clear ISRSTS bit 0  - TX Normal FIFO transmission interrupt bit
  clear TXSR bit 5 - CCAFAIL: Channel busy causes CSMA-CA fails
  clear TXSR bit 0 - Normal FIFO release status (1 - fail, retry count exceed)
*/
#define ZB_CLEAR_TX_STATUS() (TRANS_CTX().int_status &= 0xFE, TRANS_CTX().tx_status &= 0xDE)
/* check TXSR bit 0 - Normal FIFO release status (1 - fail, retry count exceed) */
#define ZB_IS_TX_CHANNEL_BUSY() (TRANS_CTX().tx_status & 0x20)
/* check TXSR bit 5 - CCAFAIL: Channel busy causes CSMA-CA fails */
#define ZB_IS_TX_RETRY_COUNT_EXCEEDED() (TRANS_CTX().tx_status & 0xDF)


/* interface macroses - vendor independent */
#define ZB_TRANS_IS_COMMAND_SEND() ZB_UBEC_GET_NORMAL_FIFO_TX_STATUS() /* not 0 means command is sent */

/* set up/down pending bit */
#define ZB_SET_PENDING_BIT() \
  ZB_READ_SHORT_REG(ZB_SREG_ACKTMOUT),                                  \
    ZB_WRITE_SHORT_REG(ZB_SREG_ACKTMOUT, ZB_MAC_GET_BYTE_VALUE()|0x80)

#define ZB_CLEAR_PENDING_BIT() \
ZB_READ_SHORT_REG(ZB_SREG_ACKTMOUT),\
ZB_WRITE_SHORT_REG(ZB_SREG_ACKTMOUT, ZB_MAC_GET_BYTE_VALUE()&0x7F)


#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() ZB_IS_TX_CHANNEL_BUSY() /* not 0 means channel busy error */
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() ZB_IS_TX_RETRY_COUNT_EXCEEDED() /* not 0 means cca fail error */
#ifdef ZB_CHANNEL_ERROR_TEST
#define ZB_TRANS_CHECK_CHANNEL_ERROR() (((ZG->nwk.nib.nwk_tx_total % 3) == 0 && ZB_MAC_GET_CHANNEL_ERROR_TEST()) ? (1) : 0)
#else
#define ZB_TRANS_CHECK_CHANNEL_ERROR() (ZB_IS_TX_CHANNEL_BUSY() || ZB_IS_TX_RETRY_COUNT_EXCEEDED())
#endif
#define ZB_TRANS_CUT_SPECIFIC_HEADER(zb_buffer)                         \
{                                                                       \
  /* 1st byte contains packet length - ubec specific */                 \
  (void)zb_buf_cut_left((zb_buffer), 1);                                \
}

/* standby mode for uz2400 */
void zb_uz2400_standby();
void zb_uz2400_register_wakeup();


/**
   Fill FIFO
 */

void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT;

/* */

/**
   Read from FIFO
 */
void zb_uz2400_fifo_read(zb_uint8_t tx_fifo, zb_buf_t *buf, zb_uint8_t len) ZB_SDCC_REENTRANT;

/**
   Initialize transiver
 */
void init_zu2400();

/**
   Read and check interrupt regs 
 */
void zb_ubec_check_int_status();

/**
   Portable macro for int status check routine
 */
#define ZB_CHECK_INT_STATUS() zb_ubec_check_int_status()


/**
   Check that transiver is in the beacon mode
 */
zb_bool_t zb_check_beacon_mode_on();
#define ZB_CHECK_BEACON_MODE_ON() ZB_TRUE


/**
   General transiver initialize
 */
zb_ret_t zb_transceiver_init();
#define ZB_TRANSCEIVER_INIT() zb_transceiver_init()
zb_ret_t zb_transceiver_send_fifo_packet(zb_uint8_t header_length, zb_int16_t fifo_addr,
                                         zb_buf_t *buf, zb_uint8_t need_tx) ZB_SDCC_REENTRANT;

/**
   Set channel os UZ2400 transiver

   @param channel_number - channel number (absolute - means, channel 0 on UZ2400
                           is 11)
 */
void zb_transceiver_set_channel(zb_uint8_t channel_number);

/**
   Portable macro for zb_transceiver_set_channel()
 */
#define ZB_TRANSCEIVER_SET_CHANNEL(channel_number) zb_transceiver_set_channel(channel_number)

/**
   Get RSSI from UZ2400

   @param rssi_value - (out) rssi value as UZ2400 returns it
 */
void zb_transceiver_get_rssi(zb_uint8_t *rssi_value);


/**
   Write 16-bit into 2 short registers
 */
void zb_uz_short_reg_write_2b(zb_uint8_t reg, zb_uint16_t v);

/**
   Assign short pan ID in UZ2400

   @param pan_id - new pan id
 */

#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id) zb_uz_short_reg_write_2b(ZB_SREG_PANIDL, (pan_id))


/**
   Assign ext (long) coordinator address in UZ2400

   @param coord_addr_long - addres to remember
 */
void zb_transceiver_set_coord_ext_addr(zb_ieee_addr_t coord_addr_long);


/**
   Portable macro for zb_transceiver_set_coord_ext_addr()
*/
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr) zb_transceiver_set_coord_ext_addr((addr))

/**
   Assign short coordinator address in UZ2400

   @param coord_addr_short - addres to remember
 */
void zb_transceiver_set_coord_short_addr(zb_uint16_t coord_addr_short);

/**
   Portable macro for zb_transceiver_set_coord_short_addr()
 */
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr) zb_transceiver_set_coord_short_addr((addr))



void zb_transceiver_update_long_mac();
#define ZB_UPDATE_LONGMAC() \
  zb_transceiver_update_long_mac()


/**
   Update UZ2400 pan id: copu PIB value to the transiver
 */

#define ZB_UPDATE_PAN_ID() \
 (ZB_TRANSCEIVER_SET_PAN_ID(MAC_PIB().mac_pan_id))



/**
   Update UZ2400 short address: copu PIB value to the transiver
 */
#define ZB_UPDATE_SHORT_ADDR() zb_uz_short_reg_write_2b(SADRL, MAC_PIB().mac_short_address)
#define ZB_CLEAR_SHORT_ADDR() zb_uz_short_reg_write_2b(SADRL, -1)

#endif  /* ZB_NS_BUILD */

#if defined ZB_ED_ROLE && !defined ZB_DBG_NO_IDLE
#define ZB_TRANS_GO_IDLE() \
if ((ZG->nwk.handle.joined==0x01)&&(!(ZB_MAC_GET_TRANS_SPLEEPING())))                      \
{                                                                                              \
    TRACE_MSG(TRACE_COMMON3, "transceiver fall asleep", (FMT__0));                             \
    ZB_MAC_SET_TRANS_SPLEEPING();                                                              \
    zb_uz2400_standby();                                                                       \
}
#else
#define ZB_TRANS_GO_IDLE()
#endif

#ifndef ZB_TRANSPORT_LINUX_SPIDEV
void zb_mac_short_read_reg(zb_uint8_t short_addr) ZB_SDCC_REENTRANT;

/**
   Read long UZ2400 reg
 */
void zb_mac_long_read_reg(zb_uint16_t long_addr) ZB_SDCC_REENTRANT;

/**
   Write short UZ2400 reg
 */
void zb_mac_short_write_reg(zb_uint8_t short_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT;

/**
   Write long UZ2400 reg
 */
void zb_mac_long_write_reg(zb_uint16_t long_addr, zb_uint8_t byte_value) ZB_SDCC_REENTRANT;
#endif


/*fifo header */
#define ZB_FIFO_HEADER_SIZE 2


typedef struct zb_uz_reg_wr_str_s
{
  zb_uint16_t reg;
  zb_uint8_t val;
} zb_uz_reg_wr_str_t;


void zb_uz24_mask_short_reg(zb_uint8_t reg, zb_uint8_t mask);

void zb_uz24_or_mask_short_reg(zb_uint8_t reg, zb_uint8_t mask);

void zb_uz_write_regs_array(zb_uz_reg_wr_str_t *arr, zb_uint8_t n);

void zb_uz_secur_handle_rx();

#define ZB_UZ_WRITE_REGS_ARRAY(arr, num) \
  {                                                                     \
      zb_uz_reg_wr_str_t *a = &arr[0];                                  \
      zb_uint8_t n = num;                                               \
      while (n--)                                                       \
      {                                                                 \
        if (a->reg > 0xff)                                              \
        {                                                               \
          ZB_WRITE_LONG_REG(a->reg, a->val);                            \
        }else                                                           \
        {                                                               \
          ZB_WRITE_SHORT_REG((zb_uint8_t)a->reg, a->val);               \
        }                                                               \
        a++;                                                            \
      }                                                                 \
  }
#endif

/* Receive queue routines */
#ifdef ZB_USE_RX_QUEUE

#ifdef ZB_RESERVED_REGS_DUMP
typedef struct zb_reserved_regs_s
{
  zb_uint8_t isrsts;
  zb_uint8_t txntrig;
  zb_uint8_t txsr;
} ZB_PACKED_STRUCT zb_reserved_regs_t;
#endif
#define ZB_RX_QUEUE_CAP 4
/* main ring buffer, that contains whole packets itself */
ZB_RING_BUFFER_DECLARE(zb_rx_queue, zb_buf_t *, ZB_RX_QUEUE_CAP);
/* small ringbuf, containing registers, that read using direct memory access, instead of*/
/* appropriate function  */
#ifdef ZB_RESERVED_REGS_DUMP
ZB_RING_BUFFER_DECLARE(zb_regs_queue, zb_reserved_regs_t *, ZB_RX_QUEUE_CAP+1);

#define DUMP_RESERVED_REGS() \
{\
	zb_uint8_t *data_ptr = NULL;\
	ZB_BUF_INITIAL_ALLOC( ZG->mac.mac_ctx.operation_buf, 1, data_ptr );\
	ZB_ASSERT(data_ptr);\
	data_ptr[0] = (ZB_SREG_ISRSTS << 1);\
	ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);\
	data_ptr[0] = MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.read_i]->isrsts;\
    ZB_DUMP_INCOMING_DATA(ZG->mac.mac_ctx.operation_buf);\
    if (MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.read_i]->txntrig!=-1)\
	{\
		data_ptr[0] = (ZB_SREG_TXNTRIG << 1);\
		ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);\
		data_ptr[0] = MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.read_i]->txntrig;\
		ZB_DUMP_INCOMING_DATA(ZG->mac.mac_ctx.operation_buf);\
	}\
    if (MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.read_i]->txsr!=-1)\
	{\
		data_ptr[0] = (ZB_SREG_TXSR << 1);\
		ZB_DUMP_OUTGOING_DATA(ZG->mac.mac_ctx.operation_buf);\
		data_ptr[0] = MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.read_i]->txsr;\
		ZB_DUMP_INCOMING_DATA(ZG->mac.mac_ctx.operation_buf);				\
	}\
	ZB_RING_BUFFER_FLUSH_GET(&MAC_CTX().regs_queue);\
}
#else
#define DUMP_RESERVED_REGS()
#endif




#define ZB_GET_ISRSTS() \
TRANS_CTX().int_status = *((zb_uint8_t ZB_XDATA *)(ZB_SREG_ISRSTS|ZB_SHORT_REGS_BASE))

#define ZB_GET_TXSR() \
TRANS_CTX().tx_status = *((zb_uint8_t ZB_XDATA *)(ZB_SREG_TXSR|ZB_SHORT_REGS_BASE))

#define ZB_CHECK_PENDING() \
((*((zb_uint8_t ZB_XDATA *)(ZB_SREG_TXNTRIG|ZB_SHORT_REGS_BASE)))&0x10)

#define ZB_PUT_RX_QUEUE() \
 if (ZB_RING_BUFFER_IS_FULL(&MAC_CTX().mac_rx_queue)) \
  { 							\
   MAC_CTX().recv_buf_full = 1; \
  } else                        \
  {    							\
  ZB_MEMCPY(ZB_BUF_BEGIN(MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i]),\
                (zb_uint8_t ZB_XDATA *)ZB_NORMAL_RXFIFO_ADDR,                 \
				   *((zb_uint8_t ZB_XDATA *)ZB_NORMAL_RXFIFO_ADDR)+ZB_MAC_PACKET_LENGTH_SIZE + ZB_MAC_EXTRA_DATA_SIZE) ; \
  MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i]->u.hdr.len =   *((zb_uint8_t ZB_XDATA *)ZB_NORMAL_RXFIFO_ADDR)+ ZB_MAC_PACKET_LENGTH_SIZE + ZB_MAC_EXTRA_DATA_SIZE ; \
  MAC_CTX().mac_rx_queue.written++;                                                     \
  MAC_CTX().mac_rx_queue.write_i = (MAC_CTX().mac_rx_queue.write_i + 1) % ZB_RX_QUEUE_CAP;           \
  }


#define ZB_GET_RX_QUEUE()       \
  {												\
	  zb_buf_t *mac_rx_queue_tmp;		 \
      ZB_DISABLE_ALL_INTER(); \
	  mac_rx_queue_tmp = MAC_CTX().recv_buf; \
	  MAC_CTX().recv_buf = MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.read_i];\
	  MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.read_i] = mac_rx_queue_tmp; \
  	  ZB_RING_BUFFER_FLUSH_GET(&MAC_CTX().mac_rx_queue);\
      if (MAC_CTX().recv_buf_full) \
	  {							   \
	  	MAC_CTX().recv_buf_full = 0;\
		ZB_PUT_RX_QUEUE();\
	  };						   \
	  ZB_ENABLE_ALL_INTER();	 \
  }
#endif


#endif /* not cc2530 */
#endif /* ZB_UBEC24XX_H */