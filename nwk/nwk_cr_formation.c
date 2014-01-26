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
PURPOSE: Network creation routine
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_mac.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "nwk_internal.h"
#include "zb_magic_macros.h"

/*! \addtogroup ZB_NWK */
/*! @{ */

#include "zb_bank_5.h"

#ifdef ZB_ROUTER_ROLE
static void call_mlme_start(zb_buf_t *buf, zb_uint16_t pan_id, zb_uint16_t channel) ZB_SDCC_REENTRANT;
#endif

#ifdef ZB_COORDINATOR_ROLE

void zb_nlme_network_formation_request(zb_uint8_t param) ZB_CALLBACK
{

  TRACE_MSG(TRACE_NWK1, ">>nwk_formation_req %hd", (FMT__H, param));

  TRACE_MSG(TRACE_NWK1, "nwk state %hd joined %hd", (FMT__H_H, ZG->nwk.handle.state, ZG->nwk.handle.joined));

  if (!(ZG->nwk.handle.state == ZB_NLME_STATE_IDLE
        && !ZG->nwk.handle.joined ))
  {
    TRACE_MSG(TRACE_NWK1, "nwk busy or joined", (FMT__0, ZG->nwk.handle.state));
    NWK_FORMATION_FAILURE_CONFIRM((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_NWK_STATUS_INVALID_REQUEST);
  }
  else
  {
    /**
      \par Network formation sequence

      See 3.6.1.1  Establishing a New Network


      * MLME-SCAN (energy scan) - skip if only 1 channel
      * MLME-SCAN (active scan) - skip if ns-3 build
      * select channel          - skip if ns-3 build
      * set PAN ID in PIB
      * set short address in PIB
      * MLME-START
      * return via NLME-NETWORK-FORMATION.confirm


      calls sequence:
      * zb_mlme_scan_request (ed scan) - zb_mlme_scan_confirm - nwk_formation_ed_scan_confirm
      * zb_mlme_scan_request (active scan) - zb_mlme_scan_confirm - nwk_formation_select_channel - call_mlme_start - zb_mlme_start_request
      * zb_mlme_start_confirm - zb_nlme_network_formation_confirm

     */

    /* request was saved in our internal buffer in zb_nlme_network_formation_request() */
    zb_nlme_network_formation_request_t *request =
      ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_nlme_network_formation_request_t);

    /* Save request values */
    ZB_MEMCPY(&ZG->nwk.handle.saved_req.formation, request, sizeof(*request));

    /* call mac to make channel scan, if there is only one channel specified
     * skip ed scan - see 3.2.2.3.3 */
    ZB_MLME_BUILD_SCAN_REQUEST((zb_buf_t *)ZB_BUF_FROM_REF(param),
                               ZG->nwk.handle.saved_req.formation.scan_channels,
                               /* scan_channels will be power of 2 if only 1
                                * bit is set - means, only 1
                                * channel specified */
                               MAGIC_IS_POWER_OF_TWO(ZG->nwk.handle.saved_req.formation.scan_channels) ? ACTIVE_SCAN : ED_SCAN,
                               ZG->nwk.handle.saved_req.formation.scan_duration);
    /* MAC will call zb_mlme_scan_confirm() */
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
    ZG->nwk.handle.state = MAGIC_IS_POWER_OF_TWO(ZG->nwk.handle.saved_req.formation.scan_channels) ?
      ZB_NLME_STATE_FORMATION_ACTIVE_SCAN : ZB_NLME_STATE_FORMATION_ED_SCAN;
    /* Call 1 or 2 scans, select channel, call mlme_start */
  }
  TRACE_MSG(TRACE_NWK1, "<<nwk_formation_req", (FMT__0));


}


