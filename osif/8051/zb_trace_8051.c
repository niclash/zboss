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
PURPOSE: Logger implementation for Unix standalone.
*/

#include <stdarg.h>
#include "zb_common.h"
#include "zb_osif.h"
#include "zb_ringbuffer.h"
#include "zb_mac_transport.h"

#if defined ZB8051 && defined ZB_TRACE_LEVEL

/*! \addtogroup ZB_TRACE */
/*! @{ */

#include "zb_bank_common.h"

/* trace function which does not use printf() */

void zb_flush_putchar();

#define PRINTU_MACRO(v)                         \
{                                               \
  static char s[6];                             \
  zb_ushort_t i = 6;                            \
  do                                            \
  {                                             \
    s[--i] = '0' + (v) % 10;                    \
    (v) /= 10;                                  \
  }                                             \
  while (v);                                    \
  while (i < 6)                                 \
  {                                             \
    zb_putchar(s[i]);                           \
    i++;                                        \
  }                                             \
}


#if 1
static void printusp(zb_uint_t v)
{
  PRINTU_MACRO(v);
  zb_putchar(' ');
}
#else
#define printusp(_v)                            \
{                                               \
  zb_uint_t v = (_v);                           \
  PRINTU_MACRO(v);                              \
  zb_putchar(' ');                              \
}
#endif

static void printx2(zb_uint8_t v)
{
  static ZB_CODE char const s_x_tbl[] = "0123456789abcdef";
  zb_putchar(s_x_tbl[((v) >> 4) & 0xf]);
  zb_putchar(s_x_tbl[((v) & 0xf)]);
}


/**
   Output trace message.

   @param file_name - source file name
   @param line_number - source file line
   @param args_size - total size of the trace arguments
   @param ... - trace arguments
 */
void zb_trace_msg_8051(zb_char_t ZB_IAR_CODE *file_name, zb_int_t line_number, zb_uint8_t args_size, ...)  ZB_SDCC_REENTRANT
{
  static zb_uint16_t global_counter = 0;

  zb_flush_putchar();

  /* Has no always running timer - print counter */
  /* %d %d %s:%d */
  printusp(global_counter);
  global_counter++;

  /* Put timer (in beacon intervals). Not normal timer (not always run, not
   * too precesious), but helpful for timeouts discover */
  printusp(ZB_TIMER_GET());

#if 0
  {
    zb_ushort_t i = 0;
    while (file_name[i])
    {
      zb_putchar(file_name[i]);
      i++;
    }
  }
#else
  while (*file_name)
  {
    zb_putchar(*file_name);
    file_name++;
  }
#endif
  zb_putchar(':');
  printusp(line_number);
  zb_flush_putchar();

  {
    va_list   arglist;
    va_start(arglist, args_size);
    while (args_size)
    {
      printx2(va_arg(arglist, unsigned char));
      args_size--;
    }
    va_end(arglist);
  }

  zb_putchar('\n');
  zb_flush_putchar();
}

#endif /* ZB8051 and trace on */

/*! @} */
