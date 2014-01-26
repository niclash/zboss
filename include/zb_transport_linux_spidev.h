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
PURPOSE: UZ2400 transport for Linux/ARM via dpdev and zigbee_intr drivers
*/

#ifndef ZB_TRANSPORT_LINUX_SPDEV_H
#define ZB_TRANSPORT_LINUX_SPDEV_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC_TRANSPORT */
/*! @{ */

#ifdef ZB_TRANSPORT_LINUX_SPIDEV

#define ZB_MAC_TRANSPORT_SPIDEV_PATH "/dev/spidev2.1"
#define ZB_MAC_TRANSPORT_INTR_PATH "/dev/zbintr"


typedef struct zb_io_ctx_s
{
  int spidev_fd;
  int intr_fd;

  unsigned long long time_delta_rest_us;
  zb_uint16_t int_counter;

  zb_buf_t *send_data_buf;
  zb_buf_t *recv_data_buf;

  FILE *dump_file;
} zb_io_ctx_t;


#define ZB_TIMER_INIT() /* nothing to do here */

#define ZB_CHECK_TIMER_IS_ON() 1 /* always on in linux */

#define ZB_START_HW_TIMER() /* nothing to do here */


zb_ret_t zb_spidev_wait_for_intr(zb_uint8_t can_wait);
#define ZB_TRY_IO() zb_spidev_wait_for_intr(ZB_FALSE)

#define ZB_GO_IDLE() 
#define CHECK_INT_N_TIMER() zb_spidev_wait_for_intr(ZB_FALSE)

zb_ret_t zb_spidev_short_read(zb_uint8_t short_addr, zb_uint8_t *v);

zb_ret_t zb_spidev_short_write(zb_ushort_t short_addr, zb_uint8_t val);

zb_ret_t zb_spidev_long_write(zb_ushort_t long_addr, zb_uint8_t val);

zb_ret_t zb_spidev_long_read(zb_ushort_t long_addr, zb_uint8_t *val);

zb_ret_t zb_spidev_long_write_fifo(zb_ushort_t long_addr, zb_uint8_t *buf, zb_ushort_t len);

zb_ret_t zb_spidev_long_read_fifo(zb_ushort_t long_addr, zb_uint8_t *buf, zb_ushort_t len);

/*! @} */
/*! \endcond */


#endif /* ZB_TRANSPORT_LINUX_SPIDEV */

#endif /* ZB_TRANSPORT_LINUX_SPDEV_H */
