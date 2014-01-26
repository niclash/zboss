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
PURPOSE: ZigBee trace. Application should include it.
*/

#ifndef ZB_LOGGER_H
#define ZB_LOGGER_H 1

/*! \addtogroup ZB_TRACE */
/*! @{ */

/**
   \par ZigBee trace subsystem.

   Has 2 parameters to switch log messages on/off: mask and level.
   Mask used to exclude some layers trace.
   Level used to trace more or less detailed messages from the same layer.
   Trace can be switched at compile time only osung 2 defines.
   ZB_TRACE_LEVEL is mandatory, ZB_TRACE_MASK is optional.
   No trace code compiled if ZB_TRACE_LEVEL is not defined.

   Trace call looks like:

   TRACE_MSG(TRACE_COMMON3, "%p calling cb %p param %hd", (FMT_P_P_H, (void*)ent, ent->func, ent->param));

   FMT_P_P_H and similar constants are defined in zb_trace_fmts.h and are sum of
   argument sizes. Actual for 8051, ignored in Unix.

   See \see tests/trace.c for usage example.
 */

#ifdef ZB_TRACE_LEVEL
#ifndef ZB_TRACE_MASK
#ifdef ZB_UZ2410
//#define ZB_TRACE_MASK -1
/* 1fb == all but MAC */
#define ZB_TRACE_MASK 0x1FB
#else
#define ZB_TRACE_MASK -1
#endif
#endif


#define TRACE_ENABLED_(mask,lev) ((lev) <= ZB_TRACE_LEVEL && ((mask) & ZB_TRACE_MASK))
/**
 Return 1 if trace at this level is enabled

 To be used in constructions like:
 if (TRACE_ENABLED(TRACE_APS3))
 {
   call_some_complex_trace();
 }

 @param m - trace level macro
 @return 1 if enabled, 0 if disabled
*/
#define TRACE_ENABLED(m) TRACE_ENABLED_(m)


#ifdef UNIX

void zb_trace_init_unix(zb_char_t *name);
void zb_trace_deinit_unix();


/**
 Initialize trace subsystem

 @param name - trace file name component
*/
#define TRACE_INIT(name)   zb_trace_init_unix(name)

/**
 Deinitialize trace subsystem
*/
#define TRACE_DEINIT zb_trace_deinit_unix

/**
 Print trace message
*/
void zb_trace_msg_unix(zb_char_t *format, zb_char_t *file_name, zb_int_t line_number, zb_int_t args_size, ...);

#define _T0(...) __VA_ARGS__
#define _T1(s, l, fmts, args) if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) & ZB_TRACE_MASK)) zb_trace_msg_unix(fmts, _T0 args)
#define TRACE_MSG(lm, fmts, args) _T1(lm, fmts, args)

#elif defined ZB8051

/*
  8051 trace does not use format string in the code to save code space.

- will modify trace at device only, Linux will work as before
- trace implementation will hex dump all arguments as set of bytes
- external utility will parse dump, divide trace arguments dump into separate arguments and
convert hex-int, unsigned etc.
- utility will get argument strings from the source files (trace macros) and find it
by file:line
- Add one more parameter to the trace macro: sum of the trace argument sizes.
Define readable constants like
#define FMT_D_HD_X 5
- create script/program to modify existing trace calls
- combine dump parse utility functionality with win_com_dump, so it will produce human-readable trace

 */

/* No trace init-deinit */
#define TRACE_INIT(name)
#define TRACE_DEINIT()

void zb_trace_msg_8051(zb_char_t ZB_IAR_CODE *file_name, zb_int_t line_number, zb_uint8_t args_size, ...) ZB_SDCC_REENTRANT;
#define _T1(s, l, args) if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) & ZB_TRACE_MASK)) zb_trace_msg_8051 args
#define TRACE_MSG(lm, fmt, args) _T1(lm, args)

#else  /* !unix, !8051 */

#error Port me!

#endif  /* unix, 8051 */


#else  /* if trace off */

#ifndef KEIL
#define TRACE_MSG(...)
#else
/* Keil does not support vararg macros */
#define TRACE_MSG(a,b,c)
#endif

#define TRACE_INIT(name)
#define TRACE_DEINIT(c)

#define TRACE_ENABLED(m) 0

#endif  /* trace on/off */


