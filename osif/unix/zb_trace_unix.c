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

#include "zb_common.h"
#include "zb_osif.h"
#include <pthread.h>

#ifdef UNIX

/*! \addtogroup ZB_TRACE */
/*! @{ */

/**
   \par Trace implementation.

   Trace works thru printf call. In Linux it writes to the file using fprintf(), at 8051
   printf() calls putchar() which writes to the serial port.

 */

#define ZB_LOG_VPRINTF(fmt, arg) vfprintf(g_trace_file ? g_trace_file : stdout, fmt, arg)
#define ZB_LOG_PRINTF(...) fprintf(g_trace_file ? g_trace_file : stdout, __VA_ARGS__)


/**
   Global trace file for Unix trace implementation
 */
FILE *g_trace_file;
/**
   Global trace mutex for Unix trace implementation
 */
static pthread_mutex_t s_trace_mutex;


/**
   Unix-specific trace init.

   @param name - process name. Trace file name generated from this name and current pid
 */
void zb_trace_init_unix(zb_char_t *name)
{
  zb_char_t namefile[255];

  sprintf(namefile, "%s%ld.log", name, (long)getpid());
  g_trace_file = fopen(namefile, "a+");
  if (g_trace_file)
  {
    /* set line buffering so not need to call fflush() */
    setvbuf(g_trace_file, NULL, _IOLBF, 512);
  }
  pthread_mutex_init( &s_trace_mutex, NULL );
}


/**
   Unix-specific trace lock: lock mutex
 */
void zb_trace_mutex_lock_unix()
{
  pthread_mutex_lock( &s_trace_mutex );
}


/**
   Unix-specific trace unlock: unlock mutex
 */
void zb_trace_mutex_unlock_unix()
{
  pthread_mutex_unlock( &s_trace_mutex );
}


/**
   Unix-specific trace deinit
 */
void zb_trace_deinit_unix()
{
  if (g_trace_file)
  {
    fflush(g_trace_file);
    fclose(g_trace_file);
    g_trace_file = NULL;
  }
  pthread_mutex_destroy(&s_trace_mutex);
}


/**
  Switch trace on/off on runtime, is useful in test purposes
*/
static char g_trace_enabled = 1;
void zb_set_trace_enabled(char val)
{
  g_trace_enabled = val;
}


/**
   Output trace message.

   @param file_name - source file name
   @param line_number - source file line
   @param mask - layers mask of the current message. Do trace if mask&ZB_TRACE_MASK != 0
   @param level - message trace level. Do trace if level <= ZB_TRACE_LEVEL
   @param format - printf-like format string
 */
void zb_trace_msg_unix(zb_char_t *format, zb_char_t *file_name, zb_int_t line_number, zb_int_t args_size, ...)
{
  /* If ZB_TRACE_LEVEL not defined, output nothing */
#ifdef ZB_TRACE_LEVEL

  va_list   arglist;

  if (!g_trace_enabled)
  {
    return;
  }

  ZB_TRACE_LOCK();

  {
    time_t    t;
    struct tm *tm;
    zb_uint_t msec = 0;
    struct timeval tmv;
    gettimeofday(&tmv, NULL);
    t = tmv.tv_sec;
    tm = localtime(&t);
    msec = tmv.tv_usec / 1000;

    ZB_LOG_PRINTF(
	    "%02d:%02d:%02d.%03ld %d %s:%d\t",
	    tm->tm_hour,
	    tm->tm_min,
	    tm->tm_sec,
	    (long)msec,
      ZB_TIMER_GET(),
      file_name, line_number
      );
  }
  va_start(arglist, args_size);
  ZB_LOG_VPRINTF(format, arglist);
  va_end(arglist);
  if (format[strlen(format) - 1] != '\n')
  {
    ZB_LOG_PRINTF("\n");
  }
  ZB_TRACE_UNLOCK();
#else
  (void)file_name;
  (void)line_number;
  (void)level;
  (void)mask;
  (void)format;
#endif
}

#endif  /* UNIX */

/*! @} */
