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
PURPOSE: Roitines specific mac data transfer for coordinator/router
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_secur.h"
#include "zb_zdo.h"

#include "zb_bank_2.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

zb_ret_t pending_queue_is_empty()
{
  zb_int8_t i;
  zb_ret_t ret = RET_ERROR;

  for (i = 0; i < ZB_MAC_PENDING_QUEUE_SIZE; i++)
  {
    if (MAC_CTX().pending_data_queue[i].pending_data !=NULL)
    {
      ret = RET_OK;
      break;
    }
  }
  return ret;
}

#ifdef ZB_COORDINATOR_ROLE

void zb_accept_data_request_cmd(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_MAC2, ">> zb_accept_data_request_cmd %hd", (FMT__H, param));
  {
    /* process request immediately*/
    MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
    /* Q: zb_handle_data_request_cmd called from here only.
       Why not insert its code here?

       A: Is it really necessary? I think compiler
       will inline it. And also, it's ready to become a callback
    */
    ret = zb_handle_data_request_cmd();
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_accept_data_request_cmd, ret: %d", (FMT__H, ret));
}



static zb_int8_t check_pending_data(zb_mac_mhr_t *mhr, zb_uint8_t index) ZB_SDCC_REENTRANT
{
  zb_int8_t i;

  zb_uint8_t data_found = 0;
  zb_uint8_t src_addr_mode;

  src_addr_mode = ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr->frame_control);

  TRACE_MSG(TRACE_MAC2, "check_pending_data: src_addr_mode %hd", (FMT__H, src_addr_mode));

  for (i = index; i < ZB_MAC_PENDING_QUEUE_SIZE; i++)
  {
    TRACE_MSG(TRACE_MAC2, "i %hd pending_data %p dst_addr_mode %hd",
              (FMT__H_P_H, i, MAC_CTX().pending_data_queue[i].pending_data,
               MAC_CTX().pending_data_queue[i].dst_addr_mode));

    if (MAC_CTX().pending_data_queue[i].pending_data != NULL &&
        src_addr_mode == MAC_CTX().pending_data_queue[i].dst_addr_mode)
    {
      /* TODO: what should I do if addr_mode is different? */
      if (src_addr_mode == ZB_ADDR_64BIT_DEV)
      {
        data_found = ZB_IEEE_ADDR_CMP(MAC_CTX().pending_data_queue[i].dst_addr.addr_long,
                                      mhr->src_addr.addr_long);
        TRACE_MSG(TRACE_MAC2, "data_found %hd pending dst " TRACE_FORMAT_64 " src " TRACE_FORMAT_64,
                  (FMT__H_A_A, data_found,
                   TRACE_ARG_64(MAC_CTX().pending_data_queue[i].dst_addr.addr_long),
                   TRACE_ARG_64(mhr->src_addr.addr_long)));
      }
      else if (src_addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST)
      {
        data_found = (mhr->src_addr.addr_short == MAC_CTX().pending_data_queue[i].dst_addr.addr_short);
      }
    }
    if (data_found)
    {
      break;
    }
  }
  return data_found ? i : -1;
}

#ifndef ZB_MANUAL_ACK
/*
  Fills frame header with zero payload length. mhr_req contains mac
  header of the incoming request.
*/
static zb_ret_t fill_empty_frame(zb_mac_mhr_t *mhr_req, zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_mac_mhr_t mhr;
  zb_uint_t mhr_len;
  zb_uint8_t *ptr;

  TRACE_MSG(TRACE_MAC3, ">> fill_empty_frame mhr_req %p, buf%p", (FMT__P_P, mhr_req, buf));

  mhr_len = zb_mac_calculate_mhr_length(ZB_FCF_GET_DST_ADDRESSING_MODE(mhr_req->frame_control),
                                        ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr_req->frame_control), 1);
  /* packet length == mhr_len */
  ZB_BUF_INITIAL_ALLOC(buf, mhr_len, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, mhr_len);
/* Fill Frame Controll then call zb_mac_fill_mhr()
   mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  /* TODO: optimize FCF fill */
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_DATA);
  /* security enable is 0 */
  /* frame pending is 0 */
  /* ack request is 0 */
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_FCF_GET_SRC_ADDRESSING_MODE(mhr_req->frame_control));
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_FCF_GET_DST_ADDRESSING_MODE(mhr_req->frame_control));

  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();

  mhr.dst_pan_id = MAC_PIB().mac_pan_id;
  ZB_MEMCPY(&mhr.dst_addr, &mhr_req->src_addr, sizeof(union zb_addr_u));
  mhr.src_pan_id = 0;
  ZB_MEMCPY(&mhr.src_addr, &mhr_req->dst_addr, sizeof(union zb_addr_u));
  zb_mac_fill_mhr(ptr, &mhr);

  TRACE_MSG(TRACE_MAC3, "<< fill_empty_frame, ret %i", (FMT__D, ret));
  return ret;
}
#endif /* ZB_MANUAL_ACK */