#if defined(UNIX)
/**
 Trace format for 64-bit address
*/
#define TRACE_FORMAT_64 "%hx.%hx.%hx.%hx.%hx.%hx.%hx.%hx"

/**
 Trace format arguments for 64-bit address
*/
#define TRACE_ARG_64(a) (zb_uint8_t)((a)[7]),(zb_uint8_t)((a)[6]),(zb_uint8_t)((a)[5]),(zb_uint8_t)((a)[4]),(zb_uint8_t)((a)[3]),(zb_uint8_t)((a)[2]),(zb_uint8_t)((a)[1]),(zb_uint8_t)((a)[0])

#elif defined(SDCC)

#define TRACE_FORMAT_64 "%A"

typedef struct zb_addr64_struct_s
{
  zb_uint16_t a1;
  zb_uint16_t a2;
  zb_uint16_t a3;
  zb_uint16_t a4;
} zb_addr64_struct_t;

/* Pass 8-bytes address as structure by value */
#define TRACE_ARG_64(a) ((zb_addr64_struct_t ZB_SDCC_XDATA * ZB_SDCC_XDATA)a)->a1, ((zb_addr64_struct_t ZB_SDCC_XDATA * ZB_SDCC_XDATA)a)->a2, ((zb_addr64_struct_t ZB_SDCC_XDATA * ZB_SDCC_XDATA)a)->a3, ((zb_addr64_struct_t ZB_SDCC_XDATA * ZB_SDCC_XDATA)a)->a4


#else
/**
 Trace format for 64-bit address - single argument for 8051
*/
#define TRACE_FORMAT_64 "%A"

typedef struct zb_addr64_struct_s
{
  zb_64bit_addr_t addr;
} zb_addr64_struct_t;

/* Pass 8-bytes address as structure by value */
#define TRACE_ARG_64(a) *((zb_addr64_struct_t *)a)

#endif  /* !UNIX */




/**
 General trace message definition: error
*/
#define TRACE_ERROR -1, 1
#define TRACE_INFO1 -1, 2
#define TRACE_INFO2 -1, 3
#define TRACE_INFO3 -1, 4

/**
  \par Trace subsystems
 */
#define TRACE_SUBSYSTEM_COMMON    0x0001
#define TRACE_SUBSYSTEM_OSIF      0x0002
#define TRACE_SUBSYSTEM_MAC       0x0004
#define TRACE_SUBSYSTEM_NWK       0x0008
#define TRACE_SUBSYSTEM_APS       0x0010
#define TRACE_SUBSYSTEM_AF        0x0020
#define TRACE_SUBSYSTEM_ZDO       0x0040
#define TRACE_SUBSYSTEM_SECUR     0x0080
#define TRACE_SUBSYSTEM_ZCL       0x0100
/* to be continued... */

/*
  Trace format constants for 8051
*/

/* Keil and sdcc put byte values to the stack as is, but IAR casts it to 16-bit
 * integers, so constant lengths differs */

#ifndef ZB_IAR

