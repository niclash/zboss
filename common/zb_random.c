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
PURPOSE: 8051-specific random number generator
*/

#include "zb_common.h"
#include "zb_bank_common.h"

#define RND_BASE ((zb_uint32_t)&g_izb)

zb_uint16_t zb_random()
{
  static zb_uint32_t v = 0;
  v = (zb_uint32_t)(RND_BASE);
  v = 1103515245l * v + 12345;
  return (v & 0xffff);
}

#ifndef ZB_LITTLE_ENDIAN
void zb_htole32(zb_uint32_t ZB_XDATA *ptr, zb_uint32_t ZB_XDATA *val)
{
  ((zb_uint8_t *)(ptr))[3] = ((zb_uint8_t *)(val))[0],
  ((zb_uint8_t *)(ptr))[2] = ((zb_uint8_t *)(val))[1],
  ((zb_uint8_t *)(ptr))[1] = ((zb_uint8_t *)(val))[2],
  ((zb_uint8_t *)(ptr))[0] = ((zb_uint8_t *)(val))[3];
}
#endif


void zb_get_next_letoh16(zb_uint16_t *dst, zb_uint8_t **src)
{
  ZB_LETOH16(dst, *src);
  (*src) += 2;
}

void zb_put_next_htole16(zb_uint8_t **dst, zb_uint16_t val)
{
  ZB_HTOLE16(*dst, &val);
  (*dst) += 2;
}


void zb_bzero_short(char *s, zb_uint8_t n)
{
  while (n--)
  {
    s[n] = 0;
  }
}

#if 0
void zb_bzero_2(char *s)
{
  s[0] = s[1] = 0;
}
#endif
