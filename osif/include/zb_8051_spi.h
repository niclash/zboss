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
PURPOSE: Declare spi transport for 8051

*/

#ifndef ZB_8051_SPI_H
#define ZB_8051_SPI_H 1

/* for SDCC: handler prototype MUST be defined in the same file with main() function */
#define DECLARE_SPI_INTER_HANDLER() \
  void spi_inter_handler(void) INTERRUPT_DEFINITION(SPI_INTER_NUMBER, REGISTER_BANK_2);

#define ZB_SPI_TX_IN_PROGRESS(spi_ctx) ((spi_ctx)->tx_in_progress == 1)

#define ZB_START_SPI_WRITE(io_ctx)                      \
  if (!ZB_SPI_TX_IN_PROGRESS(&(io_ctx)->spi_ctx))       \
  {                                                     \
    ZB_SET_SPI_TRANSMIT_FLAG();                         \
  }

#define ZB_START_SPI_RECEIVE(io_ctx)                    \
  if (!ZB_SPI_TX_IN_PROGRESS(&(io_ctx)->spi_ctx))       \
  {                                                     \
    (io_ctx)->spi_ctx.io_request = ZB_SPI_RX_REQUEST;   \
    ZB_SET_SPI_TRANSMIT_FLAG();                         \
  }

/* spi definitions */
#define ZB_8051_CKR_VALUE 1
#define ZB_ESPI_BIT    0x01 /* spi interrupt bit 0 */
#define ZB_ENABLE_SPI_INTER() (EIE1 |= ZB_ESPI_BIT)
#define ZB_DISABLE_SPI_INTER() (EIE1 &= ~ZB_ESPI_BIT)

#define ZB_SET_SPI_TRANSMIT_FLAG()   (SPIF = 1)
#define ZB_CLEAR_SPI_TRANSMIT_FLAG() (SPIF = 0)

#define ZB_NO_SPI_IO_REQUEST 0
#define ZB_SPI_RX_REQUEST    1
#define ZB_SPI_TX_REQUEST    2

typedef struct zb_8051_spi_ctx_s
{
  zb_uint8_t sent_num;         /* number of sent bytes */
  zb_uint8_t tx_in_progress:1; /* flag to show if tx is going or not */
  zb_uint8_t io_request:2;     /* io request type, can be ZB_NO_SPI_IO_REQUEST, ZB_SPI_RX_REQUEST, ZB_SPI_TX_REQUEST
                                  is used to determine if dummy send is needed or not. Dummy send is used during 
                                  spi receive - need to send 1 byte to receive synchronously 1 byte */
  zb_uint8_t reserved:5;
}
zb_8051_spi_ctx_t;

void zb_spi_init();

#endif /* ZB_8051_SPI_H */
