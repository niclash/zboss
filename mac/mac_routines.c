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
PURPOSE: Roitines specific to coordinator role
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac_globals.h"
#include "zb_mac.h"
#include "mac_internal.h"



/*! \addtogroup ZB_MAC */
/*! @{ */

#include "zb_bank_2.h"

#ifdef ZB_ROUTER_ROLE

zb_ret_t  zb_realign_pan();

zb_ret_t zb_mac_process_mlme_start() ZB_SDCC_BANKED
{
  zb_ret_t ret = RET_OK;
  zb_mlme_start_req_t *params = NULL;

/*
  MAC spec, 7.1.14.1 MLME-START.request
  1) check macShortAddress, if it is == 0xffff report NO_SHORT_ADDRESS
  (it seems local setting of short address should be checked)
  2) check CoordRealignment, if it is == TRUE, transmit a coordinator
  realignment command frame (7.5.2.3.2.) (Shel we check if we are
  Coordinator????) (*)
  - if command finishes successfully, update BeaconOrder,
  SuperframeOrder, PANId, ChannelPage, and LogicalChannel parameters, as
  described in 7.5.2.3.4: zb_mac_update_superframe_and_pib
  - confirm success: MLME-START.confirm, status == SUCCESS
  3) if CoordRealignment FALSE, update PIB parameters according to
  7.5.2.3.4: zb_mac_update_superframe_and_pib()
  4) if BeaconOrder < 15, pib.macBattLifeExt = req.BatteryLifeExtension;
  if if BeaconOrder == 15, ignore req.BatteryLifeExtension
  5) if req.SecurityLevel > 0  ----------------------------  NOT SUPPORTED YET
  - set Frame Control.Security Enabled = 1
  - use outgoing frame security procedure (7.5.8.2.1)
  - if CoordRealignment == TRUE, use CoordRealignSecurityLevel,
  CoordRealignKeyIdMode,  CoordRealignKey-Source, and CoordRealignKeyInde
  - if BeaconOrder < 15 (beacon-enabled) use BeaconSecurityLevel,
  BeaconKeyIdMode, BeaconKeySource, and BeaconKeyIndex
  - if the beacon frame length > aMaxPHYPacketSize, set status FRAME_TOO_LONG
  6) if BeaconOrder < 15 (beacon enabled)  ----------------------- NOT USED IN ZB
  - if PAN coordinator == TRUE, StartTime = 0
  - if StartTime > 0, calculate the beacon transmission time
  - beacon transmission time = time of receiving the beacon of the coordinator + StartTime
  - if beacon transmission time causes outgoing superframe to overlap
  the incoming superframe, set status SUPERFRAME_OVERLAP
  - if StartTime > 0 and the MLME is not currently tracking the beacon
  of the coordinator, set status TRACKING_OFF

  (*) CoordRealignment == TRUE case
  1) if beacon mode is on, set Frame Control.Frame Pending = 1, do NOT change
  other parameters, send scheduled beacon
  2) send realignment command
  3) if beacon mode is off, send realignment command immediately
  4) if realignment command send fails, set status CHANNEL_ACCESS_FAILURE
  5) update PIB parameters according to 7.5.2.3.4

*/

  TRACE_MSG(TRACE_MAC2, "+zb_mac_process_mlme_start, state %i", (FMT__0));

  params = ZB_GET_BUF_PARAM((zb_buf_t*)MAC_CTX().pending_buf, zb_mlme_start_req_t);
  MAC_CTX().mac_status = MAC_SUCCESS;
  if (params->beacon_order == ZB_TURN_OFF_ORDER)
  {
    params->superframe_order = ZB_TURN_OFF_ORDER;
  }
#if 1 /*seems like realignment never worked */
  if (params->coord_realignment)
  {
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
    ret = zb_realign_pan();
#endif
  }
  if (ret == RET_OK)
  {
    zb_mac_update_superframe_and_pib();
    if (MAC_PIB().mac_beacon_order < ZB_TURN_OFF_ORDER)
    {
      /* if BeaconOrder < 15, pib.macBattLifeExt = req.BatteryLifeExtension */
      MAC_PIB().mac_batt_life_ext = params->battery_life_extension;
    }
  }
  /* for zigbee it is not needed to support beacon mode, so do
   * not support beacon - oriented stuff here */
#endif
  if (ret != RET_BLOCKED)
  {
    if (ret == RET_OK)
    {
      MAC_CTX().pending_buf->u.hdr.status = MAC_SUCCESS;
    }
    else
    {
      MAC_CTX().pending_buf->u.hdr.status = MAC_CTX().mac_status;
    }
    MAC_CTX().dev_type = MAC_DEV_FFD;
    ret = ZB_SCHEDULE_CALLBACK(zb_mlme_start_confirm, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }

  TRACE_MSG(TRACE_MAC2, "<< zb_mac_process_mlme_start", (FMT__0));
  return ret;
}
#endif  /* ZB_ROUTER_ROLE */


void zb_mlme_start_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_mlme_start_req_t *params = NULL;
  zb_mac_status_t status = MAC_SUCCESS;

  TRACE_MSG( TRACE_MAC2, "+zb_mlme_start_request %hd", (FMT__H, param ));
  ZVUNUSED(status);
  params = ZB_GET_BUF_PARAM( ( zb_buf_t * )ZB_BUF_FROM_REF(param), zb_mlme_start_req_t);

  /* mac spec 7.1.14.1 MLME-START.request */
  /* Table 72 - MLME-START.request parameters */
  /* 7.5.1.1 Superframe structure */
  if (params->beacon_order > ZB_TURN_OFF_ORDER ||
      (params->superframe_order > params->beacon_order && params->superframe_order != ZB_TURN_OFF_ORDER))
  {
    /* mac spec 7.1.14.1.3 Effect on receipt */
    TRACE_MSG( TRACE_ERROR, "zb_mlme_start_request : invalid parameter!", (FMT__0 ));
    status = MAC_INVALID_PARAMETER;
  }
  /* mac spec 7.1.14.1.3 Effect on receipt */
  else if (ZB_PIB_SHORT_ADDRESS() == ZB_MAC_SHORT_ADDR_NO_VALUE )
  {
    TRACE_MSG( TRACE_ERROR, "no short address!", (FMT__0));
    status = MAC_NO_SHORT_ADDRESS;
  }

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
  if (status == MAC_SUCCESS)
  {
    /* process request immediately*/
    ZG->mac.mac_ctx.pending_buf = ZB_BUF_FROM_REF(param);
    if (!ZG->nwk.handle.joined_pro)
    {
      zb_mac_process_mlme_start();
    }
  }
#endif
  TRACE_MSG(TRACE_MAC2, "<< zb_mlme_start_request", (FMT__0));
}

