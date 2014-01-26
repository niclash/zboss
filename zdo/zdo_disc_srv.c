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
PURPOSE: ZDO Discovery services - server side.
Mandatory calls onnly. Other calls will be implemented in some other project scope.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_hash.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"

#include "zb_bank_9.h"
/*! \addtogroup ZB_ZDO */
/*! @{ */

#ifndef ZB_LIMITED_FEATURES

void zdo_send_desc_resp(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_uint16_t addr_of_interest;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind = NULL;
  zb_zdo_desc_resp_hdr_t *resp_hdr = NULL;
  zb_uint8_t *aps_body = NULL;
  zb_uint8_t tsn = 0;
  zb_uint8_t *desc_body = 0;
  zb_uint16_t resp_id = 0;

/*
  2.4.4.1.3 Node_Desc_rsp
*/
  TRACE_MSG(TRACE_ZDO3, ">>zdo_send_desc_resp %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));

  /*
    2.4.2.8 Transmission of ZDP Commands
   | Transaction sequence number (1byte) | Transaction data (variable) |
  */
  tsn = *aps_body;
  aps_body++;

  ZB_LETOH16(&addr_of_interest, aps_body);

  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_desc_resp_hdr_t), resp_hdr);

  if (ZB_NIB_NETWORK_ADDRESS() == addr_of_interest)
  {
    resp_hdr->status = ZB_ZDP_STATUS_SUCCESS;
    switch (ind->clusterid)
    {
      case ZDO_NODE_DESC_REQ_CLID:
        ZB_BUF_ALLOC_RIGHT(buf, sizeof(zb_af_node_desc_t), desc_body);
        zb_copy_node_desc((zb_af_node_desc_t*)desc_body, ZB_ZDO_NODE_DESC());
        resp_id = ZDO_NODE_DESC_RESP_CLID;
        break;

      case ZDO_POWER_DESC_REQ_CLID:
        ZB_BUF_ALLOC_RIGHT(buf, sizeof(zb_af_node_power_desc_t), desc_body);
        zb_copy_power_desc((zb_af_node_power_desc_t*)desc_body, ZB_ZDO_NODE_POWER_DESC());
        resp_id = ZDO_POWER_DESC_RESP_CLID;
        break;

      default:
        TRACE_MSG(TRACE_ZDO3, "unknown cluster id %i", (FMT__D, ind->clusterid));
        ZB_ASSERT(0);
    }
  }
  else
  {
    /* descriptor is not sent in this case */
#ifdef ZB_COORDINATOR_ROLE
    /* search for child - not implemented */
    resp_hdr->status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
    TRACE_MSG(TRACE_ZDO1, "Error,device cache is not impl", (FMT__0));
#else
    resp_hdr->status = ZB_ZDP_STATUS_INV_REQUESTTYPE;
    TRACE_MSG(TRACE_ZDO1, "Error,invalid addr", (FMT__0));
#endif
  }

  ZB_LETOH16(&resp_hdr->nwk_addr, &addr_of_interest);

  zdo_send_resp_by_short(resp_id, param, tsn, ind->src_addr);

  TRACE_MSG(TRACE_ZDO3, "<<zdo_send_desc_resp", (FMT__0));
}


