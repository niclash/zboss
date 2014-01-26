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


#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
zb_ret_t zb_send_beacon_frame(zb_beacon_frame_params_t *beacon_frame_params) ZB_SDCC_REENTRANT;

/*
  Sends coordinator realignment command
  @param is_broadcast - if TRUE, cmd is broadcast over PAN, if FALSE
  cmd is directed to orphaned device
  @param orphaned_dev_ext_addr - orphaned device extended addres
  @param orphaned_dev_short_addr - orphaned device short address,
  maybe set to 0xFFFE

  @return RET_OK if ok, error code on error
*/
zb_ret_t zb_tx_coord_realignment_cmd(zb_bool_t is_broadcast, zb_ieee_addr_t orphaned_dev_ext_addr,
                                     zb_uint16_t orphaned_dev_short_addr)
{
  zb_ret_t ret = RET_OK;
  zb_uint_t packet_length;
  zb_uint_t mhr_len;
  void *ptr = NULL;
  zb_mac_mhr_t mhr;
  zb_mlme_start_req_t *params;
  zb_coord_realignment_cmd_t coord_realign_cmd;

/*
  mac spec, 7.3.8 Coordinator realignment command
  1) MHR, Frame Control:
  - if cmd is directed to orphaned device, set dst addr mode = ZB_ADDR_64BIT_DEV;
  if cmd is broadcast to the PAN set dst addr mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST
  - src addr mode = ZB_ADDR_64BIT_DEV
  - Frame Pending = 0
  - if cmd is directed to orphaned device, set Acknowledgment Request = 1,
  set Acknowledgment Request = 0 for PAN broadcast
  - if channel page is present, set Frame Version = 0x01, if channel
  page is absent, set 0x00 if no security and 0x01 if security is on
  - set dst PAN id = 0xFFFF
  - if cmd is directed to orphaned device, dst addr = ext addr of orphaned device
  if cmd is broadcast, set dst addr = 0xFFFF
  - src PAN id = pib.mac_pan_id
  - src addr = aExtendedAddress
  2) set Command Frame Identifier = MAC_CMD_COORDINATOR_REALIGNMENT
  3) set PAN id = req.pan_id
  4) set Coordinator Short Address = pib.mac_short_address
  5) set Logical Channel = req.logical_channel
  6) if cmd is broadcast, set Short Addr = 0xFFFF
  if cmd is directed to orphaned device, set short addr = orphaned device
  short addr, or 0xFFFE if device has no short addr
  7) if req.channel_page != existing channel page, set Channel Page = req.channel_page

*/

  TRACE_MSG(TRACE_MAC1, "+zb_tx_coord_realignment_cmd", (FMT__0));

  mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_64BIT_DEV,
                                        is_broadcast ? ZB_ADDR_16BIT_DEV_OR_BROADCAST : ZB_ADDR_64BIT_DEV,
                                        0);
  packet_length = mhr_len;
  packet_length += sizeof(zb_coord_realignment_cmd_t);
  /* TODO: reduce packet_length if Channel Page is omitted */

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);

/* Fill Frame Controll then call zb_mac_fill_mhr() */
/*
  mac spec  7.2.1.1 Frame Control field
  | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |
*/
  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_COMMAND);
  /* frame pending is 0 */
  if (!is_broadcast)
  {
    /* cmd is directed to orphaned device */
    ZB_FCF_SET_ACK_REQUEST_BIT(mhr.frame_control, 1);
    ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);
  }
  else
  {
    /* cmd is broadcast to PAN */
    /* ack request bit is 0 */
    ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  }
  /* PAN id compression is 0 */
  /* TODO: set frame version correctly, case Channel Page is omitted*/

  /* if Channel Page is omitted, than frame_version field should be default */
  /* MAC_FRAME_VERSION defined in zb_config.h */
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);


  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_64BIT_DEV);

  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = ZB_MAC_DSN();
  ZB_INC_MAC_DSN();
  mhr.dst_pan_id = ZB_BROADCAST_PAN_ID; /* broadcast value for both cases orphaned/broadcast */
  if (!is_broadcast && orphaned_dev_ext_addr)
  {
    /* cmd is directed to orphaned device */
    ZB_IEEE_ADDR_COPY(mhr.dst_addr.addr_long, orphaned_dev_ext_addr);
  }
  else
  {
    /* cmd is broadcast to PAN */
    mhr.dst_addr.addr_short = ZB_MAC_SHORT_ADDR_NO_VALUE;
  }
  mhr.src_pan_id = MAC_PIB().mac_pan_id;
  /* aExtendedAddress The 64-bit (IEEE) address assigned to the device */
  ZB_MEMCPY(&mhr.src_addr, MAC_PIB().mac_extended_address, sizeof(zb_ieee_addr_t));

  /* TODO: fill Aux Security Header */

  zb_mac_fill_mhr(ptr, &mhr);

