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
PURPOSE: Test profile API
*/

#ifndef ZB_TEST_PROFILE_H
#define ZB_TEST_PROFILE_H 1

/*! \addtogroup ZB_AF */
/*! @{ */

#define ZB_TEST_PROFILE_ID       0x7f01 /*!< test profile id */
#define ZB_TEST_PROFILE_EP 240    /*!< test profile endpoint number (hardcoded) */

#define TP_TRANSMIT_COUNTED_PACKETS_CLID 0x01
#define TP_BUFFER_TEST_REQUEST_CLID      0x1C
#define TP_BUFFER_TEST_REQUEST_CLID2     0x1D
#define TP_BUFFER_TEST_RESPONSE_CLID     0x54

/**
   Transmit counted packets request
 */
typedef struct zb_tp_transmit_counted_packets_req_s
{
  zb_uint8_t len;      /*!< Length (in bytes) of the packet */
  zb_uint16_t counter; /*!< Octet sequence counter  */
}
ZB_PACKED_STRUCT
zb_tp_transmit_counted_packets_req_t;

/**
   Transmit counted packets parameters
 */
typedef struct zb_tp_transmit_counted_packets_param_s
{
  zb_uint8_t  len;            /*!< Length (in bytes) of the packet */
  zb_uint16_t packets_number; /*!< packets number to send */
  zb_uint16_t idle_time;      /*!< time before next packet is sent */
  zb_uint16_t dst_addr;       /*!< destinition address */
}
zb_tp_transmit_counted_packets_param_t;

/**
   Buffer test request
 */
typedef struct zb_buffer_test_req_s
{
  zb_uint8_t len;  /*!< Length (in bytes) of the packet */
}
ZB_PACKED_STRUCT
zb_buffer_test_req_t;

/**
   Buffer test request parameters
 */
typedef struct zb_buffer_test_req_param_s
{
  zb_uint8_t len;        /*!< Length (in bytes) of the packet */
  zb_uint16_t dst_addr;  /*!< destinition address */
  zb_uint8_t src_ep;
  zb_uint8_t dst_ep;
}
zb_buffer_test_req_param_t;

/**
   Buffer test request parameters, send by endpoint and cluster id
 */
typedef struct zb_buffer_test_req_param_EP_s
{
  zb_uint8_t len;        /*!< Length (in bytes) of the packet */
  zb_uint8_t src_ep;     /*!< src endpoint */
  zb_uint8_t dst_ep;     /*!< dst endpoint */
}
zb_buffer_test_req_param_EP_t;


#define ZB_TP_BUFFER_TEST_OK   0x00 /* Successful buffer test */
#define ZB_TP_BUFFER_TEST_FAIL 0x01 /* Transmission failure on first attempt */

/**
   Transmit buffer test response
*/
typedef struct zb_buffer_test_response_s
{
  zb_uint8_t len;     /*!< Length (in bytes) of the packet */
  zb_uint8_t status;  /*!< Status of the response */
}
ZB_PACKED_STRUCT
zb_buffer_test_response_t;

/**
   Transmit buffer test response parameters
*/
typedef struct zb_buffer_test_response_param_s
{
  zb_uint8_t len;       /*!< Length (in bytes) of the packet */
  zb_uint8_t status;    /*!< Status of the response */
  zb_uint16_t dst_addr; /*!< Status of the response */
  zb_uint8_t src_ep;
  zb_uint8_t dst_ep;
}
ZB_PACKED_STRUCT
zb_buffer_test_response_param_t;

void zb_test_profile_indication(zb_uint8_t param) ZB_CALLBACK;
void tp_send_counted_packet(zb_uint8_t param) ZB_CALLBACK;
void tp_send_req_by_short(zb_uint16_t command_id, zb_uint8_t param,zb_uint16_t addr, zb_uint8_t src_ep, zb_uint8_t dst_ep,
                          zb_uint8_t tx_options) ZB_SDCC_REENTRANT;
void zb_tp_transmit_counted_packets_req(zb_uint8_t param, zb_callback_t cb);
void tp_packet_ack(zb_uint8_t param) ZB_SDCC_REENTRANT;
void zb_tp_buffer_test_request(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;
void zb_tp_buffer_test_request_EP(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;
void zb_tp_buffer_test_request_EP_brdcast(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;

/*! @} */
#endif /* ZB_TEST_PROFILE_H */
