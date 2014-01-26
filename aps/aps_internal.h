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
PURPOSE: APS layer private header file
*/

#ifndef APS_INTERNAL_H
#define APS_INTERNAL_H 1

/*! \addtogroup ZB_APS */
/*! @{ */

typedef struct zb_aps_command_header_s
{
  zb_uint8_t fc;
  zb_uint8_t aps_counter;
  zb_uint8_t aps_command_id;
} ZB_PACKED_STRUCT zb_aps_command_header_t;


typedef struct zb_aps_command_pkt_header_s
{
  zb_uint8_t fc;
  zb_uint8_t aps_counter;
} ZB_PACKED_STRUCT zb_aps_command_pkt_header_t;


/**
   Handle packet duplicates: keep dups cache.

   @param src_addr - short address of the incoming packet
   @param aps_counter - aps_counter field from the packet
   @return 1 if this is a dup, 0 otherwhise.
 */
zb_short_t aps_check_dups(zb_uint16_t src_addr, zb_uint8_t aps_counter) ZB_SDCC_REENTRANT;

void zb_aps_in_command_handle(zb_uint8_t param) ZB_CALLBACK;

void zb_aps_in_transport_key(zb_uint8_t param);

void zb_aps_in_update_device(zb_uint8_t param);

/*! @} */

#endif /* APS_INTERNAL_H */