void nwk_formation_ed_scan_confirm(zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  zb_mac_scan_confirm_t *scan_confirm = ZB_GET_BUF_PARAM(buf, zb_mac_scan_confirm_t);
  zb_uint32_t channel_mask = 1;
  zb_ushort_t i;

  TRACE_MSG(TRACE_NWK1, ">>nwk_form_ed_scan_conf buf %p", (FMT__P, buf));
  CHECK_PARAM_RET_ON_ERROR(scan_confirm);

  /* save ed results for future channel selection */
  ZB_MEMCPY(ZG->nwk.handle.tmp.formation.energy_detect, scan_confirm->list.energy_detect,
            sizeof(scan_confirm->list.energy_detect));

  /* generate channels mask for active scan */
  for (i = 0, channel_mask <<= ZB_MAC_START_CHANNEL_NUMBER;
       i < ZB_MAC_SUPPORTED_CHANNELS;
       i++, channel_mask <<= 1)
  {
    TRACE_MSG(TRACE_NWK1, "ch %hd e_level %hd ", (FMT__H_H, i, scan_confirm->list.energy_detect[i]));

    /* skip channels which level is higher than acceptable  */
    if ( ZG->nwk.handle.saved_req.formation.scan_channels & channel_mask
         && scan_confirm->list.energy_detect[i] > ZB_NWK_CHANNEL_ACCEPT_LEVEL )
    {
      ZG->nwk.handle.saved_req.formation.scan_channels ^= channel_mask;
    }
  }

  TRACE_MSG(TRACE_NWK1, "ch mask for as 0x%hx", (FMT__H,
                         ZG->nwk.handle.saved_req.formation.scan_channels));
  if ( ZG->nwk.handle.saved_req.formation.scan_channels )
  {
    /* Starn ext neighbor: will remember information about pan ids there */
    zb_nwk_exneighbor_start();

    /* call mac to make an active scan */
    ZB_MLME_BUILD_SCAN_REQUEST(buf, ZG->nwk.handle.saved_req.formation.scan_channels,
                               ACTIVE_SCAN, ZG->nwk.handle.saved_req.formation.scan_duration);
    ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, ZB_REF_FROM_BUF(buf));
    ZG->nwk.handle.state = ZB_NLME_STATE_FORMATION_ACTIVE_SCAN;
  }
  else
  {
    NWK_FORMATION_FAILURE_CONFIRM(buf, ZB_NWK_STATUS_STARTUP_FAILURE);
    ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
  }

  TRACE_MSG(TRACE_NWK1, "<<nwk_form_ed_scan_conf", (FMT__0));
}


/**
   Select channel after active scan complete.

   Used at formation time.
   Called by zb_mlme_scan_confirm()

   @param buf - buffer with results
 */