void zdo_send_simple_desc_resp(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_uint16_t addr_of_interest;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t *ind;
  zb_zdo_simple_desc_resp_hdr_t *resp_hdr;
  zb_uint8_t tsn;
  zb_uint8_t *desc_body;
  zb_af_simple_desc_1_1_t *src_desc = NULL;
  zb_uint8_t ep;
  zb_uint8_t i;

/*
  2.4.4.1.5 Simple_Desc_rsp
*/
  TRACE_MSG(TRACE_ZDO3, ">>zdo_send_simple_desc_resp %hd", (FMT__H, param));
  {
    zb_uint8_t *aps_body;
    aps_body = ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));

    /*
      2.4.2.8 Transmission of ZDP Commands
      | Transaction sequence number (1byte) | Transaction data (variable) |
    */
    tsn = *aps_body;
    aps_body++;

    zb_get_next_letoh16(&addr_of_interest, &aps_body);
    ep = *aps_body;
  }
  ind = ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t);
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_resp_hdr_t), resp_hdr);
  resp_hdr->length = 0;

  if (ZB_NIB_NETWORK_ADDRESS() == addr_of_interest)
  {
    if (!ep)
    {
      src_desc = (zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC();
    }
    else
    {
      for (i = 0; i < ZB_ZDO_SIMPLE_DESC_NUMBER(); i++)
      {
        if (ZB_ZDO_SIMPLE_DESC_LIST()[i]->endpoint == ep)
        {
          src_desc = (zb_af_simple_desc_1_1_t*)ZB_ZDO_SIMPLE_DESC_LIST()[i];
        }
      }
    }

    if (src_desc)
    {
      resp_hdr->status = ZB_ZDP_STATUS_SUCCESS;
      resp_hdr->length = sizeof(zb_af_simple_desc_1_1_t) +
        /* take into account app_cluster_list */
        (src_desc->app_input_cluster_count + src_desc->app_output_cluster_count - 2)*sizeof(zb_uint16_t);

      ZB_BUF_ALLOC_RIGHT(buf, resp_hdr->length, desc_body);
      zb_copy_simple_desc((zb_af_simple_desc_1_1_t*)desc_body, src_desc);
    }
    else
    {
      resp_hdr->status = ZB_ZDP_STATUS_INVALID_EP;
    }
  }
  else
  {
    /* descriptor is not sent in this case */
#ifdef ZB_COORDINATOR_ROLE
    /* search for child - not implemented */
    resp_hdr->status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
    TRACE_MSG(TRACE_ZDO1, "Error,device cache is not impl", (FMT__0));
#else
    resp_hdr->status = ZB_ZDP_STATUS_INV_REQUESTTYPE;
    TRACE_MSG(TRACE_ZDO1, "Error,invalid addr", (FMT__0));
#endif
  }

  ZB_LETOH16(&resp_hdr->nwk_addr, &addr_of_interest);
  TRACE_MSG(TRACE_ZDO1, "simple_desc length: %hd", (FMT__H, resp_hdr->length));
  zdo_send_resp_by_short(ZDO_SIMPLE_DESC_RESP_CLID, param, tsn, ind->src_addr);

  TRACE_MSG(TRACE_ZDO3, "<<zdo_send_desc_resp", (FMT__0));
}


void zb_copy_node_desc(zb_af_node_desc_t *dst_desc, zb_af_node_desc_t *src_desc) ZB_SDCC_REENTRANT
{
  ZB_LETOH16(&dst_desc->node_desc_flags, &src_desc->node_desc_flags);
  dst_desc->mac_capability_flags = src_desc->mac_capability_flags;
  ZB_LETOH16(&dst_desc->manufacturer_code, &src_desc->manufacturer_code);
  dst_desc->max_buf_size = src_desc->max_buf_size;
  ZB_LETOH16(&dst_desc->max_incoming_transfer_size, &src_desc->max_incoming_transfer_size);
  ZB_LETOH16(&dst_desc->server_mask, &src_desc->server_mask);
  ZB_LETOH16(&dst_desc->max_outgoing_transfer_size, &src_desc->max_outgoing_transfer_size);
}

void zb_copy_power_desc(zb_af_node_power_desc_t *dst_desc, zb_af_node_power_desc_t *src_desc) ZB_SDCC_REENTRANT
{
  ZB_LETOH16(&dst_desc->power_desc_flags, &src_desc->power_desc_flags);
}

void zb_copy_simple_desc(zb_af_simple_desc_1_1_t* dst_desc, zb_af_simple_desc_1_1_t*src_desc) ZB_SDCC_REENTRANT
{
  zb_ushort_t i;
  zb_uint8_t out_count;
  ZB_MEMCPY(dst_desc, src_desc, sizeof(zb_af_simple_desc_1_1_t) - sizeof(zb_uint16_t)*2);
  ZB_LETOH16_XOR(dst_desc->app_profile_id);
  ZB_LETOH16_XOR(dst_desc->app_device_id);
  out_count = src_desc->app_output_cluster_count;
  TRACE_MSG(TRACE_ZDO1, "simple_desc outclusters count %hd", (FMT__H, out_count));
  TRACE_MSG(TRACE_ZDO1, "simple_desc inclusters count %hd", (FMT__H, src_desc->app_input_cluster_count));

  for (i = 0; i < src_desc->app_input_cluster_count; i++)
  {
    ZB_LETOH16(((zb_uint8_t*)(&dst_desc->app_cluster_list[i]) - 1), &src_desc->app_cluster_list[i]);
  }
  for (i = src_desc->app_input_cluster_count; i < src_desc->app_input_cluster_count + src_desc->app_output_cluster_count; i++)
  {
	ZB_LETOH16((&dst_desc->app_cluster_list[i]), (&src_desc->app_cluster_list[i]));
  }													  
  ZB_MEMCPY((zb_uint8_t*)(dst_desc->app_cluster_list + src_desc->app_input_cluster_count)-1,&out_count,1);

}

