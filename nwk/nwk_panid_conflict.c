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
PURPOSE: PAIND conflict detection and resolution at runtime
3.6.1.13, 3.4.9, 3,4,10
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_secur.h"


#include "zb_bank_6.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#ifdef ZB_ROUTER_ROLE

/**
   Reaction on panid conflict report at nwk manager (usually ZC).
 */
void zb_panid_conflict_got_network_report(zb_uint8_t param, zb_uint16_t *panids, zb_uint8_t n_panids)
{
    if (ZG->nwk.handle.state == ZB_NLME_STATE_PANID_CONFLICT_RESOLUTION
      || ZG->nwk.handle.panid_conflict)
  {
    TRACE_MSG(TRACE_NWK3, "panid conflict resolve is already in progress", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
  else
  {
    TRACE_MSG(TRACE_NWK3, "got panid report, n_panids %hd, start conflict resolve", (FMT__H, n_panids));
    ZB_BZERO(ZG->nwk.handle.known_panids, sizeof(ZG->nwk.handle.known_panids));
    if (n_panids > sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0]))
    {
      n_panids = sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0]);
    }
    ZB_MEMCPY(ZG->nwk.handle.known_panids, panids, n_panids * sizeof(zb_uint16_t));
    ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
    TRACE_MSG(TRACE_NWK3, "sending Pan ID conflict resolution network update", (FMT__0));
    zb_panid_conflict_network_update(param);
  }
}


/**
   Remember panid during panid conflict resolution process.

   Need it to choose another panid
 */
void zb_panid_conflict_remember_panid(zb_uint16_t panid) ZB_SDCC_REENTRANT
{
  zb_ushort_t i;
  for (i = 0 ; i < sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0]) ; ++i)
  {
    if (ZG->nwk.handle.known_panids[i] == panid)
    {
      return;
    }
    else if (ZG->nwk.handle.known_panids[i] == 0)
    {
      ZG->nwk.handle.known_panids[i] = panid;
      TRACE_MSG(TRACE_NWK3, "i %hd remembered panid 0x%x", (FMT__H_D, i, panid));
      return;
    }
  }
}


/**
   Start Active Scan to resolve panid conflict at nwk manager (ZC)

   Need it to know existent panids to exclude conflict after panid change.
 */
/* No use for this function, because there's no active scan during pan_id conflict resolution */
#if 0
void zb_panid_conflict_scan_start(zb_uint8_t param) ZB_CALLBACK
{
  if (ZG->nwk.handle.state != (zb_uint8_t)ZB_NLME_STATE_IDLE)
  {
    TRACE_MSG(TRACE_NWK3, "NWK is busy, will resolve PANID conflict later", (FMT__0));
    ZG->nwk.handle.panid_conflict = 1;
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
  else
  {
    ZB_BZERO(ZG->nwk.handle.known_panids, sizeof(ZG->nwk.handle.known_panids));
    ZG->nwk.handle.known_panids[0] = ZB_NIB_PAN_ID();
    ZG->nwk.handle.state = ZB_NLME_STATE_PANID_CONFLICT_ACTIVE_SCAN;
    ZB_MLME_BUILD_SCAN_REQUEST(ZB_BUF_FROM_REF(param), ZB_DEFAULT_APS_CHANNEL_MASK, ACTIVE_SCAN, ZB_DEFAULT_SCAN_DURATION);
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
  }
}
#endif

/**
   After active scan complete send network update command to entire net
 */
void zb_panid_conflict_network_update(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  /* choose new panid */
  zb_ushort_t i;
  do
  {
    ZG->nwk.handle.new_panid = ZB_RANDOM();
    for (i = 0 ;
         i < sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0])
           && ZG->nwk.handle.new_panid != ZG->nwk.handle.known_panids[i] ;
         ++i)
    {
    }
  }
  while (ZG->nwk.handle.new_panid == 0
         && i == sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0]));

  TRACE_MSG(TRACE_NWK3, "zb_panid_conflict_network_update: chosen new_panid 0x%x", (FMT__D, ZG->nwk.handle.new_panid));


  /* According to 3.6.1.13.2 change nwkUpdateId now and does not change it
   * after actual PANID change */
  zb_nwk_update_beacon_payload();
  ZB_SCHEDULE_CALLBACK(zb_panid_conflict_send_nwk_update, param);
  ZB_SCHEDULE_ALARM(zb_panid_conflict_set_panid_alarm, 0, ZB_NWK_BROADCAST_DELIVERY_TIME());
}


