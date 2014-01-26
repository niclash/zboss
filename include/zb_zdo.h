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
PURPOSE: ZDO API
*/

#ifndef ZB_ZDO_H
#define ZB_ZDO_H 1

#include "zb_af_globals.h"

/*! \cond internals_doc */
/*! \addtogroup ZB_ZDO */
/*! @{ */


/**
  Cluster ids for ZDO commands
 */
#define  ZDO_NWK_ADDR_REQ_CLID    0x0
#define  ZDO_IEEE_ADDR_REQ_CLID   0x1
#define  ZDO_NODE_DESC_REQ_CLID   0x2
#define  ZDO_POWER_DESC_REQ_CLID  0x3
#define  ZDO_SIMPLE_DESC_REQ_CLID 0x4
#define  ZDO_ACTIVE_EP_REQ_CLID   0x5
#define  ZDO_MATCH_DESC_REQ_CLID  0x6
#define  ZDO_DEVICE_ANNCE_CLID    0x13
#define  ZDO_SYSTEM_SERVER_DISCOVERY_REQ_CLID 0x15
#define  ZDO_END_DEVICE_BIND_REQ_CLID 0x20
#define  ZDO_BIND_REQ_CLID        0x21
#define  ZDO_UNBIND_REQ_CLID      0x22
#define  ZDO_MGMT_LQI_REQ_CLID    0x31
#define  ZDO_MGMT_LEAVE_REQ_CLID  0x0034
#define  ZDO_MGMT_NWK_UPDATE_REQ_CLID 0x0038
#define  ZDO_MGMT_PERMIT_JOINING_CLID 0x0036


#define  ZDO_NWK_ADDR_RESP_CLID    0x8000
#define  ZDO_IEEE_ADDR_RESP_CLID   0x8001
#define  ZDO_NODE_DESC_RESP_CLID   0x8002
#define  ZDO_POWER_DESC_RESP_CLID  0x8003
#define  ZDO_SIMPLE_DESC_RESP_CLID 0x8004
#define  ZDO_ACTIVE_EP_RESP_CLID   0x8005
#define  ZDO_MATCH_DESC_RESP_CLID  0x8006
#define  ZDO_SYSTEM_SERVER_DISCOVERY_RESP_CLID 0x8015
#define  ZDO_END_DEVICE_BIND_RESP_CLID 0x8020
#define  ZDO_BIND_RESP_CLID        0x8021
#define  ZDO_UNBIND_RESP_CLID      0x8022
#define  ZDO_MGMT_LQI_RESP_CLID    0x8031
#define  ZDO_MGMT_LEAVE_RESP_CLID  0x8034
#define  ZDO_MGMT_NWK_UPDATE_NOTIFY_CLID 0x8038


/*! @} */
/*! \endcond */

/*! \addtogroup zdo_base */
/*! @{ */

/**
   \par Device start

   Startup procedure as defined in 2.5.5.5.6.2    Startup Procedure
*/

/**
   ZDP status values
   (2.4.5 ZDP Enumeration Description)
 */
typedef enum zb_zdp_status_e
{
  ZB_ZDP_STATUS_SUCCESS         = 0x00, /*!< The requested operation or transmission was completed successfully */
  ZB_ZDP_STATUS_INV_REQUESTTYPE = 0x80, /*!< The supplied request type was invalid. */
  ZB_ZDP_STATUS_DEVICE_NOT_FOUND = 0x81, /*!< The requested device did not exist on a device following a child descriptor request to a parent. */
  ZB_ZDP_STATUS_INVALID_EP = 0x82, /*!< The supplied endpoint was equal to 0x00 or between 0xf1 and 0xff. */
  ZB_ZDP_STATUS_NOT_ACTIVE = 0x83, /*!< The requested endpoint is not described by a simple descriptor. */
  ZB_ZDP_STATUS_NOT_SUPPORTED = 0x84, /*!< The requested optional feature is not supported on the target device. */
  ZB_ZDP_STATUS_TIMEOUT = 0x85, /*!< A timeout has occurred with the requested operation. */
  ZB_ZDP_STATUS_NO_MATCH = 0x86, /*!< The end device bind request was unsuccessful due to a failure to match any suitable clusters. */
  ZB_ZDP_STATUS_NO_ENTRY = 0x88, /*!< The unbind request was unsuccessful due to the coordinator or source device not
                                   having an entry in its binding table to unbind. */
  ZB_ZDP_STATUS_NO_DESCRIPTOR = 0x89, /*!< A child descriptor was not available following a discovery request to a parent. */
  ZB_ZDP_STATUS_INSUFFICIENT_SPACE = 0x8a, /*!< The device does not have storage space to support the requested operation. */
  ZB_ZDP_STATUS_NOT_PERMITTED = 0x8b, /*!< The device is not in the proper state to support the requested operation. */
  ZB_ZDP_STATUS_TABLE_FULL = 0x8c, /*!< The device does not have table space to support the operation. */
  ZB_ZDP_STATUS_NOT_AUTHORIZED = 0x8d /*!< The permissions configuration table on the target indicates that the request is not
                                        authorized from this device. */
} ZB_PACKED_STRUCT
zb_zdp_status_t;


/*! @} */


/*! \addtogroup zdo_init */
/*! @{ */


/**
   Typical device start: init, load some parameters from nvram and proceed with startup.

   Startup means either Formation (for ZC), rejoin or discovery/association
   join.
   After startup complete zb_zdo_startup_complete callback is called, so
   application will know when to do some useful things.

   Precondition: stack must be inited by zb_init() call.
   zb_init() loads IB from NVRAM or set its defaults, so caller has a chanse to
   change some parameters.
   Note: ZB is not looped in this routine. Instead, it schedules callback and returns.
   Caller must run zdo_main_loop() after this routine.

   @b Example:
@code
  zb_init("zdo_zc", "1", "1");
  ZB_AIB().aps_designated_coordinator = 1;
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;
  ZG->nwk.nib.max_children = 1;
  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }
@endcode
 */
zb_ret_t zdo_dev_start() ZB_SDCC_REENTRANT;

/**
   Application main loop.

   Must be called after zb_init() and zdo_dev_start().

   @b Example:
@code
  zb_init("zdo_zc", "1", "1");
  ZB_AIB().aps_designated_coordinator = 1;
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;
  ZG->nwk.nib.max_children = 1;
  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }
@endcode
 */
void zdo_main_loop();


/**
   Callback which will be called after device startup complete.

   Must be defined in the application.

   @param param - ref to buffer with startup status

@b Example:
@code
void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %hd", (FMT__D, buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(data_indication);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTE FAILED status %hd", (FMT__D, buf->u.hdr.status));
  }
  zb_free_buf(buf);
}
@endcode
 */
void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK;


/*! @} */


/*! \addtogroup af_api */
/*! @{ */



/**
   This function setup user callback to be called for APS data packets not
   parsed internally.
   To be used mainly for tests.

   @param cb - callback to call when AF got APS packet to the endpoint is has no
   explicit handler for. \see zb_apsde_data_indication_t

   @b Example:
@code

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %hd", (FMT__D, buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    zb_af_set_data_indication(data_indication);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %hd", (FMT__D, buf->u.hdr.status));
  }
  zb_free_buf(buf);
}

void data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ushort_t i;
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = ZB_GET_BUF_PARAM(asdu, zb_apsde_data_indication_t);

  ptr = ZB_APS_HDR_CUT(asdu);

  TRACE_MSG(TRACE_APS3, "apsde_data_indication: packet %p len %hd status 0x%hx from %d",
            (FMT__P_D_D_D, asdu, ZB_BUF_LEN(asdu), asdu->u.hdr.status, ind->src_addr));

  for (i = 0 ; i < ZB_BUF_LEN(asdu) ; ++i)
  {
    TRACE_MSG(TRACE_APS3, "%x %c", (FMT__D_C, (int)ptr[i], ptr[i]));
  }
  zb_free_buf(apsdu);
}

@endcode
*/
void zb_af_set_data_indication(zb_callback_t cb);


