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
PURPOSES: APS subsystem. Binding.
*/

#include "zb_common.h"
#include "zb_aps.h"
#include "aps_internal.h"
#include "zb_scheduler.h"
#include "zb_hash.h"
#include "zb_address.h"
#include "zb_zdo.h"

#include "zb_bank_6.h"

#ifndef ZB_LIMITED_FEATURES
/*! \addtogroup ZB_APS */
/*! @{ */

zb_uint8_t aps_find_src_ref(zb_address_ieee_ref_t src_addr_ref, zb_uint8_t src_end, zb_uint16_t cluster_id)
{
  zb_uint8_t l = 0;
  do
  {
    if ((ZG->aps.binding.src_table[l].src_addr == src_addr_ref)
        && (ZG->aps.binding.src_table[l].src_end == src_end)
        && (ZG->aps.binding.src_table[l].cluster_id == cluster_id))
    {
      return l;
    }
    l++;
  } while (l < ZG->aps.binding.src_n_elements);
  return (zb_uint8_t)-1;
}

void zb_apsme_bind_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *aps = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  /* zb_apsme_binding_req_t *apsreq = (zb_apsme_binding_req_t *)ZB_BUF_BEGIN(aps);*/
  zb_apsme_binding_req_t *apsreq = ZB_GET_BUF_PARAM(aps, zb_apsme_binding_req_t);
  zb_address_ieee_ref_t src_addr_ref;
  zb_address_ieee_ref_t dst_addr_ref;
  zb_uint8_t s = 0;
  zb_uint8_t d = 0;

  TRACE_MSG(TRACE_APS1, "+zb_apsme_bind_request %p", (FMT__P, aps));

  aps->u.hdr.status = zb_address_by_ieee(apsreq->src_addr, ZB_TRUE, ZB_FALSE, &src_addr_ref);
  if ( aps->u.hdr.status == RET_OK )
  {
    s = aps_find_src_ref(src_addr_ref, apsreq->src_endpoint, apsreq->clusterid);
    s = ( s == (zb_uint8_t)-1 ) ? ZG->aps.binding.src_n_elements : s;
    d = ZG->aps.binding.dst_n_elements;

    if ( s < ZB_APS_SRC_BINDING_TABLE_SIZE
         && d < ZB_APS_DST_BINDING_TABLE_SIZE )
    {
      if ( (int)apsreq->addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT )
      {
        aps->u.hdr.status = zb_address_by_ieee(apsreq->dst_addr.addr_long, ZB_TRUE, ZB_FALSE, &dst_addr_ref);
      }
      else if ( (int)apsreq->addr_mode == ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT )
      {
        /* do nothing */
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "invalid DstAddrMode %hd", (FMT__H, apsreq->addr_mode));
        aps->u.hdr.status = RET_INVALID_BINDING;
      }

      if ( aps->u.hdr.status == RET_OK )
      {
        if ( s == ZG->aps.binding.src_n_elements )
        {
          /* Add new source binding */
          ZG->aps.binding.src_table[s].src_addr = src_addr_ref;
          ZG->aps.binding.src_table[s].src_end = apsreq->src_endpoint;
          ZG->aps.binding.src_table[s].cluster_id = apsreq->clusterid;
          ZG->aps.binding.src_n_elements++;
        }

        if ( (int)apsreq->addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT )
        {
          ZG->aps.binding.dst_table[d].dst_addr_mode = ZB_APS_BIND_DST_ADDR_LONG;
          ZG->aps.binding.dst_table[d].u.long_addr.dst_addr = dst_addr_ref;
          ZG->aps.binding.dst_table[d].u.long_addr.dst_end = apsreq->dst_endpoint;
        }
        else
        {
          ZG->aps.binding.dst_table[d].dst_addr_mode = ZB_APS_BIND_DST_ADDR_GROUP;
          ZG->aps.binding.dst_table[d].u.group_addr = apsreq->dst_addr.addr_short;
        }

        ZG->aps.binding.dst_table[d].src_table_index = s;
        ZG->aps.binding.dst_n_elements++;
      }
    }
    else
    {
      aps->u.hdr.status = ZB_ZDP_STATUS_TABLE_FULL;
    }
  }

  TRACE_MSG(TRACE_APS1, "-zb_apsme_bind_request status %d", (FMT__D, aps->u.hdr.status));
}