/*
  mac spec 7.3.8 Coordinator realignment command
  | MHR    | Command  Frame Id | PAN Id | Coord Short Addr | Logical Channel | Short Address | Channel page |
  |var size| 1 byte            | 2 b    | 2 b              | 1 b             | 2 b           | 0/1 b        |
*/
  params = ZB_GET_BUF_PARAM((zb_buf_t*)ZG->mac.mac_ctx.pending_buf, zb_mlme_start_req_t);

  coord_realign_cmd.cmd_frame_id = MAC_CMD_COORDINATOR_REALIGNMENT;
  ZB_HTOLE16(&coord_realign_cmd.pan_id, &params->pan_id);
  ZB_HTOLE16(&coord_realign_cmd.coord_short_addr, &MAC_PIB().mac_short_address);
  coord_realign_cmd.logical_channel = params->logical_channel;
  if (!is_broadcast)
  {
    /* cmd is directed to orphaned device */
    ZB_HTOLE16(&coord_realign_cmd.short_addr, &orphaned_dev_short_addr);
  }
  else
  {
    /* cmd is broadcast to PAN */
    /* do not use HTOLE16 macro because 0xffff value is symmetric */
    coord_realign_cmd.short_addr = ZB_MAC_SHORT_ADDR_NO_VALUE;
  }
  coord_realign_cmd.channel_page = params->channel_page; /* TODO: it maybe omitted! */

  /* TODO: copy size maybe reduced if channel_page is not used */
  ZB_MEMCPY((zb_uint8_t*)ptr + mhr_len, &coord_realign_cmd, sizeof(zb_coord_realignment_cmd_t));

  MAC_ADD_FCS(MAC_CTX().operation_buf);
  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  TRACE_MSG( TRACE_MAC1, "<< zb_tx_coord_realignment_cmd, ret %i", (FMT__D, ret ));
  return ret;
}


/*
  Performs PAN realigning. mac spec 7.5.2.3.2 Realigning a PAN
  @return RET_OK, RET_BLOCKED, error code on error
*/
zb_ret_t  zb_realign_pan()
{
  zb_ret_t ret = RET_OK;
  zb_beacon_frame_params_t beacon_frame_params;
  zb_mlme_start_req_t *start_req_params;

/*
  MAC 7.5.2.3.2 Realigning a PAN
  1) if beacon mode is on, set Frame Control.Frame Pending = 1, do NOT change
  other parameters, send scheduled beacon

  2) send realignment command
  3) if beacon mode is off, send realignment command immediately
  4) if realignment command send fails, set status CHANNEL_ACCESS_FAILURE
  5) update PIB parameters according to 7.5.2.3.4 zb_mac_update_superframe_and_pib()
*/

  TRACE_MSG(TRACE_MAC1, "+zb_realign_pan", (FMT__0));
  if (ZB_CHECK_BEACON_MODE_ON())
  {
    start_req_params = ZB_GET_BUF_PARAM((zb_buf_t*)MAC_CTX().pending_buf, zb_mlme_start_req_t);
    beacon_frame_params.src_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    beacon_frame_params.security_enabled = 0;
    beacon_frame_params.beacon_order = start_req_params->beacon_order;
    beacon_frame_params.superframe_order = start_req_params->superframe_order;
    beacon_frame_params.ble = start_req_params->battery_life_extension;
    beacon_frame_params.frame_pending = 1;
    ret = zb_send_beacon_frame(&beacon_frame_params);
  }
  /* For realignment, it seems like we should not break the procedure */
  ZB_WAIT_FOR_TX();
  if (ret == RET_OK)
  {
    /* send realignment command*/
    ret = zb_tx_coord_realignment_cmd(ZB_FALSE, NULL, 0);
  }
  ZB_WAIT_FOR_TX();
  /* do not break */
  if (ret == RET_OK)
  {
    ret = zb_check_cmd_tx_status();
  }
  TRACE_MSG(TRACE_MAC1, "<< zb_realign_pan, ret %i", (FMT__D, ret));
  return ret;
}
#endif  /* ZB_COORDINATOR_ROLE || ZB_ROUTER_ROLE */
/*
  performs processing of the MLME-Start.request.
  return RET_OK, RET_BLOCKED, error code on error
*/