/*
  sends beacon request command, mac spec 7.3.7 Beacon request command
  return RET_OK, RET_ERROR
*/
zb_ret_t zb_beacon_request_command()
{
  zb_ret_t ret;
  zb_uint8_t mhr_len;
  zb_uint8_t packet_length;
  zb_uint8_t *ptr = NULL;
  zb_mac_mhr_t mhr;

/*
  7.3.7 Beacon request command
  1. Fill MHR fields
  - set dst pan id = 0xffff
  - set dst addr = 0xffff
  2. Fill FCF
  - set frame pending = 0, ack req = 0, security enabled = 0
  - set dst addr mode to ZB_ADDR_16BIT_DEV_OR_BROADCAST
  - set src addr mode to ZB_ADDR_NO_ADDR
  3. Set command frame id = 0x07 (Beacon request)
*/

  TRACE_MSG(TRACE_MAC2, "+zb_beacon_request_command", (FMT__0));

/* Fill Frame Controll then call zb_mac_fill_mhr() */
/*
  mac spec  7.2.1.1 Frame Control field
  | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/

  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_16BIT_DEV_OR_BROADCAST, ZB_ADDR_NO_ADDR, 0);
  packet_length = mhr_len;
  packet_length += 1;           /* command id */

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);

  ZB_BZERO2(mhr.frame_control);
  /* TODO: optimize FCF fill */
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  /* security_enabled is 0 */
  /* frame pending is 0 */
  /* ack request is 0 */
  /* PAN id compression is 0 */
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);

  /* TODO: set frame version correctly, case Channel Page is omitted */

  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_NO_ADDR);

  /* 7.2.1 General MAC frame format */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();

  mhr.dst_pan_id = ZB_BROADCAST_PAN_ID;
  mhr.dst_addr.addr_short = ZB_MAC_SHORT_ADDR_NO_VALUE;
  /* src pan id and src addr are ignored */

  zb_mac_fill_mhr(ptr, &mhr);

  *(ptr + mhr_len) = MAC_CMD_BEACON_REQUEST;

  MAC_ADD_FCS(MAC_CTX().operation_buf);

  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  TRACE_MSG(TRACE_MAC2, "<< zb_beacon_request_command ret %i", (FMT__D, ret));
  return ret;
}

