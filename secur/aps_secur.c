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
PURPOSE: APS frames security routines
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "../aps/aps_internal.h"
#include "zb_secur.h"

#include "zb_bank_common.h"
#ifdef ZB_SECURITY

/*! \addtogroup ZB_SECUR */
/*! @{ */

#ifdef APS_FRAME_SECURITY

void zb_secur_aps_aux_hdr_fill(zb_uint8_t *p, zb_bool_t nwk_key)
{
  zb_aps_nwk_aux_frame_hdr_t *aux = (zb_aps_nwk_aux_frame_hdr_t *)p;

  if (nwk_key)
  {
    ZB_HTOLE32(&aux->frame_counter, &ZG->nwk.nib.outgoing_frame_counter);

    TRACE_MSG(TRACE_SECUR3, "aps aux hdr - nwk key crypt", (FMT__0));

    ZG->nwk.nib.outgoing_frame_counter++;

    aux->key_seq_number = ZG->nwk.nib.active_key_seq_number;;
    aux->secur_control = ZB_APS_NWK_STD_SECUR_CONTROL;

#ifdef ZB_COORDINATOR_ROLE
    /* If counter is near limit, switch key */
    if (ZG->nwk.handle.is_tc && ZG->nwk.nib.outgoing_frame_counter == ZB_SECUR_NWK_COUNTER_LIMIT)
    {
      TRACE_MSG(TRACE_SECUR3, "time to switch nwk key", (FMT__0));
      zb_get_out_buf_delayed(zb_secur_switch_nwk_key_br);
    }
#endif
  }
  else
  {
    /* fill aux->frame_counter using APS DATA security material */
    aux->secur_control = ZB_APS_DATA_STD_SECUR_CONTROL;
    TRACE_MSG(TRACE_SECUR3, "aps aux hdr - app key crypt", (FMT__0));
  }
}


zb_ushort_t zb_aps_secur_aux_size(zb_uint8_t *p)
{
  zb_aps_nwk_aux_frame_hdr_t *aux = (zb_aps_nwk_aux_frame_hdr_t *)p;

  switch (ZB_SECUR_AUX_HDR_GET_KEY_TYPE(aux->secur_control))
  {
    case ZB_SECUR_DATA_KEY:
      return sizeof(zb_aps_data_aux_frame_hdr_t);
    case ZB_SECUR_NWK_KEY:
      return sizeof(zb_aps_nwk_aux_frame_hdr_t);
    default:
      return (zb_ushort_t)-1;
  }
}


zb_bool_t zb_aps_command_add_secur(zb_buf_t *buf, zb_uint8_t command_id)
{
  zb_bool_t secure = ZB_TRUE;
  zb_ushort_t hdr_size = sizeof(zb_aps_command_header_t);
  zb_aps_command_header_t *hdr;

  /* fc | counter | aux | command_id */
  if (0)
  {
    /* first try APS data key */
    hdr_size += sizeof(zb_aps_data_aux_frame_hdr_t);
  }
  if (hdr_size == sizeof(zb_aps_command_header_t)
      && !ZG->nwk.nib.secure_all_frames)
  {
    /* if could not find APS data key and no mandatory NWK security, use
     * NWK key encryption of APS frame */
    hdr_size += sizeof(zb_aps_nwk_aux_frame_hdr_t);
  }
  ZB_BUF_ALLOC_LEFT(buf, hdr_size, hdr);
  hdr->fc = 0;
  /* command id is just after aux hdr - at hdr end*/
  *((zb_uint8_t *)hdr + hdr_size - 1) = command_id;
  if (hdr_size != sizeof(zb_aps_command_header_t))
  {
    ZB_APS_FC_SET_SECURITY(hdr->fc, 1);
    buf->u.hdr.encrypt_type = ZB_SECUR_APS_ENCR;
    secure = 0;             /* secured at APS - does not secure at NWK */
    zb_secur_aps_aux_hdr_fill(
      /* aux hdr is before command id */
      &hdr->aps_command_id,
      (hdr_size == sizeof(zb_aps_command_header_t) + sizeof(zb_aps_nwk_aux_frame_hdr_t)));
  }
  return secure;
}


