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
PURPOSE: MAC layer/transport states
*/

#ifndef _ZB_MAC_STATE_H_
#define _ZB_MAC_STATE_H_ 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */

typedef enum
{
  ZB_MAC_LAYER_1 = 0,
  ZB_MAC_LAYER_2,
  ZB_MAC_LAYER_3, /* 3rd layer - for general purpose functions, like zb_mac_get_indirect_data */
  ZB_MAC_IO_LAYER_RX,  /* layer is used for i/o state */
  ZB_MAC_IO_LAYER_TX
} zb_mac_layer_t;

typedef enum
{
  ZB_MAC_STATE_IDLE = 0,
  ZB_MAC_STATE_READREG_INTERRUPT_STATUS,
  ZB_MAC_STATE_DATA_WAIT_FOR_RECV, /* 2 */
  ZB_MAC_STATE_WAIT_FOR_SEND_SHORTREAD_REG,
  ZB_MAC_STATE_WAIT_FOR_RECV_SHORTREAD_REG,
  ZB_MAC_STATE_WAIT_FOR_SEND_LONGREAD_REG,
  ZB_MAC_STATE_WAIT_FOR_RECV_LONGREAD_REG,
  ZB_MAC_STATE_WAIT_FOR_SEND_SHORTWRITE_REG,
  ZB_MAC_STATE_WAIT_FOR_SEND_LONGWRITE_REG,
  ZB_MAC_STATE_WAIT_FOR_FIFO_SEND,
  ZB_MAC_STATE_MLME_START_WAIT_TXMCR,
  ZB_MAC_STATE_MLME_START_WAIT_ORDER,
  ZB_MAC_STATE_MLME_START_SUPER_FRAME_STEP1,
  ZB_MAC_STATE_MLME_START, /* 13 */
  ZB_MAC_STATE_DATA_FRAME,

  ZB_MAC_STATE_MLME_WAIT_REAILIGNMENT_SENT,
  ZB_MAC_STATE_WAIT_CMD_TX_FINISH, /* 16 */
  ZB_MAC_STATE_WAIT_CMD_TX_FINISH_2,
  ZB_MAC_STATE_WAIT_CMD_TX_FINISH_3, /* 18 */
  ZB_MAC_STATE_WAIT_SYNC_TIMER,
  ZB_MAC_STATE_WAIT_ACK, /* 20 */
  ZB_MAC_STATE_WAIT_INDIRECT_DATA,
  ZB_MAC_STATE_WAIT_RESP,  /* 22 */
  ZB_MAC_STATE_MLME_SCAN,
  ZB_MAC_STATE_MLME_ASSOCIATE, /* 24 */
  ZB_MAC_STATE_MLME_ASSOCIATE_RESPONSE,
  ZB_MAC_STATE_DATA_REQUEST,
  ZB_MAC_STATE_DATA_REQUEST_CMD,
  ZB_MAC_STATE_ASS_REQUEST_CMD,
  ZB_MAC_STATE_POLL_REQUEST_CMD,

  /* state for MAC 3rd layer, i/o operations */
  ZB_MAC_STATE_IO_ERROR,         /* 30 */
  ZB_MAC_STATE_IO_RECV_PENDING
} zb_mac_state_e;

#define MAC_STATE_FOR_LAYER( layer )                                    \
    (                                                    \
    (zb_uint8_t) ZG->mac.mac_ctx.state[ (layer) ] + 0                                \
    )

/* void mac_dump_states(); */ /* removed along with mac states */
#if 1
#define MAC_DUMP_STATES() mac_dump_states()
#else
#define MAC_DUMP_STATES()
#endif


#define MAC_SET_STATE_FOR_LAYER( layer, newstate )              \
{                                                               \
  TRACE_MSG(TRACE_MAC2, "mac set state %hd %hd -> %hd", (FMT__H_H_H, layer, ZG->mac.mac_ctx.state[ (layer) ], newstate)); \
  ZG->mac.mac_ctx.state[ (layer) ] = (newstate);                \
  MAC_DUMP_STATES();                                            \
}

#define MAC_SET_STATE_FOR_LAYER_NO_LOG( layer, newstate )       \
{                                                               \
  ZG->mac.mac_ctx.state[ (layer) ] = (newstate);                \
}

/*! @} */
/*! \endcond */

#endif /* _ZB_MAC_STATE_H_ */
