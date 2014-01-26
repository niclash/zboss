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
PURPOSE: debug stuff
*/

#include "zb_common.h"
#include "zb_osif.h"

#include "zb_bank_common.h"

/*! \addtogroup ZB_DEBUG */
/*! @{ */

#if defined DEBUG || defined USE_ASSERT

/**
   Abort execution in system-specific manner.

   Write trace message before abort.
 */
void zb_abort(char *caller_file, int caller_line)
{
  TRACE_MSG (TRACE_ERROR, "Abort from %s:%d ", (FMT__D, caller_file, (int)caller_line));
  ZB_ABORT();
}

zb_void_t zb_assert(zb_char_t *file_name, zb_int_t line_number/*, zb_bool_t expr*//*, zb_char_t *expr_name*/)
{
  TRACE_MSG (TRACE_ERROR, "Assertion failed", (file_name, line_number, 0));
  ZB_ABORT();
}

#endif  /* DEBUG */


#ifdef ZB_MAC_TESTING_MODE
void dump_traf(zb_uint8_t *buf, zb_ushort_t len)
{
  zb_ushort_t i;

  TRACE_MSG(TRACE_MAC2, "len %hd", (FMT__H, len));

#define HEX_ARG(n) buf[i+n]

  for (i = 0 ; i < len ; )
  {
    if (len - i >= 8)
    {
      TRACE_MSG(TRACE_MAC2, "%hx %hx %hx %hx %hx %hx %hx %hx", 
                (FMT__H_H_H_H_H_H_H_H,
                 HEX_ARG(0), HEX_ARG(1), HEX_ARG(2), HEX_ARG(3),
                 HEX_ARG(4), HEX_ARG(5), HEX_ARG(6), HEX_ARG(7)));
      i += 8;
    }
    else
    {
      switch (len - i)
      {
        case 7:
          TRACE_MSG(TRACE_MAC2, "%hx %hx %hx %hx %hx %hx %hx", 
                    (FMT__H_H_H_H_H_H_H,
                     HEX_ARG(0), HEX_ARG(1), HEX_ARG(2), HEX_ARG(3),
                     HEX_ARG(4), HEX_ARG(5), HEX_ARG(6)));
          break;
        case 6:
          TRACE_MSG(TRACE_MAC2, "%hx %hx %hx %hx %hx %hx", 
                    (FMT__H_H_H_H_H_H,
                     HEX_ARG(0), HEX_ARG(1), HEX_ARG(2), HEX_ARG(3),
                     HEX_ARG(4), HEX_ARG(5)));
          break;
        case 5:
          TRACE_MSG(TRACE_MAC2, "%hx %hx %hx %hx %hx", 
                    (FMT__H_H_H_H_H,
                     HEX_ARG(0), HEX_ARG(1), HEX_ARG(2), HEX_ARG(3),
                     HEX_ARG(4)));
          break;
        case 4:
          TRACE_MSG(TRACE_MAC2, "%hx %hx %hx %hx",
                    (FMT__H_H_H_H_H_H_H,
                     HEX_ARG(0), HEX_ARG(1), HEX_ARG(2), HEX_ARG(3)));
          break;
        case 3:
          TRACE_MSG(TRACE_MAC2, "%hx %hx %hx", 
                    (FMT__H_H_H,
                     HEX_ARG(0), HEX_ARG(1), HEX_ARG(2)));
          break;
        case 2:
          TRACE_MSG(TRACE_MAC2, "%hx %hx",
                    (FMT__H_H,
                     HEX_ARG(0), HEX_ARG(1)));
          break;
        case 1:
          TRACE_MSG(TRACE_MAC2, "%hx",
                    (FMT__H,
                     HEX_ARG(0)));
          break;
      }
      i = len;
    }
  }
}
#endif



/*! @} */