void zb_aps_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst)
{
  zb_uint8_t *aps_hdr;
  zb_uint8_t *payload;
  zb_secur_ccm_nonce_t nonce;
  zb_uint8_t *dp;
  zb_uint8_t *key;
  zb_ushort_t hdrs_size;
  zb_aps_nwk_aux_frame_hdr_t *aux;

  {
    zb_nwk_hdr_t *nwk_hdr = (zb_nwk_hdr_t *)(ZB_BUF_BEGIN(src) + mac_hdr_size);
    aps_hdr = (zb_uint8_t *)nwk_hdr + ZB_NWK_HDR_SIZE(nwk_hdr->frame_control);
  }

  aux = (zb_aps_nwk_aux_frame_hdr_t *)(aps_hdr + ZB_APS_HDR_SIZE(*aps_hdr));

  if (ZB_SECUR_AUX_HDR_GET_KEY_TYPE(aux->secur_control) == ZB_SECUR_DATA_KEY)
  {
    payload = (zb_uint8_t *)aux + sizeof(zb_aps_data_aux_frame_hdr_t);
    /* get src and dst address from APS header, get data key */
    key = 0;                    /* aps data key */
  }
  else
  {
    /* nwk key */
    payload = (zb_uint8_t *)aux + sizeof(zb_aps_nwk_aux_frame_hdr_t);
    key = ZG->nwk.nib.secur_material_set[ZG->nwk.nib.active_secur_material_i].key;
  }
  /* fill nonce - see 4.5.2.2 */

  nonce.frame_counter = aux->frame_counter;
  nonce.secur_control = aux->secur_control;
  ZB_IEEE_ADDR_COPY(nonce.source_address, ZB_PIB_EXTENDED_ADDRESS());

  hdrs_size = payload - ZB_BUF_BEGIN(src);

  /* Secure  */
  (void)zb_ccm_encrypt_n_auth(key,
                              (zb_uint8_t *)&nonce,
                              (zb_uint8_t *)aps_hdr, (payload - aps_hdr),
                              (zb_uint8_t *)payload, (ZB_BUF_LEN(src) - hdrs_size),
                              dst);
  ZB_BUF_ALLOC_LEFT(dst, (aps_hdr - ZB_BUF_BEGIN(src)), dp);
  /* copy headers */
  ZB_MEMCPY(dp, ZB_BUF_BEGIN(src), (aps_hdr - ZB_BUF_BEGIN(src)));

  /* clear security level - see 4.4.1.1/11 */
  aux = (zb_aps_nwk_aux_frame_hdr_t *)(ZB_BUF_BEGIN(dst) + ((zb_uint8_t*)aux - ZB_BUF_BEGIN(src)));
  if (ZB_SECUR_AUX_HDR_GET_KEY_TYPE(aux->secur_control) == ZB_SECUR_DATA_KEY)
  {
    aux->secur_control = ZB_APS_DATA_STD_SECUR_CONTROL_ZEROED_LEVEL;
  }
  else
  {
    aux->secur_control = ZB_APS_NWK_STD_SECUR_CONTROL_ZEROED_LEVEL;
  }

  TRACE_MSG(TRACE_SECUR3, "secured aps frm %p[%hd] -> %p hdrs_size %hd fcnt %lx", (FMT__P_H_P_H_L,
                           src, ZB_BUF_LEN(src), dst, hdrs_size, aux->frame_counter));

}


