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
#include "zb_secur.h"

#ifdef ZB_NS_BUILD
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000)
#else
#define SEND_TMOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(17000)
#endif

/*! \addtogroup ZB_TESTS */
/*! @{ */

/* For NS build first ieee addr byte should be unique */
#ifdef ZB_NS_BUILD
zb_ieee_addr_t g_ieee_addr = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#else
zb_ieee_addr_t g_ieee_addr = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
#endif

void group_add_conf1(zb_uint8_t param) ZB_CALLBACK;
void group_add_conf2(zb_uint8_t param) ZB_CALLBACK;
void group_add_conf3(zb_uint8_t param) ZB_CALLBACK;
void group_add_conf4(zb_uint8_t param) ZB_CALLBACK;
void zr1_send_grp(zb_uint8_t param) ZB_CALLBACK;
void data_indication(zb_uint8_t param) ZB_CALLBACK;

static zb_ushort_t wants_endp = 66;
static zb_ushort_t was_err = 0;


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
  ZB_INIT("zdo_zr1", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr1", "2", "2");
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);

#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_PAN_ID();
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
    zb_af_set_data_indication(data_indication);
    {
      zb_apsme_add_group_req_t *req;
      zb_buf_reuse(buf);
      req = ZB_GET_BUF_PARAM(buf, zb_apsme_add_group_req_t);
      req->group_address = 10;
      req->endpoint = 66;
      zb_zdo_add_group_req(param, group_add_conf1);
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}


void group_add_conf1(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_add_group_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_req_t);

  req->group_address = 15;
  req->endpoint = 77;
  zb_zdo_add_group_req(param, group_add_conf2);
}


void group_add_conf2(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_add_group_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_req_t);

  req->group_address = 15;
  req->endpoint = 88;
  zb_zdo_add_group_req(param, group_add_conf3);
}


void group_add_conf3(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_add_group_req_t *req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_req_t);

  req->group_address = 15;
  req->endpoint = 99;
  zb_zdo_add_group_req(param, group_add_conf4);
}


void group_add_conf4(zb_uint8_t param) ZB_CALLBACK
{
  ZB_SCHEDULE_ALARM(zr1_send_grp, param, SEND_TMOUT);
}


void zr1_send_grp(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req = ZB_GET_BUF_PARAM(buf, zb_apsde_data_req_t);
  zb_uint8_t *ptr = NULL;
  zb_short_t i;

  ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->addr_mode = ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT;
  req->dst_addr.addr_short = 15;
  req->clusterid = 77;
  req->tx_options = 0;
  req->radius = 0;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 0;
  buf->u.hdr.handle = 0x11;
  for (i = 0 ; i < ZB_TEST_DATA_SIZE ; ++i)
  {
    ptr[i] = i % 32 + '0';
  }
  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}


void data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);

  TRACE_MSG(TRACE_APS3, "apsde_data_indication: param %hd len %hd group %d dst_endp %hd src %d",
            (FMT__H_H_D_H, param, ZB_BUF_LEN(asdu), ind->group_addr, ind->dst_endpoint, ind->src_addr));

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS3, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
    if (ptr[i] != i % 32 + '0')
    {
      TRACE_MSG(TRACE_ERROR, "Bad data %hx %c wants %x %c", (FMT__H_C_D_C, ptr[i], ptr[i],
                              (int)(i % 32 + '0'), (char)i % 32 + '0'));
    }
  }


  if (ind->dst_endpoint != wants_endp)
  {
    TRACE_MSG(TRACE_ERROR, "Got packet to endpoint %hd, wants to %hd", (FMT__H_H, ind->dst_endpoint, wants_endp));
    was_err = 1;
  }
  if (wants_endp == 99)
  {
    TRACE_MSG(TRACE_APS1, "TEST PASSED", (FMT__0));
  }
  if (wants_endp > 99)
  {
    TRACE_MSG(TRACE_APS1, "Extra packet!!", (FMT__0));
  }
  wants_endp += 11;


  zb_free_buf(asdu);
}



/*! @} */
