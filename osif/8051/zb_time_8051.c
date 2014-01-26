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
PURPOSE: time functions implementation for 8051 for the Common bank
*/

#include "zb_common.h"
#include "zb_osif.h"

/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#include "zb_bank_common.h"

/**
   8051 timer0 interrupt handler
 */
#ifdef ZB_IAR
#pragma register_bank=REGISTER_BANK_2
#pragma vector=TIMER0_INTER_NUMBER
#endif
ZB_INTERRUPT timer0_inter_handler(void) INTERRUPT_DECLARATION(TIMER0_INTER_NUMBER, REGISTER_BANK_2)
{
  ZB_TIMER_CTX().timer++;

  /* Stop timer if it expired or not running. */
  if (!ZB_TIMER_CTX().started ||
       ZB_TIME_GE((zb_time_t)ZB_TIMER_CTX().timer, (zb_time_t)ZB_TIMER_CTX().timer_stop))
  {
    ZB_STOP_8051_TIMER();
    ZB_TIMER_CTX().timer_stop = ZB_TIMER_CTX().timer;
    ZB_TIMER_CTX().started = 0;
  }
  else
  {
    /* TODO: check if setting TR0 = 1 is needed */
    ZB_START_8051_TIMER();
  }
}

/*! @} */
