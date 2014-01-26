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
PURPOSE: MAC security routines
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_secur.h"

#include "zb_bank_common.h"
#ifdef ZB_MAC_SECURITY

/*! \addtogroup ZB_SECUR */
/*! @{ */

/**
   Secure MAC frame. Frame already have aux header filled

   @param src - source (unencrypted) frame
   @param mac_hdr_size_wo_aux size of mac hdr, without aux security header
   @param dst - destination (encrypted) frame
 */
zb_ret_t zb_mac_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst)
{
  zb_ret_t ret = RET_OK;
  zb_secur_ccm_nonce_t nonce;
  zb_uint8_t *aux;

  ZB_CHK_ARR(ZB_BUF_BEGIN(src), ZB_BUF_LEN(src));

  aux = ZB_BUF_BEGIN(src) + mac_hdr_size;
  mac_hdr_size += MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE;

  /* nonce - see 7.6.3.2 CCM* Nonce. */
  ZB_IEEE_ADDR_COPY(nonce.source_address, ZB_PIB_EXTENDED_ADDRESS());
  /* frame counter */
  ZB_MEMCPY(&nonce.frame_counter, aux+1, 4);
  nonce.secur_control = ZB_MAC_SECURITY_LEVEL;

  /* Secure. zb_ccm_encrypt_n_auth() allocs space for a and m, but not mhr.  */
  ret = zb_ccm_encrypt_n_auth(MAC_PIB().mac_key,
                              (zb_uint8_t *)(&nonce),
                              ZB_BUF_BEGIN(src), mac_hdr_size, /* a */
                              ZB_BUF_BEGIN(src) + mac_hdr_size, ZB_BUF_LEN(src) - mac_hdr_size, /* m */
                              dst);
  TRACE_MSG(TRACE_SECUR2, "ret zb_mac_secure_frame %hd", (FMT__D, ret));
  return ret;
}


zb_ret_t zb_mac_unsecure_frame(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_ret_t ret = RET_OK;
  zb_ushort_t i;
  zb_secur_ccm_nonce_t nonce;
  zb_ushort_t mhr_len;
  zb_ushort_t is_long_addr;

  {
    zb_mac_mhr_t mhr;

    mhr_len = zb_parse_mhr(&mhr, ZB_BUF_BEGIN(buf));
    is_long_addr = (ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr.frame_control) == ZB_ADDR_64BIT_DEV);

    /* fill nonce, check frame counter */

    /* to fill nonce, needs long address. If device or its address absent in
     * macDeviceTable, can't decrypt. */

    /* find dev in device table: need long address and packets counter. */
    for (i = 0 ; i < MAC_PIB().mac_device_table_entries ; ++i)
    {
      TRACE_MSG(TRACE_SECUR2, "i %hd is_long %hd panid 0x%x mhr_panid 0x%x",
                (FMT__H_H_D_D, i, is_long_addr, MAC_PIB().mac_device_table[i].pan_id, mhr.src_pan_id));

      if (MAC_PIB().mac_device_table[i].pan_id == mhr.src_pan_id)
      {
        if (is_long_addr)
        {
          if (ZB_IEEE_ADDR_CMP(MAC_PIB().mac_device_table[i].long_address, mhr.src_addr.addr_long))
          {
            break;
          }
        }
        else
        {
          TRACE_MSG(TRACE_SECUR2, "i %hd addr 0x%x mhr_addr 0x%x",
                    (FMT__H_D_D, i, MAC_PIB().mac_device_table[i].short_address, mhr.src_addr.addr_short));
          if (MAC_PIB().mac_device_table[i].short_address == mhr.src_addr.addr_short)
          {
            break;
          }
        }
      }
    }
    if (i == MAC_PIB().mac_device_table_entries)
    {
      /* no such device - can't decrypt */
      TRACE_MSG(TRACE_SECUR1, "device not found - MAC unsecure failed", (FMT__0));
      ret = RET_ERROR;
    }
    else if (MAC_PIB().mac_device_table[i].frame_counter > mhr.frame_counter
             || MAC_PIB().mac_device_table[i].frame_counter == (zb_uint32_t)~0)
    {
      ret = RET_ERROR;
      TRACE_MSG(TRACE_SECUR1, "frm cnt %ld->%ld shift back - MAC unsecure failed",
                (FMT__L_L, MAC_PIB().mac_device_table[i].frame_counter > mhr.frame_counter));
    }

    if (ret == RET_OK)
    {
      /* nonce - see 7.6.3.2 CCM* Nonce. */
      ZB_IEEE_ADDR_COPY(nonce.source_address, MAC_PIB().mac_device_table[i].long_address);
      ZB_HTOLE32(&nonce.frame_counter, &mhr.frame_counter);
      nonce.secur_control = ZB_MAC_SECURITY_LEVEL;
    }
  }

  if (ret == RET_OK)
  {
    zb_uint8_t save_tail[ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME];
    zb_uint8_t *p;
    ZB_MEMCPY(save_tail,
              ZB_BUF_BEGIN(buf) + ZB_BUF_LEN(buf) - ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME,
              ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME);
    ZB_BUF_CUT_RIGHT(buf, ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME);

    ret = zb_ccm_decrypt_n_auth_stdsecur(MAC_PIB().mac_key,
                                         (zb_uint8_t *)&nonce,
                                         buf, mhr_len,
                                         ZB_BUF_LEN(buf) - mhr_len);
    TRACE_MSG(TRACE_SECUR3, "MAC packet unsecure ret %d", (FMT__D, ret));
    ZB_BUF_ALLOC_RIGHT(buf, ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME, p);
    ZB_MEMCPY(p, save_tail, ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME);
    /* Note: FCS became invalid, but we never use it. */
  }
  return ret;
}

#endif  /* ZB_MAC_SECURITY */

/*! @} */
