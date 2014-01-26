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
PURPOSE: Buffer pool globals declaration
*/

#ifndef ZB_BUFPOOL_GLOBALS_H
#define ZB_BUFPOOL_GLOBALS_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_BASE */
/*! @{ */

#include "zb_bufpool.h"

typedef struct
{
  zb_callback_t callback;
  zb_ret_t      ret_code;
} zb_buf_pool_delayed_t;

ZB_RING_BUFFER_DECLARE( zb_ring_buf_pool, zb_buf_pool_delayed_t, ZB_SCHEDULER_Q_SIZE );

typedef struct zb_buf_pool_s
{
  zb_ring_buf_pool_t delayed_allocs;
  zb_buf_s_t pool[ZB_IOBUF_POOL_SIZE];
  zb_buf_t ZB_XDATA *head;
  zb_uint8_t bufs_allocated[2];
  
} zb_buf_pool_t;

/*! @} */
/*! \endcond */

#endif /* ZB_BUFPOOL_GLOBALS_H */
