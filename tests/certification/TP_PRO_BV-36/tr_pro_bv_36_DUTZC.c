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
PURPOSE: TP/PRO/BV-36 Frequency Agility - Receipt of a
Mgmt_NWK_Update_req - ZR, coordinator side
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#define TEST_SCAN_DURATION       5
#define TEST_SCAN_DURATION_ERROR 6
#define TEST_SCAN_COUNT          1

#ifdef ZB_NS_BUILD
#define SEND_TMOUT 1000
#else
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(40000)
#endif


zb_ieee_addr_t g_ieee_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

zb_int_t g_errors = 0;

MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif

  zb_set_default_ffd_descriptor_values(ZB_COORDINATOR);

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;

  /* set ieee addr */
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif

  /* accept only one child */
  ZB_NWK().max_children = 1;

  if ( zdo_dev_start() != RET_OK )
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}



void mgmt_nwk_update_ok_cb(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_nwk_update_notify_hdr_t *notify_resp = (zb_zdo_mgmt_nwk_update_notify_hdr_t *)zdp_cmd;

  TRACE_MSG(TRACE_APS3,
            "notify_resp status %hd, scanned_channels %x %x, total_transmissions %hd, "
            "transmission_failures %hd, scanned_channels_list_count %hd, buf len %hd",
            (FMT__H_D_D_H_H_H_H, notify_resp->status, (zb_uint16_t)notify_resp->scanned_channels,
             *((zb_uint16_t*)&notify_resp->scanned_channels + 1),
             notify_resp->total_transmissions, notify_resp->transmission_failures,
             notify_resp->scanned_channels_list_count, ZB_BUF_LEN(buf)));

  if (notify_resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    TRACE_MSG(TRACE_APS2, "mgmt_nwk_update_notify received, Ok", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "mgmt_nwk_update_notify received, ERROR incorrect status %x",
              (FMT__D, notify_resp->status));
    g_errors++;
  }

  if (!g_errors)
  {
    TRACE_MSG(TRACE_APS2, "Test is finished, status: OK", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_APS2, "Test is finished, status: Failed, errors %d", (FMT__D, g_errors));
  }

  zb_free_buf(buf);
}

void mgmt_nwk_update_error_cb(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_nwk_update_notify_hdr_t *notify_resp = (zb_zdo_mgmt_nwk_update_notify_hdr_t *)zdp_cmd;
  zb_zdo_mgmt_nwk_update_req_t *req;

  if (notify_resp->status == ZB_NWK_STATUS_INVALID_REQUEST)
  {
    TRACE_MSG(TRACE_APS2, "mgmt_nwk_update_notify received, invalid status - Ok", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "mgmt_nwk_update_notify received, ERROR incorrect status %x",
              (FMT__D, notify_resp->status));
    g_errors++;
  }

  req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_req_t);

  req->hdr.scan_channels = ZB_MAC_ALL_CHANNELS_MASK;
  req->hdr.scan_duration = TEST_SCAN_DURATION;
  req->scan_count = TEST_SCAN_COUNT;
  req->update_id = ZB_NIB_UPDATE_ID();

  req->dst_addr = 1; //TODO: take it correctly

  zb_zdo_mgmt_nwk_update_req(param, mgmt_nwk_update_ok_cb);
}

void mgmt_nwk_update_error(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *asdu;

  ZVUNUSED(param);

  asdu = zb_get_out_buf();
  if (!asdu)
  {
    TRACE_MSG(TRACE_ERROR, "out buf alloc failed!", (FMT__0));
  }
  else
  {
    zb_zdo_mgmt_nwk_update_req_t *req;

    req = ZB_GET_BUF_PARAM(asdu, zb_zdo_mgmt_nwk_update_req_t);

    req->hdr.scan_channels = ZB_MAC_ALL_CHANNELS_MASK;
    req->hdr.scan_duration = TEST_SCAN_DURATION_ERROR;
    req->scan_count = TEST_SCAN_COUNT;
    req->update_id = ZB_NIB_UPDATE_ID();
    req->dst_addr = 1; //TODO: take it correctly

    zb_zdo_mgmt_nwk_update_req(ZB_REF_FROM_BUF(asdu), mgmt_nwk_update_error_cb);
  }

}


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ERROR, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    zb_schedule_alarm(mgmt_nwk_update_error, 0, SEND_TMOUT);
  }
  else
  {
    TRACE_MSG(TRACE_APS2, "Device START FAILED", (FMT__0));
  }

  zb_free_buf(buf);
}