/*! @} */


/*! \addtogroup zdo_disc */
/*! @{ */


/**
   2.4.3.1, 2.4.4.1
*/
#define ZB_ZDO_SINGLE_DEVICE_RESP   0 /*!< Single device response */
#define ZB_ZDO_EXTENDED_DEVICE_RESP 1 /*!< Extended response */

/**
   NWK_addr_req command primitive.
 */
typedef struct zb_zdo_nwk_addr_req_s
{
  zb_ieee_addr_t   ieee_addr;    /*!< The IEEE address to be matched by the
                                   Remote Device  */
  zb_uint8_t       request_type; /*!< Request type for this command:
                                   0x00  Single device response
                                   0x01  Extended response  */
  zb_uint8_t       start_index; /*!< If the Request type for this command is
                                  Extended response, the StartIndex
                                  provides the starting index for the
                                  requested elements of the associated
                                  devices list  */
} ZB_PACKED_STRUCT zb_zdo_nwk_addr_req_t;

/**
   Parameters for nwk_addr_req command
*/
typedef struct zb_zdo_nwk_addr_req_param_s
{
  zb_uint16_t      dst_addr;     /*!< Destinitions address */
  zb_ieee_addr_t   ieee_addr;    /*!< The IEEE address to be matched by the
                                   Remote Device  */
  zb_uint8_t       request_type; /*!< Request type for this command:
                                   0x00  Single device response
                                   0x01  Extended response  */
  zb_uint8_t       start_index; /*!< If the Request type for this command is
                                  Extended response, the StartIndex
                                  provides the starting index for the
                                  requested elements of the associated
                                  devices list  */
}
zb_zdo_nwk_addr_req_param_t;

typedef struct zb_zdo_nwk_addr_resp_head_s
{
  zb_uint8_t status; /*!< The status of the NWK_addr_req command. */
  zb_ieee_addr_t ieee_addr; /*!< 64-bit address for the Remote Device. */
  zb_uint16_t nwk_addr; /*!< 16-bit address for the Remote Device. */
}
ZB_PACKED_STRUCT
zb_zdo_nwk_addr_resp_head_t;


/**
   NWK_addr_req primitive.

   @param param - index of buffer with primitive parameters - \see zb_zdo_nwk_addr_req_param_t
   @param cb    - user's function to call when got response from the
                  remote. \see zb_zdo_nwk_addr_resp_head_t passed to cb as parameter.

@b Example:
@code
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_req_param_t *req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_nwk_addr_req_param_t);

  req_param->dst_addr = 0; // send req to ZC
  zb_address_ieee_by_ref(req_param->ieee_addr, short_addr);
  req_param->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
  req_param->start_index = 0;
  zb_zdo_nwk_addr_req(param, zb_get_peer_short_addr_cb);
}

void zb_get_peer_short_addr_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  zb_uint16_t nwk_addr;
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));

  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2, "resp status %hd, nwk addr %d", (FMT__H_D, resp->status, resp->nwk_addr));
  ZB_DUMP_IEEE_ADDR(resp->ieee_addr);
  if (resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);
    zb_address_update(ieee_addr, nwk_addr, ZB_TRUE, &addr_ref);
  }
  zb_free_buf(buf);
}
@endcode
*/
void zb_zdo_nwk_addr_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;


/**
   Parameters of IEEE_addr_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_ieee_addr_req_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
  zb_uint8_t       request_type; /*!< Request type for this command:
                                   0x00  Single device response
                                   0x01  Extended response  */
  zb_uint8_t       start_index; /*!< If the Request type for this command is
                                  Extended response, the StartIndex
                                  provides the starting index for the
                                  requested elements of the associated
                                  devices list  */
} ZB_PACKED_STRUCT zb_zdo_ieee_addr_req_t;


/**
   IEEE_addr_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_ieee_addr_req_t. Parameters mut be
   put into buffer as data (allocated).
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code
{
  zb_zdo_ieee_addr_req_t *req = NULL;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

  req->nwk_addr = ind->src_addr;
  req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
  req->start_index = 0;
  zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
}

void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  zb_uint16_t nwk_addr;
  zb_address_ieee_ref_t addr_ref;

  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));

  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2, "resp status %hd, nwk addr %d", (FMT__H_D, resp->status, resp->nwk_addr));
  ZB_DUMP_IEEE_ADDR(resp->ieee_addr);
  if (resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);
    zb_address_update(ieee_addr, nwk_addr, ZB_TRUE, &addr_ref);
  }
  zb_free_buf(buf);
}

@endcode
*/
void zb_zdo_ieee_addr_req(zb_uint8_t param, zb_callback_t cb);


/**
   Parameters of Node_desc_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_node_desc_req_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
} ZB_PACKED_STRUCT zb_zdo_node_desc_req_t;


/**
   Header of Node_desc_resp primitive.
 */
typedef struct zb_zdo_desc_resp_hdr_s
{
  zb_zdp_status_t status;   /*!< The status of the Desc_req command */
  zb_uint16_t     nwk_addr; /*!< NWK address for the request  */
} ZB_PACKED_STRUCT
zb_zdo_desc_resp_hdr_t;

/**
   Parameters of Node_desc_resp primitive.
 */
typedef struct zb_zdo_node_desc_resp_s
{
  zb_zdo_desc_resp_hdr_t hdr;  /*!< header for response */
  zb_af_node_desc_t node_desc; /*!< Node Descriptor */
} ZB_PACKED_STRUCT
zb_zdo_node_desc_resp_t;

/**
   Header of Node_desc_resp primitive.
 */
typedef struct zb_zdo_simple_desc_resp_hdr_s
{
  zb_zdp_status_t status;   /*!< The status of the Desc_req command */
  zb_uint16_t     nwk_addr; /*!< NWK address for the request  */
  zb_uint8_t      length;   /*!< Length of the simple descriptor */
} ZB_PACKED_STRUCT
zb_zdo_simple_desc_resp_hdr_t;

/**
   Parameters of simple_desc_resp primitive.
 */
typedef struct zb_zdo_simple_desc_resp_s
{
  zb_zdo_simple_desc_resp_hdr_t hdr;  /*!< header for response */
  zb_af_simple_desc_1_1_t simple_desc; /*!< Simple Descriptor */
} ZB_PACKED_STRUCT
zb_zdo_simple_desc_resp_t;

/**
   Parameters of Power_desc_resp primitive.
 */
typedef struct zb_zdo_power_desc_resp_s
{
  zb_zdo_desc_resp_hdr_t hdr;  /*!< header for response */
  zb_af_node_power_desc_t power_desc; /*!< Power Descriptor */
} ZB_PACKED_STRUCT
zb_zdo_power_desc_resp_t;


