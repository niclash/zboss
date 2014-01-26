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
PURPOSE: ZigBee small part from MAC packet format. Used to extract destination
address from the MAC packet.
*/

#include "mac_frame.h"
#include <string.h>
#include <iostream>

using namespace std;

void swap(void *ptr)
{
  uint8_t *p = (uint8_t*)ptr;
  uint8_t c = p[0];
  p[0] = p[1];
  p[1] = c;
}

void swap32(void *ptr)
{
  uint8_t *p = (uint8_t*)ptr;
  uint8_t c = p[0];
  p[0] = p[3];
  p[3] = c;

  c = p[1];
  p[1] = p[2];
  p[2] = c;
}

void get_mac_addr_16bit_addr_fileds(uint8_t *ptr, mac_addr_16bit_addr_fileds_t *ret)
{
  memcpy(ret, ptr, sizeof(mac_addr_16bit_addr_fileds_t));
#ifdef ZB_BIG_ENDIAN
  swap(&ret->dest_pan_id);
  swap(&ret->dest_addr);
  swap(&ret->source_pan_id);
  swap(&ret->source_addr);
#endif
}

void get_mac_addr_64bit_addr_fileds(uint8_t *ptr, mac_addr_64bit_addr_fileds_t *ret)
{
  memcpy(ret, ptr, sizeof(mac_addr_64bit_addr_fileds_t));
#ifdef ZB_BIG_ENDIAN
  swap(&ret->dest_pan_id);
  swap(&ret->source_pan_id);
#endif
}

void get_mac_frame_control(uint8_t *ptr, mac_frame_control_t *ret)
{
  memcpy(ret, ptr, sizeof(mac_frame_control_t));
#ifdef ZB_BIG_ENDIAN
  swap(ret);
#endif
}

void get_mac_frame_format_16bit_addr(uint8_t *ptr, mac_frame_format_16bit_addr_t *ret)
{
#ifdef ZB_LITTLE_ENDIAN
  memcpy(ret, ptr, sizeof(mac_frame_format_16bit_addr_t));
#else
  mac_frame_format_16bit_addr_t *val = (mac_frame_format_16bit_addr_t*)ptr;
  get_mac_frame_control((uint8_t*)(&val->frame_control), &ret->frame_control);
  ret->sequence_number = val->sequence_number;
  get_mac_addr_16bit_addr_fileds((uint8_t*)(&val->addr), &ret->addr);
#endif
}

void get_mac_frame_format_64bit_addr(uint8_t *ptr, mac_frame_format_64bit_addr_t *ret)
{
#ifdef ZB_LITTLE_ENDIAN
  memcpy(ret, ptr, sizeof(mac_frame_format_64bit_addr_t));
#else
  mac_frame_format_64bit_addr_t *val = (mac_frame_format_64bit_addr_t*)ptr;
  get_mac_frame_control((uint8_t*)(&val->frame_control), &ret->frame_control);
  ret->sequence_number = val->sequence_number;
  get_mac_addr_64bit_addr_fileds((uint8_t*)(&val->addr), &ret->addr);
#endif
}

void get_nwk_frame_control(uint8_t *ptr, nwk_frame_control_t *ret)
{
  memcpy(ret, ptr, sizeof(nwk_frame_control_t));
#ifdef ZB_BIG_ENDIAN
  swap(ret);
#endif
}

void get_nwk_hdr(uint8_t *ptr, nwk_hdr_t *ret)
{
  memcpy(ret, ptr, sizeof(nwk_hdr_t));
#ifdef ZB_BIG_ENDIAN
  swap(&ret->dst_addr);
  swap(&ret->src_addr);
#endif
}

void nwk_aux_frame_hdr(uint8_t *ptr, nwk_aux_frame_hdr_t *ret)
{
  memcpy(ret, ptr, sizeof(nwk_aux_frame_hdr_t));
#ifdef ZB_BIG_ENDIAN
  swap32(&ret->frame_counter);
#endif
}

void get_secur_ccm_nonce(uint8_t *ptr, secur_ccm_nonce_t *ret)
{
  memcpy(ret, ptr, sizeof(secur_ccm_nonce_t));
#ifdef ZB_BIG_ENDIAN
  swap32(&ret->frame_counter);
#endif
}

void get_aps_frame_control(uint8_t *ptr, aps_frame_control_t *ret)
{
  memcpy(ret, ptr, sizeof(aps_frame_control_t));
}
