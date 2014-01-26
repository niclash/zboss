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
PURPOSE: Globals for linux pipes transport
*/

#ifndef ZB_TRANSPORT_LINUX_PIPES_H
#define ZB_TRANSPORT_LINUX_PIPES_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC_TRANSPORT */
/*! @{ */

#ifdef ZB_TRANSPORT_LINUX_PIPES

#define ZB_MAC_TRANSPORT_DEFAULT_RPIPE_PATH "/tmp/zb_mac_rpipe.tmp"
#define ZB_MAC_TRANSPORT_DEFAULT_WPIPE_PATH "/tmp/zb_mac_wpipe.tmp"

/**
   \par Time implementation based on transport based on pipes in Linux.

   The idea is to block inside select with desired timeout.
   Use gettimeofday() for time intervals check.

   All that functions are internal timer implementation.
   See zb_scheduler.h for the public timer API.
 */


typedef struct zb_io_ctx_s
{
  int rpipe;
  int wpipe;

  zb_char_t *rpipe_path;
  zb_char_t *wpipe_path;

  int timeout;
  int time_delta_rest_ms;
  int time_delta_rest_us;

  zb_buf_t *send_data_buf;
  zb_buf_t *recv_data_buf;

  unsigned out_buf_written;
  unsigned in_buf_read;

  zb_uint8_t recv_finished;
  zb_uint8_t send_finished;

  FILE *dump_file;
} zb_io_ctx_t;


#define ZB_TIMER_INIT() /* nothing to do here */

#define ZB_CHECK_TIMER_IS_ON() 1 /* always on in linux */

#define ZB_START_HW_TIMER() /* nothing to do here */

void zb_mac_wait_for_ext_event();
#define ZB_TRY_IO() (zb_mac_wait_for_ext_event(), RET_OK)

#define ZB_GO_IDLE()
#define CHECK_INT_N_TIMER()
#define ZB_CORE_IDLE()

/*! @} */
/*! \endcond */

#endif  /* ZB_TRANSPORT_LINUX_PIPES */

#endif /* ZB_TRANSPORT_LINUX_PIPES_H */