/*
  Handles data request command, coordinator side. Finds pending data
  for the device if any and sends it. spec 7.3.4 Data request
  command, 7.5.6.3 Extracting pending data from a coordinator.
  return RET_OK, RET_ERROR, RET_BLOCKED
*/
zb_ret_t zb_handle_data_request_cmd() ZB_SDCC_REENTRANT
{
  zb_ret_t ret = RET_OK;
  zb_int8_t data_found_index;
  zb_int8_t data_found = 0;
  zb_uint8_t *fcf = NULL;
  zb_uint8_t *cmd_ptr = NULL;
  zb_uint8_t mhr_pend_len = 0;
  /* TODO: get rid of 2 mhrs, try partial mhr parse */
  zb_mac_mhr_t mhr_req;
  zb_mac_mhr_t mhr_pend;


  TRACE_MSG(TRACE_MAC2, ">> zb_handle_data_request_cmd, pending_buf %p",
            (FMT__P, MAC_CTX().pending_buf));

  if (!MAC_CTX().pending_buf)
  {
    /* this situation is possible if this command is resent by remote */
    TRACE_MSG(TRACE_MAC2, "<< zb_handle_data_request_cmd ret (invalid params) %i", (FMT__D, ret));
    return ret;
  }

  {
    zb_mac_data_req_ctx_t *data_req_ctx;

    TRACE_MSG(TRACE_MAC3, "idle", (FMT__0));
    cmd_ptr = ZB_BUF_BEGIN(MAC_CTX().pending_buf);
    zb_parse_mhr(&mhr_req, cmd_ptr);
    data_found_index = check_pending_data(&mhr_req, 0);
    /* save context information, because pending_buf can be reused
     * and request mhr will be lost */
    data_req_ctx = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_data_req_ctx_t);
    ZB_MEMCPY(&data_req_ctx->mhr, &mhr_req, sizeof(zb_mac_mhr_t));
    data_req_ctx->data_index = data_found_index;
  }

  TRACE_MSG(TRACE_MAC3, "index %hd", (FMT__H, data_found_index));
  if (data_found_index >= 0)
  {
    TRACE_MSG(TRACE_MAC3, "pending dat %p", (FMT__P, MAC_CTX().pending_data_queue[data_found_index].pending_data));
    fcf = ZB_MAC_GET_FCF_PTR(ZB_BUF_BEGIN(MAC_CTX().pending_data_queue[data_found_index].pending_data));
    data_found = 1;

    /* get MHR stored in pending_data, need to analyse its frame type*/
    cmd_ptr = ZB_BUF_BEGIN(MAC_CTX().pending_data_queue[data_found_index].pending_data);
    mhr_pend_len = zb_parse_mhr(&mhr_pend, cmd_ptr);
  }
  TRACE_MSG(TRACE_MAC2, "data_found_index %hd, fcf %p, data_found %hd", (FMT__H_P_H, data_found_index, fcf, data_found));
  ZB_SET_MAC_STATUS(MAC_SUCCESS);



#ifdef ZB_MANUAL_ACK
  /* 7.5.6.3 Extracting pending data from a coordinator.
     check if there is pending data for the device and send Ack with
     set/cleared pending bit */
  ret = zb_mac_send_ack(mhr_req.seq_number, data_found);
  if (ret == RET_OK)
  {
    ret = zb_check_cmd_tx_status();
  }

#else /* ZB_MANUAL_ACK */

  /* if ack is sent automatically and it was sent with pending data
   * bit == 1, we must send data frame with zero payload */
  if (!data_found && !pending_queue_is_empty())
  {
    fill_empty_frame(&mhr_req, MAC_CTX().pending_buf);
    fcf = ZB_MAC_GET_FCF_PTR(ZB_BUF_BEGIN(MAC_CTX().pending_buf));

    /* FIXME: Rewrite empty frame sending routine, now send will crush */
#if 0
    data_found = 1; /* set data found just to send empty data frame */
#endif
  }