void zb_apsme_unbind_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *aps = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_apsme_binding_req_t *apsreq = ZB_GET_BUF_PARAM(aps, zb_apsme_binding_req_t);
  zb_address_ieee_ref_t src_addr_ref = 0;
  zb_address_ieee_ref_t dst_addr_ref = 0;
  zb_uint8_t s, d = 0;
  zb_uint8_t found = 0;

  TRACE_MSG(TRACE_APS1, "+zb_apsme_unbind_request %p", (FMT__P, aps));

  aps->u.hdr.status = zb_address_by_ieee(apsreq->src_addr, ZB_FALSE, ZB_FALSE, &src_addr_ref);
  if ( aps->u.hdr.status == RET_OK )
  {
    s = aps_find_src_ref(src_addr_ref, apsreq->src_endpoint, apsreq->clusterid);
    if (s == (zb_uint8_t)-1)
    {
      aps->u.hdr.status = RET_INVALID_BINDING;
    }
    else
    {
      if ( (int)apsreq->addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT )
      {
        aps->u.hdr.status = zb_address_by_ieee(apsreq->dst_addr.addr_long, ZB_TRUE, ZB_FALSE, &dst_addr_ref);
      }
      else if ( (int)apsreq->addr_mode == ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT )
      {
        /* do nothing */
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "invalid DstAddrMode %hd", (FMT__H, apsreq->addr_mode));
        aps->u.hdr.status = RET_INVALID_BINDING;
      }

      if ( aps->u.hdr.status == RET_OK )
      {
        /* remove all bindings with this dst and src */
        do
        {
          if ( ZG->aps.binding.dst_table[d].src_table_index == s
               && ( ((int)apsreq->addr_mode == ZB_APS_ADDR_MODE_64_ENDP_PRESENT
                     && ZG->aps.binding.dst_table[d].dst_addr_mode == ZB_APS_BIND_DST_ADDR_LONG
                     && ZG->aps.binding.dst_table[d].u.long_addr.dst_addr == dst_addr_ref
                     && ZG->aps.binding.dst_table[d].u.long_addr.dst_end == apsreq->dst_endpoint)
                    || ((int)apsreq->addr_mode == ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT
                        && ZG->aps.binding.dst_table[d].dst_addr_mode == ZB_APS_BIND_DST_ADDR_GROUP
                        && ZG->aps.binding.dst_table[d].u.group_addr == apsreq->dst_addr.addr_short) )
            )
          {
            /* move all records left */
            ZG->aps.binding.dst_n_elements--;
            ZB_MEMMOVE(&ZG->aps.binding.dst_table[d],
                       &ZG->aps.binding.dst_table[d+1],
                       sizeof(zb_aps_bind_dst_table_t)*(ZG->aps.binding.dst_n_elements - d));
          }
          else
          {
            if ( ZG->aps.binding.dst_table[d].src_table_index == s )
            {
              found = 1;
            }
            d++;
          }
        }
        while ( d < ZG->aps.binding.dst_n_elements );

        if ( !found )
        {
          /* remove from src table useless binding record */
          ZG->aps.binding.src_n_elements--;
          ZB_MEMMOVE(&ZG->aps.binding.src_table[s],
                     &ZG->aps.binding.src_table[s+1],
                     sizeof(zb_aps_bind_src_table_t)*(ZG->aps.binding.src_n_elements - s));

          /* correct dst table indexes */
          d = 0;
          do
          {
            if ( ZG->aps.binding.dst_table[d].src_table_index > s )
            {
              ZG->aps.binding.dst_table[d].src_table_index--;
            }
            d++;
          }
          while ( d < ZG->aps.binding.dst_n_elements );
        }
      }
    }
  }
  else
  {
    aps->u.hdr.status = RET_ILLEGAL_REQUEST;
  }

  TRACE_MSG(TRACE_APS1, "-zb_apsme_unbind_request status %d", (FMT__D, aps->u.hdr.status));
}


void zb_apsme_bind_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;

  ZB_MEMCPY(&ret, ZB_BUF_FROM_REF(param), sizeof(ret));
  TRACE_MSG(TRACE_APS2, "+bind_confirm status %d", (FMT__D, ret));
}

void zb_apsme_unbind_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_ret_t ret;

  ZB_MEMCPY(&ret, ZB_BUF_FROM_REF(param), sizeof(ret));
  TRACE_MSG(TRACE_APS2, "+unbind_confirm status %d", (FMT__D, ret));
}