zb_uint8_t * zb_nwk_fill_out_command(zb_uint8_t param, zb_uint16_t dest, zb_uint8_t command_id, zb_uint8_t size) ZB_SDCC_REENTRANT
{
  zb_nwk_hdr_t *nwhdr;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_ieee_addr_t dst_ieee;
  zb_ushort_t hdr_size;
  zb_bool_t have_long_dst;

  TRACE_MSG(TRACE_NWK1, "zb_nwk_fill_out_command param %hd dest %d command_id %hd size %hd",
            (FMT__H_D_H_H, param, dest, command_id, size));

  /* do we have dest long addr? */
  have_long_dst = (zb_address_ieee_by_short(dest, dst_ieee) == RET_OK);
  hdr_size = have_long_dst ? ZB_NWK_FULL_HDR_SIZE(0) : ZB_NWK_HALF_HDR_SIZE(0);

  {
#ifdef ZB_SECURITY
    zb_bool_t secure = (ZG->nwk.nib.secure_all_frames && ZG->nwk.nib.security_level);
#endif

#ifdef ZB_SECURITY
    if (secure)
    {
      hdr_size += sizeof(zb_nwk_aux_frame_hdr_t);
    }
#endif
    ZB_BUF_INITIAL_ALLOC(buf, hdr_size + size + 1, nwhdr);

    /* fill nwk header */
    ZB_BZERO2(nwhdr->frame_control);
    ZB_NWK_FRAMECTL_SET_FRAME_TYPE_N_PROTO_VER(nwhdr->frame_control, ZB_NWK_FRAME_TYPE_COMMAND, ZB_PROTOCOL_VERSION);
    ZB_NWK_FRAMECTL_SET_DISCOVER_ROUTE(nwhdr->frame_control, 1);
    ZB_NWK_FRAMECTL_SET_SRC_DEST_IEEE(nwhdr->frame_control, 1, have_long_dst);

    /* yes, in host-endian. Will be converted in nwk_forward() */
    nwhdr->src_addr = ZB_NIB_NETWORK_ADDRESS();
    nwhdr->dst_addr = dest;
    nwhdr->radius = (zb_uint8_t)(ZB_NIB_MAX_DEPTH() << 1);
    nwhdr->seq_num = ZB_NIB_SEQUENCE_NUMBER();
    ZB_NIB_SEQUENCE_NUMBER_INC();
    ZB_IEEE_ADDR_COPY(nwhdr->src_ieee_addr, ZB_PIB_EXTENDED_ADDRESS());
    if ( have_long_dst )
    {
      ZB_IEEE_ADDR_COPY(nwhdr->dst_ieee_addr, dst_ieee);
    }

#ifdef ZB_SECURITY
    if (secure)
    {
      ZB_NWK_FRAMECTL_SET_SECURITY(nwhdr->frame_control, 1);
      buf->u.hdr.encrypt_type = ZB_SECUR_NWK_ENCR;
    }
#endif
  }

  {
    zb_uint8_t *ptr = ((zb_uint8_t *)nwhdr) + hdr_size;
    *ptr = command_id;
    return ptr + 1;
  }
}


/**
 */
