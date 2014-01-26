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
PURPOSE: Security - API to be used from the applications level
*/

#ifndef ZB_SECUR_API_H
#define ZB_SECUR_API_H 1

/*! \addtogroup secur_api */
/*! @{ */

/**
   Setup pre-configured key to be used by ZCP tests.

   @param key - key to be used
   @param i - key number (0-3)
*/

void zb_secur_setup_preconfigured_key(zb_uint8_t *key, zb_uint8_t i);



/**
   Send new network key to all devices in the net via broadcast

   4.6.3.4  Network Key Update
   4.6.3.4.1  Trust Center Operation

   @param param - buffer with single parameter - short broadcast address. Valid
          values are 0xffff, 0xfffd
 */
void zb_secur_send_nwk_key_update_br(zb_uint8_t param) ZB_CALLBACK;



/**
   Generate switch key.

   According to test 14.24TP/SEC/BV-01-I Security NWK Key Switch (No Pre-
   configured Key)-ZR, this command can be send either to broadcast or unicast
   to all rx-on-when-idle from the neighbor.
   When send unicast, it encrypted by the new (!) key, when send proadcast - by
   the old key.
   That mean, switch our key _after_ this frame transfer and secure - in the
   command send confirm.


   @param param - packet buffer with single parameter - broadcast address. If 0,
          send unicast.
 */
void zb_secur_send_nwk_key_switch(zb_uint8_t param) ZB_CALLBACK;

/**
   Clear preconfigures key (key number 0)
 */
void secur_clear_preconfigured_key();

/*! @} */

#endif /* ZB_SECUR_API_H */