#define FMT__0                                   __FILE__,__LINE__, 0
#define FMT__A                                   __FILE__,__LINE__, 8
#define FMT__A_A                                 __FILE__,__LINE__, 16
#define FMT__A_D_A_P                             __FILE__,__LINE__, 21
#define FMT__A_D_D_P_H                           __FILE__,__LINE__, 16
#define FMT__A_D_H                               __FILE__,__LINE__, 11
#define FMT__C                                   __FILE__,__LINE__, 1
#define FMT__D                                   __FILE__,__LINE__, 2
#define FMT__D_A                                 __FILE__,__LINE__, 10
#define FMT__D_A_D_D_D_D_D_D_D_D                 __FILE__,__LINE__, 26
#define FMT__D_A_D_P_H_H_H                       __FILE__,__LINE__, 18
#define FMT__D_A_P                               __FILE__,__LINE__, 13
#define FMT__A_P                                 __FILE__,__LINE__, 11
#define FMT__D_C                                 __FILE__,__LINE__, 3
#define FMT__D_D                                 __FILE__,__LINE__, 4
#define FMT__D_D_A_D                             __FILE__,__LINE__, 14
#define FMT__D_D_A_D_D_D_D                       __FILE__,__LINE__, 20
#define FMT__D_D_D                               __FILE__,__LINE__, 6
#define FMT__D_D_D_C                             __FILE__,__LINE__, 7
#define FMT__D_D_D_D                             __FILE__,__LINE__, 8
#define FMT__D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D   __FILE__,__LINE__, 34
#define FMT__D_D_D_P                             __FILE__,__LINE__, 9
#define FMT__D_D_P                               __FILE__,__LINE__, 7
#define FMT__D_D_P_D                             __FILE__,__LINE__, 9
#define FMT__D_D_P_P_P                           __FILE__,__LINE__, 13
#define FMT__D_H                                 __FILE__,__LINE__, 3
#define FMT__D_D_H                               __FILE__,__LINE__, 5
#define FMT__D_H_H                               __FILE__,__LINE__, 4
#define FMT__D_H_H_H_H_H_H_D_D_D_D               __FILE__,__LINE__, 16
#define FMT__D_H_P                               __FILE__,__LINE__, 6
#define FMT__D_P                                 __FILE__,__LINE__, 5
#define FMT__D_P_D                               __FILE__,__LINE__, 7
#define FMT__D_P_H_H_D_H_H                       __FILE__,__LINE__, 11
#define FMT__D_P_P                               __FILE__,__LINE__, 8
#define FMT__D_P_P_D_D_H_H                       __FILE__,__LINE__, 14
#define FMT__D_P_P_H                             __FILE__,__LINE__, 9
#define FMT__H                                   __FILE__,__LINE__, 1
#define FMT__H_A                                 __FILE__,__LINE__, 9
#define FMT__H_A_A                               __FILE__,__LINE__, 17
#define FMT__H_A_H_H_H_H_H_H_H_H                 __FILE__,__LINE__, 17
#define FMT__H_C_D_C                             __FILE__,__LINE__, 5
#define FMT__H_D                                 __FILE__,__LINE__, 3
#define FMT__H_D_A_H_D                           __FILE__,__LINE__, 14
#define FMT__H_D_A_H_H_H_H                       __FILE__,__LINE__, 15
#define FMT__H_D_D                               __FILE__,__LINE__, 5
#define FMT__H_D_D_D_H_H_D                       __FILE__,__LINE__, 11
#define FMT__H_H                                 __FILE__,__LINE__, 2
#define FMT__H_H_D                               __FILE__,__LINE__, 4
#define FMT__H_H_H                               __FILE__,__LINE__, 3
#define FMT__H_H_H_H                             __FILE__,__LINE__, 4
#define FMT__H_H_P                               __FILE__,__LINE__, 5
#define FMT__H_P                                 __FILE__,__LINE__, 4
#define FMT__L_L                                 __FILE__,__LINE__, 8
#define FMT__P                                   __FILE__,__LINE__, 3
#define FMT__P_D                                 __FILE__,__LINE__, 5
#define FMT__P_D_D                               __FILE__,__LINE__, 7
#define FMT__P_D_D_D                             __FILE__,__LINE__, 9
#define FMT__P_D_D_D_D_D                         __FILE__,__LINE__, 13
#define FMT__P_D_D_D_D_D_D                       __FILE__,__LINE__, 15
#define FMT__P_D_D_D_D_D_D_D                     __FILE__,__LINE__, 17
#define FMT__P_D_D_D_H_D                         __FILE__,__LINE__, 12
#define FMT__P_D_H                               __FILE__,__LINE__, 6
#define FMT__P_D_P                               __FILE__,__LINE__, 8
#define FMT__P_H                                 __FILE__,__LINE__, 4
#define FMT__P_H_D                               __FILE__,__LINE__, 6
#define FMT__P_H_H                               __FILE__,__LINE__, 5
#define FMT__P_H_H_L                             __FILE__,__LINE__, 9
#define FMT__P_H_L                               __FILE__,__LINE__, 8
#define FMT__P_H_P_H_L                           __FILE__,__LINE__, 12
#define FMT__P_H_P_P                             __FILE__,__LINE__, 10
#define FMT__P_H_P_P_P                           __FILE__,__LINE__, 13
#define FMT__P_P                                 __FILE__,__LINE__, 6
#define FMT__P_P_D                               __FILE__,__LINE__, 8
#define FMT__P_P_D_D_H                           __FILE__,__LINE__, 11
#define FMT__P_P_D_H_H                           __FILE__,__LINE__, 10
#define FMT__P_P_H                               __FILE__,__LINE__, 7
#define FMT__P_P_P                               __FILE__,__LINE__, 9
#define FMT__H_H_H_D_D_H_A_H_A                   __FILE__,__LINE__, 25
#define FMT__H_H_P_P_P                           __FILE__,__LINE__, 11
#define FMT__D_H_D_P_D                           __FILE__,__LINE__, 10
#define FMT__D_D_D_D_D                           __FILE__,__LINE__, 10
#define FMT__H_D_D_D_D                           __FILE__,__LINE__, 9
#define FMT__D_D_D_D_H                           __FILE__,__LINE__, 9
#define FMT__D_H_H_D                             __FILE__,__LINE__, 6
#define FMT__D_P_D_D                             __FILE__,__LINE__, 9
#define FMT__H_H_H_D                             __FILE__,__LINE__, 5
#define FMT__H_D_H_H                             __FILE__,__LINE__, 5
#define FMT__P_H_H_H_H_H_H_H                     __FILE__,__LINE__, 10
#define FMT__P_H_H_H_H_H_H                       __FILE__,__LINE__, 9
#define FMT__D_D_H_D_H                           __FILE__,__LINE__, 8
#define FMT__H_D_D_H_H_H_H                       __FILE__,__LINE__, 9
#define FMT__H_H_A_A                             __FILE__,__LINE__, 18
#define FMT__P_H_P_P_H                           __FILE__,__LINE__, 11
#define FMT__P_H_P_H                             __FILE__,__LINE__, 8
#define FMT__A_D_D                               __FILE__,__LINE__, 12
#define FMT__P_H_H_H                             __FILE__,__LINE__, 6
#define FMT__P_H_P                               __FILE__,__LINE__, 7
#define FMT__P_P_H_H                             __FILE__,__LINE__, 8
#define FMT__D_P_H_H_D_D                         __FILE__,__LINE__, 11
#define FMT__A_H                                 __FILE__,__LINE__, 9
#define FMT__P_H_D_L                             __FILE__,__LINE__, 10
#define FMT__H_H_H_P                             __FILE__,__LINE__, 6
#define FMT__A_D_P_H_H_H                         __FILE__,__LINE__, 16
#define FMT__H_P_H_P_H_H                         __FILE__,__LINE__, 10
#define FMT__H_P_H_H_H_H                         __FILE__,__LINE__, 8
#define FMT_H_D_H_H_H_H_H_H                      __FILE__,__LINE__, 9
#define FMT__H_D_D_H_H_H                         __FILE__,__LINE__, 8
#define FMT__D_D_H_H                             __FILE__,__LINE__, 6
#define FMT__H_H_D_H                             __FILE__,__LINE__, 5
#define FMT__D_H_H_H_H                           __FILE__,__LINE__, 6
#define FMT__H_H_H_D_H                           __FILE__,__LINE__, 6
#define FMT__H_D_H                               __FILE__,__LINE__, 4
#define FMT__H_D_H_D                             __FILE__,__LINE__, 6
#define FMT__D_H_D_H_H                           __FILE__,__LINE__, 7
#define FMT__H_P_H_P_H                           __FILE__,__LINE__, 9
#define FMT__H_P_H_P_H_H                         __FILE__,__LINE__, 10
#define FMT__H_P_H_H_H                           __FILE__,__LINE__, 7
#define FMT__D_H_D_H                             __FILE__,__LINE__, 6
#define FMT__D_H_H_H                             __FILE__,__LINE__, 5
#define FMT__H_H_D_H_P                           __FILE__,__LINE__, 8
#define FMT__H_H_H_D_H_P                         __FILE__,__LINE__, 9
#define FMT__A_H_H                               __FILE__,__LINE__, 10
#define FMT__P_H_H_H_H                           __FILE__,__LINE__, 7