void zb_panid_conflict_schedule_network_report(zb_uint8_t param, zb_uint16_t panid)
{
  if (ZG->nwk.handle.panid_conflict
      && ZG->nwk.handle.new_panid != 0
      && ZG->nwk.handle.new_panid != panid)
  {
    TRACE_MSG(TRACE_NWK1, "panid conflict resolve is in progres, received new_panid 0x%x != conflicting panid 0x%x",
              (FMT__D_D, ZG->nwk.handle.new_panid, panid));
    zb_free_buf(ZB_BUF_FROM_REF(param));
    return;
  }
  if (!ZG->nwk.handle.panid_conflict)
  {
    ZB_BZERO(ZG->nwk.handle.known_panids, sizeof(ZG->nwk.handle.known_panids));
  }
  zb_panid_conflict_remember_panid(panid);
  if (!ZG->nwk.handle.panid_conflict)
  {
    TRACE_MSG(TRACE_NWK1, "panid conflict detected, schedule panid conflict report", (FMT__0));
    ZB_SCHEDULE_ALARM(zb_panid_conflict_send_network_report, param, ZB_DEFAULT_SCAN_DURATION);
    ZG->nwk.handle.panid_conflict = 1;
    ZG->nwk.handle.new_panid = 0;
  }
  else
  {
    zb_free_buf(ZB_BUF_FROM_REF(param));
  }
}


/**
   Sent network report command after panid conflict detect on ZR
 */
void zb_panid_conflict_send_network_report(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_nwk_report_cmd_t *rep;
  zb_uint16_t *p;

  if (ZG->nwk.handle.panid_conflict
      && ZG->nwk.handle.new_panid != 0)
  {
    TRACE_MSG(TRACE_NWK1, "Already got new panid - not need to send nwk report", (FMT__0));
    zb_free_buf(ZB_BUF_FROM_REF(param));
    return;
  }

  /* first count panids */
  for (i = 0 ;
       i < sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0])
         && ZG->nwk.handle.known_panids[i] != 0 ;
        ++i)
  {
  }

  rep =
    (zb_nwk_report_cmd_t *)zb_nwk_fill_out_command(
      param, ZG->nwk.nib.nwk_manager_addr, ZB_NWK_CMD_NETWORK_REPORT, sizeof(zb_nwk_report_cmd_t) + (i - 1) * sizeof(zb_uint16_t));
  p = &rep->panids[0];

  rep->command_options = i;     /* counter */
  ZB_IEEE_ADDR_COPY(rep->epid, ZB_NIB_EXT_PAN_ID());

  for (i = 0 ;
       i < sizeof(ZG->nwk.handle.known_panids) / sizeof(ZG->nwk.handle.known_panids[0])
         && ZG->nwk.handle.known_panids[i] != 0 ;
        ++i)
  {
    p[i] = ZG->nwk.handle.known_panids[i];
    ZB_PAN_ID_TO_LE16(p[i]);
  }

  TRACE_MSG(TRACE_NWK2, "zb_panid_conflict_send_network_report param %hd %hd panids panid[0] = %hd",
            (FMT__H_H_D, param, i, p[0]));

  /* send request */
  ZB_SET_BUF_PARAM(ZB_BUF_FROM_REF(param), ZB_NWK_INTERNAL_NSDU_HANDLE, zb_uint8_t);
  ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);

  /* done with panid conflict info collecting */
}


void zb_panid_conflict_send_nwk_update(zb_uint8_t param) ZB_CALLBACK
{
  zb_nwk_update_cmd_t *upd =
    (zb_nwk_update_cmd_t *)zb_nwk_fill_out_command(
      param, ZB_NWK_BROADCAST_ALL_DEVICES, ZB_NWK_CMD_NETWORK_UPDATE, sizeof(zb_nwk_update_cmd_t));

  TRACE_MSG(TRACE_NWK1, ">> zb_panid_conflict_send_network_update param %hd", (FMT__H_D, param));

  upd->command_options = 1;     /* counter */
  ZB_IEEE_ADDR_COPY(upd->epid, ZB_NIB_EXT_PAN_ID());
  upd->update_id = ZB_NIB_UPDATE_ID();
  ZB_HTOLE16(&upd->new_panid, &ZG->nwk.handle.new_panid);

  /* send request */
  ZB_SET_BUF_PARAM(ZB_BUF_FROM_REF(param), ZB_NWK_INTERNAL_NSDU_HANDLE, zb_uint8_t);
  ZB_SCHEDULE_CALLBACK(zb_nwk_forward, param);

  zb_get_out_buf_delayed(zb_panid_conflict_send_status_ind);

  TRACE_MSG(TRACE_NWK1, "<< zb_panid_conflict_send_network_update", (FMT__0));
}