void nwk_formation_select_channel(zb_buf_t *buf) ZB_SDCC_REENTRANT
{
  zb_uint8_t channel = ZB_MAC_START_CHANNEL_NUMBER;
  zb_uint8_t channel_networks = 0xff;
  zb_uint8_t channel_ed = 0xff;
  zb_uint16_t pan_id = MAC_PIB().mac_pan_id;
  zb_uint_t unique_pan_id = 0x00;
  zb_uint8_t *channel_pan_count = &ZG->nwk.handle.tmp.formation.channel_pan_count[-ZB_MAC_START_CHANNEL_NUMBER];
  zb_uint8_t *panid_handled_bm = &ZG->nwk.handle.tmp.formation.panid_handled_bm[0];
  zb_uint_t i = 0;


  TRACE_MSG(TRACE_NWK1, ">>sel_ch buf %p", (FMT__P, buf));

  ZB_BZERO(panid_handled_bm, sizeof(ZG->nwk.handle.tmp.formation.panid_handled_bm));
  ZB_BZERO(ZG->nwk.handle.tmp.formation.channel_pan_count,
            sizeof(ZG->nwk.handle.tmp.formation.channel_pan_count));

  /* calc number of networks on each channel */
  for (i = 0 ; i < ZG->nwk.neighbor.ext_neighbor_used ; ++i)
  {
    if (ZB_CHECK_BIT_IN_BIT_VECTOR(panid_handled_bm, ZG->nwk.neighbor.ext_neighbor[i].panid_ref))
    {
      continue;
    }
    ZB_SET_BIT_IN_BIT_VECTOR(panid_handled_bm, ZG->nwk.neighbor.ext_neighbor[i].panid_ref);

    /*
     * There was check for logical_channel < ZB_MAC_SUPPORTED_CHANNELS.
     * It is impossible to overflow: logical_channel is 4 bit while
     * ZB_MAC_SUPPORTED_CHANNELS is 15. TODO: add some assertion? */
    channel_pan_count[ZG->nwk.neighbor.ext_neighbor[i].logical_channel]++;
  } /* networks per channel calculation loop */

  /* Try to find suitable channel, first select with the minimal numbers of
   * networks, then with the lowest energy  */
  for (i = ZB_MAC_START_CHANNEL_NUMBER ; i <=ZB_MAC_MAX_CHANNEL_NUMBER ; i++)
  {
    if ((ZG->nwk.handle.saved_req.formation.scan_channels & (1l << i))
         && channel_pan_count[i] <= channel_networks)
    {
      TRACE_MSG(TRACE_NWK3,
                "cur ch %hd ch_nw %hd en_lev %hd", (FMT__H_H_H,
                channel, channel_networks, channel_ed));
      TRACE_MSG(TRACE_NWK3, "i pan_cnt %hd en_lev %hd", (FMT__H_H,
                             i, channel_pan_count[i], ZG->nwk.handle.tmp.formation.energy_detect[i]));

      /* select with the lower energy */
      if ( channel_pan_count[i] == channel_networks
           && ZG->nwk.handle.tmp.formation.energy_detect[i] > channel_ed )
      {
        /* skip it */
        continue;
      }

      channel = i;
      channel_networks = channel_pan_count[i];
      channel_ed = ZG->nwk.handle.tmp.formation.energy_detect[i];

      /* found channel without networks ? */
      if ( channel_pan_count[i] == 0 )
      {
        if (pan_id == 0)
        {
          pan_id = ZB_RANDOM();
        }
        unique_pan_id = 1;
        break;
      }
    }
  }

  TRACE_MSG(TRACE_NWK1,
            "sel ch %hd # nw %hd en_lev %hd", (FMT__H_H_H,
             channel, channel_networks, channel_ed));
  if ( channel != ZB_MAC_START_CHANNEL_NUMBER )
  {
    /* generate pan_id if necessary */
    while ( !unique_pan_id )
    {
      pan_id = ZB_RANDOM();
      TRACE_MSG(TRACE_NWK1, "generated pan_id %d", (FMT__D, pan_id));

      /* check pan_id not used in this channel */
      for (i = 0, unique_pan_id = 1; i < ZG->nwk.neighbor.ext_neighbor_used; i++)
      {
        if ( ZG->nwk.neighbor.ext_neighbor[i].logical_channel == channel)
        {
          zb_uint16_t nt_panid;
          zb_address_get_short_pan_id(ZG->nwk.neighbor.ext_neighbor[i].panid_ref, &nt_panid);
          if (nt_panid == pan_id)
          {
            TRACE_MSG(TRACE_NWK1, "pan_id %d is on ch %hd", (FMT__D_H, pan_id, channel));
            unique_pan_id = 0;
            break;
          }
        }
      }
    }
  }

  if ( channel <= ZB_MAC_MAX_CHANNEL_NUMBER
       && unique_pan_id)
  {
    TRACE_MSG(TRACE_NWK1, "sel pan_id %d", (FMT__D, pan_id));

    ZG->nwk.nib.depth = 0;
    zb_nwk_set_device_type(ZB_NWK_DEVICE_TYPE_COORDINATOR);
    ZG->nwk.nib.cskip = zb_nwk_daa_calc_cskip(ZB_NIB_DEPTH());

    call_mlme_start(buf, pan_id, channel);
  }
  else
  {
    TRACE_MSG(TRACE_NWK1, "ch sel failed", (FMT__0));
    NWK_FORMATION_FAILURE_CONFIRM(buf, ZB_NWK_STATUS_STARTUP_FAILURE);
    ZG->nwk.handle.state = ZB_NLME_STATE_IDLE;
  }

  TRACE_MSG(TRACE_NWK1, "<<sel_ch", (FMT__0));
}

#endif /* ZB_COORDINATOR_ROLE */


#ifdef ZB_ROUTER_ROLE
/* Note that coordinator role suppose router role. */