#endif /* ZB_MANUAL_ACK */
#ifdef ZB_MANUAL_ACK
send_data_resp:
#endif
  if (ret == RET_OK && data_found)
  {
    /* pending data is found, send it to the device */

    TRACE_MSG(TRACE_MAC3, "will send data, frame type %i", (FMT__D, ZB_FCF_GET_FRAME_TYPE(fcf)));

    switch (ZB_FCF_GET_FRAME_TYPE(fcf))
    {
      case MAC_FRAME_COMMAND:
      case MAC_FRAME_DATA: /* data frame is sent */
      {
        /* data and command frames are sent using the same call */
#ifdef ZB_SECURITY
        ret = zb_mac_check_security(MAC_CTX().pending_data_queue[data_found_index].pending_data);
#endif
        if (ret == RET_OK)
        {
          /* TODO: exclude 3 mhr in the same function */
          zb_mac_mhr_t mhr;
          zb_buf_t *b_tmp;

          ZB_TX_TOTAL_INC();
#ifdef ZB_SECURITY
          if (MAC_CTX().pending_data_queue[data_found_index].pending_data->u.hdr.encrypt_type != ZB_SECUR_NO_ENCR)
          {
            b_tmp = MAC_CTX().encryption_buf;
            /* we need to add fcs to not not encrypted buffer, we need it later to reencrypt and resend */
            MAC_ADD_FCS(MAC_CTX().pending_data_queue[data_found_index].pending_data);
          }
          else
#endif
          {
            b_tmp = MAC_CTX().pending_data_queue[data_found_index].pending_data;
          }

          if (check_pending_data(&mhr_req, data_found_index+1)!=-1)
          {
            ZB_MEMCPY(&mhr.frame_control, ZB_BUF_BEGIN(b_tmp), 2);
            ZB_FCF_SET_FRAME_PENDING_BIT(mhr.frame_control, 1);
            ZB_MEMCPY(ZB_BUF_BEGIN(b_tmp), &mhr.frame_control, 2);
          }
          MAC_ADD_FCS(b_tmp);
          ret = ZB_TRANS_SEND_COMMAND(mhr_pend_len, b_tmp);
          if (ZB_FCF_GET_ACK_REQUEST_BIT(fcf))
          {
            MAC_CTX().ack_dsn = mhr_pend.seq_number;
          }
        }
        break;
      }
      default:
        /* make correct tx call */
        ZB_ASSERT(0);
    }
#ifdef ZB_MANUAL_ACK
    if (ret == RET_OK && fcf)
    {
      TRACE_MSG(TRACE_MAC2, "ack req bit %i", (FMT__D, ZB_FCF_GET_ACK_REQUEST_BIT(fcf)));
      if (ZB_FCF_GET_ACK_REQUEST_BIT(fcf))
      {
        if (ret == RET_OK)
        {
          /* ack_dsn was set in MAC_FRAME_COMMAND case */
          ret = zb_mac_check_ack();
          if (ret == RET_PENDING)
          {
            /* retry data send */
            TRACE_MSG(TRACE_MAC2, "ack is not received, try again", (FMT__0));
            ret = RET_OK;
            goto send_data_resp;
          }
        }
      }
    }
#endif
  }
  if (data_found)
  {
    MAC_CTX().mac_storage.mac_data_req.fcf = fcf;
    MAC_CTX().mac_storage.mac_data_req.cmd_ptr = cmd_ptr;
    MAC_CTX().mac_storage.mac_data_req.mhr_pend_len = mhr_pend_len;
    MAC_CTX().mac_storage.mac_data_req.data_found_index = data_found_index;
    MAC_CTX().mac_storage.mac_data_req.mhr_pend = mhr_pend;
/*_continue routine scheduled for the previously sent data packet
  ret = ZB_TRANS_SEND_COMMAND(mhr_pend_len, b_tmp); TX waiting callback could
  be scheduled before or after sending, because there's no option for
  scheduler_loop, which calls tx-waiting callback, to be called. */
    if (data_found)MAC_CTX().tx_wait_cb = zb_handle_data_request_cmd_continue;
    TRACE_MSG(TRACE_MAC2, "<< zb_handle_data_request_cmd ret, continue scheduled", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_MAC2, "<< zb_handle_data_request_cmd ret", (FMT__0));
    if (MAC_CTX().pending_buf)
    {
      zb_free_buf(MAC_CTX().pending_buf);
      MAC_CTX().pending_buf = NULL;
    }
  }
  return ret;
}

