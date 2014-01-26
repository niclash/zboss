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
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */

/*
  ZE joins to ZC(ZR), then sends APS packet.
*/

/* AIB */
void set_aib_information(zb_uint8_t param);
void set_aib_design_coord(zb_uint8_t param);
void set_aib_ext_pan_id(zb_uint8_t param);
void set_aib_unsupported_attr(zb_uint8_t param);

void get_aib_information(zb_uint8_t param);
void get_aib_design_coord(zb_uint8_t param);
void get_aib_ext_pan_id(zb_uint8_t param);
void get_aib_unsupported_attr(zb_uint8_t param);

/* NIB */
void set_nib_information(zb_uint8_t param);
void set_nib_sequence_number(zb_uint8_t param);
void set_nib_max_broadcast_retries(zb_uint8_t param);
void set_nib_max_children(zb_uint8_t param);
void set_nib_unsupported_attr(zb_uint8_t param);

void get_nib_information(zb_uint8_t param);
void get_nib_sequence_number(zb_uint8_t param);
void get_nib_max_broadcast_retries(zb_uint8_t param);
void get_nib_max_children(zb_uint8_t param);
void get_nib_unsupported_attr(zb_uint8_t param);

/* PIB */
void set_pib_information(zb_uint8_t param);
void set_pib_beacon_payload_length(zb_uint8_t param);
void set_pib_beacon_order(zb_uint8_t param);
void set_pib_bsn(zb_uint8_t param);
void set_pib_unsupported_attr(zb_uint8_t param);

void get_pib_information(zb_uint8_t param);
void get_pib_beacon_payload_length(zb_uint8_t param);
void get_pib_beacon_order(zb_uint8_t param);
void get_pib_bsn(zb_uint8_t param);
void get_pib_unsupported_attr(zb_uint8_t param);


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
  ZB_INIT("start_ze", argv[1], argv[2]);
#else
  ZB_INIT("start_ze", "3", "3");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif

  if (zdo_dev_start() != RET_OK)
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


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    set_aib_information(param);
    ZB_SCHEDULE_ALARM(get_aib_information, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(20000));

    set_nib_information(param);
    ZB_SCHEDULE_ALARM(get_nib_information, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(30000));

    set_pib_information(param);
    ZB_SCHEDULE_ALARM(get_pib_information, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(40000));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

/* AIB */
/* Set AIB */
void set_aib_information(zb_uint8_t param)
{
  ZVUNUSED(param);

  TRACE_MSG(TRACE_APS2, "Set Aib test", (FMT__0));
  TRACE_MSG(TRACE_APS2, "aps_designated_coordinator %hd", (FMT__H, ZB_AIB().aps_designated_coordinator));
  TRACE_MSG(TRACE_APS2, "aps_use_extended_pan_id "TRACE_FORMAT_64"", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));

  ZB_GET_OUT_BUF_DELAYED(set_aib_design_coord);
  ZB_GET_OUT_BUF_DELAYED(set_aib_ext_pan_id);
  ZB_GET_OUT_BUF_DELAYED(set_aib_unsupported_attr);
}

#ifdef KEIL
#pragma OT(0)
#endif
void set_aib_design_coord(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_set_request_t) + sizeof(zb_uint8_t), req);
  req->aib_attr = ZB_APS_AIB_DESIGNATED_COORD;
  req->aib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_apsme_set_request_t);
  *ptr = 1;
  zb_apsme_set_request(param);
}

void set_aib_ext_pan_id(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_ieee_addr_t ieee_addr = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
  zb_apsme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_set_request_t) + sizeof(zb_ieee_addr_t), req);
  req->aib_attr = ZB_APS_AIB_USE_EXT_PANID;
  req->aib_length = sizeof(zb_ieee_addr_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_apsme_set_request_t);
  ZB_EXTPANID_COPY(ptr, ieee_addr);
  zb_apsme_set_request(param);
}