void zb_nlme_start_router_request(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>start_router_req %hd", (FMT__H, param));

  /* check that we are already not a router */
  if (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER )
  {
    zb_mlme_start_req_t req;
    zb_nlme_start_router_request_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_nlme_start_router_request_t);


    TRACE_MSG(TRACE_NWK1, "b_ord %hd sf_ord %hd bat_life_ext %hd", (FMT__H_H_H,
              request->beacon_order, request->superframe_order, request->battery_life_extension));
    req.pan_id = MAC_PIB().mac_pan_id;
    req.logical_channel = MAC_CTX().current_channel;
    req.beacon_order = request->beacon_order;
    req.pan_coordinator = 0;
    req.superframe_order = request->superframe_order;
    req.battery_life_extension = request->battery_life_extension;
    req.coord_realignment = ZG->nwk.handle.router_started ? 1 : 0;
    ZG->nwk.handle.router_started = 1;

    ZB_MEMCPY(ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_start_req_t), &req, sizeof(req));
    ZB_SCHEDULE_CALLBACK(zb_mlme_start_request, param);

    ZG->nwk.handle.state = ZB_NLME_STATE_ROUTER;
  }
  else
  {
    NWK_CONFIRM_STATUS((zb_buf_t *)ZB_BUF_FROM_REF(param), ZB_NWK_STATUS_INVALID_REQUEST, zb_nlme_start_router_confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<start_router_req", (FMT__0));
}


/**
   Fill parameters and call zb_mlme_start_request primitive

   @param buf - packet buffer to use
   @param pan_id - pan id for the network
   @param channel - channel to use
 */
static void call_mlme_start(zb_buf_t *buf, zb_uint16_t pan_id, zb_uint16_t channel) ZB_SDCC_REENTRANT
{
  zb_mlme_start_req_t req;

  /* the NLME will choose 0x0000 as the 16-bit short MAC address */
  ZB_PIB_SHORT_ADDRESS() = 0;

  /* if the NIB attribute  nwkExtendedPANId is equal to 0x0000000000000000, this
     attribute will be initialized with the value of the MAC constant
     macExtendedAddress.  */
  /* TODO: add check if extended pan id in 0 or not */  
  if (ZB_IEEE_ADDR_IS_ZERO(ZB_NIB_EXT_PAN_ID()))
  {
    ZB_EXTPANID_COPY(ZB_NIB_EXT_PAN_ID(), ZB_PIB_EXTENDED_ADDRESS());
  }

  /* TODO: fill zero params if necessary */
  req.pan_id = pan_id;
  req.logical_channel = channel;
  TRACE_MSG(TRACE_NWK2, "ch %hd", (FMT__H, channel));
  req.channel_page = 0; /* TODO: set correct value */
  req.pan_coordinator = 1;      /* will be coordinator */
  req.coord_realignment = 0;
  req.beacon_order = ZB_TURN_OFF_ORDER;
  req.superframe_order = ZB_TURN_OFF_ORDER;

  ZB_MEMCPY(ZB_GET_BUF_PARAM(buf, zb_mlme_start_req_t), &req, sizeof(req));
  ZB_SCHEDULE_CALLBACK(zb_mlme_start_request, ZB_REF_FROM_BUF(buf));

  ZG->nwk.handle.state = ZB_NLME_STATE_FORMATION;
}




/**
   Assign beacon payload stored in MAC PIB.

   Most fields can be derived from the existing information, but let's store it
   separatly for speedup. MAC need just use data, do not call any function.
   Also, it is more closely to the spec.

   This function must be called after MAC START confirm and after change any of
   parameter: router_capacity, end_device_capacity, nwk_update_id.
 */
void zb_nwk_update_beacon_payload()
{
  TRACE_MSG(TRACE_NWK1, ">>update_beacon_pl", (FMT__0));

  ZB_BZERO(&ZB_PIB_BEACON_PAYLOAD(), sizeof(zb_mac_beacon_payload_t));
  /* TODO: when will implement PRO, handle PRO device participate in 2007 network as ZE. */
  ZB_PIB_BEACON_PAYLOAD().stack_profile = ZB_STACK_PROFILE;
  ZB_PIB_BEACON_PAYLOAD().protocol_version = ZB_PROTOCOL_VERSION;
  ZB_PIB_BEACON_PAYLOAD().device_depth = ZB_NIB_DEPTH();
  ZB_EXTPANID_COPY(ZB_PIB_BEACON_PAYLOAD().extended_panid, ZB_NIB_EXT_PAN_ID());
  ZB_MEMSET(&ZB_PIB_BEACON_PAYLOAD().txoffset,0xFF, sizeof(ZB_PIB_BEACON_PAYLOAD().txoffset));
  /*
    Currently mac_association_permit changed when we out of joined children #.
    TODO: count # of ZR and ZE separately.
   */
  if ( ZB_NIB_DEPTH() < ZB_NWK_MAX_DEPTH )
  {
    ZB_PIB_BEACON_PAYLOAD().router_capacity = MAC_PIB().mac_association_permit;
    ZB_PIB_BEACON_PAYLOAD().end_device_capacity = MAC_PIB().mac_association_permit;
  }
  else
  {
    ZB_PIB_BEACON_PAYLOAD().router_capacity = 0;
    ZB_PIB_BEACON_PAYLOAD().end_device_capacity = 0;
	MAC_PIB().mac_association_permit = 0;
  }
  /* we do not need to increment updateID here */
  /* ZB_NIB_UPDATE_ID() = ZB_NIB_UPDATE_ID() + 1;*/
  TRACE_MSG(TRACE_NWK3, "new update_id %hd", (FMT__H, ZB_NIB_UPDATE_ID()));
  ZB_PIB_BEACON_PAYLOAD().nwk_update_id = ZB_NIB_UPDATE_ID();
  MAC_PIB().mac_beacon_payload_length = sizeof(ZB_PIB_BEACON_PAYLOAD());

  TRACE_MSG(TRACE_NWK1, "<<update_beacon_pl", (FMT__0));
}

#endif  /* ZB_ROUTER_ROLE */

#if 0
/**
   Confirmation to the zb_mlme_reset_request().

   Continuation of the NWK formation state machine.


   Not used now!!
 */
/* not banked: in the same bank with MAC */
void zb_mlme_reset_confirm(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK1, ">>mlme_reset_conf %hd", (FMT__H, param));
#ifdef ZB_COORDINATOR_ROLE
  {
/* TODO: check if parameter type is correct - zb_mlme_reset_request_t*
   is put on zb_nlme_network_formation_request(),zb_mlme_reset_request() call */

  /* request was saved in our internal buffer in zb_nlme_network_formation_request() */
    zb_nlme_network_formation_request_t *request =
    ZB_GET_BUF_PARAM(&ZG->nwk.handle.buf, zb_nlme_network_formation_request_t);

  /* Save request values */

    ZB_MEMCPY(&ZG->nwk.handle.saved_req.formation, request, sizeof(*request));

  /* call mac to make channel scan, if there is only one channel specified
   * skip ed scan - see 3.2.2.3.3 */
    ZB_MLME_BUILD_SCAN_REQUEST((zb_buf_t *)ZB_BUF_FROM_REF(param),
                             request->scan_channels,
                             /* scan_channels will be power of 2 if only 1
                              * bit is set - means, only 1
                              * channel specified */
                             MAGIC_IS_POWER_OF_TWO(request->scan_channels) ? ACTIVE_SCAN : ED_SCAN,
                               request->scan_duration);
  /* MAC will call zb_mlme_scan_confirm() */
  ZB_SCHEDULE_CALLBACK(zb_mlme_scan_request, param);
  ZG->nwk.handle.state = MAGIC_IS_POWER_OF_TWO(request->scan_channels) ?
    ZB_NLME_STATE_FORMATION_ACTIVE_SCAN : ZB_NLME_STATE_FORMATION_ED_SCAN;
  /* Call 1 or 2 scans, select channel, call mlme_start */
  }
#endif  /* ZB_COORDINATOR_ROLE */

  TRACE_MSG(TRACE_NWK1, "<<mlme_reset_conf", (FMT__0));
}

void zb_nlme_network_formation_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_NWK1, ">>zb_nlme_network_formation vbuf %hd", (FMT__H, param));

  if ( buf )
  {
    zb_nlme_network_formation_confirm_t *confirm = (zb_nlme_network_formation_confirm_t *)ZB_BUF_BEGIN(buf);
    ZVUNUSED(confirm);
  }

  TRACE_MSG(TRACE_NWK1, "<<zb_nlme_network_formation", (FMT__0));
}

#endif

/*! @} */
