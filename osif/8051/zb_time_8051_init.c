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
PURPOSE: time functions implementation for 8051: init
*/

#include "zb_common.h"
#include "zb_osif.h"

/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#include "zb_bank_common.h"

#ifdef ZB_CC25XX
void zb_8051_init_timer()
{
  T1CTL |= 0x0C;    /* 000'1100 Set prescaler divider value to 128 */

  // Set compare value
  T1CCTL0 = 0x44;   /* Set conmpare mode with interrupt enabled */

  // clear interrupt pending flag, disable interrupt
  T1STAT &= ~0x01;  // T1STAT.CH1IF = 0
  IEN1   &= ~0x02;  // IEN1.T1EN = 0
  init_clock(); /* transceiver needs clock for carrying frequency modulation */
  clock_set_src();
  IEN1 |=  0x02; // IEN1.T1EN = 1
}
                        
#else

/* Timer0 is used */
void zb_8051_init_timer()
{
/* TCON: Timer Control */
  TR0 = 0; /* stop timer0 */

/* IE: Interrupt Enable */
  ET0 = 1; /* enable timer0 interrupt */

/*
  TMOD: Timer Mode
  Bits 0-1, T0M0=1, T0M1=0: Timer 0 Mode Select, Mode 1: 16-bit counter/timer
  bit 2, C/T0 = 0  Timer 0 incremented by clock defined by T0M bit (CKCON.3)
  bit 3, GATE0 = 0  Timer 0 enabled when TR0 = 1 irrespective of /INT0 logic level

  1111.0000 == 0xf0
  0000.0001 == 0x01
*/
  TMOD &= 0xF0;
  TMOD |= 0x01;

/*
  CKCON: Clock Control
  Bit3: T0M: Timer 0 Clock Select.
  T0M = 0 Counter/Timer 0 uses the clock defined by the prescale bits, SCA1-SCA0.
  Bits1 - 0: SCA1 - SCA0: Timer 0/1 Prescale Bits
  SCA1 = 1, SCA0 = 0, System clock divided by 48

  0000.0010 == 0x02
*/
#ifdef C8051F120
  CKCON |= 0x02; /* TODO: get from UBEC information about this reg in 2410!!*/
#endif
}
#endif /* ZB_CC25XX */
/*! @} */