void zdo_device_nwk_addr_res(zb_uint8_t param, zb_uint8_t fc) ZB_SDCC_REENTRANT
{
  zb_uint8_t status = ZB_ZDP_STATUS_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t ind;
  zb_zdo_nwk_addr_req_t req;
  zb_uint8_t *aps_body = NULL;
  zb_uint8_t tsn;
  zb_uint16_t nwk_addr = ZB_UNKNOWN_SHORT_ADDR;

  TRACE_MSG(TRACE_ZDO3, "zdo_dev_nwk_addr_res %hd, fc %hd", (FMT__H_H, param, fc));

  aps_body = ZB_BUF_BEGIN(buf);
  tsn = *aps_body;
  aps_body++;

  ZB_MEMCPY(&req, (zb_zdo_nwk_addr_req_t *)aps_body, sizeof(req));
  ZB_MEMCPY(&ind, ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t), sizeof(ind));

  /* TODO: make HTOL conversion */

  TRACE_MSG(TRACE_ZDO3, "request_type %hd, start_index %hd", (FMT__H_H, req.request_type, req.start_index));
  ZB_DUMP_IEEE_ADDR(req.ieee_addr);

  if (req.request_type == ZB_ZDO_SINGLE_DEV_RESPONSE)
  {
    if (ZB_64BIT_ADDR_CMP(ZB_PIB_EXTENDED_ADDRESS(), req.ieee_addr))
    {
      nwk_addr = ZB_PIB_SHORT_ADDRESS();
      TRACE_MSG(TRACE_ZDO3, "local found %d", (FMT__D, nwk_addr));
    }
    else
    {
      zb_uint8_t i;
      zb_ieee_addr_t ieee_addr;

      for (i = 0; i < ZB_NEIGHBOR_TABLE_SIZE; i++ )
      {
        if (ZG->nwk.neighbor.base_neighbor[i].used)
        {
          zb_address_ieee_by_ref(ieee_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);
          if (ZB_64BIT_ADDR_CMP(ieee_addr, req.ieee_addr))
          {
            zb_address_short_by_ref(&nwk_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);
            TRACE_MSG(TRACE_ZDO3, "remote found %d", (FMT__D, nwk_addr));
            break;
          }
        }
      } /* for */
    }
  }

  if (nwk_addr == ZB_UNKNOWN_SHORT_ADDR)
  {
    if (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_UNICAST)
    {
      status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
    }

    /* FIXME: If there is no match and the command was received as a
       broadcast, the request shall be discarded and no response generated.
    */
#if 0
    else if (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_BROADCAST)
    {
      return;
    }
#endif
  }
  if ((status == ZB_ZDP_STATUS_SUCCESS) &&
      (req.request_type > ZB_ZDO_EXTENDED_RESPONSE))
  {
    status  = ZB_ZDP_STATUS_INV_REQUESTTYPE;
  }


  if ((status == ZB_ZDP_STATUS_SUCCESS) &&
      (req.request_type == ZB_ZDO_SINGLE_DEV_RESPONSE))
  {
    zb_zdo_nwk_addr_resp_head_t *resp;

    TRACE_MSG(TRACE_ZDO3, "make resp, nwk addr %d", (FMT__D, nwk_addr));
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_nwk_addr_resp_head_t), resp);
    resp->status = status;
    ZB_HTOLE64(resp->ieee_addr, req.ieee_addr);
    ZB_HTOLE16(&resp->nwk_addr, &nwk_addr);

#if 0
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 11; /* Table 2.89: Status + IEEEAddrRemoteDev +
                           * NWKAddrRemoteDev */

    TRACE_MSG(TRACE_ZDO3, "make resp, nwk addr %d", (FMT__D, nwk_addr));
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr++ = status;
    ZB_MEMCPY(ptr, req.ieee_addr, sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, nwk_addr);