#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
zb_ret_t zb_send_beacon_frame(zb_beacon_frame_params_t *beacon_frame_params) ZB_SDCC_REENTRANT
{
  zb_uint8_t packet_length;
  zb_uint8_t mhr_len;
  zb_mac_mhr_t mhr;
  zb_uint8_t *ptr = NULL;
  zb_ret_t ret;

  TRACE_MSG(TRACE_MAC1, ">>zb_send_beacon_frame", (FMT__0));

/* mac spec  7.2.2.1 Beacon frame format
   | MHR | Superframe | GTS | Pending address | Beacon Payload | MFR |
*/
  mhr_len = zb_mac_calculate_mhr_length(beacon_frame_params->src_addr_mode, ZB_ADDR_NO_ADDR,
                                        0);
  packet_length = mhr_len + sizeof(zb_super_frame_spec_t) + 2*sizeof(zb_uint8_t) + MAC_PIB().mac_beacon_payload_length;

  TRACE_MSG(TRACE_MAC1, "packet length %hd, payload len %hd", (FMT__H_H, packet_length, MAC_PIB().mac_beacon_payload_length));

  ZB_BUF_INITIAL_ALLOC(MAC_CTX().operation_buf, packet_length, ptr);
  ZB_ASSERT(ptr);

  ZB_BZERO(ptr, packet_length);

/* mac spec  7.2.1.1 Frame Control field
   | Frame Type | Security En | Frame Pending | Ack.Request | PAN ID Compres | Reserv | Dest.Addr.Mode | Frame Ver | Src.Addr.gMode |

   mac spec 7.2.2.1.1 Beacon frame MHR fields
   Frame Control field
   - Frame type = MAC_FRAME_BEACON
   - src addr mode specified by coordinator
   - set security enabled = 1 if specified
   - security enabled == 1 set frame ver = 1, 0 otherwise
   - If a broadcast data or command frame is pending, set frame pending = 1
   - set other fields to 0
*/
  ZB_BZERO(&mhr, sizeof(zb_mac_mhr_t));
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_BEACON);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, beacon_frame_params->src_addr_mode);
  /* MAC_FRAME_VERSION defined in zb_config.h */
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_VERSION);
  ZB_FCF_SET_FRAME_PENDING_BIT(mhr.frame_control, beacon_frame_params->frame_pending);


/*
  mac spec 7.2.2.1.1 Beacon frame MHR fields
  MHR field
  - Sequence Number = pib.macBSN
  - src pan id = pib.pan_id
  - src pan addr = pan addres
  - if security enabled, set Auxiliary Security Header field (7.2.1.7 Auxiliary Security Header field)
*/
  mhr.seq_number = ZB_MAC_BSN();
  ZB_INC_MAC_BSN();
  mhr.src_pan_id = MAC_PIB().mac_pan_id;

  if (beacon_frame_params->src_addr_mode == ZB_ADDR_64BIT_DEV)
  {
    /* aExtendedAddress The 64-bit (IEEE) address assigned to the device */
    ZB_MEMCPY(&mhr.src_addr, MAC_PIB().mac_extended_address, sizeof(zb_ieee_addr_t));
  }
  else
  {
    mhr.src_addr.addr_short = MAC_PIB().mac_short_address;
    TRACE_MSG(TRACE_MAC1, "beacon set short addr %x", (FMT__D, mhr.src_addr.addr_short));
  }
  /* TODO: fill Aux Security Header */

  zb_mac_fill_mhr(ptr, &mhr);
  ptr += mhr_len;

/*
  7.2.2.1.2 Superframe Specification field
  | Beacon    | Superframe | Final CAP | Battery Life Extension | Reserved | PAN Coordinator | Association |
  | Order 0-3 | Order 4-7  | Slot 8-11 | (BLE) 12               | 13       | 14              | Permit 15   |
  - Beacon order = req.beacon_order
  - superframe order = req.superframe_order
  - final CAP slot = aMinCAPLength
  - BLE = req.BLE (Battery Life Extension)
  - set PAN coordinator = ZB_COORDINATOR_ROLE
  - set association permit = pib.macAssociationPermit
*/
  ZB_BZERO2(ptr);
  ZB_SUPERFRAME_SET_BEACON_ORDER(ptr, beacon_frame_params->beacon_order);
  ZB_SUPERFRAME_SET_SUPERFRAME_ORDER(ptr, beacon_frame_params->superframe_order);