/**
   Node_desc_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_node_desc_req_t. Parameters must be
   put into buffer as data (allocated).
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code

{
  ZB_BUF_INITIAL_ALLOC(asdu, sizeof(zb_zdo_node_desc_req_t), req);
  req->nwk_addr = 0; //send to coordinator
  zb_zdo_node_desc_req(ZB_REF_FROM_BUF(asdu), node_desc_callback);
}


void node_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_node_desc_resp_t *resp = (zb_zdo_node_desc_resp_t*)(zdp_cmd);
  zb_zdo_power_desc_req_t *req;

  TRACE_MSG(TRACE_APS1, "node_desc_callback: status %hd, addr 0x%x",
            (FMT__H_D, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
    g_error++;
  }

  TRACE_MSG(TRACE_APS1, "logic type %hd, aps flag %hd, frequency %hd",
            (FMT__H_H_H, ZB_GET_NODE_DESC_LOGICAL_TYPE(&resp->node_desc), ZB_GET_NODE_DESC_APS_FLAGS(&resp->node_desc),
             ZB_GET_NODE_DESC_FREQ_BAND(&resp->node_desc)));
  if (ZB_GET_NODE_DESC_LOGICAL_TYPE(&resp->node_desc) != 0 || ZB_GET_NODE_DESC_APS_FLAGS(&resp->node_desc) != 0 ||
      ZB_GET_NODE_DESC_FREQ_BAND(&resp->node_desc) != ZB_FREQ_BAND_2400 )
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect type/flags/freq", (FMT__0));
    g_error++;
  }

  TRACE_MSG(TRACE_APS1, "mac cap 0x%hx, manufact code %hd, max buf %hd, max transfer %hd",
            (FMT__H_H_H_H, resp->node_desc.mac_capability_flags, resp->node_desc.manufacturer_code,
             resp->node_desc.max_buf_size, resp->node_desc.max_incoming_transfer_size));
  if ((resp->node_desc.mac_capability_flags & 0xB) != 0xB || (resp->node_desc.mac_capability_flags & ~0x4f) != 0 ||
      resp->node_desc.manufacturer_code != 0 ||
      resp->node_desc.max_incoming_transfer_size != 0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect cap/manuf code/max transfer", (FMT__0));
    g_error++;
  }

  zb_free_buf(buf);
}

@endcode
*/
void zb_zdo_node_desc_req(zb_uint8_t param, zb_callback_t cb);


/**
   Parameters of Power_desc_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_power_desc_req_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
} ZB_PACKED_STRUCT zb_zdo_power_desc_req_t;

/**
   Power_desc_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_power_desc_req_t. Parameters must be
   put into buffer as data (allocated).
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code
{
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_power_desc_req_t), req);
  req->nwk_addr = 0; //send to coordinator
  zb_zdo_power_desc_req(ZB_REF_FROM_BUF(buf), node_power_desc_callback);
}


void node_power_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_power_desc_resp_t *resp = (zb_zdo_power_desc_resp_t*)(zdp_cmd);
  zb_zdo_simple_desc_req_t *req;

  TRACE_MSG(TRACE_APS1, " node_power_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
    g_error++;
  }

  TRACE_MSG(TRACE_APS1, "power mode %hd, avail power src %hd, cur power src %hd, cur power level %hd",
            (FMT__H_H_H_H, ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc),
             ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(&resp->power_desc)));
  // PowerDescriptor=Current power mode=0b0000, Available power mode=0b0111, Current
  // power source=0b0001, Current power source level=0b110001
  if (ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc) != 0 ||
      ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(&resp->power_desc) != 0x7 ||
      ZB_GET_POWER_DESC_CUR_POWER_SOURCE(&resp->power_desc) != 0x1 ||
      ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(&resp->power_desc) != 0xC)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect power desc", (FMT__0));
    g_error++;
  }
  zb_free_buf(buf);
}

@endcode
*/
void zb_zdo_power_desc_req(zb_uint8_t param, zb_callback_t cb);



/**
   Parameters of Power_desc_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_simple_desc_req_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
  zb_uint8_t       endpoint;    /*!< The endpoint on the destination  */
} ZB_PACKED_STRUCT zb_zdo_simple_desc_req_t;


/**
   Simple_desc_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_simple_desc_req_t.
   @param cb    - user's function to call when got response from the remote.

   @b Example:

@code
{
  zb_zdo_simple_desc_req_t *req;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
  req->nwk_addr = 0; //send to coordinator
  req->endpoint = 1;
  zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf), simple_desc_callback);
}


void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)(zdp_cmd);
  zb_uint_t i;
  zb_zdo_active_ep_req_t *req;

  TRACE_MSG(TRACE_APS1, "simple_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
    g_error++;
  }

//simple descriptor for test SimpleDescriptor=
//Endpoint=0x01, Application profile identifier=0x0103, Application device
//identifier=0x0000, Application device version=0b0000, Application
//flags=0b0000, Application input cluster count=0x0A, Application input
//cluster list=0x00 0x03 0x04 0x38 0x54 0x70 0x8c 0xc4 0xe0 0xff,
//Application output cluster count=0x0A, Application output cluster
//list=0x00 0x01 0x02 0x1c 0x38 0x70 0x8c 0xa8 0xc4 0xff

  TRACE_MSG(TRACE_APS1, "ep %hd, app prof %d, dev id %d, dev ver %hd, input count 0x%hx, output count 0x%hx",
            (FMT__H_D_D_H_H_H, resp->simple_desc.endpoint, resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id, resp->simple_desc.app_device_version,
           resp->simple_desc.app_input_cluster_count, resp->simple_desc.app_output_cluster_count));

  TRACE_MSG(TRACE_APS1, "clusters:", (FMT__0));
  for(i = 0; i < resp->simple_desc.app_input_cluster_count + resp->simple_desc.app_output_cluster_count; i++)
  {
    TRACE_MSG(TRACE_APS1, " 0x%hx", (FMT__H, *(resp->simple_desc.app_cluster_list + i)));
  }
  zb_free_buf(buf);
}

@endcode
*/
void zb_zdo_simple_desc_req(zb_uint8_t param, zb_callback_t cb);


/**
   Parameters of Active_desc_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_active_ep_req_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
} ZB_PACKED_STRUCT zb_zdo_active_ep_req_t;


/**
   Active EP response
 */
typedef struct zb_zdo_ep_resp_s
{
  zb_uint8_t status;    /*!< The status of the Active_EP_req command. */
  zb_uint16_t nwk_addr; /*!< NWK address for the request. */
  zb_uint8_t ep_count;  /*!< The count of active endpoints on the Remote Device. */
}
ZB_PACKED_STRUCT
zb_zdo_ep_resp_t;

/**
   Active_desc_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_active_ep_req_t. Parameters must be
   put into buffer as data (allocated).
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code
{
  zb_zdo_active_ep_req_t *req;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);
  req->nwk_addr = 0; //coord addr
  zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);

void active_ep_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)zdp_cmd;
  zb_uint8_t *ep_list = zdp_cmd + sizeof(zb_zdo_ep_resp_t);

  TRACE_MSG(TRACE_APS1, "active_ep_callback status %hd, addr 0x%x",
            (FMT__H, resp->status, resp->nwk_addr));

  if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
    g_error++;
  }

  TRACE_MSG(TRACE_APS1, " ep count %hd, ep %hd", (FMT__H_H, resp->ep_count, *ep_list));
  if (resp->ep_count != 1 || *ep_list != 1)
  {
    TRACE_MSG(TRACE_APS3, "Error incorrect ep count or ep value", (FMT__0));
    g_error++;
  }

  zb_free_buf(buf);
}
@endcode
*/
void zb_zdo_active_ep_req(zb_uint8_t param, zb_callback_t cb);

/**
   Parameters of match_desc_req primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_match_desc_param_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
  zb_uint16_t      profile_id;  /*!< Profile ID to be matched at the
                                  destination.  */
  zb_uint8_t       num_in_clusters; /*!< The number of Input Clusters
                                      provided for matching within the
                                      InClusterList.  */
  zb_uint8_t       num_out_clusters; /*!< The number of Output Clusters
                                       provided for matching within
                                       OutClusterList.  */
  zb_uint16_t      cluster_list[1]; /*!< variable size: [num_in_clusters] +  [num_out_clusters]
                                         List of Input ClusterIDs to be used
                                         for matching; the InClusterList is
                                         the desired list to be matched by
                                         the Remote Device (the elements
                                         of the InClusterList are the
                                         supported output clusters of the
                                         Local Device).
                                         List of Output ClusterIDs to be
                                         used for matching; the
                                         OutClusterList is the desired list to
                                         be matched by the Remote Device
                                         (the elements of the
                                         OutClusterList are the supported
                                         input clusters of the Local
                                         Device). */
}
ZB_PACKED_STRUCT
zb_zdo_match_desc_param_t;

/**
   Match_desc_req head
 */