#endif
  }
  else if ((status == ZB_ZDP_STATUS_SUCCESS) &&
           (req.request_type == ZB_ZDO_EXTENDED_RESPONSE) &&
           ((ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR) ||
            (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)))
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;
    zb_uint8_t  i = 0;
    zb_uint8_t  count = ZG->nwk.neighbor.base_neighbor_used;
    zb_uint8_t  addr_counter = 0;

    len  = 11; /* Table 2.89: Status + IEEEAddrRemoteDev + NWKAddrRemoteDev */
    len += 2;  /* Table 2.89: NumAssocDev + StartIndex */

    {
      /* FIXME: about real size, current values as max header size */
      zb_uint8_t headers_size = ( 23 + /* mac header */
                                  25 + /* nwk header */
                                  14 + /* secur header */
                                  8 /* aps header */);

      addr_counter = count - req.start_index;
      if ( (addr_counter * sizeof(zb_uint16_t)) + headers_size > ZB_IO_BUF_SIZE )
      {
        addr_counter = 0;
        do
        {
          len += 2;
          addr_counter++;
        } while (len + 2 + headers_size < ZB_IO_BUF_SIZE);
      }
      else
      {
        if (addr_counter == 0)
        {
          zb_free_buf(buf);
          return;
        }
        else
        {
          len += (addr_counter * 2); /* Table 2.89: NWKAddrAssocDevList */
        }
      }
    }
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_MEMCPY(ptr, &ZB_PIB_EXTENDED_ADDRESS(), sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, ZB_PIB_SHORT_ADDRESS());
    *ptr = count;
    ptr++;
    *ptr = 0;
    ptr++;
    {
      zb_neighbor_tbl_ent_t *ent = NULL;
      zb_uint16_t addr = 0;
      zb_uint8_t found = 0;

      for ( i = 0, ent = &ZG->nwk.neighbor.base_neighbor[0]; i < ZB_NEIGHBOR_TABLE_SIZE && addr_counter; i++ )
      {
        if ( ent->used )
        {
          found++;
          if (found>=req.start_index)
          {
            zb_address_short_by_ref(&addr, ent->addr_ref);
            zb_put_next_htole16(&ptr, addr);
            addr_counter--;
            ent++;
            TRACE_MSG(TRACE_ZDO3, "associated device addr: %d", (FMT__D, addr));
          }

        }
      }
    }
  }
  if (status)
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 11; /* Table 2.89: Status + IEEEAddrRemoteDev + NWKAddrRemoteDev */

    TRACE_MSG(TRACE_ZDO3, "make response with error %hd", (FMT__H, status));
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_MEMCPY(ptr, &req.ieee_addr, sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, ZB_PIB_SHORT_ADDRESS());
  }
  zdo_send_resp_by_short(ZDO_NWK_ADDR_RESP_CLID, param, tsn, ind.src_addr);
}