void zb_handle_data_request_cmd_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t ret;
  zb_uint8_t data_found  = 1;
  zb_uint8_t *fcf = MAC_CTX().mac_storage.mac_data_req.fcf;
  zb_uint8_t *cmd_ptr = MAC_CTX().mac_storage.mac_data_req.cmd_ptr;
  zb_uint8_t data_found_index = MAC_CTX().mac_storage.mac_data_req.data_found_index;
  zb_uint8_t mhr_pend_len = MAC_CTX().mac_storage.mac_data_req.mhr_pend_len;
  zb_mac_mhr_t mhr_pend;

  TRACE_MSG(TRACE_MAC2, ">> zb_handle_data_request_cmd ret_continue", (FMT__0));
  mhr_pend = MAC_CTX().mac_storage.mac_data_req.mhr_pend;
  ZVUNUSED(param);
  ZB_SKIP_TX_CHK();
  ret = zb_check_cmd_tx_status();
  if (ret != RET_OK && ret != (zb_uint8_t)RET_BLOCKED)
  {
    ZB_TX_FAIL_INC();
  }
  /* we need to send status indication for some commands */
  if (ret != (zb_uint8_t)RET_BLOCKED && data_found && fcf)
  {
    TRACE_MSG(TRACE_MAC2, "frame type %i data_found_index %hd", (FMT__D_H, ZB_FCF_GET_FRAME_TYPE(fcf), data_found_index));
    if (ZB_FCF_GET_FRAME_TYPE(fcf) == MAC_FRAME_COMMAND)
    {
      cmd_ptr += mhr_pend_len;
      TRACE_MSG(TRACE_MAC2, "cmd_ptr %i", (FMT__D, *cmd_ptr));
      /* Warning: If you add some more functions, that need comm_status */
      /* Make sure, that there will be no races and context parameters are safe! */
      /*FIXME: possibly, it's better to make some additional refactoring or just */
      /* add some macro */
      if (*cmd_ptr == MAC_CMD_ASSOCIATION_RESPONSE)
      {
        zb_uint8_t status;
        if (ret == RET_OK)
        {
          status = MAC_SUCCESS;
        }
        else
        {
          status = (ZB_GET_MAC_STATUS() != MAC_SUCCESS) ? ZB_GET_MAC_STATUS() : MAC_INVALID_PARAMETER;
        }
        /* checked only 2nd parameter, coz it used only for zb_send_comm_status */
        TRACE_MSG(TRACE_MAC3, "calling zb_mac_send_comm_status", (FMT__0));
        zb_mac_send_comm_status(MAC_CTX().pending_data_queue[data_found_index].pending_data, status, MAC_CTX().pending_buf);
        MAC_CTX().pending_buf = NULL;
      }
    }

    if (data_found)
    {
      /* clear pending data slot */
      if (ZB_FCF_GET_FRAME_TYPE(&mhr_pend.frame_control) == MAC_FRAME_COMMAND)
      {
        TRACE_MSG(TRACE_MAC3, "mac cmd, free buffer %p", (FMT__P, MAC_CTX().pending_data_queue[data_found_index].pending_data));
        zb_free_buf(MAC_CTX().pending_data_queue[data_found_index].pending_data);
      }
      else
      {
        TRACE_MSG(TRACE_MAC3, "it's data. call confirm", (FMT__0));
        ZB_SCHEDULE_CALLBACK(zb_mcps_data_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_data_queue[data_found_index].pending_data));
      }
      /* MAC_CTX().pending_data_queue[data_found_index].pending_data = NULL; */

      ZB_SCHEDULE_ALARM_CANCEL(zb_mac_pending_data_timeout, data_found_index);
      ZB_CLEAR_PENDING_QUEUE_SLOT(data_found_index);

      if (pending_queue_is_empty())
      {
        TRACE_MSG(TRACE_MAC1, "Clearing pending bit", (FMT__0));
        ZB_CLEAR_PENDING_BIT();
      }
    }
  }

  if (ret != (zb_uint8_t)RET_BLOCKED)
  {
    if (MAC_CTX().pending_buf)
    {
      zb_free_buf(MAC_CTX().pending_buf);
      MAC_CTX().pending_buf = NULL;
    }
  }
  TRACE_MSG(TRACE_MAC2, "<< zb_handle_data_request_cmd_continue", (FMT__0));
}

#endif /* ZB_COORDINATOR_ROLE */

/*! @} */
