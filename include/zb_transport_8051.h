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
PURPOSE: Globals for 8051 transport (MAC, trace and dump)
*/

#ifndef ZB_TRANSPORT_8051_H
#define ZB_TRANSPORT_8051_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */

#include "zb_8051_serial.h"
#ifndef ZB_SNIFFER
#include "zb_8051_spi.h"
#endif

typedef struct zb_io_ctx_s
{
#ifndef ZB_SNIFFER
  zb_buf_t *send_data_buf;    /* pointer to zigbee data buffer to send, can be sent via UART or SPI */
  zb_buf_t *recv_data_buf;    /* pointer to buffer to receive to */
  zb_ushort_t bytes_to_recv;  /* bytes number to receive; if 0, calculate this number automatically */
  zb_uint8_t recv_finished;    /* receive status - ZB_NO_IO, ZB_IO_ERROR, ZB_RECV_PENDING, ZB_RECV_FINISHED */
  zb_uint8_t send_finished;    /* send status - ZB_NO_IO, ZB_IO_ERROR, ZB_SEND_IN_PROGRESS, ZB_SEND_FINISHED */
#endif
#ifdef ZB_TRANSPORT_8051_UART
  zb_8051_serial_ctx_t serial_ctx;
#endif
#ifdef ZB_TRANSPORT_8051_DATA_SPI
  zb_8051_spi_ctx_t spi_ctx;
#endif
  zb_uint16_t int_counter;
}
zb_io_ctx_t;


/*! @} */
/*! \endcond */

#endif /* ZB_TRANSPORT_MAC_8051_H */
