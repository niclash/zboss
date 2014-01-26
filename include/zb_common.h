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

#ifndef ZB_COMMON_H
#define ZB_COMMON_H 1

#include "zb_config.h"
#include "zb_types.h"
#include "zb_errors.h"
#include "zb_debug.h"

#include "zb_g_context.h"

#include "zb_osif.h"
#include "zb_trace.h"

/*! \addtogroup init_api */
/*! @{ */

/**
   Global stack initialization.

   To be called from main() at start.

   Usual initialization sequence: zb_init(), then assign some IB values, then zdo_startup().

   @param trace_comment - trace file name component (for Unix)
   @param rx_pipe - rx pipe name (for Unix/ns build) or node number (for ns build
                            in 8051 simulator)
   @param tx_pipe - tx pipe (for Unix)

   @b Example:
@code
#ifndef ZB8051
  zb_init("zdo_zc", argv[1], argv[2]);
#else
  zb_init("zdo_zc", "1", "1");
#endif
@endcode
 */

#ifdef ZB_INIT_HAS_ARGS
void zb_init(zb_char_t *trace_comment, zb_char_t *rx_pipe, zb_char_t *tx_pipe) ZB_CALLBACK;
#define ZB_INIT(a,b,c) zb_init((zb_char_t *)a, (zb_char_t *)b, (zb_char_t *)c)
#else
void zb_init()ZB_CALLBACK;
#define ZB_INIT(a,b,c) zb_init()
#endif

void zb_handle_parms_before_start();

/*! @} */


/*! \internal \addtogroup ZB_BASE */
/*! @{ */

/**
   Load Informational Bases from NVRAM or file
 */
void zb_ib_load() ZB_CALLBACK;


/**
   Set Informational Bases refaults.

   @param rx_pipe - rx pipe name (for Unix) or node number (for ns build
                            in 8051 simulator)
 */
void zb_ib_set_defaults(zb_char_t *rx_pipe) ZB_CALLBACK;


/**
   Save Informational Bases to NVRAM or other persistent storage
 */
void zb_ib_save() ZB_CALLBACK;


/*! @} */

#endif /* ZB_COMMON_H */