void set_aib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_set_request_t) + sizeof(zb_uint8_t), req);
  req->aib_attr = 0xaa;
  req->aib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_apsme_set_request_t);
  *ptr = 1;
  zb_apsme_set_request(param);
}

/* GET */
void get_aib_information(zb_uint8_t param)
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_APS2, "Get Aib test", (FMT__0));

  TRACE_MSG(TRACE_APS2, "aps_designated_coordinator %hd", (FMT__H, ZB_AIB().aps_designated_coordinator));
  TRACE_MSG(TRACE_APS2, "aps_use_extended_pan_id "TRACE_FORMAT_64"", (FMT__A, TRACE_ARG_64(ZB_AIB().aps_use_extended_pan_id)));

  ZB_GET_OUT_BUF_DELAYED(get_aib_design_coord);
  ZB_GET_OUT_BUF_DELAYED(get_aib_ext_pan_id);
  ZB_GET_OUT_BUF_DELAYED(get_aib_unsupported_attr);
}

void get_aib_design_coord(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_request_t), req);
  req->aib_attr = ZB_APS_AIB_DESIGNATED_COORD;
  zb_apsme_get_request(param);
}

void get_aib_ext_pan_id(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_request_t), req);
  req->aib_attr = ZB_APS_AIB_USE_EXT_PANID;
  zb_apsme_get_request(param);
}

void get_aib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_apsme_get_request_t), req);
  req->aib_attr = 0xaa;
  zb_apsme_get_request(param);
}

/* NIB */
void set_nib_information(zb_uint8_t param)
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_NWK2, "Set Nib test", (FMT__0));

  TRACE_MSG(TRACE_NWK2, "sequence number %hd", (FMT__H, ZB_NIB_SEQUENCE_NUMBER()));
  TRACE_MSG(TRACE_NWK2, "max_broadcast_retries %hd", (FMT__H, ZG->nwk.nib.max_broadcast_retries));
  TRACE_MSG(TRACE_NWK2, "max_children %hd", (FMT__H, ZG->nwk.nib.max_children));

  ZB_GET_OUT_BUF_DELAYED(set_nib_sequence_number);
  ZB_GET_OUT_BUF_DELAYED(set_nib_max_broadcast_retries);
  ZB_GET_OUT_BUF_DELAYED(set_nib_max_children);
  ZB_GET_OUT_BUF_DELAYED(set_nib_unsupported_attr);
}

void set_nib_sequence_number(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER;
  req->attr_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_nlme_set_request_t);
  *ptr = 3;
  zb_nlme_set_request(param);
}

void set_nib_max_broadcast_retries(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES;
  req->attr_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_nlme_set_request_t);
  *ptr = 3;
  zb_nlme_set_request(param);
}

void set_nib_max_children(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_MAX_CHILDREN;
  req->attr_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_nlme_set_request_t);
  *ptr = 3;
  zb_nlme_set_request(param);
}

void set_nib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->nib_attribute = 0xaa;
  req->attr_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_nlme_set_request_t);
  *ptr = 3;
  zb_nlme_set_request(param);
}

void get_nib_information(zb_uint8_t param)
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_NWK2, "Get Nib test", (FMT__0));

  TRACE_MSG(TRACE_NWK2, "sequence number %hd", (FMT__H, ZB_NIB_SEQUENCE_NUMBER()));
  TRACE_MSG(TRACE_NWK2, "max_broadcast_retries %hd", (FMT__H, ZG->nwk.nib.max_broadcast_retries));
  TRACE_MSG(TRACE_NWK2, "max_children %hd", (FMT__H, ZG->nwk.nib.max_children));

  ZB_GET_OUT_BUF_DELAYED(get_nib_sequence_number);
  ZB_GET_OUT_BUF_DELAYED(get_nib_max_broadcast_retries);
  ZB_GET_OUT_BUF_DELAYED(get_nib_max_children);
  ZB_GET_OUT_BUF_DELAYED(get_nib_unsupported_attr);
}

