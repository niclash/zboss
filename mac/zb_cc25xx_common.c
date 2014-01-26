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
PURPOSE: UZ2400 interrupt handler.

Moved into separate file to include it into common code bank.
*/


#include "zb_common.h"
#include "zb_cc25xx.h"

#ifndef ZB_SNIFFER
#include "zb_mac.h"
#include "zb_mac_transport.h"
#else
#include "zb_sniffer_tools.h" 
#endif /* ZB_SNIFFER */


/*! \addtogroup ZB_MAC */
/*! @{ */

#if defined ZB_CC25XX


/**
   ubec zb transceiver specific interrupt handler
 */


#ifdef ZB_IAR
#pragma vector=ZB_CC2530_RF_INTERRUPT
#endif
ZB_INTERRUPT zb_cc25xx_handler(void) INTERRUPT_DECLARATION_NOBANK(UBEC_2400_INTER_NUMBER)
{
  ZB_DISABLE_ALL_INTER();
  ZB_GET_ISRSTS();
  ZB_CLEAR_ISRSTS();
#ifndef ZB_SNIFFER
  if (ZB_MAC_GET_INDIRECT_DATA_REQUEST())
  {
    if (ZB_CHECK_PENDING())
    {
      ZB_MAC_SET_PENDING_DATA();
    } else
    {
      ZB_MAC_CLEAR_PENDING_DATA();
    }
  }
#endif /* ZB_SNIFFER */
  if (ZB_UBEC_GET_RX_DATA_STATUS())
  {
#ifndef ZB_SNIFFER
    ZB_PUT_RX_QUEUE();
#else
    zb_put_out_queue();
#endif /* ZB_SNIFFER */
  }
#ifndef ZB_SNIFFER
  if (ZB_UBEC_GET_TX_DATA_STATUS())
  {
    /* TODO: implement check for errors!!! */
    ZB_GET_TXSR();
#ifdef ZB_SECURITY	
    if (ZB_MAC_GET_SECURITY_PROCESS()) ZB_MAC_CLEAR_SECURITY_PROCESS();
    else
#endif
      MAC_CTX().tx_cnt++;
      /* ZB_CLEAR_NORMAL_FIFO_BUSY(); redundant*/
  }
#endif /* ZB_SNIFFER */
  ZB_ENABLE_ALL_INTER();
  ZB_SET_TRANS_INT();
}
#endif  /* ZB8051 */

/*! @} */