void zdo_device_ieee_addr_res(zb_uint8_t param, zb_uint8_t fc) ZB_SDCC_REENTRANT
{
  zb_uint8_t status = ZB_ZDP_STATUS_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t ind;
  zb_zdo_ieee_addr_req_t req;
  zb_uint8_t *aps_body;
  zb_uint8_t tsn;
  zb_uint16_t nwk_addr;

  TRACE_MSG(TRACE_ZDO3, "zdo_dev_ieee_addr_res %hd, fc %hd", (FMT__H_H, param, fc));

  aps_body = ZB_BUF_BEGIN(buf);
  tsn = *aps_body;
  aps_body++;

  ZB_MEMCPY(&req, (zb_zdo_ieee_addr_req_t *)aps_body, sizeof(req));
  ZB_MEMCPY(&ind, ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t), sizeof(ind));
  ZB_HTOLE16(&nwk_addr, &req.nwk_addr);

  if (ZB_PIB_SHORT_ADDRESS() != nwk_addr)
  {
    if (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_UNICAST)
    {
      status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
    }
    else if (ZB_APS_FC_GET_DELIVERY_MODE(fc) == ZB_APS_DELIVERY_BROADCAST)
    {
      return;
    }
  }
  else if ((status == ZB_ZDP_STATUS_SUCCESS) &&
           (req.request_type > ZB_ZDO_EXTENDED_RESPONSE))
  {
    status  = ZB_ZDP_STATUS_INV_REQUESTTYPE;
  }
  else if ((status == ZB_ZDP_STATUS_SUCCESS) &&
           (req.request_type == ZB_ZDO_SINGLE_DEV_RESPONSE))
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;

    len  = 11; /*sizeof(zb_uint8_t)+sizeof(zb_ieee_addr_t)+sizeof(zb_uint16_t)*/
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_MEMCPY(ptr, &ZB_PIB_EXTENDED_ADDRESS(), sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, nwk_addr);
  }
  else if ((status == ZB_ZDP_STATUS_SUCCESS) &&
           (req.request_type == ZB_ZDO_EXTENDED_RESPONSE) &&
           ((ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR) ||
            (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)))
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;
    zb_uint8_t  i = 0;
    zb_uint8_t  count = ZG->nwk.neighbor.base_neighbor_used;
    zb_uint8_t  addr_counter = 0;

    len  = 11; /* Table 2.89: Status + IEEEAddrRemoteDev + NWKAddrRemoteDev */
    len += 2;  /* Table 2.89: NumAssocDev + StartIndex */

    {
      /* FIXME: about real size, current values as max header size */
      zb_uint8_t headers_size = ( 23 + /* mac header */
                                  25 + /* nwk header */
                                  14 + /* secur header */
                                  8 /* aps header */);

      addr_counter = count - req.start_index;
      if ( addr_counter * sizeof(zb_uint16_t) + headers_size > ZB_IO_BUF_SIZE )
      {
        addr_counter = 0;
        do
        {
          len += 2;
          addr_counter++;
        } while (len + 8 + headers_size < ZB_IO_BUF_SIZE);
      }
      else
      {
        if (addr_counter == 0)
        {
          zb_free_buf(buf);
          return;
        }
        else
        {
          len += (addr_counter * 2); /* Table 2.89: NWKAddrAssocDevList */
        }
      }
    }
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_MEMCPY(ptr, &ZB_PIB_EXTENDED_ADDRESS(), sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, ZB_PIB_SHORT_ADDRESS());
    *ptr = addr_counter;
    ptr++;
    *ptr = 0;
    ptr++;
    {
      zb_neighbor_tbl_ent_t *ent = NULL;
      zb_uint16_t addr = 0;
      zb_uint8_t found = 0;

      for ( i = 0, ent = &ZG->nwk.neighbor.base_neighbor[0]; i < ZB_NEIGHBOR_TABLE_SIZE && addr_counter; i++ )
      {
        if ( ent->used )
        {
          found++;
          if (found>=req.start_index)
          {
            zb_address_short_by_ref(&addr, ent->addr_ref);
            zb_put_next_htole16(&ptr, addr);
            addr_counter--;
            ent++;
            TRACE_MSG(TRACE_ZDO3, "associated device addr: %d", (FMT__D, addr));
          }
        }
      }
    }
  }
/* replaced, because of Table 2.90. There must be associated device's short
 * addresses, not IEEE */

/*
      zb_neighbor_tbl_ent_t *ent = NULL;
      zb_ieee_addr_t ieee_addr;

      for ( i = req.start_index, ent = &ZG->nwk.neighbor.base_neighbor[i]; i < ZB_NEIGHBOR_TABLE_SIZE && addr_counter; i++ )
      {
        if ( ent->used )
        {
          zb_address_ieee_by_ref(ieee_addr, ent->addr_ref);
          ZB_MEMCPY(ptr, &ieee_addr, sizeof(zb_ieee_addr_t));
          ptr += 8;
        }
      }
    }
    }*/

  if (status)
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;

    len  = 11; /*sizeof(zb_uint8_t)+sizeof(zb_ieee_addr_t)+sizeof(zb_uint16_t)*/
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_MEMCPY(ptr, &ZB_PIB_EXTENDED_ADDRESS(), sizeof(zb_ieee_addr_t));
    ptr +=8;
    zb_put_next_htole16(&ptr, nwk_addr);
  }
  zdo_send_resp_by_short(ZDO_IEEE_ADDR_RESP_CLID, param, tsn, ind.src_addr);
}

