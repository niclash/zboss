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
PURPOSE: Stubs (empty functions) to build MAC-only stack
*/

#ifndef ZB_MAC_ONLY_STUBS_H
#define ZB_MAC_ONLY_STUBS_H 1

/*! \addtogroup ZB_MAC */
/*! @{ */

#ifdef ZB_MAC_ONLY_STACK


/**
   \par How to use mac-only stubs

   That stubs are created to be able to compile mac-only stack.
   The problem is that MAC calls some callbacks by name (that are .confirm and
   .indication).

   Test which uses only MAC layer, must have defines for functions which test
   re-declares and then include this header file.
 */

void zb_nwk_init()
{
}


void zb_aps_init()
{
}


void zb_zdo_init()
{
}

void secur_generate_keys()
{
}


zb_ret_t zb_nwk_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst)
{
  ZVUNUSED(src);
  ZVUNUSED(dst);
  ZVUNUSED(mac_hdr_size);
  return 0;
}


void zb_zdo_check_fails(zb_uint8_t param) ZB_CALLBACK
{
  if ( param )
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
}

#ifndef USE_ZB_MLME_RESET_CONFIRM
void zb_mlme_reset_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MLME_SET_CONFIRM
void zb_mlme_set_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MLME_START_CONFIRM
void zb_mlme_start_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MCPS_DATA_CONFIRM
void zb_mcps_data_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MLME_BEACON_NOTIFY_INDICATION
void zb_mlme_beacon_notify_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MLME_POLL_CONFIRM
void zb_mlme_poll_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MLME_ORPHAN_INDICATION
void zb_mlme_orphan_indication(zb_uint8_t param)
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MLME_ASSOCIATE_CONFIRM
void zb_mlme_associate_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MLME_ASSOCIATE_INDICATION
void zb_mlme_associate_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MLME_SCAN_CONFIRM
void zb_mlme_scan_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MCPS_DATA_INDICATION
void zb_mcps_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#ifndef USE_ZB_MLME_COMM_STATUS_INDICATION
void zb_mlme_comm_status_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif


#ifndef USE_ZB_MLME_PURGE_CONFIRM
void zb_mlme_purge_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_free_buf(ZB_BUF_FROM_REF(param));
}
#endif

#endif  /* ZB_MAC_ONLY_STACK */


#endif /* ZB_MAC_ONLY_STUBS_H */

/*! @}  */
