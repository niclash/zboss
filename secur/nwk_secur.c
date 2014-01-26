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
PURPOSE: NWK security routines
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_secur.h"

#include "zb_bank_common.h"
#ifdef ZB_SECURITY

/*! \addtogroup ZB_SECUR */
/*! @{ */

static void zb_secur_nwk_status(zb_uint8_t param, zb_uint16_t addr_short, zb_uint8_t status);

zb_ret_t zb_nwk_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst)
{
  zb_ret_t ret = RET_OK;
  zb_secur_ccm_nonce_t nonce;
  zb_nwk_hdr_t *nwhdr = (zb_nwk_hdr_t *)(ZB_BUF_BEGIN(src) + mac_hdr_size);
  zb_ushort_t hdr_size = ZB_NWK_HDR_SIZE(nwhdr->frame_control);
  zb_nwk_aux_frame_hdr_t *aux;

  ZB_CHK_ARR(ZB_BUF_BEGIN(src), ZB_BUF_LEN(src));

  aux = (zb_nwk_aux_frame_hdr_t *)((zb_uint8_t *)nwhdr + hdr_size - sizeof(*aux));

  /* fill aux header - see 4.5.1 */
  aux->secur_control = ZB_NWK_STD_SECUR_CONTROL;
#ifdef ZB_ROUTER_ROLE
  if (src->u.hdr.use_same_key
      && aux->key_seq_number != ZG->nwk.nib.active_key_seq_number)
  {
    TRACE_MSG(TRACE_SECUR3, "use same key number (%hd) and counter %hd", (FMT__H_H, aux->key_seq_number, ZG->nwk.nib.prev_outgoing_frame_counter));
    ZB_HTOLE32(&aux->frame_counter, &ZG->nwk.nib.prev_outgoing_frame_counter);
    ZG->nwk.nib.prev_outgoing_frame_counter++;
  }
  else
#endif
  {
    ZB_HTOLE32(&aux->frame_counter, &ZG->nwk.nib.outgoing_frame_counter);
    ZG->nwk.nib.outgoing_frame_counter++;
    aux->key_seq_number = ZG->nwk.nib.active_key_seq_number;
    TRACE_MSG(TRACE_SECUR3, "use active key number - %hd", (FMT__H, aux->key_seq_number));
  }
  ZB_IEEE_ADDR_COPY(aux->source_address, ZB_PIB_EXTENDED_ADDRESS());

  /* fill nonce - see 4.5.2.2 */
  ZB_IEEE_ADDR_COPY(nonce.source_address, ZB_PIB_EXTENDED_ADDRESS());
  nonce.frame_counter = aux->frame_counter;
  nonce.secur_control = aux->secur_control;
  TRACE_MSG(TRACE_SECUR2, "secure nwk frm %p[%hd - %hd] hdr_size %hd fcnt %lx",
            (FMT__P_H_H_H_L, src, ZB_BUF_LEN(src), mac_hdr_size, hdr_size, aux->frame_counter));

  {
    zb_uint8_t *key = secur_nwk_key_by_seq(aux->key_seq_number);
    if (!key)
    {
      TRACE_MSG(TRACE_ERROR, "Can't get key by seq# %hd", (FMT__H, aux->key_seq_number));
      ret = RET_ERROR;
    }
    else
    {
      /* Secure. zb_ccm_encrypt_n_auth() allocs space for a and m, but not mhr.  */
      ret = zb_ccm_encrypt_n_auth(key,
                                  (zb_uint8_t *)&nonce,
                                  (zb_uint8_t *)nwhdr, hdr_size,
                                  (zb_uint8_t *)nwhdr + hdr_size, ZB_BUF_LEN(src) - hdr_size - mac_hdr_size,
                                  dst);
    }
  }
  if (ret == RET_OK)
  {
    {
      zb_uint8_t *dp;
      ZB_BUF_ALLOC_LEFT(dst, mac_hdr_size, dp);
      ZB_MEMCPY(dp, ZB_BUF_BEGIN(src), mac_hdr_size);
    }

    /* clear security level - see 4.3.1.1/8 */
    aux = (zb_nwk_aux_frame_hdr_t *)(ZB_BUF_BEGIN(dst) + ((zb_uint8_t*)aux - ZB_BUF_BEGIN(src)));
    aux->secur_control = ZB_NWK_STD_SECUR_CONTROL_ZEROED_LEVEL;

    TRACE_MSG(TRACE_SECUR2, "secured frm %p[%hd] %d fcnt %ld", (FMT__P_H_D_L,
                                                                dst, ZB_BUF_LEN(dst), ret, ZG->nwk.nib.outgoing_frame_counter));

#ifdef ZB_COORDINATOR_ROLE
    /* If counter is near limit, switch key */
    if (ZG->nwk.handle.is_tc && ZG->nwk.nib.outgoing_frame_counter == ZB_SECUR_NWK_COUNTER_LIMIT)
    {
      TRACE_MSG(TRACE_SECUR3, "time to switch nwk key", (FMT__0));
      zb_get_out_buf_delayed(zb_secur_switch_nwk_key_br);
    }
#endif
  }
  return ret;
}


