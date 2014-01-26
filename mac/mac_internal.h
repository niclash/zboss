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
PURPOSE: MAC  layer main module
*/

#ifndef MAC_INTERNAL_INCLUDED
#define MAC_INTERNAL_INCLUDED 1

/*! \addtogroup ZB_MAC */
/*! @{ */


#include "zb_common.h"
#include "zb_ringbuffer.h"

typedef enum
{
  WC_RECV = 1,
  WC_SEND = 2
} zb_mac_wait_cond_t;

typedef enum
{
  ZB_MAC_DATA_REQUEST = 0,
  ZB_MAC_SCAN_REQUEST,
  ZB_MAC_MLME_START_REQUEST,
  ZB_MAC_MLME_ASSOCIATE_REQUEST,
  ZB_MAC_MLME_ASSOCIATE_RESPONSE,
  ZB_MAC_DATA_REQUEST_CMD,
  ZB_MAC_ASS_REQUEST_CMD,
  ZB_MAC_DATA_FRAME,
  ZB_MAC_MLME_ORPHAN_RESPONSE,
  ZB_MAC_POLL_REQUEST
} zb_mac_request_type_e;

#define ZB_UINT_BACKOFF_PERIOD 20
#define ZB_TURNAROUND_TIME 12
#define ZB_PHY_SHR_DURATION 40 /* 3,7,10,40 */
#define ZB_PHY_SYMBOLS_PER_OCTET 2.8 /* 0.4, 1.6, 2,8 */

#define ZB_MAC_BASE_SLOT_DURATION 60
#define ZB_MAC_NUM_SUPERFRAME_SLOTS 16
#define ZB_MAC_TRANSACTION_PERSISTENCE_TIME 0x01f4

#ifdef ZB_MANUAL_ACK
zb_ret_t zb_mac_check_ack();
#endif
zb_ret_t call_indirect_data_callback(zb_callback_type_t cb_type, zb_uint8_t cb_status, zb_buf_t *buf);
void indirect_data_callback_caller(zb_uint8_t param) ZB_CALLBACK;
zb_ret_t zb_mac_check_security(zb_buf_t *data_buf);

void zb_tx_total_inc();
void zb_tx_fail_inc();
#define ZB_TX_TOTAL_INC() zb_tx_total_inc()
#define ZB_TX_FAIL_INC() zb_tx_fail_inc()

void zb_mac_put_request_to_queue(zb_buf_t *request, zb_mac_request_type_e req_type) ZB_CALLBACK;
void zb_mac_store_pan_desc(zb_buf_t *beacon_buf);

/*! @} */

#endif