void get_nib_sequence_number(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_request_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_SEQUENCE_NUMBER;
  zb_nlme_get_request(param);
}

void get_nib_max_broadcast_retries(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_request_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_MAX_BROADCAST_RETRIES;
  zb_nlme_get_request(param);
}

void get_nib_max_children(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_request_t), req);
  req->nib_attribute = ZB_NIB_ATTRIBUTE_MAX_CHILDREN;
  zb_nlme_get_request(param);
}

void get_nib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_nlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_nlme_get_request_t), req);
  req->nib_attribute = 0xaa;
  zb_nlme_get_request(param);
}

/* PIB */
void set_pib_information(zb_uint8_t param)
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC2, "Set Pib test", (FMT__0));

  TRACE_MSG(TRACE_MAC2, "mac_beacon_payload_length %hd", (FMT__H, MAC_PIB().mac_beacon_payload_length));
  TRACE_MSG(TRACE_MAC2, "mac_beacon_order %hd", (FMT__H, MAC_PIB().mac_beacon_order));
  TRACE_MSG(TRACE_MAC2, "BSN() %hd", (FMT__H, ZB_MAC_BSN()));

  ZB_GET_OUT_BUF_DELAYED(set_pib_beacon_payload_length);
  ZB_GET_OUT_BUF_DELAYED(set_pib_beacon_order);
  ZB_GET_OUT_BUF_DELAYED(set_pib_bsn);
  ZB_GET_OUT_BUF_DELAYED(set_pib_unsupported_attr);
}
void set_pib_beacon_payload_length(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH;
  req->pib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_mlme_set_request_t);
  *ptr = 4;
  zb_mlme_set_request(param);
}

void set_pib_beacon_order(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BEACON_ORDER;
  req->pib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_mlme_set_request_t);
  *ptr = 4;
  zb_mlme_set_request(param);
}

void set_pib_bsn(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BSN;
  req->pib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_mlme_set_request_t);
  *ptr = 4;
  zb_mlme_set_request(param);
}

void set_pib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_set_request_t *req = NULL;
  zb_uint8_t *ptr = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_set_request_t) + sizeof(zb_uint8_t), req);
  req->pib_attr = 0xaa;
  req->pib_length = sizeof(zb_uint8_t);
  ptr = ((zb_uint8_t*)req) + sizeof(zb_mlme_set_request_t);
  *ptr = 4;
  zb_mlme_set_request(param);
}

void get_pib_information(zb_uint8_t param)
{
  ZVUNUSED(param);
  TRACE_MSG(TRACE_MAC2, "Get Pib test", (FMT__0));

  TRACE_MSG(TRACE_MAC2, "mac_beacon_payload_length %hd", (FMT__H, MAC_PIB().mac_beacon_payload_length));
  TRACE_MSG(TRACE_MAC2, "mac_beacon_order %hd", (FMT__H, MAC_PIB().mac_beacon_order));
  TRACE_MSG(TRACE_MAC2, "BSN() %hd", (FMT__H, ZB_MAC_BSN()));

  ZB_GET_OUT_BUF_DELAYED(get_pib_beacon_payload_length);
  ZB_GET_OUT_BUF_DELAYED(get_pib_beacon_order);
  ZB_GET_OUT_BUF_DELAYED(get_pib_bsn);
  ZB_GET_OUT_BUF_DELAYED(get_pib_unsupported_attr);
}

void get_pib_beacon_payload_length(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_request_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH;
  zb_mlme_get_request(param);
}

void get_pib_beacon_order(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_request_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BEACON_ORDER;
  zb_mlme_get_request(param);
}

void get_pib_bsn(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_request_t), req);
  req->pib_attr = ZB_PIB_ATTRIBUTE_BSN;
  zb_mlme_get_request(param);
}

void get_pib_unsupported_attr(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_mlme_get_request_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_mlme_get_request_t), req);
  req->pib_attr = 0xaa;
  zb_mlme_get_request(param);
}

/*! @} */
