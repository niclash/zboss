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
PURPOSE: Common include file for ZigBee
*/

#include "zb_config.h"

#ifdef ZB_CC25XX
#ifndef TI_SPECIFIC_INCLUDED
#define TI_SPECIFIC_INCLUDED

#define CLOCK_XTAL   0  
#define CLOCK_RC     1
#define CLOCK_SOURCE CLOCK_XTAL

#define CLOCK_XTAL_SRC   0x00  /* XTAL oscillator */
#define CLOCK_RC_SRC   0x01  /* RC oscillator */
#define CLOCK_SRC CLOCK_XTAL_SRC

#define ZB_RX_FIFO 0x6000
#define ZB_NORMAL_TX_FIFO 0x6080
#define ZB_NORMAL_FIFO_ADDR 0x6080
#define ZB_NORMAL_RXFIFO_ADDR 0x6000

#define ZB_CC2530_RF_INTERRUPT 0x83

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26


#define ZB_SET_TRANS_INT() (TRANS_CTX().interrupt_flag = 1)
#define ZB_CLEAR_TRANS_INT() (TRANS_CTX().interrupt_flag = 0)
#define ZB_GET_TRANS_INT() (TRANS_CTX().interrupt_flag)

#define ZB_CC2530_FIFO_TAIL 2 /* crc, rssi */

#define ZB_CHECK_INT_STATUS() zb_ubec_check_int_status()

#define ZB_UBEC_CLEAR_RX_DATA_STATUS() (TRANS_CTX().int_status_0 &= (~0x40)) 

#define ZB_TRANS_CUT_SPECIFIC_HEADER(zb_buffer)                         \
{                                                                       \
  /* 1st byte contains packet length*/                                  \
  (void)zb_buf_cut_left((zb_buffer), 1);                                \
}

void clock_set_src();
void init_clock();

void zb_uz_short_reg_write_2b(zb_uint8_t reg, zb_uint16_t v);

/* need further modification for TI */
#define ZB_IS_TX_RETRY_COUNT_EXCEEDED() (TRANS_CTX().tx_status & 0xDF)
#define ZB_IS_TX_CHANNEL_BUSY() (TRANS_CTX().tx_status & 0x20)
/* possibly, need further research for TI */
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr) ZVUNUSED(0)
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr) ZVUNUSED(0)

#define ISFLUSHRX()             RFST = 0xEC
#define SFLUSHRX()              RFST = 0xDD
#define ISFLUSHTX()             RFST = 0xEE
#define ISRXON()                RFST = 0xE3
#define ISTXON()                RFST = 0xE9
#define ISTXONCCA()             RFST = 0xEA
#define ISRFOFF()               RFST = 0xEF

#define ZB_CLEAR_PENDING_BIT() SRCMATCH &=(~0x06)
#define ZB_SET_PENDING_BIT() SRCMATCH |=0x06




#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() ZB_IS_TX_CHANNEL_BUSY() /* not 0 means channel busy error */
#define ZB_TRANS_CHECK_CHANNEL_ERROR() (ZB_IS_TX_CHANNEL_BUSY() || ZB_IS_TX_RETRY_COUNT_EXCEEDED())
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() ZB_IS_TX_RETRY_COUNT_EXCEEDED() /* not 0 means cca fail error */

#define ZB_UPDATE_SHORT_ADDR() SHORT_ADDR0 =  MAC_PIB().mac_short_address & 0xFF; SHORT_ADDR1 = (MAC_PIB().mac_short_address >>8)&0xFF
#define ZB_CLEAR_SHORT_ADDR() (SHORT_ADDR0 = 0xFF, SHORT_ADDR1 = 0xFF)
#define ZB_UBEC_GET_RX_DATA_STATUS() (TRANS_CTX().int_status_0 & 0x40)
#define ZB_UBEC_GET_TX_DATA_STATUS() (TRANS_CTX().int_status_1 & 0x02)

#define ZB_CLEAR_TX_STATUS() (TRANS_CTX().int_status_1 &= ~0x02, TRANS_CTX().tx_status = 0)

#define ZB_UBEC_CLEAR_NORMAL_FIFO_TX_STATUS() (TRANS_CTX().int_status_1 &= (~0x02)) /* clear bit 0 */

#define ZB_UPDATE_PAN_ID() \
 (ZB_TRANSCEIVER_SET_PAN_ID(MAC_PIB().mac_pan_id))


#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id) zb_set_pan_id(pan_id)
void zb_set_pan_id (zb_uint16_t pan_id);