void zdo_active_ep_res(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_uint8_t status = ZB_ZDP_STATUS_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t ind;
  zb_zdo_active_ep_req_t req;
  zb_uint8_t *aps_body;
  zb_uint8_t tsn;
  zb_uint16_t nwk_addr;

  TRACE_MSG(TRACE_ZDO3, "zdo_active_ep_res %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(buf);
  tsn = *aps_body;
  aps_body++;

  ZB_MEMCPY(&req, (zb_zdo_active_ep_req_t *)aps_body, sizeof(req));
  ZB_MEMCPY(&ind, ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t), sizeof(ind));
  ZB_HTOLE16(&nwk_addr, &req.nwk_addr);

  if ((ZB_PIB_SHORT_ADDRESS() != nwk_addr) &&
      (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ED))
  {
    status  = ZB_ZDP_STATUS_INV_REQUESTTYPE;
  }
  else if ((status == ZB_ZDP_STATUS_SUCCESS) &&
           (ZB_PIB_SHORT_ADDRESS() != req.nwk_addr))
  {
    status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
  }
  else
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;

    len  = 5; /* Table 2.94: Status + NWKAddrOfInterest + ActiveEPCount + ActiveEPList */
    // TODO: it is for ZB_MAX_EP_NUMBER = 1

    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    zb_put_next_htole16(&ptr, nwk_addr);
    *ptr = 1;
    ptr++;
    *ptr = ZB_ZDO_SIMPLE_DESC_LIST()[0]->endpoint;
    ptr++;
  }
  if (status)
  {
    zb_uint8_t *ptr = NULL;
    zb_uint8_t  len = 0;

    len  = 4; /* Table 2.94: Status + NWKAddrOfInterest + ActiveEPCount */
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    zb_put_next_htole16(&ptr, req.nwk_addr);
    *ptr = 0;
    ptr++;
  }
  zdo_send_resp_by_short(ZDO_ACTIVE_EP_RESP_CLID, param, tsn, ind.src_addr);
}

/* adds value to sorted list, if the same value was not added before exists */
static zb_uint8_t add_ep_sorted(zb_uint8_t *ep_list, zb_uint8_t ep_num, zb_uint8_t endpoint) ZB_SDCC_REENTRANT
{
  zb_uint8_t i;
  zb_uint8_t index;

  TRACE_MSG(TRACE_ZDO3, "add_ep_sorted ep_num %hd, ep %hd", (FMT__H_H, ep_num, endpoint));
  for (index = 0; index < ep_num; index++)
  {
    if (endpoint <= ep_list[index])
    {
      break;
    }
  }
  TRACE_MSG(TRACE_ZDO3, "index %hd, ep[i] %hd", (FMT__H_H, index, ep_list[index]));
  if (ep_list[index] != endpoint)
  {
    for (i = ep_num; i > index; i--)
    {
      ep_list[i] = ep_list[i - 1];
    }
    ep_list[index] = endpoint;
    ep_num++;
  }
  TRACE_MSG(TRACE_ZDO3, "<< add_ep_sorted ret %hd", (FMT__H, ep_num));
  return ep_num;
}

zb_uint8_t* copy_cluster_id(zb_uint8_t *cluster_dst, zb_uint8_t *cluster_src, zb_uint8_t cluster_num) ZB_SDCC_REENTRANT
{
  zb_uint8_t i;
  for (i = 0; i < cluster_num; i++)
  {
    ZB_HTOLE16(cluster_dst + i * sizeof(zb_uint16_t), cluster_src + i * sizeof(zb_uint16_t));
  }
  return (zb_uint8_t*)(cluster_dst + cluster_num * sizeof(zb_uint16_t));
}

