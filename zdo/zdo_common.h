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
PURPOSE: ZDO common functions, both client and server side
*/

#ifndef ZDO_COMMON_H
#define ZDO_COMMON_H 1

#define ZB_TSN_HASH(tsn) ZB_1INT_HASH_FUNC(tsn) % ZDO_TRAN_TABLE_SIZE

void register_zdo_cb(zb_uint8_t tsn, zb_callback_t cb, zb_uint8_t resp_counter) ZB_SDCC_REENTRANT;

void zdo_send_req_by_short(zb_uint16_t command_id, zb_uint8_t param, zb_callback_t cb, zb_uint16_t addr,
                           zb_uint8_t resp_count);
void zdo_send_req_by_long(zb_uint8_t command_id, zb_uint8_t param, zb_callback_t cb, zb_ieee_addr_t addr);
void zdo_send_resp_by_short(zb_uint16_t command_id, zb_uint8_t param, zb_uint8_t tsn, zb_uint16_t addr) ZB_SDCC_REENTRANT;

/*
   Signal user that response came.
   param param - index of buffer with response
*/
zb_ret_t zdo_af_resp(zb_uint8_t param);


#endif /* ZDO_COMMON_H */
