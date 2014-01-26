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
PURPOSE: OS and platform depenednt stuff for Unix platform
*/

#ifndef ZB_OSIF_UNIX_H
#define ZB_OSIF_UNIX_H 1

/*! \addtogroup ZB_OSIF_UNIX */
/*! @{ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>


#define ZB_ABORT abort

#define ZB_SDCC_REENTRANT
#define ZB_SDCC_XDATA
#define ZB_CALLBACK
#define ZB_SDCC_BANKED
#define ZB_KEIL_REENTRANT

/* Unix-specific trace */

extern FILE *g_trace_file;

void zb_trace_mutex_lock_unix();
void zb_trace_mutex_unlock_unix();

#define ZB_TRACE_LOCK zb_trace_mutex_lock_unix
#define ZB_TRACE_UNLOCK zb_trace_mutex_unlock_unix

#define ZB_OSIF_GLOBAL_LOCK()
#define ZB_OSIF_GLOBAL_UNLOCK()

#define ZB_XDATA
#define ZB_CODE
#define ZB_STOP_WATCHDOG()

#define DECLARE_SERIAL_INTER_HANDLER()
#define ZB_START_DEVICE()

/**
   Return random value
 */
zb_uint16_t zb_random();
#define ZB_RANDOM() zb_random()

/* use macros to be able to redefine */
#define ZB_MEMCPY memcpy
#define ZB_MEMMOVE memmove
#define ZB_MEMSET memset
#define ZB_MEMCMP memcmp

#define ZVUNUSED(v) (void)v


#define ZB_ENABLE_ALL_INTER()
#define ZB_DISABLE_ALL_INTER()

#define ZB_VOLATILE

#define ZB_BZERO(s,l) ZB_MEMSET((char*)(s), 0, (l))
#define ZB_BZERO2(s) ZB_BZERO(s, 2)

/*! @} */

#endif /* ZB_OSIF_UNIX_H */