typedef struct zb_zdo_match_desc_req_head_s
{
  zb_uint16_t      nwk_addr;    /*!< NWK address that is used for IEEE
                                  address mapping.  */
  zb_uint16_t      profile_id;  /*!< Profile ID to be matched at the
                                  destination.  */
  zb_uint8_t       num_in_clusters; /*!< The number of Input Clusters
                                      provided for matching within the
                                      InClusterList.  */
}
ZB_PACKED_STRUCT
zb_zdo_match_desc_req_head_t;

/**
   Match_desc_req tail
 */
typedef struct zb_zdo_match_desc_req_tail_s
{
  zb_uint8_t       num_out_clusters; /*!< The number of Output Clusters
                                       provided for matching within
                                       OutClusterList.  */
}
ZB_PACKED_STRUCT
zb_zdo_match_desc_req_tail_t;

/**
   2.4.4.1.7 Match_Desc_rsp response structure
 */
typedef struct zb_zdo_match_desc_resp_s
{
  zb_uint8_t status;    /*!< The status of the Match_Desc_req command.*/
  zb_uint16_t nwk_addr; /*!< NWK address for the request. */
  zb_uint8_t match_len; /*!< The count of endpoints on the Remote Device that match the
                          request criteria.*/
}
ZB_PACKED_STRUCT
zb_zdo_match_desc_resp_t;


/**
   Match_desc_req primitive.

   @param param - index of buffer with primitive parameters \see zb_zdo_match_desc_param_t.
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code
{
  zb_zdo_match_desc_param_t *req;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_match_desc_param_t) + (2 + 3) * sizeof(zb_uint16_t), req);

  req->nwk_addr = 0; //send to coordinator
  req->profile_id = 0x103;
  req->num_in_clusters = 2;
  req->num_out_clusters = 3;
  req->cluster_list[0] = 0x54;
  req->cluster_list[1] = 0xe0;

  req->cluster_list[2] = 0x1c;
  req->cluster_list[3] = 0x38;
  req->cluster_list[4] = 0xa8;

  zb_zdo_match_desc_req(param, match_desc_callback);
}

void match_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_match_desc_resp_t *resp = (zb_zdo_match_desc_resp_t*)zdp_cmd;
  zb_uint8_t *match_list = (zb_uint8_t*)(resp + 1);

  TRACE_MSG(TRACE_APS1, "match_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->status, resp->nwk_addr));
  if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
    g_error++;
  }
  //asdu=Match_Descr_rsp(Status=0x00=Success, NWKAddrOfInterest=0x0000,
  //MatchLength=0x01, MatchList=0x01)
  TRACE_MSG(TRACE_APS1, "match_len %hd, list %hd ", (FMT__H_H, resp->match_len, *match_list));
  if (resp->match_len != 1 || *match_list != 1)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect match result", (FMT__0));
    g_error++;
  }
  zb_free_buf(buf);
}
@endcode
*/
void zb_zdo_match_desc_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;


/**
   Request parameters for 2.4.3.1.13 System_Server_Discovery_req
 */
typedef struct zb_zdo_system_server_discovery_req_s
{
  zb_uint16_t server_mask; /*!< Server mask for device discovery */
}
ZB_PACKED_STRUCT
zb_zdo_system_server_discovery_req_t;

/**
   Parameters for 2.4.3.1.13 System_Server_Discovery_req call
 */
typedef zb_zdo_system_server_discovery_req_t zb_zdo_system_server_discovery_param_t;


/**
   Response parameters for 2.4.4.1.10 System_Server_Discovery_rsp
 */
typedef struct zb_zdo_system_server_discovery_resp_s
{
  zb_uint8_t status;       /*!< Status of the operation */
  zb_uint16_t server_mask; /*!< Mask of the supported features */
}
ZB_PACKED_STRUCT
zb_zdo_system_server_discovery_resp_t;


/**
   Performs System_Server_Discovery_req
   @param param - index of buffer with request parameters \see zb_zdo_system_server_discovery_param_t
   @param cb    - user's function to call when got response from the
   remote. \see zb_zdo_system_server_discovery_resp_t

   @b Example:
@code
{
  zb_zdo_system_server_discovery_param_t *req_param;

  req_param = ZB_GET_BUF_PARAM(asdu, zb_zdo_system_server_discovery_param_t);
  req_param->server_mask = ZB_NETWORK_MANAGER;

  zb_zdo_system_server_discovery_req(ZB_REF_FROM_BUF(asdu), get_nwk_manager_cb);
}

void get_nwk_manager_cb(zb_uint8_t param)
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_system_server_discovery_resp_t *resp = (zb_zdo_system_server_discovery_resp_t*)(zdp_cmd);

  if (resp->status == ZB_ZDP_STATUS_SUCCESS && resp->server_mask & ZB_NETWORK_MANAGER )
  {
    TRACE_MSG(TRACE_APS3, "system_server_discovery received, status: OK", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "ERROR receiving system_server_discovery status %x, mask %x",
              (FMT__D_D, resp->status, resp->server_mask));
  }
  zb_free_buf(buf);
}

@endcode
 */
void zb_zdo_system_server_discovery_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;


/*! @} */


/*! \addtogroup zdo_mgmt */
/*! @{ */