#define FMT__H_D_P_H_H_H_H_H                     __FILE__,__LINE__, 11
#define FMT__P_H_H_H_L                           __FILE__,__LINE__, 10
#define FMT__H_H_H_H_H_H_H_H                     __FILE__,__LINE__, 8
#define FMT__H_H_H_H_H_H_H                       __FILE__,__LINE__, 7
#define FMT__H_H_H_H_H_H                         __FILE__,__LINE__, 6
#define FMT__H_H_H_H_H                           __FILE__,__LINE__, 5
#define FMT__H_D_H_H_H                           __FILE__,__LINE__, 6
#define FMT__D_D_D_D_D_D                         __FILE__,__LINE__, 12
#define FMT__P_H_H_H_H_D                         __FILE__,__LINE__, 7
#define FMT__H_D_D_H_D_H                         __FILE__,__LINE__, 9
#define FMT__H_P_H                               __FILE__,__LINE__, 5
#define FMT__H_H_D_D                             __FILE__,__LINE__, 6

#else  /* IAR */

#define FMT__0                                   __FILE__,__LINE__, 0
#define FMT__A                                   __FILE__,__LINE__, 8
#define FMT__A_A                                 __FILE__,__LINE__, 16
#define FMT__A_D_A_P                             __FILE__,__LINE__, 20
#define FMT__A_D_D_P_H                           __FILE__,__LINE__, 16
#define FMT__A_D_H                               __FILE__,__LINE__, 12
#define FMT__C                                   __FILE__,__LINE__, 2
#define FMT__D                                   __FILE__,__LINE__, 2
#define FMT__D_A                                 __FILE__,__LINE__, 10
#define FMT__D_A_D_D_D_D_D_D_D_D                 __FILE__,__LINE__, 26
#define FMT__D_A_D_P_H_H_H                       __FILE__,__LINE__, 20
#define FMT__D_A_P                               __FILE__,__LINE__, 12
#define FMT__A_P                                 __FILE__,__LINE__, 10
#define FMT__D_C                                 __FILE__,__LINE__, 4
#define FMT__D_D                                 __FILE__,__LINE__, 4
#define FMT__D_D_A_D                             __FILE__,__LINE__, 14
#define FMT__D_D_A_D_D_D_D                       __FILE__,__LINE__, 20
#define FMT__D_D_D                               __FILE__,__LINE__, 6
#define FMT__D_D_D_C                             __FILE__,__LINE__, 8
#define FMT__D_D_D_D                             __FILE__,__LINE__, 8
#define FMT__D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D   __FILE__,__LINE__, 34
#define FMT__D_D_D_P                             __FILE__,__LINE__, 8
#define FMT__D_D_P                               __FILE__,__LINE__, 6
#define FMT__D_D_P_D                             __FILE__,__LINE__, 8
#define FMT__D_D_P_P_P                           __FILE__,__LINE__, 10
#define FMT__D_H                                 __FILE__,__LINE__, 4
#define FMT__D_D_H                               __FILE__,__LINE__, 6
#define FMT__D_H_H                               __FILE__,__LINE__, 6
#define FMT__D_H_H_H_H_H_H_D_D_D_D               __FILE__,__LINE__, 22
#define FMT__D_H_P                               __FILE__,__LINE__, 6
#define FMT__D_P                                 __FILE__,__LINE__, 4
#define FMT__D_P_D                               __FILE__,__LINE__, 6
#define FMT__D_P_H_H_D_H_H                       __FILE__,__LINE__, 14
#define FMT__D_P_P                               __FILE__,__LINE__, 6
#define FMT__D_P_P_D_D_H_H                       __FILE__,__LINE__, 14
#define FMT__D_P_P_H                             __FILE__,__LINE__, 8
#define FMT__H                                   __FILE__,__LINE__, 2
#define FMT__H_A                                 __FILE__,__LINE__, 10
#define FMT__H_A_A                               __FILE__,__LINE__, 18
#define FMT__H_A_H_H_H_H_H_H_H_H                 __FILE__,__LINE__, 26
#define FMT__H_C_D_C                             __FILE__,__LINE__, 8
#define FMT__H_D                                 __FILE__,__LINE__, 4
#define FMT__H_D_A_H_D                           __FILE__,__LINE__, 16
#define FMT__H_D_A_H_H_H_H                       __FILE__,__LINE__, 20
#define FMT__H_D_D                               __FILE__,__LINE__, 6
#define FMT__H_D_D_D_H_H_D                       __FILE__,__LINE__, 14
#define FMT__H_H                                 __FILE__,__LINE__, 4
#define FMT__H_H_D                               __FILE__,__LINE__, 6
#define FMT__H_H_H                               __FILE__,__LINE__, 6
#define FMT__H_H_H_H                             __FILE__,__LINE__, 8
#define FMT__H_H_P                               __FILE__,__LINE__, 6
#define FMT__H_P                                 __FILE__,__LINE__, 4
#define FMT__L_L                                 __FILE__,__LINE__, 8
#define FMT__P                                   __FILE__,__LINE__, 2
#define FMT__P_D                                 __FILE__,__LINE__, 4
#define FMT__P_D_D                               __FILE__,__LINE__, 6
#define FMT__P_D_D_D                             __FILE__,__LINE__, 8
#define FMT__P_D_D_D_D_D                         __FILE__,__LINE__, 12
#define FMT__P_D_D_D_D_D_D                       __FILE__,__LINE__, 14
#define FMT__P_D_D_D_D_D_D_D                     __FILE__,__LINE__, 16
#define FMT__P_D_D_D_H_D                         __FILE__,__LINE__, 12
#define FMT__P_D_H                               __FILE__,__LINE__, 6
#define FMT__P_D_P                               __FILE__,__LINE__, 6
#define FMT__P_H                                 __FILE__,__LINE__, 4
#define FMT__P_H_D                               __FILE__,__LINE__, 6
#define FMT__P_H_H                               __FILE__,__LINE__, 6
#define FMT__P_H_H_L                             __FILE__,__LINE__, 10
#define FMT__P_H_L                               __FILE__,__LINE__, 8
#define FMT__P_H_P_H_L                           __FILE__,__LINE__, 12
#define FMT__P_H_P_P                             __FILE__,__LINE__, 8
#define FMT__P_H_P_P_P                           __FILE__,__LINE__, 10
#define FMT__P_P                                 __FILE__,__LINE__, 4
#define FMT__P_P_D                               __FILE__,__LINE__, 6
#define FMT__P_P_D_D_H                           __FILE__,__LINE__, 10
#define FMT__P_P_D_H_H                           __FILE__,__LINE__, 10
#define FMT__P_P_H                               __FILE__,__LINE__, 6
#define FMT__P_P_P                               __FILE__,__LINE__, 6
#define FMT__H_H_H_D_D_H_A_H_A                   __FILE__,__LINE__, 30
#define FMT__H_H_P_P_P                           __FILE__,__LINE__, 10
#define FMT__D_H_D_P_D                           __FILE__,__LINE__, 10
#define FMT__D_D_D_D_D                           __FILE__,__LINE__, 10
#define FMT__H_D_D_D_D                           __FILE__,__LINE__, 10
#define FMT__D_D_D_D_H                           __FILE__,__LINE__, 10
#define FMT__D_H_H_D                             __FILE__,__LINE__, 8
#define FMT__D_P_D_D                             __FILE__,__LINE__, 8
#define FMT__H_H_H_D                             __FILE__,__LINE__, 8
#define FMT__H_D_H_H                             __FILE__,__LINE__, 8
#define FMT__P_H_H_H_H_H_H_H                     __FILE__,__LINE__, 16
#define FMT__P_H_H_H_H_H_H                       __FILE__,__LINE__, 14
#define FMT__D_D_H_D_H                           __FILE__,__LINE__, 10
#define FMT__H_D_D_H_H_H_H                       __FILE__,__LINE__, 14
#define FMT__H_H_A_A                             __FILE__,__LINE__, 20
#define FMT__P_H_P_P_H                           __FILE__,__LINE__, 10
#define FMT__P_H_P_H                             __FILE__,__LINE__, 8
#define FMT__A_D_D                               __FILE__,__LINE__, 12
#define FMT__P_H_H_H                             __FILE__,__LINE__, 8
#define FMT__P_H_P                               __FILE__,__LINE__, 6
#define FMT__P_P_H_H                             __FILE__,__LINE__, 8
#define FMT__D_P_H_H_D_D                         __FILE__,__LINE__, 12
#define FMT__A_H                                 __FILE__,__LINE__, 10
#define FMT__P_H_D_L                             __FILE__,__LINE__, 10
#define FMT__H_H_H_P                             __FILE__,__LINE__, 8
#define FMT__H_P_H_H_H_H                         __FILE__,__LINE__, 12
#define FMT__A_D_P_H_H_H                         __FILE__,__LINE__, 18
#define FMT_H_D_H_H_H_H_H_H                      __FILE__,__LINE__, 9
#define FMT__H_D_D_H_H_H                         __FILE__,__LINE__, 12
#define FMT__D_D_H_H                             __FILE__,__LINE__, 8
#define FMT__H_H_D_H                             __FILE__,__LINE__, 8
#define FMT__D_H_H_H_H                           __FILE__,__LINE__, 10
#define FMT__H_H_H_D_H                           __FILE__,__LINE__, 10
#define FMT__H_D_H                               __FILE__,__LINE__, 6
#define FMT__H_D_H_D                             __FILE__,__LINE__, 8
#define FMT__D_H_D_H_H                           __FILE__,__LINE__, 10
#define FMT__H_P_H_P_H                           __FILE__,__LINE__, 10
#define FMT__D_H_D_H                             __FILE__,__LINE__, 8
#define FMT__D_H_H_H                             __FILE__,__LINE__, 8
#define FMT__H_H_D_H_P                           __FILE__,__LINE__, 10
#define FMT__H_H_H_D_H_P                         __FILE__,__LINE__, 12
#define FMT__A_H_H                               __FILE__,__LINE__, 12
#define FMT__P_H_H_H_H                           __FILE__,__LINE__, 10
#define FMT__H_D_P_H_H_H_H_H                     __FILE__,__LINE__, 16
#define FMT__P_H_H_H_L                           __FILE__,__LINE__, 12
#define FMT__H_H_H_H_H_H_H_H                     __FILE__,__LINE__, 16
#define FMT__H_H_H_H_H_H_H                       __FILE__,__LINE__, 14
#define FMT__H_H_H_H_H_H                         __FILE__,__LINE__, 12
#define FMT__H_H_H_H_H                           __FILE__,__LINE__, 10
#define FMT__H_D_H_H_H                           __FILE__,__LINE__, 10
#define FMT__D_D_D_D_D_D                         __FILE__,__LINE__, 12
#define FMT__H_P_H                               __FILE__,__LINE__, 6
#define FMT__H_H_D_D                             __FILE__,__LINE__, 8
#define FMT__H_D_D_H_D_H                         __FILE__,__LINE__, 9
#endif  /* IAR */

