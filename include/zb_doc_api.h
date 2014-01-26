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
PURPOSE: This header not used for build. Its meaning is to define groups for
generation of API reference document.
*/

#ifndef ZB_DOC_API_H
#define ZB_DOC_API_H 1

/**
  @mainpage ZBOSS v1.0

  <p>
  <strong>ZBOSS v1.0</strong> is the open-source <em>ZigBee®</em> protocol stack implementing <em>ZigBee® 2007</em> specification<br/>
  certified by the <em>ZigBee® Alliance</em>. <strong>ZBOSS</strong> is a high-performance, small memory footprint,
  cross-platform solution.<br/>
  This document provides <em>ZBOSS v1.0 API manual</em>, go to <a href="modules.html">API sections</a> or 
  <a href="annotated.html">Data structure</a> tabs for details.
  </p>
<!--
  \see init_api
  \see zdo_init
  \see zdo_api
  \see af_api
  \see aps_api
  \see nwk_api
  \see mac_api
  \see secur_api

  \see base_types
  \see buf
  \see sched
  \see time
  \see ZB_CONFIG
/>
 */
/**
   @defgroup init_api Stack initialization API
*/
/*! @{ */
/**
   @defgroup zdo_init ZDO init and main() structure
*/
/*! @} */
/**
   @defgroup zdo_api ZDO API
 */
/*! @{ */
/**
   @defgroup zdo_ib ZDO Informational Base
   @defgroup zdo_base ZDO base constants and definitions
   @defgroup zdo_disc ZDO discovery services
   @defgroup zdo_mgmt ZDO management services
*/
/*! @} */

/**
   @defgroup af_api AF functions visible to applications
   @defgroup aps_api APS functions visible to applications
*/
/*! @{ */
/*!   @defgroup aps_ib APS Informational Base */
/*! @} */

/*! @defgroup nwk_api NWK functions visible to applications */
/*! @{ */
/*!   @defgroup nwk_ib NWK Informational Base */
/*! @} */

/**
   @defgroup mac_api MAC API
   @defgroup secur_api Security subsystem API
*/

/*! @defgroup ll Low level API */
/*! @{ */
/**
   @defgroup ZB_CONFIG Compile-time configuration parameters
   @defgroup base_types Base typedefs
   @defgroup buf Packet buffers pool
   @defgroup sched Scheduler
   @defgroup time Time
   @defgroup ZB_TRACE Debug trace
*/
/*! @} */

#endif /* ZB_DOC_API_H */