/**
   Header of parameters for Mgmt_NWK_Update_req
*/
typedef struct zb_zdo_mgmt_nwk_update_req_hdr_s
{
  zb_uint32_t scan_channels;   /*!< Channels bitmask */
  zb_uint8_t scan_duration;    /*!< A value used to calculate the
                               * length of time to spend scanning
                               * each channel. */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_nwk_update_req_hdr_t;

/**
   Parameters for Mgmt_NWK_Update_req
*/
typedef struct zb_zdo_mgmt_nwk_update_req_s
{
  zb_zdo_mgmt_nwk_update_req_hdr_t hdr; /*!< Request header */
  zb_uint8_t scan_count;       /*!< This field represents the number
                                * of energy scans to be conducted and reported */
  zb_uint8_t update_id;     /*!< This value is set by the Network
                               * Channel Manager prior to sending
                               * the message. This field shall only
                               * be present of the ScanDuration is 0xfe or 0xff */
  zb_uint16_t manager_addr; /*!< This field shall be present only
                               * if the ScanDuration is set to 0xff,
                               * and, where present, indicates the
                               * NWK address for the device with the
                               * Network Manager bit set in its Node Descriptor. */
  zb_uint16_t dst_addr;     /*!< Destinition address */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_nwk_update_req_t;


/**
   Header parameters for mgmt_nwk_update_notify
*/
typedef struct zb_zdo_mgmt_nwk_update_notify_hdr_s
{
  zb_uint8_t status;     /*!< The status of the Mgmt_NWK_Update_notify command. */
  zb_uint32_t scanned_channels;  /*!< List of channels scanned by the request */
  zb_uint16_t total_transmissions;  /*!< Count of the total transmissions reported by the device */
  zb_uint16_t transmission_failures;  /*!< Sum of the total transmission failures reported by the device */
  zb_uint8_t scanned_channels_list_count;  /*!< The list shall contain the number of records
                                            * contained in the EnergyValues parameter. */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_nwk_update_notify_hdr_t;

/**
   Parameters for mgmt_nwk_update_notify
*/
typedef struct zb_zdo_mgmt_nwk_update_notify_param_s
{
  zb_zdo_mgmt_nwk_update_notify_hdr_t hdr;             /*!< Fixed parameters set */
  zb_uint8_t energy_values[ZB_MAC_SUPPORTED_CHANNELS]; /*!< ed scan values */
  zb_uint16_t dst_addr;  /*!< destinition address */
  zb_uint8_t tsn;        /*!< tsn value */
}
zb_zdo_mgmt_nwk_update_notify_param_t;

/**
   Performs Mgmt_NWK_Update_req request

   @param param - index of buffer with call parameters. Parameters mut be
   put into buffer as parameters. \see zb_zdo_mgmt_nwk_update_req_t
   @param cb    - user's function to call when got response from the remote.
   \see zb_zdo_mgmt_nwk_update_notify_hdr_t

   @b Example:
@code
{
  zb_zdo_mgmt_nwk_update_req_t *req;

  req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_nwk_update_req_t);

  req->hdr.scan_channels = ZB_MAC_ALL_CHANNELS_MASK;
  req->hdr.scan_duration = TEST_SCAN_DURATION;
  req->scan_count = TEST_SCAN_COUNT;
  req->update_id = ZB_NIB_UPDATE_ID();

  req->dst_addr = 0;

  zb_zdo_mgmt_nwk_update_req(param, mgmt_nwk_update_ok_cb);
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
    TRACE_MSG(TRACE_APS3, "mgmt_nwk_update_notify received, Ok", (FMT__0));
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "mgmt_nwk_update_notify received, ERROR incorrect status %x",
              (FMT__D, notify_resp->status));
  }

  zb_free_buf(buf);
}
@endcode
 */
void zb_zdo_mgmt_nwk_update_req(zb_uint8_t param, zb_callback_t cb);


/**
   Parameters for 2.4.3.3.2 Mgmt_Lqi_req
 */
typedef struct zb_zdo_mgmt_lqi_param_s
{
  zb_uint8_t start_index; /*!< Starting Index for the requested elements
                           * of the Neighbor Table */
  zb_uint16_t dst_addr;   /*!< destinition address */
}
zb_zdo_mgmt_lqi_param_t;

/**
   Request for 2.4.3.3.2 Mgmt_Lqi_req
 */
typedef struct zb_zdo_mgmt_lqi_req_s
{
  zb_uint8_t start_index; /*!< Starting Index for the requested elements
                           * of the Neighbor Table */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_lqi_req_t;

/**
   Response for 2.4.4.3.2 Mgmt_Lqi_rsp
 */
typedef struct zb_zdo_mgmt_lqi_resp_s
{
  zb_uint8_t status; /*!< The status of the Mgmt_Lqi_req command.*/
  zb_uint8_t neighbor_table_entries; /*!< Total number of Neighbor
                                      * Table entries within the Remote Device */
  zb_uint8_t start_index; /*!< Starting index within the Neighbor
                           * Table to begin reporting for the NeighborTableList.*/
  zb_uint8_t neighbor_table_list_count; /*!< Number of Neighbor Table
                                         * entries included within NeighborTableList*/
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_lqi_resp_t;


/* bits 0 - 1, mask 0x3 */
#define ZB_ZDO_RECORD_SET_DEVICE_TYPE(var, type) ( var &= ~3, var |= type )
#define ZB_ZDO_RECORD_GET_DEVICE_TYPE(var) ( var & 3 )

/* bits 2 - 3, mask 0xC */
#define ZB_ZDO_RECORD_SET_RX_ON_WHEN_IDLE(var, type) ( var &= ~0xC, var |= (type << 2) )
#define ZB_ZDO_RECORD_GET_RX_ON_WHEN_IDLE(var) ( (var & 0xC) >> 2 )

/* bits 4 - 6, mask 0x70 */
#define ZB_ZDO_RECORD_SET_RELATIONSHIP(var, type) ( var &= ~0x70, var |= (type << 4) )
#define ZB_ZDO_RECORD_GET_RELATIONSHIP(var) ( (var & 0x70) >> 4 )

/**
   NeighborTableList Record Format for mgmt_lqi_resp
 */
typedef struct zb_zdo_neighbor_table_record_s
{
  zb_ext_pan_id_t ext_pan_id;   /*!< The 64-bit extended PAN
                                 * identifier of the neighboring device.*/
  zb_ieee_addr_t  ext_addr;     /*!< 64-bit IEEE address that is
                                 * unique to every device.*/
  zb_uint16_t     network_addr; /*!< The 16-bit network address of the
                                 * neighboring device */
  zb_uint8_t      type_flags;   /*!< device type, rx_on_when_idle,
                                 * relationship */
  zb_uint8_t      permit_join;  /*!< An indication of whether the
                                 * neighbor device is accepting join requests*/
  zb_uint8_t      depth;        /*!< The tree depth of the neighbor device. */
  zb_uint8_t      lqi;          /*!< The estimated link quality for RF
                                 * transmissions from this device */
}
ZB_PACKED_STRUCT
zb_zdo_neighbor_table_record_t;


/**
   Sends 2.4.3.3.2 Mgmt_Lqi_req
   @param param - index of buffer with Lqi request parameters. \see zb_zdo_mgmt_lqi_param_t
   @param cb    - user's function to call when got response from the remote.
   \see zb_zdo_mgmt_lqi_resp_t \see zb_zdo_neighbor_table_record_t

   @b Example:
@code
{
  zb_zdo_mgmt_lqi_param_t *req_param;

  req_param = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_lqi_param_t);
  req_param->start_index = 0;
  req_param->dst_addr = 0; //coord short addr

  zb_zdo_mgmt_lqi_req(ZB_REF_FROM_BUF(buf), get_lqi_cb);
}


void get_lqi_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_lqi_resp_t *resp = (zb_zdo_mgmt_lqi_resp_t*)(zdp_cmd);
  zb_zdo_neighbor_table_record_t *record = (zb_zdo_neighbor_table_record_t*)(resp + 1);
  zb_uint_t i;

  TRACE_MSG(TRACE_APS1, "get_lqi_cb status %hd, neighbor_table_entries %hd, start_index %hd, neighbor_table_list_count %d",
            (FMT__H_H_H_H, resp->status, resp->neighbor_table_entries, resp->start_index, resp->neighbor_table_list_count));

  for (i = 0; i < resp->neighbor_table_list_count; i++)
  {
    TRACE_MSG(TRACE_APS1, "#%hd: long addr " TRACE_FORMAT_64 " pan id " TRACE_FORMAT_64,
              (FMT__H_A_A, i, TRACE_ARG_64(record->ext_addr), TRACE_ARG_64(record->ext_pan_id)));

    TRACE_MSG(TRACE_APS1,
      "#%hd: network_addr %d, dev_type %hd, rx_on_wen_idle %hd, relationship %hd, permit_join %hd, depth %hd, lqi %hd",
      (FMT_H_D_H_H_H_H_H_H, i, record->network_addr,
       ZB_ZDO_RECORD_GET_DEVICE_TYPE(record->type_flags),
       ZB_ZDO_RECORD_GET_RX_ON_WHEN_IDLE(record->type_flags),
       ZB_ZDO_RECORD_GET_RELATIONSHIP(record->type_flags),
       record->permit_join, record->depth, record->lqi));

    record++;
  }
}

@endcode
*/
void zb_zdo_mgmt_lqi_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;


/**
   Parameters for 2.4.3.2.2 Bind_req API call
 */
typedef struct zb_zdo_bind_req_param_s
{
  zb_ieee_addr_t src_address; /*!< The IEEE address for the source. */
  zb_uint8_t src_endp;  /*!< The source endpoint for the binding entry. */
  zb_uint16_t cluster_id;  /*!< The identifier of the cluster on the
                             source device that is bound to the destination. */
  zb_uint8_t dst_addr_mode;  /*!< The addressing mode for the
                               destination address used in this
                               command. This field can take one of
                               the non-reserved values from the
                               following list:
                               0x00 = reserved
                               0x01 = 16-bit group address for
                               DstAddress and DstEndp not present
                               0x02 = reserved
                               0x03 = 64-bit extended address for
                               DstAddress and DstEndp present
                               0x04 . 0xff = reserved*/
  union zb_addr_u dst_address;  /*!< The destination address for the
                                 * binding entry. */
  zb_uint8_t dst_endp;  /*!< This field shall be present only if the
                          DstAddrMode field has a value of
                          0x03 and, if present, shall be the
                          destination endpoint for the binding
                          entry. */
  zb_uint16_t req_dst_addr; /*!< Destinition address of the request */
}
zb_zdo_bind_req_param_t;


/**
   2.4.3.2.2 Bind_req request head send to the remote
 */
typedef struct zb_zdo_bind_req_head_s
{
  zb_ieee_addr_t src_address; /*!< The IEEE address for the source. */
  zb_uint8_t src_endp;  /*!< The source endpoint for the binding entry. */
  zb_uint16_t cluster_id;  /*!< The identifier of the cluster on the
                             source device that is bound to the destination. */
  zb_uint8_t dst_addr_mode;  /*!< The addressing mode for the
                               destination address used in this
                               command. This field can take one of
                               the non-reserved values from the
                               following list:
                               0x00 = reserved
                               0x01 = 16-bit group address for
                               DstAddress and DstEndp not present
                               0x02 = reserved
                               0x03 = 64-bit extended address for
                               DstAddress and DstEndp present
                               0x04 . 0xff = reserved*/
}
ZB_PACKED_STRUCT
zb_zdo_bind_req_head_t;

/**
   2.4.3.2.2 Bind_req request tail 1st variant send to the remote
 */
typedef struct zb_zdo_bind_req_tail_1_s
{
  zb_uint16_t dst_addr; /*!< The destination address for the
                         * binding entry. */
}
ZB_PACKED_STRUCT
zb_zdo_bind_req_tail_1_t;

/**
   2.4.3.2.2 Bind_req request tail 2nd variant send to the remote
 */
typedef struct zb_zdo_bind_req_tail_2_s
{
  zb_ieee_addr_t dst_addr; /*!< The destination address for the
                            * binding entry. */
  zb_uint8_t dst_endp;  /*!< The destination address for the
                         * binding entry. */
}
ZB_PACKED_STRUCT
zb_zdo_bind_req_tail_2_t;

typedef struct zb_zdo_bind_resp_s
{
  zb_uint8_t status;
}
ZB_PACKED_STRUCT
zb_zdo_bind_resp_t;


/**
   Bind_req request

   @param param - index of buffer with request. \see zb_apsme_binding_req_t
   @param cb    - user's function to call when got response from the
   remote. \see zb_zdo_bind_resp_t

   @b Example:
@code
{
  zb_apsme_binding_req_t *req;

  req = ZB_GET_BUF_PARAM(buf, zb_apsme_binding_req_t);
  ZB_MEMCPY(&req->src_addr, &g_ieee_addr, sizeof(zb_ieee_addr_t));
  req->src_endpoint = i;
  req->clusterid = 1;
  req->addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
  ZB_MEMCPY(&req->dst_addr.addr_long, &g_ieee_addr_d, sizeof(zb_ieee_addr_t));
  req->dst_endpoint = 240;

  zb_zdo_bind_req(ZB_REF_FROM_BUF(buf), zb_bind_callback);
  ret = buf->u.hdr.status;
  if (ret == RET_TABLE_FULL)
  {
    TRACE_MSG(TRACE_ERROR, "TABLE FULL %d", (FMT__D, ret));
  }
}

void zb_bind_callback(zb_uint8_t param)
{
  zb_ret_t ret = RET_OK;
  zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t *aps_body = NULL;

  aps_body = ZB_BUF_BEGIN(buf);
  ZB_MEMCPY(&ret, aps_body, sizeof(ret));

  TRACE_MSG(TRACE_INFO1, "zb_bind_callback %hd", (FMT__H, ret));
  if (ret == RET_OK)
  {
    // bind ok
  }
}
@endcode
 */
void zb_zdo_bind_req(zb_uint8_t param, zb_callback_t cb);


/**
   Unbind_req request

   @param param - index of buffer with request. \see zb_zdo_bind_req_param_t
   @param cb    - user's function to call when got response from the
   remote. \see zb_zdo_bind_resp_t

   @b Example:
@code
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_req_param_t *bind_param;

  TRACE_MSG(TRACE_COMMON1, "unbind_device_1", (FMT__0));

  zb_buf_initial_alloc(buf, 0);
  bind_param = ZB_GET_BUF_PARAM(buf, zb_zdo_bind_req_param_t);
  ZB_MEMCPY(bind_param->src_address, g_ieee_addr_ed1, sizeof(zb_ieee_addr_t));
  bind_param->src_endp = TEST_ED1_EP;
  bind_param->cluster_id = TP_BUFFER_TEST_REQUEST_CLID;
  bind_param->dst_addr_mode = ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
  ZB_MEMCPY(bind_param->dst_address.addr_long, g_ieee_addr_ed2, sizeof(zb_ieee_addr_t));
  bind_param->dst_endp = TEST_ED2_EP;
  bind_param->req_dst_addr = zb_address_short_by_ieee(g_ieee_addr_ed1);
  TRACE_MSG(TRACE_COMMON1, "dst addr %d", (FMT__D, bind_param->req_dst_addr));

  zb_zdo_unbind_req(param, unbind_device1_cb);
}


void unbind_device1_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_bind_resp_t *bind_resp = (zb_zdo_bind_resp_t*)ZB_BUF_BEGIN(buf);

  TRACE_MSG(TRACE_COMMON1, "unbind_device1_cb resp status %hd", (FMT__H, bind_resp->status));
  if (bind_resp->status != ZB_ZDP_STATUS_SUCCESS)
  {
    TRACE_MSG(TRACE_COMMON1, "Error bind device 1. Test status failed", (FMT__0));
  }
  zb_free_buf(buf);

}
@endcode

 */
void zb_zdo_unbind_req(zb_uint8_t param, zb_callback_t cb);



/**
   Request for 2.4.3.3.5 Mgmt_Leave_req


   Problem in the specification:
   in 2.4.3.3.5  Mgmt_Leave_req only one DeviceAddress exists.
   But, in such case it is impossible to satisfy 2.4.3.3.5.1:
   "The Mgmt_Leave_req is generated from a Local Device requesting that a Remote
   Device leave the network or to request that another device leave the network."
   Also, in the PRO TC document, 14.2TP/NWK/BV-04 ZR-ZDO-APL RX Join/Leave is
   following note:
   "gZC sends Mgmt_Leave.request with DevAddr=all zero, DstAddr=ZR"
 */
typedef struct zb_zdo_mgmt_leave_param_s
{
  zb_ieee_addr_t device_address;   /*!< 64 bit IEEE address */
  zb_uint16_t    dst_addr;          /*!< destinition address. Not defined in
                                     * the spac - let's it be short address */
  zb_bitfield_t reserved:6;        /* reserv */
  zb_bitfield_t remove_children:1; /* Remove children */
  zb_bitfield_t rejoin:1;          /* Rejoin */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_leave_param_t;

/**
   Request for 2.4.3.3.5 Mgmt_Leave_req
 */
typedef struct zb_zdo_mgmt_leave_req_s
{
  zb_ieee_addr_t device_address;   /*!< 64 bit IEEE address */
  zb_bitfield_t reserved:6;        /* reserv */
  zb_bitfield_t remove_children:1; /* Remove children */
  zb_bitfield_t rejoin:1;          /* Rejoin */
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_leave_req_t;

/**
   Response for 2.4.4.3.5 Mgmt_Leave_rsp
 */
typedef struct zb_zdo_mgmt_leave_res_s
{
  zb_uint8_t status;
}
ZB_PACKED_STRUCT
zb_zdo_mgmt_leave_res_t;

/**
   Sends 2.4.3.3.2 Mgmt_Leave_req

   @param param - index of buffer with Lqi request parameters. \see zb_zdo_mgmt_leave_param_t
   @param cb    - user's function to call when got response from the remote.

   @b Example:
@code
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_mgmt_leave_param_t *req = NULL;
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_ERROR, "zb_leave_req", (FMT__0));

  req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_leave_param_t);

  ZB_MEMSET(req->device_address, 0, sizeof(zb_ieee_addr_t));
  req->remove_children = ZB_FALSE;
  req->rejoin = ZB_FALSE;
  req->dst_addr = 1;
  zdo_mgmt_leave_req(param, leave_callback);
}

void leave_callback(zb_uint8_t param)
{
  zb_uint8_t *ret = (zb_uint8_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));

  TRACE_MSG(TRACE_ERROR, "LEAVE CALLBACK status %hd", (FMT__H, *ret));
}
@endcode
*/
void zdo_mgmt_leave_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;



/**
   2.4.3.2.1 End_Device_Bind_req command head
*/
typedef struct zb_zdo_end_device_bind_req_head_s
{
  zb_uint16_t binding_target;   /*!< The address of the target for the
                                 * binding. This can be either the
                                 * primary binding cache device or the
                                 * short address of the local device. */
  zb_ieee_addr_t src_ieee_addr; /*!< The IEEE address of the device generating the request */
  zb_uint8_t src_endp;          /*!< The endpoint on the device generating the request */
  zb_uint16_t profile_id;       /*!< ProfileID which is to be matched
                                 * between two End_Device_Bind_req
                                 * received at the ZigBee Coordinator */
  zb_uint8_t num_in_cluster;    /*!< The number of Input Clusters
                                 * provided for end device binding
                                 * within the InClusterList. */
}
ZB_PACKED_STRUCT
zb_zdo_end_device_bind_req_head_t;

/**
   2.4.3.2.1 End_Device_Bind_req command head
*/
typedef struct zb_zdo_end_device_bind_req_tail_s
{
  zb_uint8_t num_out_cluster;   /*!< The number of Output Clusters
                                 * provided for matching within OutClusterList */
}
ZB_PACKED_STRUCT
zb_zdo_end_device_bind_req_tail_t;

/**
   Parameters for 2.4.3.2.1 End_Device_Bind_req
*/
typedef struct zb_end_device_bind_req_param_s
{
  zb_uint16_t dst_addr;         /*!< Destinition address */
  zb_zdo_end_device_bind_req_head_t head_param; /*!< Parameters for command head */
  zb_zdo_end_device_bind_req_tail_t tail_param; /*!< Parameters for command tail */
  zb_uint16_t cluster_list[1];  /*!< List of Input and Output
                                 * ClusterIDs to be used for matching */
} ZB_PACKED_STRUCT
zb_end_device_bind_req_param_t;

typedef struct zb_zdo_end_device_bind_resp_s
{
  zb_uint8_t status;
}
ZB_PACKED_STRUCT
zb_zdo_end_device_bind_resp_t;


/**
   Parameters for 2.4.3.3.7 Mgmt_Permit_Joining_req
*/
typedef struct zb_zdo_mgmt_permit_joining_req_s
{
  zb_uint8_t permit_duration;
  zb_uint8_t tc_significance;
} ZB_PACKED_STRUCT
zb_zdo_mgmt_permit_joining_req_t;

/**
   Parameters for zb_zdo_mgmt_permit_joining_req
*/
typedef struct zb_zdo_mgmt_permit_joining_req_param_s
{
  zb_uint16_t dest_addr;
  zb_uint8_t permit_duration;
  zb_uint8_t tc_significance;
} ZB_PACKED_STRUCT
zb_zdo_mgmt_permit_joining_req_param_t;


/**
   ZDO interface for ADD-GROUP.request

   Note that zb_apsme_add_group_request does not call comfirm callback.

   @param param - (in/out) buffer with parameters
      - in - \ref zb_apsme_add_group_req_t
      - out - \ref zb_apsme_add_group_conf_t
   @param cb - user's callback to be used as APSME-ADD-GROUP.confirm. \see zb_apsme_add_group_conf_t

   @b Example
@code
{
  zb_apsme_add_group_req_t *req;
  zb_buf_reuse(buf);
  req = ZB_GET_BUF_PARAM(buf, zb_apsme_add_group_req_t);
  req->group_address = 10;
  req->endpoint = 66;
  zb_zdo_add_group_req(param, group_add_conf1);
}

void group_add_conf1(zb_uint8_t param) ZB_CALLBACK
{
  zb_apsme_add_group_conf_t *conf = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_conf_t);
  conf->status = status;

  TRACE_MSG(TRACE_ERROR, "group add status %hd", (FMT__H, conf->status));
}

@endcode
 */
void zb_zdo_add_group_req(zb_uint8_t param, zb_callback_t cb);


/*! @} */


/*! \cond internals_doc */
/*! \addtogroup ZB_ZDO ZDO internals */
/*! @{ */


/**
   Parameters of Device_annce primitive.

   To be put into buffer as data (means - after space alloc).
 */
typedef struct zb_zdo_device_annce_s
{
  zb_uint8_t       tsn;
  zb_uint16_t      nwk_addr;    /*!< NWK address for the Local Device  */
  zb_ieee_addr_t   ieee_addr;   /*!< IEEE address for the Local Device  */
  zb_uint8_t       capability; /*!< Capability of the local device */
} ZB_PACKED_STRUCT zb_zdo_device_annce_t;

/**
   Device_annce primitive.

   "This command shall be invoked for all ZigBee end devices upon join or rejoin. This
   command may also be invoked by ZigBee routers upon join or rejoin as part of
   NWK address conflict resolution."

   @param param - index of buffer with primitive parameters. Parameters must be
   put into buffer as data (allocated).
*/
void zb_zdo_device_annce(zb_uint8_t param) ZB_SDCC_REENTRANT;


/**
   Actually send Device annonce command
 */
void zdo_send_device_annce(zb_uint8_t param);


/**
   Node_desc_resp/Power_Node_desc_resp primitive.

   @param param - index of buffer to fill with primitive parameters.
*/
void zdo_send_desc_resp(zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Simple_desc_resp primitive.

   @param param - index of buffer to fill with primitive parameters.
*/
void zdo_send_simple_desc_resp(zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Copies node descriptor, taking into accaunt endian
   @param dst_desc - destinition descriptor
   @param dst_desc - source descriptor
*/
void zb_copy_node_desc(zb_af_node_desc_t *dst_desc, zb_af_node_desc_t *src_desc) ZB_SDCC_REENTRANT;

/**
   Copies node power descriptor, taking into accaunt endian
   @param dst_desc - destinition descriptor
   @param dst_desc - source descriptor
*/
void zb_copy_power_desc(zb_af_node_power_desc_t *dst_desc, zb_af_node_power_desc_t *src_desc) ZB_SDCC_REENTRANT;

/**
   Copies simple descriptor, taking into accaunt endian
   @param dst_desc - destinition descriptor
   @param dst_desc - source descriptor
*/
void zb_copy_simple_desc(zb_af_simple_desc_1_1_t* dst_desc, zb_af_simple_desc_1_1_t*src_desc) ZB_SDCC_REENTRANT;

/**
   NWK_addr_req  primitive.

   @param param - index of buffer to fill with primitive parameters.
   @param fc - APS FC of the response
*/
void zdo_device_nwk_addr_res(zb_uint8_t param, zb_uint8_t fc) ZB_SDCC_REENTRANT;
#define ZB_ZDO_SINGLE_DEV_RESPONSE  0x00
#define ZB_ZDO_EXTENDED_RESPONSE    0x01

/**
   IEEE_addr_req  primitive.

   @param param - index of buffer to fill with primitive parameters.
   @param fc - APS FC of the response
*/
void zdo_device_ieee_addr_res(zb_uint8_t param, zb_uint8_t fc) ZB_SDCC_REENTRANT;

/**
   Starts energy scan. Fuction is used as argument of ZB_GET_IN_BUF_DELAYED() macro
   @param param - index of buffer
*/
void zb_start_ed_scan(zb_uint8_t param) ZB_CALLBACK;

/**
   Sends update notify command
   @param param - index of buffer
*/
void zb_zdo_nwk_upd_notify(zb_uint8_t param) ZB_CALLBACK;

/**
   Active_EP_res primitive.

   @param param - index of buffer to fill with primitive parameters.
*/
void zdo_active_ep_res(zb_uint8_t param);

/**
   Match_Desc_res primitive.

   @param param - index of buffer to fill with primitive parameters.
*/
void zdo_match_desc_res(zb_uint8_t param);

/**
   Poll parent.
   This function periodically polls parent to check if it has data for us.
   It runs only when mac_rx_on_when_idle is false.

   @param param - buffer.
*/
void zb_zdo_poll_parent(zb_uint8_t param) ZB_CALLBACK;


/**
   Reschedule zb_zdo_poll_parent call after timeout

   If periodical poll is meaningful (that is, rx-on-when-idle = 0), cancel
   current poll alarm and schedule new one.

   To nbe used to optimize cases when ZED sends some request and supposes quick
   replay (for example, sends APS packet with ACK TX option).

   @param timeout - timeout to set.
 */
void zb_zdo_reschedule_poll_parent(zb_uint16_t timeout);

/**
   Performs channel interference reporting and resolution
   @param param - unused
*/
void zb_zdo_check_fails(zb_uint8_t param) ZB_CALLBACK;

/**
   Timer callback to set limit for channel check action
   @param param - unused
 */
void zb_zdo_channel_check_timer_cb(zb_uint8_t param) ZB_CALLBACK;

/**
   Callback to get ED scan result during channel check
   @param param - index of buffer with scan results
 */
void zb_zdo_channel_check_scan_result(zb_uint8_t param) ZB_CALLBACK;

/**
   Callback to finish channel check action, is called on
   Mgmt_NWK_Update_notify acknowledgement
   @param param - index of buffer with results
*/
void zb_zdo_channel_check_finish_cb(zb_uint8_t param) ZB_CALLBACK;

/**
   initiate rejoin tothe same PAN
 */
zb_ret_t zdo_initiate_rejoin(zb_buf_t *buf) ZB_SDCC_REENTRANT;


/**
   Handles Mgmt_NWK_Update_req request

   @param param - index of buffer with request
 */
void zb_zdo_mgmt_nwk_update_handler(zb_uint8_t param) ZB_SDCC_REENTRANT;


/**
   Performs channel change procedure, channel manager side
   @param param - index of buffer with mgmt_nwk_update_notify response
 */
void zdo_change_channel(zb_uint8_t param) ZB_SDCC_REENTRANT;


/**
   Update device information, when both address are known.
 */
zb_neighbor_tbl_ent_t *zdo_device_info_upd(zb_buf_t *buf, zb_ieee_addr_t ieee_addr, zb_uint16_t addr) ZB_SDCC_REENTRANT;

/**
   Bind/Unbind response primitive

   @param param - index of buffer with request
   @param bind - true for bind, false for unbind
 */
void zb_zdo_bind_unbind_res(zb_uint8_t param, zb_bool_t bind) ZB_SDCC_REENTRANT;

/**
   Sends 2.4.4.3.2 Mgmt_Lqi_rsp
   @param param - index of buffer with Lqi request
 */
void zdo_lqi_resp(zb_uint8_t param) ZB_SDCC_REENTRANT;


/**
   Sends 2.4.4.3.2 Mgmt_Leave_rsp
   @param param - index of buffer with Lqi request
 */

void zdo_mgmt_leave_res(zb_uint8_t param);


/**
   Allocates buffer and schedules to execute zb_get_peer_short_addr()
   @param dst_addr_ref - reference to destinition ieee address
   @param cb - callback to run on get peer address finish
   @param param - parameter for callback function
 */
void zb_start_get_peer_short_addr(zb_address_ieee_ref_t dst_addr_ref, zb_callback_t cb, zb_uint8_t param);

/**
   Sends response for System_Server_Discovery_req
   @param param - index of buffer with request
 */
void zdo_system_server_discovery_res(zb_uint8_t param);


/**
   sends 2.4.3.2.1 End_Device_Bind_req command
   @param param - index of buffer with request
   @param cp    - user's function to call when got response from the remote.
 */
void zb_end_device_bind_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;

/**
   Sends 2.4.4.2.1 End_Device_Bind_rsp command
   @param param - index of buffer to use for i/o
   @param status - End_Device_Bind_req command status
*/
void zb_zdo_end_device_bind_resp(zb_uint8_t param, zb_zdp_status_t status) ZB_SDCC_REENTRANT;

/**
   Handle end_device_bind_req command
   @param param - index of buffer with request
 */
void zb_zdo_end_device_bind_handler(zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Sends nlme reset command

   @param param - index of buffer with request
   @param warm_start - This parameter has a value of FALSE if
   the request is expected reset all stack
   values to their initial default values. If this
   value is TRUE, the device is expected to
   resume operations according to the NIB
   settings prior to the call.
   @param cb - callback to be called after reset confirm
 */
void zb_zdo_reset(zb_uint8_t param, zb_uint8_t warm_start, zb_callback_t cb) ZB_SDCC_REENTRANT;


/**
   Handle incoming mgmt_leave_req

   @param param - buffer with request
 */
void zdo_mgmt_leave_srv(zb_uint8_t param) ZB_SDCC_REENTRANT;

/**
   Try to send mgmt_leave_rsp if somebody waiting for it.

   mgmt_leave operation is registered in the leave pending list: after LEAVE
   command send confirm must send responce.
   Search is done using buffer ref: it always same.
   If nothing found in the pending list, can proceed with leave.confirm
   operations (clear addresses), else need to wait until response sent.

   @param param - ref to the buffer used for overall LEAVE operation
   @param status - status to pass to the peer

   @return TRUE if resp sent, FALSE otherwhise
 */
zb_bool_t zdo_try_send_mgmt_leave_rsp(zb_uint8_t param, zb_uint8_t status, zb_uint8_t will_leave) ZB_SDCC_REENTRANT;

/**
   Try to complete LEAVE after mgmt_leave_rsp send confirm

   @param param - buffer to be used internally if necessary.

   @return TRUE if leave done and buffer used, FALSE otherwhise
 */
zb_bool_t zdo_try_mgmt_leave_complete(zb_uint8_t param);


/**
   handle Mgmt_Permit_Joining_req request

   @param param - permit joining parameters

   @return nothing
 */
void zb_zdo_mgmt_permit_joining_handle(zb_uint8_t param) ZB_CALLBACK;

/**
   sends 2.4.3.3.7 Mgmt_Permit_Joining_req
   @param param - index of buffer with request
   @param cp    - user's function to call when got response from the remote.
 */
void zb_zdo_mgmt_permit_joining_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;

/*! @} */
/*! \endcond */


void zb_zdo_mgmt_nwk_update_req(zb_uint8_t param, zb_callback_t cb) ZB_SDCC_REENTRANT;

#ifndef ZB_LITTLE_ENDIAN
#define ZB_LETOH16_XOR(val) \
  ((zb_uint8_t*)&val)[0] = ((zb_uint8_t)val)^(*((zb_uint8_t*)&val));\
  ((zb_uint8_t*)&val)[1] = (*((zb_uint8_t*)&val))^((zb_uint8_t)val);\
  ((zb_uint8_t*)&val)[0] = (*((zb_uint8_t*)&val))^(zb_uint8_t)val;
#else
#define ZB_LETOH16_XOR(val)
#endif


#define ZB_GET_SIMPLE_DESC_RESP(a) \
(zb_zdo_simple_desc_resp_t*)(a);\
{								\
	zb_uint8_t counter;			\
  zb_uint8_t _i; \
	counter = 					\
	  *((zb_uint8_t*)(((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_cluster_list+((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_input_cluster_count)-1); \
  TRACE_MSG(TRACE_APS1, "simple_desc_counter %hd", (FMT__H, counter));\
	ZB_MEMMOVE(((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_cluster_list, \
             &((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_output_cluster_count, \
             ((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_input_cluster_count*sizeof(zb_uint16_t)); \
	((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_output_cluster_count = counter;\
  for (_i = 0; _i<((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_input_cluster_count + \
         ((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_output_cluster_count; _i++) \
            {\
               ZB_LETOH16_XOR(((zb_zdo_simple_desc_resp_t*)(a))->simple_desc.app_cluster_list);\
            }\
}


#endif /* ZB_ZDO_H */