/**
   Transiver context specific for cc25xx
 */
/* should be checked */
#define ZB_START_NORMAL_FIFO_TX(retries, need_ack)  \
  (MAC_CTX().tx_cnt = 0,\
  RFST = 0xE9)

#ifndef ZB_SNIFFER

#define ZB_TRANS_SEND_COMMAND(header_length, buf)          \
  zb_transceiver_send_fifo_packet((header_length), ZB_NORMAL_TX_FIFO, (buf), 1)

zb_ret_t zb_transceiver_send_fifo_packet(zb_uint8_t header_length, zb_uint16_t fifo_addr,
                                         zb_buf_t *buf, zb_uint8_t need_tx) ZB_SDCC_REENTRANT;

#endif /* ZB_SNIFFER */

/* legacy compatibility stub */
#define ZB_CHECK_BEACON_MODE_ON() ZB_TRUE


#pragma vector=ZB_CC2530_RF_INTERRUPT
ZB_INTERRUPT zb_cc25xx_handler(void);



void zb_transceiver_update_long_mac();

#define ZB_UPDATE_LONGMAC() \
  zb_transceiver_update_long_mac()
void zb_transceiver_get_rssi(zb_uint8_t *rssi_value);

void zb_transceiver_set_channel(zb_uint8_t channel_number);

#define ZB_TRANSCEIVER_SET_CHANNEL(channel_number) zb_transceiver_set_channel(channel_number)

#ifndef ZB_SNIFFER
void zb_uz2400_fifo_write(zb_uint16_t long_addr, zb_buf_t *buf) ZB_SDCC_REENTRANT;
#endif

typedef struct zb_ubec_fifo_header_s
{
  zb_uint8_t frame_length;
} ZB_PACKED_STRUCT
zb_ubec_fifo_header_t;


typedef struct zb_transceiver_ctx_s
{
  zb_uint8_t int_status_0;
  zb_uint8_t int_status_1;
  zb_uint8_t tx_status;
  zb_uint8_t interrupt_flag;
}
zb_transceiver_ctx_t;


/* initialize MCU and RF */
void init_zu2400();

/* check interrupts routine, usually called from scheduler loop iteration */
void zb_ubec_check_int_status();




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
#ifndef ZB_SNIFFER
#define ZB_RX_QUEUE_CAP 2
/* main ring buffer, that contains whole packets itself */
ZB_RING_BUFFER_DECLARE(zb_rx_queue, zb_buf_t *, ZB_RX_QUEUE_CAP);
#endif
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


#define ZB_CLEAR_ISRSTS() \
  (RFIRQF0 =0, S1CON = 0, RFIRQF1 = 0);

#define ZB_GET_ISRSTS() \
TRANS_CTX().int_status_0 = RFIRQF0;\
TRANS_CTX().int_status_1 = RFIRQF1

/* TODO: should be modified for TI */
#define ZB_GET_TXSR() \
TRANS_CTX().tx_status = 0
/* TODO: should be modified for TI */
#define ZB_CHECK_PENDING() 1

#if 0
    ZB_MEMCPY(ZB_BUF_BEGIN(MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i]),\
       (zb_uint8_t ZB_XDATA *)ZB_NORMAL_RXFIFO_ADDR,                 \
				   *((zb_uint8_t ZB_XDATA *)ZB_NORMAL_RXFIFO_ADDR)) ; 

#endif


#define ZB_PUT_RX_QUEUE() \
 if (ZB_RING_BUFFER_IS_FULL(&MAC_CTX().mac_rx_queue)) \
  { 							\
   MAC_CTX().recv_buf_full = 1; \
  } else                        \
  {    							\
    zb_uint8_t i, len;\
    len = RFD;\
    MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i]->u.hdr.len = len;\
    *(zb_uint8_t *)(ZB_BUF_BEGIN(MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i])) = len;\
    for (i = 1; i<len;i++)\
    {\
     *(zb_uint8_t *)(ZB_BUF_BEGIN(MAC_CTX().mac_rx_queue.ring_buf[MAC_CTX().mac_rx_queue.write_i])+i) = RFD;\
    }\
    len = RFD;len=RFD;/* skip CRC */\
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
	  {			\
  	    MAC_CTX().recv_buf_full = 0;\
            ZB_PUT_RX_QUEUE();\
	  };						   \
	  ZB_ENABLE_ALL_INTER();	 \
  }
#endif




#endif /* include */
#endif /* cc25xx */