#endif


#ifndef ZB_LIMITED_FEATURES

void zb_panid_conflict_send_status_ind(zb_uint8_t param) ZB_CALLBACK
{
  zb_nlme_status_indication_t *status =  ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_status_indication_t);

  TRACE_MSG(TRACE_NWK1, ">> zb_panid_conflict_send_status_ind %hd", (FMT__H, param));

  /* NetworkAddr parameter set to 0 and the
     Status parameter set to PAN Identifier Update */
  status->status = ZB_NWK_COMMAND_STATUS_PAN_IDENTIFIER_UPDATE;
  status->network_addr = ZG->nwk.handle.new_panid;
  /* notify */
  ZB_SCHEDULE_CALLBACK(zb_nlme_status_indication, param);
  ZG->nwk.handle.panid_conflict = 0;
}


/**
   Reaction on nwk update command frame recv - change panid
 */
void zb_panid_conflict_network_update_recv(zb_nwk_update_cmd_t *upd)
{
  ZB_NIB_UPDATE_ID() = upd->update_id;
#ifdef ZB_ROUTER_ROLE
  /* Update beacon payload now, before actual panid update */
  ZB_NIB_UPDATE_ID()--;         /* zb_nwk_update_beacon_payload increments it
                                 * while must set it to the value from the command */
  zb_nwk_update_beacon_payload();
#endif
  ZB_LETOH16(&ZG->nwk.handle.new_panid, &upd->new_panid);
  TRACE_MSG(TRACE_NWK1, "zb_panid_conflict_network_update_recv update_id %hd panid %d",
            (FMT__H_D, upd->update_id, ZG->nwk.handle.new_panid));
  ZB_SCHEDULE_ALARM(zb_panid_conflict_set_panid_alarm, 0, ZB_NWK_BROADCAST_DELIVERY_TIME());

  zb_get_out_buf_delayed(zb_panid_conflict_send_status_ind);
}


/**
   Allocate buffer to set new panid
 */
void zb_panid_conflict_set_panid_alarm(zb_uint8_t param) ZB_CALLBACK
{
  ZVUNUSED(param);
  zb_get_out_buf_delayed(zb_panid_conflict_set_panid);
}


/**
   Call MLME-START.request to set new panid
*/
void zb_panid_conflict_set_panid(zb_uint8_t param) ZB_CALLBACK
{
  zb_mlme_start_req_t * req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mlme_start_req_t);

  TRACE_MSG(TRACE_NWK1, "Change PANID to %d", (FMT__D, ZG->nwk.handle.new_panid));

  ZG->nwk.handle.panid_conflict = 0;

  ZB_BZERO(req, sizeof(*req));
  req->pan_id = ZG->nwk.handle.new_panid;
  req->logical_channel = MAC_CTX().current_channel;
//  req->channel_page = 0;
//  req->coord_realignment = 0;
  req->pan_coordinator = (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR);
  req->beacon_order = ZB_TURN_OFF_ORDER;
  req->superframe_order = ZB_TURN_OFF_ORDER;

  ZG->nwk.handle.state = ZB_NLME_STATE_PANID_CONFLICT_RESOLUTION;

  ZB_SCHEDULE_CALLBACK(zb_mlme_start_request, param);
}

#endif  /* ZB_LIMITED_FEATURES */

/*! @} */
