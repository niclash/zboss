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
PURPOSE: Some stuff specific to ns-3: simplified routines for MAC
*/

#ifndef ZB_NS3_H
#define ZB_NS3_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */

#ifdef ZB_NS_BUILD

zb_ret_t zb_mac_ns_send_packet(zb_buf_t *buf, zb_bool_t release_buf) ZB_SDCC_REENTRANT;

#define ZB_TRANS_SEND_COMMAND(hdr_len, buf) zb_mac_ns_send_packet(buf, 0)

zb_ret_t read_from_pipe();

#define ZB_TRANS_RECV_PACKET(buf) \
  ( zb_mac_transport_start_recv((buf), 0), read_from_pipe() )


#define ZB_TRANSCEIVER_INIT()
#define ZB_GET_TRANS_INT() 0
#define ZB_CLEAR_TRANS_INT()
#define ZB_CHECK_INT_STATUS()
#define ZB_CHECK_BEACON_MODE_ON() 0

#define ZB_TRANS_IS_COMMAND_SEND() (ZG->ioctx.send_data_buf == NULL)

#ifdef ZB_CHANNEL_ERROR_TEST
/* testing channel interference, emulate channel errors */
#define ZB_TRANS_CHECK_CHANNEL_ERROR() (((ZG->nwk.nib.nwk_tx_total % 3) == 0 && ZB_MAC_GET_CHANNEL_ERROR_TEST()) ? (1) : 0)
#else
#define ZB_TRANS_CHECK_CHANNEL_ERROR() 0
#endif /* ZB_CHANNEL_ERROR_TEST */


#define ZB_IS_TX_RETRY_COUNT_EXCEEDED() 0
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() 0
#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() 0
#define ZB_TRANS_CHECK_CHANNEL_CCA_FAIL_ERROR() 0
#define ZB_TRANSCEIVER_SET_CHANNEL(channel_number) (MAC_CTX().current_channel = channel_number, 0)
#define ZB_TRANSCEIVER_SET_BEACON_ONLY_MODE() while(0)
#define ZB_TRANSCEIVER_NORMAL_MODE() while(0)
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr) 0
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr) ZVUNUSED(0)
#define ZB_UPDATE_PAN_ID() while(0)
#define ZB_UPDATE_LONGMAC() while(0)

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26
#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id) (RET_OK)


#define ZB_SET_NSS_DOWN() 0
#define ZB_8051_DELAY() 0

#define ZB_UPDATE_SHORT_ADDR()

#define ZB_CLEAR_PENDING_BIT()
#define ZB_SET_PENDING_BIT()

/**
   Fake transiver context
 */
typedef struct zb_transceiver_ctx_s
{
  int dummy;
}
zb_transceiver_ctx_t;


#define ZB_TRANS_CUT_SPECIFIC_HEADER(zb_buffer)                         \
{                                                                       \
  /* 1st byte contains packet length - ubec specific */                 \
  (void)zb_buf_cut_left((zb_buffer), 1);                                \
}

#endif  /* ZB_NS_BUILD */

/*! @} */
/*! \endcond */

#endif /* ZB_NS3_H */