zb_ret_t zb_aps_unsecure_frame(zb_buf_t *buf)
{
  zb_ret_t ret = RET_OK;
  zb_uint8_t *aps_hdr;
  zb_aps_nwk_aux_frame_hdr_t *aux;
  zb_ushort_t a_size;
  zb_uint8_t *key;
  zb_uint8_t *payload;
  zb_address_ieee_ref_t addr_ref;

  {
    zb_apsde_data_indication_t * ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
    ret = zb_address_by_short(ind->src_addr, ZB_FALSE, ZB_FALSE, &addr_ref);
    if (ret != RET_OK)
    {
      TRACE_MSG(TRACE_SECUR3, "can't get addr %d", (FMT__D, ind->src_addr));
    }
  }
  if (ret == RET_OK)
  {
    aps_hdr = ZB_BUF_BEGIN(buf);
    aux = (zb_aps_nwk_aux_frame_hdr_t *)(aps_hdr + ZB_APS_HDR_SIZE(*aps_hdr));
    a_size = ((zb_uint8_t *)aux - aps_hdr);

    if (ZB_SECUR_AUX_HDR_GET_KEY_TYPE(aux->secur_control) == ZB_SECUR_DATA_KEY)
    {
      aux->secur_control = ZB_APS_DATA_STD_SECUR_CONTROL;
      payload = (zb_uint8_t *)aux + sizeof(zb_aps_data_aux_frame_hdr_t);

      /* TODO: implement Data key */

      /* obtain secure material. */

      /* check frame counters */

      key = 0;
      a_size += sizeof(zb_aps_data_aux_frame_hdr_t);
    }
    else
    {
      /* NWK key */

      zb_neighbor_tbl_ent_t *nbe;

      /* Update security level which was zeroed before send */
      aux->secur_control = ZB_APS_NWK_STD_SECUR_CONTROL;
      payload = (zb_uint8_t *)aux + sizeof(zb_aps_nwk_aux_frame_hdr_t);
      key = secur_nwk_key_by_seq(aux->key_seq_number);
      a_size += sizeof(zb_aps_nwk_aux_frame_hdr_t);

      /* Get neighbor table entry by source address. */
      ret = zb_nwk_neighbor_get(addr_ref, ZB_TRUE, &nbe);

      if (ret == RET_OK)
      {
        if (key && nbe->key_seq_number != aux->key_seq_number)
        {
          /* Peer now use another nwk key */
          nbe->incoming_frame_counter = 0;
          nbe->key_seq_number = aux->key_seq_number;
          TRACE_MSG(TRACE_SECUR3, "peer switched key", (FMT__0));
        }
        {
          /* Check NWK FrameCounter */
          zb_uint32_t frame_counter;
          ZB_LETOH32(&frame_counter, &aux->frame_counter);
          if (nbe->incoming_frame_counter > frame_counter
              || nbe->incoming_frame_counter == (zb_uint32_t)~0)
          {
            ret = RET_ERROR;
            TRACE_MSG(TRACE_SECUR3, "frm cnt %ld->%ld shift back", (FMT__L_L,
                                     nbe->incoming_frame_counter, frame_counter));
          }
          else
          {
            nbe->incoming_frame_counter = frame_counter;
          }
        }
      }
      else
      {
        TRACE_MSG(TRACE_SECUR3, "can't get neighbor", (FMT__0));
      }
    }
  }

  if (ret == RET_OK && !key)
  {
    /* set 'frame security failed' */
    ret = RET_ERROR;
    TRACE_MSG(TRACE_SECUR3, "no key by seq %hd", (FMT__H, aux->key_seq_number));
  }

  if (ret == RET_OK)
  {
    /* decrypt */
    zb_secur_ccm_nonce_t nonce;

    zb_address_ieee_by_ref(nonce.source_address, addr_ref);
    nonce.frame_counter = aux->frame_counter;
    nonce.secur_control = aux->secur_control;

    ret = zb_ccm_decrypt_n_auth_stdsecur(key,
                                         (zb_uint8_t *)&nonce,
                                         buf, a_size,
                                         (ZB_BUF_BEGIN(buf) + ZB_BUF_LEN(buf)) - (aps_hdr + a_size));
    if (ret == RET_OK)
    {
      /* Remove MIC */
      TRACE_MSG(TRACE_SECUR3, "unsecured frm %p[%hd] ok", (FMT__P_H, buf, ZB_BUF_LEN(buf)));
    }
    else
    {
      TRACE_MSG(TRACE_SECUR3, "unsecure failed", (FMT__0));
    }
  }
  return ret;
}

#endif  /* APS_FRAME_SECURITY */

/*! @} */

#endif  /* ZB_SECURITY */
