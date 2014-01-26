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
PURPOSE: Initialize off-chip XRAM for 8051
*/
#include "zb_common.h"
#include "zb_bank_common.h"

#ifdef C8051F120
void zb_xram_init()  
{
  char SFRPAGE_SAVE;
  SFRPAGE_SAVE = SFRPAGE;
  SFRPAGE = CONFIG_PAGE;
  P4MDOUT |= 0xE0;
  P5MDOUT = 0xFF;
  P6MDOUT = 0xFF;
  P7MDOUT = 0xFF;
					
					
  P4 |= 0xE0;
  P5 = 0xFF;
  P6 = 0xFF;
  P7 = 0xFF;

  P1MDIN = 0xff;      // Port 1 as digital input mode
  SFRPAGE = SFRPAGE_SAVE;

//Program the external memory interface
//6-7: Don't care
//5: 1->EMIF active on port4-7
//4: 1->non-multiplexed mode
//2-3: 01->Below 8K, on Chip, Above 8K external SRAM only
//0-1: 00->ALE pulse width 1 SYSCLK cycle
  EMI0CF = 0x34;
  EMI0TC = 0x8e;

}
#endif 