zb_ret_t zb_nwk_unsecure_frame(zb_uint8_t param, zb_mac_mhr_t *mhr, zb_uint8_t mhr_len)
{
  zb_buf_t *nsdu = ZB_BUF_FROM_REF(param);
  zb_neighbor_tbl_ent_t *nbe;
  zb_uint8_t *key;
  zb_address_ieee_ref_t addr_ref;
  zb_ret_t ret = RET_OK;
  zb_ushort_t hdr_size;
  zb_nwk_aux_frame_hdr_t *aux;
  zb_uint16_t nwk_addr;

  if (!ZG->aps.authenticated)
  {
    TRACE_MSG(TRACE_SECUR3, "Not authenticated - drop this frame", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
    return RET_ERROR;
  }

  {
    zb_nwk_hdr_t *nwk_hdr = (zb_nwk_hdr_t *)ZB_BUF_BEGIN(nsdu);

    hdr_size = ZB_NWK_HDR_SIZE(nwk_hdr->frame_control);
    aux = (zb_nwk_aux_frame_hdr_t *)((zb_uint8_t *)nwk_hdr + hdr_size - sizeof(zb_nwk_aux_frame_hdr_t));

    /*
      Have both long and short address - upadte address translation table - deal
      with NWK encrypted Device_annce when we have no both addresses yet.

      Can't use src_addr from the nwk header: frame can be not from its originator (and
      re-encrypted). Must use address got from MHR.
    */
    if (ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control) == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
    {
      ret = zb_address_update(aux->source_address, mhr->src_addr.addr_short, ZB_FALSE, &addr_ref);
    }
    if (ret == RET_OK)
    {
      /* Get neighbor table entry.
         It is possible to have no dev in the neighbor.
         Create entry in the neighbor only if this is direct transmit.
      */
      ZB_LETOH16(&nwk_addr, &nwk_hdr->src_addr);
      TRACE_MSG(TRACE_SECUR2, "nwk addr: 0x%x, mac addr 0x%x", (FMT__D_D, nwk_addr, mhr->src_addr.addr_short));
      ret = zb_nwk_neighbor_get(addr_ref, (zb_bool_t)(nwk_hdr->src_addr == mhr->src_addr.addr_short), &nbe);
    }
  }
  if (ret == RET_OK)
  {
    key = secur_nwk_key_by_seq(aux->key_seq_number);

    /* If this is child, set its state to 'not authenticated' */
    if (nbe->relationship == ZB_NWK_RELATIONSHIP_CHILD)
    {
      nbe->relationship = ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD;
    }

    if (nbe->key_seq_number != aux->key_seq_number)
    {
      nbe->incoming_frame_counter = 0;
      nbe->key_seq_number = aux->key_seq_number;
      TRACE_MSG(TRACE_SECUR3, "peer switched key to seq %hd", (FMT__H, nbe->key_seq_number));
    }

    if (!key)
    {
      /* set 'frame security failed' */
      ret = RET_ERROR;
      TRACE_MSG(TRACE_SECUR3, "no key by seq %hd", (FMT__H, aux->key_seq_number));
      zb_secur_nwk_status(param, mhr->src_addr.addr_short, ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER);
    }
    else
    {
      /* Check FrameCount */

      zb_uint32_t frame_counter;
      ZB_LETOH32(&frame_counter, &aux->frame_counter);
      if (nbe->incoming_frame_counter > frame_counter
          || nbe->incoming_frame_counter == (zb_uint32_t)~0)
      {
        ret = RET_ERROR;
        TRACE_MSG(TRACE_SECUR1, "frm cnt %ld->%ld shift back - indicate status", (FMT__L_L,
                                 nbe->incoming_frame_counter, frame_counter));
        zb_secur_nwk_status(param, mhr->src_addr.addr_short, ZB_NWK_COMMAND_STATUS_BAD_FRAME_COUNTER);
      }
      else
      {
        nbe->incoming_frame_counter = frame_counter;
      }
    }
  }
  else
  {
    TRACE_MSG(TRACE_SECUR1, "can't get neighbor - indicate nwk status", (FMT__0));
    zb_secur_nwk_status(param, mhr->src_addr.addr_short, ZB_NWK_COMMAND_STATUS_FRAME_SECURITY_FAILED);
  }

  if (ret == RET_OK)
  {
    zb_ushort_t payload_size = ZB_BUF_LEN(nsdu) - hdr_size;
    /* Check that payload len >= ZB_CCM_M */
    if (ret == RET_OK
        && payload_size < ZB_CCM_M)
    {
      ret = RET_ERROR;
      TRACE_MSG(TRACE_SECUR3, "too short nsdu: %hd hdr %hd - indicate status", (FMT__H_H, ZB_BUF_LEN(nsdu), hdr_size));
      zb_secur_nwk_status(param, mhr->src_addr.addr_short, ZB_NWK_COMMAND_STATUS_FRAME_SECURITY_FAILED);
    }

    if (ret == RET_OK)
    {
      /* decrypt */
      zb_secur_ccm_nonce_t nonce;

      aux->secur_control = ZB_NWK_STD_SECUR_CONTROL;
      ZB_IEEE_ADDR_COPY(nonce.source_address, aux->source_address);
      nonce.frame_counter = aux->frame_counter;
      nonce.secur_control = aux->secur_control;

      ret = zb_ccm_decrypt_n_auth_stdsecur(key,
                                           (zb_uint8_t *)&nonce,
                                           nsdu, hdr_size,
                                           payload_size);
    }
    if (ret == RET_OK)
    {
      /* re-insert MAC MHR before NWK header. Use key as temporary variable! */
      /* Dirty hack: satisfy nwk_get_mac_source_addr */
      ZB_BUF_ALLOC_LEFT(nsdu, mhr_len+1, key);
      zb_mac_fill_mhr(key+1, mhr);
      nsdu->u.hdr.mac_hdr_offset = ZB_BUF_OFFSET(nsdu)+1;
      ZB_BUF_CUT_LEFT(nsdu, mhr_len+1, key);

      /* if this is child and authentication ok, set its state to 'authenticated' */
      if (nbe->relationship == ZB_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD)
      {
        nbe->relationship = ZB_NWK_RELATIONSHIP_CHILD;
      }

      /* Remove MIC */
      /*(nsdu)->u.hdr.len -= ZB_CCM_M; It's already done in decrypt procedure */

      TRACE_MSG(TRACE_SECUR3, "unsecured frm %p[%hd] ok", (FMT__P_H, nsdu, ZB_BUF_LEN(nsdu)));
    }
    else
    {
      TRACE_MSG(TRACE_SECUR3, "unsecure failed %d - indicate nwk status", (FMT__D, ret));
      zb_secur_nwk_status(param, mhr->src_addr.addr_short, ZB_NWK_COMMAND_STATUS_FRAME_SECURITY_FAILED);
    }
  }
  if (ret == RET_OK
      /* exclude key switch back. More precise checks done in secur_nwk_key_switch. */
      && ((aux->key_seq_number - ZG->nwk.nib.active_key_seq_number)!=0)
	  &&((zb_uint8_t)(aux->key_seq_number - ZG->nwk.nib.active_key_seq_number) < (zb_uint8_t)127))
  {
    /*
      Implicit key switch: according to 4.3.1.2 Security Processing of Incoming
      Frames

      "7 If the sequence number of the received frame belongs to a newer entry in the
      nwkSecurityMaterialSet, set the nwkActiveKeySeqNumber to the received
      sequence number."
     */
    TRACE_MSG(TRACE_SECUR1, "implicit key switch to %hd", (FMT__H, aux->key_seq_number));
    secur_nwk_key_switch(aux->key_seq_number);
  }
  return ret;
}


static void zb_secur_nwk_status(zb_uint8_t param, zb_uint16_t addr_short, zb_uint8_t status)
{
  zb_nlme_status_indication_t *cmd = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_status_indication_t);
  cmd->status = (zb_nwk_command_status_t)status;
  cmd->network_addr = addr_short;
  ZB_SCHEDULE_CALLBACK(zb_nlme_status_indication, param);
}

#endif  /* ZB_SECURITY */

/*! @} */