/* sends MLME-COMM-STATUS.indication to higher level. Parameters for
 * indication are taken from panding_buf.mhr */
zb_ret_t zb_mac_send_comm_status(zb_buf_t *pending_buf, zb_uint8_t mac_status, zb_buf_t *buffer) ZB_SDCC_REENTRANT
{
  zb_mac_mhr_t mhr_pend;
  zb_mlme_comm_status_indication_t *ind_params;
  zb_uint8_t *cmd_ptr;

  TRACE_MSG(TRACE_MAC2, ">> zb_mac_send_comm_status pend %p, status %hi, buf %p", (FMT__P_H_P,
                                                                                   pending_buf, mac_status, buffer));

  cmd_ptr = ZB_BUF_BEGIN(pending_buf);
  zb_parse_mhr(&mhr_pend, cmd_ptr);

  ind_params = ZB_GET_BUF_PARAM(buffer, zb_mlme_comm_status_indication_t);
  ind_params->status = mac_status;
  buffer->u.hdr.status = mac_status;

  ZB_MEMCPY(&ind_params->src_addr, &mhr_pend.src_addr, sizeof(union zb_addr_u));
  ind_params->src_addr_mode = ZB_FCF_GET_SRC_ADDRESSING_MODE(&mhr_pend.frame_control);
  ZB_MEMCPY(&ind_params->dst_addr, &mhr_pend.dst_addr, sizeof(union zb_addr_u));
  ind_params->dst_addr_mode = ZB_FCF_GET_DST_ADDRESSING_MODE(&mhr_pend.frame_control);

  ZB_SCHEDULE_CALLBACK(zb_mlme_comm_status_indication, ZB_REF_FROM_BUF(buffer));
  TRACE_MSG(TRACE_MAC2, "<< zb_mac_send_comm_status pend RET_OK", (FMT__0));
  return RET_OK;
}


#ifdef ZB_MAC_TESTING_MODE
void zb_mlme_purge_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_mlme_purge_request_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_purge_request_t);

  TRACE_MSG(TRACE_MAC2, ">> zb_mcps_purge_request %hd handle 0x%hx",
            (FMT__H_H, param, req->msdu_handle));

  for (i = 0; i < ZB_MAC_PENDING_QUEUE_SIZE; i++)
  {
    TRACE_MSG(TRACE_MAC2, "i %hd pending_data %p handle 0x%hx",
              (FMT__H_P_H, i, MAC_CTX().pending_data_queue[i].pending_data,
               MAC_CTX().pending_data_queue[i].pending_data->u.hdr.handle));

    if (MAC_CTX().pending_data_queue[i].pending_data != NULL &&
        MAC_CTX().pending_data_queue[i].pending_data->u.hdr.handle == req->msdu_handle)
    {
      break;
    }
  }

  if (i == ZB_MAC_PENDING_QUEUE_SIZE)
  {
    TRACE_MSG(TRACE_MAC1, "Purge: handle 0x%hx not found", (FMT__H, req->msdu_handle));
    ZB_BUF_FROM_REF(param)->u.hdr.status = MAC_INVALID_HANDLE;
  }
  else
  {
    ZB_BUF_FROM_REF(param)->u.hdr.status = 0;
    ZB_SCHEDULE_ALARM_CANCEL(zb_mac_pending_data_timeout, i);
    ZB_CLEAR_PENDING_QUEUE_SLOT(i);
    if (pending_queue_is_empty())
    {
      TRACE_MSG(TRACE_MAC1, "Purge: Clearing pending bit", (FMT__0));
      ZB_CLEAR_PENDING_BIT();
    }
  }
  ZB_SCHEDULE_CALLBACK(zb_mlme_purge_confirm, param);

  TRACE_MSG(TRACE_MAC2, "<< zb_mcps_purge_request %hd", (FMT__0));
}
#endif