/* spec says final cap slot should be not less then MAC_MIN_CAP_LENGTH, but this
 * const == 440 and doesn't fit into 4 bits; other stacks use value 0x0f */
  ZB_SUPERFRAME_SET_FINAL_CAP_SLOT(ptr, 0x0F);
  ZB_SUPERFRAME_SET_BLE(ptr, beacon_frame_params->ble);
#ifdef ZB_COORDINATOR_ROLE
  if (ZB_AIB().aps_designated_coordinator ==  1) ZB_SUPERFRAME_SET_PAN_COORD(ptr, 1);
#else
  /* pan coord bit is set to 0 already */
#endif

  ZB_SUPERFRAME_SET_ASS_PERMIT(ptr, MAC_PIB().mac_association_permit);

  TRACE_MSG(TRACE_MAC3, "superframe %x ass perm %hd", (FMT__D_H, *(zb_uint16_t*)ptr, (zb_uint8_t)MAC_PIB().mac_association_permit));

  ptr += sizeof(zb_uint16_t);

/*
  7.2.2.1.3 GTS Specification field
  - set all fields to 0
  7.2.2.1.5 GTS List field
  is omitted
  7.2.2.1.6 Pending Address Specification field
  - set all fields to 0
  7.2.2.1.7 Address List field
  is omitted
*/
  /* next 2 bytes value is already == 0 */
  ptr += 2*sizeof(zb_uint8_t);

/*
  7.2.2.1.8 Beacon Payload field
  cfill beacon payload with value pib.macBeaconPayload. Size of data is
  sizeof(zb_mac_beacon_payload_t) == 15 bytes (zigbee spec, 3.6.7 NWK
  Information in the MAC Beacons)
*/
  ZB_MEMCPY(ptr, (zb_uint8_t*)&MAC_PIB().mac_beacon_payload, MAC_PIB().mac_beacon_payload_length);

  MAC_ADD_FCS(MAC_CTX().operation_buf);

  ret = ZB_TRANS_SEND_COMMAND(mhr_len, MAC_CTX().operation_buf);

  /* call function after beacon sent if configured */
  if ( MAC_CTX().beacon_sent )
  {
    MAC_CTX().beacon_sent();
  }

  TRACE_MSG(TRACE_MAC1, "<< zb_send_beacon_frame, ret %i", (FMT__D, ret));
  return ret;
}
#endif

void zb_mac_update_superframe_and_pib()
{
  zb_mlme_start_req_t *params;

/*
  mac spec 7.5.2.3.4:
  - pib.macBeaconOrder =  req.BeaconOrder
  - if pib.macBeaconOrder == 15, pib.macSuperframeOrder = 15 (NON beacon PAN case)
  - if pib.macBeaconOrder < 15, pib.macSuperframeOrder = req.SuperframeOrder
  - pib.macPANID = req.PANId
  - pib.phyCurrentPage = req.ChannelPage (use PLME-SET.request)
  - pib.phyCurrentChannel = req.LogicalChannel (use PLME-SET.request)
*/

  TRACE_MSG( TRACE_MAC1, ">>zb_mac_update_superframe_and_pib", (FMT__0 ));

  params = ZB_GET_BUF_PARAM((zb_buf_t*)MAC_CTX().pending_buf, zb_mlme_start_req_t);

  MAC_PIB().mac_beacon_order = params->beacon_order;
  if (MAC_PIB().mac_beacon_order == ZB_TURN_OFF_ORDER)
  {
    MAC_PIB().mac_superframe_order = ZB_TURN_OFF_ORDER;
  }
  else
  {
    MAC_PIB().mac_superframe_order = params->superframe_order;
  }
  MAC_PIB().mac_pan_id = params->pan_id;
  MAC_PIB().phy_current_page = params->channel_page;
  MAC_PIB().phy_current_channel = params->logical_channel;
  TRACE_MSG(TRACE_MAC1, "set pan id 0x%x channel 0x%hx",
            (FMT__D_H, MAC_PIB().mac_pan_id, MAC_PIB().phy_current_channel));
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_SHORT_ADDR();
  ZB_UPDATE_PAN_ID();
  ZB_TRANSCEIVER_SET_CHANNEL(MAC_PIB().phy_current_channel);
  TRACE_MSG(TRACE_MAC1, "<<zb_mac_update_superframe_and_pib", (FMT__0));
}

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
void zb_handle_beacon_req(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret = RET_OK;
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC1, ">> zb_handle_beacon_req dev_type 0x%x mac_beacon_payload_length 0x%x", (FMT__D_D, MAC_CTX().dev_type, MAC_PIB().mac_beacon_payload_length));
  /* Ignore beacon requests if we are not ZC/ZR or if not started net/joined */
  if (MAC_CTX().dev_type == MAC_DEV_FFD
#ifndef ZB_MAC_TESTING_MODE
      /* skip this check in test mode */
      && MAC_PIB().mac_beacon_payload_length
#endif
    )
  {
    zb_beacon_frame_params_t beacon_frame_params;

#ifdef ZB_MAC_TESTING_MODE
    if (ZB_NIB_NETWORK_ADDRESS() == ZB_MAC_SHORT_ADDR_NOT_ALLOCATED)
    {
      beacon_frame_params.src_addr_mode = ZB_ADDR_64BIT_DEV;
    }
    else
    {
      beacon_frame_params.src_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    }
#else
    beacon_frame_params.src_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
#endif
    beacon_frame_params.security_enabled = 0;
    beacon_frame_params.beacon_order = MAC_PIB().mac_beacon_order;
    beacon_frame_params.superframe_order = MAC_PIB().mac_superframe_order;
    beacon_frame_params.ble = MAC_PIB().mac_batt_life_ext;
    beacon_frame_params.frame_pending = 0;
    ret = zb_send_beacon_frame(&beacon_frame_params);
  }
  else
  {
    TRACE_MSG(TRACE_MAC1, "we are %d (!FFD) or no b.payload - ignore beacon req", (FMT__D, MAC_CTX().dev_type));
  }