void zdo_match_desc_res(zb_uint8_t param) ZB_SDCC_REENTRANT
{
  zb_uint8_t status = ZB_ZDP_STATUS_SUCCESS;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_indication_t ind;
  zb_zdo_match_desc_param_t match_params;
  zb_uint16_t *cluster_id;
  zb_uint8_t *aps_body;
  zb_uint8_t tsn;
  zb_uint16_t dst_addr;
  zb_zdo_match_desc_resp_t *match_resp;
  zb_uint8_t ep_num = 0;
  zb_uint8_t ep_list[ZB_MAX_EP_NUMBER];
  zb_uint8_t i, j, k;

  TRACE_MSG(TRACE_ZDO2, ">> zdo_match_desc_res %hd", (FMT__H, param));

  aps_body = ZB_BUF_BEGIN(buf);
  tsn = *aps_body;
  aps_body++;

  ZB_HTOLE16(&match_params.nwk_addr, aps_body);
  aps_body += sizeof(zb_uint16_t);
  ZB_HTOLE16(&match_params.profile_id, aps_body);
  aps_body += sizeof(zb_uint16_t);
  match_params.num_in_clusters = *aps_body;
  aps_body++;

  match_params.num_out_clusters = *(aps_body + match_params.num_in_clusters * sizeof(zb_uint16_t));

  ZB_MEMCPY(&ind, ZB_GET_BUF_PARAM(buf, zb_apsde_data_indication_t), sizeof(ind));
  dst_addr = ind.src_addr;

  TRACE_MSG(TRACE_ZDO3, "nwk_addr %d, profile_id %d, num in clust %hd, num out clust %hd",
    (FMT__D_D_H_H, match_params.nwk_addr, match_params.profile_id, match_params.num_in_clusters, match_params.num_out_clusters));

  /*
    cluster_id is really aligned to 2 here: every buffer is aligned, subtract
    multiple of 2 bytes from its tail.
    but aps_body does not.
  */
  cluster_id = ZB_GET_BUF_TAIL(buf, (match_params.num_in_clusters + match_params.num_out_clusters) * sizeof(zb_uint16_t));
  aps_body = copy_cluster_id((zb_uint8_t *)cluster_id, aps_body, match_params.num_in_clusters);

  aps_body++; /* num_out_clusters field */
  aps_body = copy_cluster_id((zb_uint8_t *)(cluster_id + match_params.num_in_clusters),
                             aps_body, match_params.num_out_clusters);

  /* TODO: add child device analysis */
  if ((ZB_PIB_SHORT_ADDRESS() != match_params.nwk_addr) &&
      match_params.nwk_addr != ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE)
  {
    status = ZB_ZDP_STATUS_INV_REQUESTTYPE;
    TRACE_MSG(TRACE_ZDO3, "invalid requesttype", (FMT__0));
  }
  else
  {
    zb_bool_t found;

    TRACE_MSG(TRACE_ZDO3, "simple desc num %hd", (FMT__H, ZB_ZDO_SIMPLE_DESC_NUMBER()));
    for (i = 0; i < ZB_ZDO_SIMPLE_DESC_NUMBER(); i++)
    {
      found = ZB_FALSE;
      TRACE_MSG(TRACE_ZDO3, "app_profile_id %d", (FMT__D, ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_profile_id));
      if (ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_profile_id == match_params.profile_id)
      {
        TRACE_MSG(TRACE_ZDO3, "app in cluster count %hd", (FMT__H, ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_input_cluster_count));
        for (j = 0; j < ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_input_cluster_count && !found; j++)
        {
          for (k = 0; k < match_params.num_in_clusters && !found; k++)
          {
            if (ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_cluster_list[j] == cluster_id[k])
            {
              ep_num = add_ep_sorted(ep_list, ep_num, ZB_ZDO_SIMPLE_DESC_LIST()[i]->endpoint);
              found = ZB_TRUE;
            }
          }
        }  /* for j .. app_output_cluster_count */

        TRACE_MSG(TRACE_ZDO3, "app out cluster count %hd", (FMT__H, ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_output_cluster_count));
        for (j = 0; j < ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_output_cluster_count && !found; j++)
        {
          for (k = 0; k < match_params.num_out_clusters && !found; k++)
          {
            if (ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_cluster_list[ZB_ZDO_SIMPLE_DESC_LIST()[i]->app_input_cluster_count + j] ==
                cluster_id[match_params.num_in_clusters + k])
            {
              ep_num = add_ep_sorted(ep_list, ep_num, ZB_ZDO_SIMPLE_DESC_LIST()[i]->endpoint);
              found = ZB_TRUE;
            }
          }
        } /* for j .. app_output_cluster_count */
      } /* if app_profile_id == profile_id */
    } /* for i .. simple_desc_number */
  }

  TRACE_MSG(TRACE_ZDO3, "ep_num %hd", (FMT__H, ep_num));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_match_desc_resp_t) + ep_num * sizeof(zb_uint8_t), match_resp);
  ZB_HTOLE16(&match_resp->nwk_addr, &match_params.nwk_addr);
  match_resp->match_len = ep_num;
  if (ep_num)
  {
    status = ZB_ZDP_STATUS_SUCCESS;
    ZB_MEMCPY((zb_uint8_t*)(match_resp + 1), ep_list, ep_num * sizeof(zb_uint8_t));
  }
  else
  {
    status = ZB_ZDP_STATUS_NO_DESCRIPTOR;
  }
  match_resp->status = status;

  zdo_send_resp_by_short(ZDO_MATCH_DESC_RESP_CLID, param, tsn, dst_addr);

  TRACE_MSG(TRACE_ZDO2, "<< zdo_match_desc_res %hd", (FMT__H, status));

