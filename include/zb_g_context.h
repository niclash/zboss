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
PURPOSE: Global context definition
*/

#ifndef ZB_G_CONTEXT_H
#define ZB_G_CONTEXT_H 1

#include "zb_osif.h"

/*! \cond internals_doc */
/*! \addtogroup ZB_BASE */
/*! @{ */

/**
   \par Define 'global context' - comon global data structure.
   Vladimir got strange problems in Keil with accessing global variables
   implemented in another modules.
   Also, wants to easiely track memory usage.
   So, unite it all into global context.

   Initially suppose global context will be defined here and implemented in the
   single .c module and passed to all functions by pointer.
   To be able to exclude such pointer pass let's define global context access by
   a macro. The macro can be easiely substituted by the global variable access
   or parameter access.

   Globals can be accessed using ZG macro in constructions like ZG->foo.

   Some subsystems has its own structures in the globals (for example, APS
   globals). It can be accesses by special macros, like APSG->bar.
 */
#ifndef ZB_SNIFFER
struct zb_globals_s;
typedef struct zb_globals_s zb_globals_t;
#endif

struct zb_intr_globals_s;
typedef ZB_VOLATILE struct zb_intr_globals_s zb_intr_globals_t;

#ifndef ZB_SNIFFER
extern ZB_SDCC_XDATA zb_globals_t g_zb;
#endif
extern ZB_SDCC_XDATA zb_intr_globals_t g_izb;

/**
   Macro to access globals
 */
/* Hope compiler can optimize &g_zb-> to g_zb. */
#ifndef ZB_SNIFFER
#define ZG (&g_zb)
#endif

#define ZIG (&g_izb)


/*
  Per-subsystem globals files are named like zb_xxx_globals.h and included here.
 */

#ifndef ZB_SNIFFER
#include "zb_scheduler.h"
#include "zb_bufpool_globals.h"
#include "zb_addr_globals.h"
#include "zb_mac_globals.h"
#include "zb_nwk_globals.h"
#include "zb_aps_globals.h"
#include "zb_af_globals.h"
#include "zb_zdo_globals.h"
#include "zb_zcl_globals.h"
#include "zb_ubec24xx.h" /* TODO: configure this include depending
                          * on transceiver */
#else
#include "zb_ringbuffer.h"
#endif /* ZB_SNIFFER */

#include "zb_time.h"
#include "zb_transport_globals.h"

#ifdef ZB_CC25XX
#include "zb_cc25xx.h"
#endif

#ifndef ZB_SNIFFER
/**
   Global data area for data not to be accessed from interrupt handlers
 */
struct zb_globals_s
{
  zb_sched_globals_t      sched;
  zb_buf_pool_t           bpool;
  zb_mac_globals_t        mac;
  zb_nwk_globals_t        nwk;
  zb_aps_globals_t        aps;
  zb_addr_globals_t       addr;
  zb_zdo_globals_t        zdo;
  zb_zcl_globals_t        zcl;
};

#endif

/**
   Global data area for data to be accessed from interrupt handlers
 */
struct zb_intr_globals_s
{
  zb_io_ctx_t             ioctx;
  zb_timer_t              time;
  zb_transceiver_ctx_t    transceiver;
};

#define ZB_IOCTX() g_izb.ioctx
#define ZB_TIMER_CTX() g_izb.time
#define TRANS_CTX() (g_izb.transceiver)
#define SER_CTX() ZB_IOCTX().serial_ctx
#define SPI_CTX() ZB_IOCTX().spi_ctx


/*! @} */
/*! \endcond */

#endif /* ZB_G_CONTEXT_H */
