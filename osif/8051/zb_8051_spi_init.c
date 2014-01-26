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
PURPOSE: SPI transport for 8051: code for init
*/


#include "zb_common.h"
#include "zb_mac_transport.h"
#include "zb_bufpool.h"                                           
#include "zb_ringbuffer.h"

#include "zb_bank_common.h"
#ifdef ZB_TRANSPORT_8051_DATA_SPI 

void zb_spi_init()
{
  SFRPAGE = CONFIG_PAGE; //switching to 0xf sfr page
  OSCICN = 0x83; /* internal oscillator      */
  while(!(OSCICN & 0x40)) /* wait for clock is ready */
  {
  }
  XBR0 = 0xf7; /* enables SPI, UART   */
  XBR2 = 0x44; /* enable the Crossbar */
  P0MDOUT = 0x34; /* push-pull f/spi  */
  SFRPAGE = 0;
   /* set spi clock rate. F = SYSCLK / (2 x (SPI0CKR + 1)) */
   /* spi clock for UBEC device should be not more then 10 MHz */
   /* default internal oscilator in C8051F12x is 24.5 MHz */
   /* SPI CLK = 24.5M/4 = 6.125M */
  SPI0CKR = ZB_8051_CKR_VALUE+1; /* changed by den, experimentally, coz of some errors*/

   /* configuration register:
     - set master mode bit 6 = 1
     - data centered on first edge of SCK period, bit 5 = 0
     - SCK line low in idle state, bit 4 = 0 */
  SPI0CFG &= 0x8F;
  SPI0CFG |= 0x40;
  SPI0CN = 0x0D;

   /* SPI0CN regidter - spi control
     use 4 wire mode, NSS is controled by NSSMD0 */
  NSSMD1 = 1;
  NSSMD0 = 1;
   /* set trans as slave-selected device */
   /* TODO: check if always selected device is ok or not and it's
    * better to select device only when i/o is performed */

   /* enable spi */
  SPIEN = 1;
  //EIP1|=0x01;  
 ZB_ENABLE_SPI_INTER();
}

#endif /* ZB_TRANSPORT_8051_DATA_SPI */