void zb_apsme_add_group_request(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t status = 0;
  zb_apsme_add_group_req_t req;

  ZB_MEMCPY(&req, ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_req_t), sizeof(req));
  TRACE_MSG(TRACE_APS3, "zb_apsme_add_group_request group_addr %d endpoint %hd", (FMT__D_H, req.group_address, req.endpoint));

  {
    zb_ushort_t i, j;

    for (i = 0 ; i < ZG->aps.group.n_groups ; ++i)
    {
      if (ZG->aps.group.groups[i].group_addr == req.group_address)
      {
        for (j = 0 ; j < ZG->aps.group.groups[i].n_endpoints ; ++j)
        {
          if (ZG->aps.group.groups[i].endpoints[j] == req.endpoint)
          {
            goto done;
          }
        }
        if (j < ZB_APS_ENDPOINTS_IN_GROUP_TABLE)
        {
          TRACE_MSG(TRACE_APS3, "Add endpoint #%d", (FMT__H, j));
          ZG->aps.group.groups[i].endpoints[j] = req.endpoint;
          ZG->aps.group.groups[i].n_endpoints++;
        }
        else
        {
          TRACE_MSG(TRACE_ERROR, "No more space for endpoints", (FMT__0));
          status = ZB_APS_STATUS_TABLE_FULL;
        }
        goto done;
      }
    }
    if (i < ZB_APS_GROUP_TABLE_SIZE)
    {
      ZG->aps.group.groups[i].group_addr = req.group_address;
      ZG->aps.group.groups[i].endpoints[0] = req.endpoint;
      ZG->aps.group.groups[i].n_endpoints = 1;
      TRACE_MSG(TRACE_APS3, "Add group #%d", (FMT__H, i));
      ZG->aps.group.n_groups++;
    }
    else
    {
      TRACE_MSG(TRACE_ERROR, "No more space for groups", (FMT__0));
      status = ZB_APS_STATUS_TABLE_FULL;
    }
  }

  done:
  {
    zb_apsme_add_group_conf_t *conf = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_apsme_add_group_conf_t);
    conf->group_address = req.group_address;
    conf->endpoint = req.endpoint;
    conf->status = status;
    /* don not call APSME-ADD-GROUP.confirm via callback. This routine is to be
     * called from zdo_bind_manage.c */
    /* FIXME: implement full scheme with return via callback */
  }
}


#if 0  /* APS Group management not supported */
void zb_apsme_add_group_request(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;
  zb_apsme_group_req_t *apsgrp = ZB_GET_BUF_TAIL(grp, sizeof(zb_apsme_group_req_t));
  zb_address_ieee_ref_t addr_ref;
  zb_ushort_t l = 0;

  TRACE_MSG(TRACE_APS1, ">>apsme_add_group_req %p", (FMT__P, v_grp));
  grp->u.hdr.status = RET_OK;
  if ((apsgrp->group_addr == 0) || (apsgrp->endpoint == 0))
  {
    grp->u.hdr.status = RET_INVALID_PARAMETER;
  }
  else
  {
    grp->u.hdr.status = zb_address_by_short( apsgrp->group_addr, ZB_FALSE, &addr_ref);
    if (grp->u.hdr.status == RET_OK)
    {
      l = ZG->aps.group.grp_n_elements;
      if (l < ZB_APS_GROUP_TABLE_SIZE)
      {
        ZG->aps.group.group_addr[l] = addr_ref;
        ZG->aps.group.endpoint[l] = apsgrp->endpoint;
        ZG->aps.group.grp_n_elements++;
        zb_address_lock( ZG->aps.group.group_addr[l]);
      }
      else
      {
        grp->u.hdr.status = ZB_APS_STATUS_TABLE_FULL;
      }
    }
    else
    {
      grp->u.hdr.status = RET_INVALID_PARAMETER;
    }
  }
  TRACE_MSG(TRACE_APS1, "<<apsme_add_group_req status %d", (FMT__D, grp->u.hdr.status));
}

