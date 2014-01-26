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
PURPOSE: low level pre-start nitialization
*/
#include "zb_common.h"

/* this function called before main, it's compiler dependent */
int zb_low_level_init(void)
{
  /* first of all, prevent our reset by watchdog during init */
  ZB_STOP_WATCHDOG();
/*  Address: 0x93
	Bits Name Description Reset Value R/W
	7 COBKEN Enable common bank scheme 0 R/W
	6-3 Reserved - - -
	2-0 PBS Program bank selection bit 0x0 R/W

  COBKEN :
	1: Enable common-bank scheme that uses a common memory which located from 0x0000 to 0x7fff.
	Furthermore, data located between 0x8000 and 0xffff can be placed in the different bank.
	0: Keep Mentor M8051EW external memory (MEXT) setting.
*/
#if defined ZB_UZ2410_256 && !defined ZB_UZ2410_256_UNBANKED
 COBK |=0x80;
#endif
#if defined ZB_TRANSPORT_8051_UART
  /* serial used for trace on 8051 */
 zb_init_8051_serial();
#endif
#if defined C8051F120
  zb_spi_init();
  zb_xram_init();
#endif
#if defined ZB8051 && !defined ZB_NS_BUILD && !defined ZB_CC25XX
  /* RX interrupt for transceiver, external or internal */
  zb_ext_int_init();
#endif
  return(1);
}