#ifdef ZB_MULTIPLE_BEACONS
  MAC_CTX().beacons_sent++;
#endif

  TRACE_MSG(TRACE_MAC1, "<< zb_handle_beacon_req %i", (FMT__D, ret));
}


void zb_mlme_handle_orphan_response(zb_uint8_t param) ZB_CALLBACK
{
  zb_mac_orphan_response_t oresp;
  zb_mlme_start_req_t *req;

  MAC_CTX().pending_buf = ZB_BUF_FROM_REF(param);
  ZB_MEMCPY(&oresp, ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_orphan_response_t), sizeof(oresp));
  req = ZB_GET_BUF_PARAM((zb_buf_t*)ZG->mac.mac_ctx.pending_buf, zb_mlme_start_req_t);
  TRACE_MSG(TRACE_MAC1, ">>zb_mlme_handle_orphan_response", (FMT__0));

  /* send realign command */
  ZB_DUMP_IEEE_ADDR(oresp.orphan_addr);

  req->pan_id = ZB_PIB_SHORT_PAN_ID();
  req->logical_channel = MAC_CTX().current_channel;
  req->channel_page = 0;
  MAC_CTX().tx_wait_cb = zb_mlme_handle_orphan_response_continue;
  MAC_CTX().tx_wait_cb_arg = param;
  zb_tx_coord_realignment_cmd(ZB_FALSE, oresp.orphan_addr, oresp.short_addr);
  TRACE_MSG(TRACE_MAC1, "<<zb_mlme_handle_orphan_response, continue scheduled", (FMT__0));
}


void zb_mlme_handle_orphan_response_continue(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;
  zb_mac_orphan_response_t oresp; /* possible better to send it via MAC_CTX() */
  /* check command TX status */
  ZB_SKIP_TX_CHK();
  ret = zb_check_cmd_tx_status();
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC1, ">>zb_mlme_handle_orphan_response_continue", (FMT__0));
  ZB_MEMCPY(&oresp, ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mac_orphan_response_t), sizeof(oresp));
  if ( ret == RET_OK )
  {
    zb_mlme_comm_status_indication_t *ind = ZB_GET_BUF_PARAM(MAC_CTX().pending_buf, zb_mlme_comm_status_indication_t);
    zb_ieee_addr_t orphan_addr;
    zb_uint16_t short_addr = oresp.short_addr;

    ZB_IEEE_ADDR_COPY(orphan_addr, oresp.orphan_addr);

    ind->status = MAC_SUCCESS;
    ind->src_addr_mode = ZB_ADDR_64BIT_DEV;
    ZB_IEEE_ADDR_COPY(ind->src_addr.addr_long, orphan_addr);
    ind->dst_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    ind->dst_addr.addr_short = short_addr;

    /* call nwk comm status */
    ZB_SCHEDULE_CALLBACK(zb_mlme_comm_status_indication, ZB_REF_FROM_BUF(MAC_CTX().pending_buf));
  }
  TRACE_MSG(TRACE_MAC1, "<<zb_mlme_handle_orphan_response_continue", (FMT__0));
}

#endif

/*! @} */