void zb_apsme_remove_group_request(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;
  zb_apsme_group_req_t *apsgrp = ZB_GET_BUF_TAIL(grp, sizeof(zb_apsme_group_req_t));
  zb_address_ieee_ref_t addr_ref;
  zb_ushort_t l = 0;
  zb_short_t  found = 0;

  TRACE_MSG(TRACE_APS1, ">>apsme_remove_group_req %p", (FMT__P, v_grp));
  grp->u.hdr.status = RET_OK;
  if ((apsgrp->group_addr == 0) || (apsgrp->endpoint == 0))
  {
    grp->u.hdr.status = RET_INVALID_PARAMETER;
  }
  else
  {
    grp->u.hdr.status = zb_address_by_short( apsgrp->group_addr, ZB_FALSE, &addr_ref);
    if (grp->u.hdr.status == RET_OK)
    {
      l = 0;
      do
      {
        if ((ZG->aps.group.group_addr[l] == addr_ref)
            && (ZG->aps.group.endpoint[l] == apsgrp->endpoint))
        {
          if (found == 0)
          {
            found = 1;
          }
          do
          {
            ZG->aps.group.group_addr[l] = ZG->aps.group.group_addr[l+1];
            ZG->aps.group.endpoint[l] = ZG->aps.group.endpoint[l+1];
            l++;
          } while (l < ZG->aps.group.grp_n_elements-1);
          if (found)
          {
            zb_address_unlock( addr_ref);
          }
          ZG->aps.group.grp_n_elements--;
          l++;
        }
      } while (l < ZG->aps.group.grp_n_elements);
    }
    else
    {
      grp->u.hdr.status = RET_INVALID_PARAMETER;
    }
    if (found == 0)
    {
      grp->u.hdr.status = RET_INVALID_GROUP;
    }
  }
  TRACE_MSG(TRACE_APS1, "<<apsme_remove_group_req status %d", (FMT__D, grp->u.hdr.status));
}

void zb_apsme_remove_all_group_request(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;
  zb_apsme_group_req_t *apsgrp = ZB_GET_BUF_TAIL(grp, sizeof(zb_apsme_group_req_t));
  zb_ushort_t l, s = 0;
  zb_short_t  found, unlock = 0;

  TRACE_MSG(TRACE_APS1, ">>apsme_remove_all_group_req %p", (FMT__P, v_grp));
  grp->u.hdr.status = RET_OK;
  if (apsgrp->endpoint == 0)
  {
    grp->u.hdr.status = RET_INVALID_PARAMETER;
  }
  else
  {
    if (grp->u.hdr.status == RET_OK)
    {
      l = 0;
      do
      {
        if (s != 0)
        {
          l = s;
          s = 0;
        }
        if (ZG->aps.group.endpoint[l] == apsgrp->endpoint)
        {
          if (found == 0)
          {
            found = 1;
            unlock = 1;
          }
          s = l;
          do
          {
            ZG->aps.group.group_addr[l] = ZG->aps.group.group_addr[l+1];
            ZG->aps.group.endpoint[l] = ZG->aps.group.endpoint[l+1];
          } while (l < ZG->aps.group.grp_n_elements-1);
          if (unlock)
          {
            zb_address_unlock( ZG->aps.group.group_addr[s]);
            unlock = 0;
          }
          ZG->aps.group.grp_n_elements--;
        }
      } while ((l < ZG->aps.group.grp_n_elements) && (s != 0));
    }
    else
    {
      grp->u.hdr.status = RET_INVALID_PARAMETER;
    }
    if (found == 0)
    {
      grp->u.hdr.status = RET_INVALID_PARAMETER;
    }
  }
  TRACE_MSG(TRACE_APS1, "<<apsme_remove_all_group_req status %d", (FMT__D, grp->u.hdr.status));
}

void zb_apsme_add_group_confirm(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;

  TRACE_MSG(TRACE_APS2, "+add_group_confirm %p status %d", (FMT__P_D, grp, grp->u.hdr.status));
  zb_free_buf( grp);
}

void zb_apsme_remove_group_confirm(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;

  TRACE_MSG(TRACE_APS2, "+remove_group_confirm %p status %d", (FMT__P_D, grp,grp->u.hdr.status));
  zb_free_buf( grp);
}

void zb_apsme_remove_all_group_confirm(ZPAR void *v_grp)
{
  zb_buf_t *grp = (zb_buf_t *)v_grp;

  TRACE_MSG(TRACE_APS2, "+remove_all_group_confirm %p status %d", (FMT__P_D, grp, grp->u.hdr.status));
  zb_free_buf( grp);
}
#endif /* APS Group management not supported */
#endif /* ZB_LIMITED_FEATURES */
/*! @} */
