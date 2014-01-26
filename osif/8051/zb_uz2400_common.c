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
#include "zb_mac.h"
#include "zb_mac_transport.h"
#include "zb_ubec24xx.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

#if defined ZB8051 && !defined ZB_NS_BUILD && (defined ZB_UZ2400 || defined ZB_UZ2410)
#include "zb_bank_common.h"

/**
   ubec zb transceiver specific interrupt handler
 */
#ifdef ZB_IAR
#pragma vector=UBEC_2400_INTER_NUMBER
#endif
ZB_INTERRUPT ubec_2400_handler(void) INTERRUPT_DECLARATION_NOBANK(UBEC_2400_INTER_NUMBER)
{
#ifdef ZB_USE_RX_QUEUE
ZB_DISABLE_ALL_INTER();
ZB_GET_ISRSTS();

#ifdef ZB_RESERVED_REGS_DUMP
 MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.write_i]->isrsts = ZG->transceiver.int_status;
#endif
 if (ZB_MAC_GET_INDIRECT_DATA_REQUEST())
 	{
#ifdef ZB_RESERVED_REGS_DUMP
	    MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.write_i]->txntrig =
		 *((zb_uint8_t ZB_XDATA *)(ZB_SREG_TXNTRIG|ZB_SHORT_REGS_BASE));
#endif
		if (ZB_CHECK_PENDING())
		 	{
			 	ZB_MAC_SET_PENDING_DATA();
			} else
			{
				ZB_MAC_CLEAR_PENDING_DATA();
			}
	}
#ifdef ZB_RESERVED_REGS_DUMP
	else MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.write_i]->txntrig = -1;
#endif
 if (ZB_UBEC_GET_RX_DATA_STATUS())
 {
	ZB_PUT_RX_QUEUE();
 }
 if	(ZB_UBEC_GET_TX_DATA_STATUS())
 {

#ifdef ZB_RESERVED_REGS_DUMP
	MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.write_i]->txsr = ZG->transceiver.tx_status;
#endif
	ZB_GET_TXSR();
#ifdef ZB_SECURITY	
	if (ZB_MAC_GET_SECURITY_PROCESS()) ZB_MAC_CLEAR_SECURITY_PROCESS();
 	else
#endif
	 MAC_CTX().tx_cnt++;
	/* ZB_CLEAR_NORMAL_FIFO_BUSY(); redundant*/
 }
#ifdef ZB_RESERVED_REGS_DUMP
 else
 {
 	 MAC_CTX().regs_queue.ring_buf[MAC_CTX().regs_queue.write_i]->txsr = -1;
 }
 ZB_RING_BUFFER_FLUSH_PUT(&MAC_CTX().regs_queue);
#endif
 ZB_ENABLE_ALL_INTER();
#endif
  ZB_SET_TRANS_INT();
}
#endif  /* ZB8051 */

/*! @} */