/**
 \par per-subsystem trace definitions
*/
#define TRACE_COMMON1 TRACE_SUBSYSTEM_COMMON, 1
#define TRACE_COMMON2 TRACE_SUBSYSTEM_COMMON, 2
#define TRACE_COMMON3 TRACE_SUBSYSTEM_COMMON, 3

#define TRACE_OSIF1 TRACE_SUBSYSTEM_OSIF, 1
#define TRACE_OSIF2 TRACE_SUBSYSTEM_OSIF, 2
#define TRACE_OSIF3 TRACE_SUBSYSTEM_OSIF, 3

#define TRACE_MAC1 TRACE_SUBSYSTEM_MAC, 1
#define TRACE_MAC2 TRACE_SUBSYSTEM_MAC, 2
#define TRACE_MAC3 TRACE_SUBSYSTEM_MAC, 3

#define TRACE_NWK1 TRACE_SUBSYSTEM_NWK, 1
#define TRACE_NWK2 TRACE_SUBSYSTEM_NWK, 2
#define TRACE_NWK3 TRACE_SUBSYSTEM_NWK, 3

#define TRACE_APS1 TRACE_SUBSYSTEM_APS, 1
#define TRACE_APS2 TRACE_SUBSYSTEM_APS, 2
#define TRACE_APS3 TRACE_SUBSYSTEM_APS, 3

#define TRACE_AF1 TRACE_SUBSYSTEM_AF, 1
#define TRACE_AF2 TRACE_SUBSYSTEM_AF, 2
#define TRACE_AF3 TRACE_SUBSYSTEM_AF, 3

#define TRACE_ZDO1 TRACE_SUBSYSTEM_ZDO, 1
#define TRACE_ZDO2 TRACE_SUBSYSTEM_ZDO, 2
#define TRACE_ZDO3 TRACE_SUBSYSTEM_ZDO, 3

#define TRACE_SECUR1 TRACE_SUBSYSTEM_SECUR, 1
#define TRACE_SECUR2 TRACE_SUBSYSTEM_SECUR, 2
#define TRACE_SECUR3 TRACE_SUBSYSTEM_SECUR, 3

#define TRACE_ZCL1 TRACE_SUBSYSTEM_ZCL, 1
#define TRACE_ZCL2 TRACE_SUBSYSTEM_ZCL, 2
#define TRACE_ZCL3 TRACE_SUBSYSTEM_ZCL, 3

/*! @} */

#endif /* ZB_LOGGER_H */