void zb_mac_pending_data_timeout(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_MAC2, "pend_data_tmo param %hd", (FMT__H, param));

  TRACE_MSG(TRACE_MAC3, "calling zb_mac_send_comm_status", (FMT__0));
  zb_mac_send_comm_status(MAC_CTX().pending_data_queue[param].pending_data, MAC_TRANSACTION_EXPIRED,
                          MAC_CTX().pending_data_queue[param].pending_data);
  /* MAC_CTX().pending_data_queue[param].pending_data = NULL; */
  ZB_CLEAR_PENDING_QUEUE_SLOT(param);
  if (pending_queue_is_empty())
  {
    TRACE_MSG(TRACE_MAC1, "Clearing pending bit", (FMT__0));
    ZB_CLEAR_PENDING_BIT();
  }
}

/*
  Puts data to pending queue. It is used for indirect
  transmission. Coordinator side
  return RET_PENDING on success, RET_ERROR on error
*/
zb_ret_t zb_mac_put_data_to_pending_queue(zb_mac_pending_data_t *pend_data) ZB_SDCC_BANKED
{
  zb_ret_t ret = RET_PENDING;
  zb_uint8_t i;

/*
  7.1.1.1.3 Effect on receipt
  - if indirect transmission is requested, put data frame to pending
  trasaction queue
*/

  TRACE_MSG(TRACE_MAC2, ">> zb_mac_put_data_to_pending_queue ptr %p", (FMT__P, pend_data));
  TRACE_MSG(TRACE_MAC1, "Setting pending bit", (FMT__0));
  ZB_SET_PENDING_BIT();
  /* find free slot */
  for (i = 0; i < ZB_MAC_PENDING_QUEUE_SIZE; i++)
  {
    if (MAC_CTX().pending_data_queue[i].pending_data == NULL)
    {
      break;
    }
  }
  if (i >= ZB_MAC_PENDING_QUEUE_SIZE)
  {
    TRACE_MSG(TRACE_MAC1, "error, TRANSACTION OVERFLOW", (FMT__0));
    ret = RET_ERROR;
    ZB_SET_MAC_STATUS(MAC_TRANSACTION_OVERFLOW);
  }
  else
  {
    ZB_MEMCPY(&MAC_CTX().pending_data_queue[i], pend_data, sizeof(zb_mac_pending_data_t));

    /* timeout value = macTransactionPersistenceTime (in unites),
       unite period = aBaseSuperframeDuration
       Our time quant, which ZB_SCHEDULE_ALARM uses, is beacon interval.
       1 beacon interval(us) = aBaseSuperframeDuration * symbol duration(us)
       Need to schedule alarm for (us):

       mac_transaction_persistence_time * aBaseSuperframeDuration * symbol_duration(us).

       In beacon intervals it will be mac_transaction_persistence_time.

    */
    ZB_SCHEDULE_ALARM(zb_mac_pending_data_timeout, i,
                      ZB_MAC_PIB_TRANSACTION_PERSISTENCE_TIME);
  }

  TRACE_MSG(TRACE_MAC2, "<< zb_mac_put_data_to_pending_queue, ret %i", (FMT__D, ret));
  return ret;
}

void zb_mac_put_request_to_queue (zb_buf_t *request, zb_mac_request_type_e req_type) ZB_CALLBACK
{
  zb_mac_request_t *ent;
  /* just put packet to the out queue and process it later */
  ZB_ASSERT(!ZB_RING_BUFFER_IS_FULL(&ZG->mac.mac_ctx.out));
  ent = ZB_RING_BUFFER_PUT_RESERVE(&MAC_CTX().out);
  ZB_ASSERT(ent);
  ent->type = req_type;
  ent->buf = request;

  ZB_RING_BUFFER_FLUSH_PUT(&MAC_CTX().out);

  TRACE_MSG(TRACE_MAC3, "put request to queue ent %p, request %p; r.b. empty %hd",
            (FMT__P_P_H, ent, request,
             ZB_RING_BUFFER_IS_EMPTY( &MAC_CTX().out )));

}

/*! @} */