#if 0
  if ((ZB_PIB_SHORT_ADDRESS() != req.nwk_addr) &&
      (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ED))
  {
    status  = ZB_ZDP_STATUS_INV_REQUESTTYPE;
  }
  else if ((ZB_PIB_SHORT_ADDRESS() != req.nwk_addr) &&
           ((ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_COORDINATOR) ||
            (ZB_NIB_DEVICE_TYPE() == ZB_NWK_DEVICE_TYPE_ROUTER)))
  {
    zb_uint8_t i;
    zb_neighbor_tbl_ent_t *nbt = NULL;
    zb_address_ieee_ref_t addr_ref;
    zb_uint8_t  found = 0;

    status = zb_address_by_short(req.nwk_addr, ZB_FALSE, ZB_FALSE, &addr_ref);
    if (status == RET_OK)
    {
      for ( i = 0, nbt = &ZG->nwk.neighbor.base_neighbor[i]; i < ZB_NEIGHBOR_TABLE_SIZE; i++ )
      {
        if ((nbt->relationship == ZB_NWK_RELATIONSHIP_CHILD) &&
            (addr_ref == nbt->addr_ref))
        {
          if (ZG->af_desc.simple_desc_number)
          {
            zb_uint8_t j = 0;

            /* FIXME: use blocked memory alloc!!!! */
            out_buf = zb_get_out_buf();
            ZB_BUF_INITIAL_ALLOC(out_buf, 4, ptr);
            match_status = ptr;
            ptr++;
            ZB_MEMCPY(ptr, &req.nwk_addr, sizeof(req.nwk_addr));
            ptr += 2;
            match_len = ptr;
            ptr++;
            for (j=0;j<ZG->af_desc.simple_desc_number;j++)
            {
              if(ZG->af_desc.simple_desc_list[j]->app_profile_id == req.profile_id)
              {
                zb_uint8_t k = 0;
                zb_uint8_t cluster_count = 0;

                cluster_count = *aps_body;
                aps_body++;
                if ((cluster_count != 0) && (ZG->af_desc.simple_desc_list[j]->app_cluster_list[0] != 0))
                {
                  zb_uint16_t in_cluster = 0;
                  zb_uint8_t m = 0;

                  for (m=0;m<cluster_count;m++)
                  {
                    ZB_MEMCPY(&in_cluster, aps_body, sizeof(in_cluster));
                    for(k=0;k<ZG->af_desc.simple_desc_list[j]->app_input_cluster_count;k++)
                    {
                      ZB_MEMCPY(&in_cluster, aps_body, sizeof(in_cluster));
                      if (ZG->af_desc.simple_desc_list[j]->app_cluster_list[k] == in_cluster)
                      {
                        if (!found)
                        {
                          found = 1;
                        }
                        (*match_len)++;
                        ZB_BUF_ALLOC_LEFT(out_buf, 1, ptr);
                        *ptr = ZG->af_desc.simple_desc_list[j]->endpoint;
                      }
                    }
                  }
                }
                if (!found)
                {
                  cluster_count = *aps_body;
                  aps_body++;
                  if ((cluster_count != 0) && (ZG->af_desc.simple_desc_list[j]->app_cluster_list[0] != 0))
                  {
                    zb_uint16_t out_cluster = 0;
                    zb_uint8_t m = 0;

                    for (m=0;m<cluster_count;m++)
                    {
                      ZB_MEMCPY(&out_cluster, aps_body, sizeof(out_cluster));
                      for(k=0;k<ZG->af_desc.simple_desc_list[j]->app_input_cluster_count;k++)
                      {
                        if (ZG->af_desc.simple_desc_list[j]->app_cluster_list[k] == out_cluster)
                        {
                          (*match_len)++;
                          ZB_BUF_ALLOC_LEFT(out_buf, 1, ptr);
                          *ptr = ZG->af_desc.simple_desc_list[j]->endpoint;
                        }
                      }

                    }
                  }
                }
              }
            }
          }
        }
      }
      if (found == 0)
      {
        status = ZB_ZDP_STATUS_NO_DESCRIPTOR;
      }
    }
    else
    {
      status = ZB_ZDP_STATUS_DEVICE_NOT_FOUND;
    }
    *match_status = status;
  }

  if (status)
  {
    len  = 4; /* Table 2.95: Status + NWKAddrOfInterest + MatchLength */
    ZB_BUF_INITIAL_ALLOC(buf, len, ptr);
    *ptr = status;
    ptr++;
    ZB_HTOLE16(ptr, &req.nwk_addr);
    ptr += 2;
    *ptr = 0;
    ptr++;
  }
#endif

}

#endif
/*! @} */
