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
PURPOSE: Main header for OS and platform depenednt stuff
*/

#ifndef ZB_OSIF_H
#define ZB_OSIF_H 1

/*! \addtogroup ZB_OSIF */
/*! @{ */

#ifdef __IAR_SYSTEMS_ICC__
#ifndef ZB_IAR
#define ZB_IAR
#endif
#endif

#ifdef UNIX
#include "zb_osif_unix.h"
#elif defined ZB8051 || defined ZB_UZ2410 || defined ZB_CC25XX
#include "zb_osif_8051.h"
#else
#error Port me!
#endif

#if !defined KEIL

#define MAIN() int main(int argc, char **argv)
#define FAKE_ARGV
#define ARGV_UNUSED ZVUNUSED(argc) ; ZVUNUSED(argv)
#define MAIN_RETURN(v) return (v)

#else

#define MAIN() void main(void)
#define FAKE_ARGV char **argv = NULL
#define ARGV_UNUSED
#define MAIN_RETURN(v)

#endif  /* KEIL */

#if defined SDCC && defined ZB_BANKED_BUILD

#define  banked
#define ZB_CB_NAME_MACRO(a) a ## func

/**
   \par stub is function placed in the common bank which calls real function
   from anothe bank.
 */
#define ZB_CB_STUB(name)                                \
void ZB_CB_NAME_MACRO(name)(zb_uint8_t param) ZB_SDCC_REENTRANT; \
void name(zb_uint8_t param) ZB_SDCC_REENTRANT                    \
{                                                       \
  ZB_CB_NAME_MACRO(name) (param);                          \
}
#else  /* SDCC && ZB_BANKED_BUILD */
#endif  /* SDCC && ZB_BANKED_BUILD */

/*! @} */

#endif /* ZB_OSIF_H */
