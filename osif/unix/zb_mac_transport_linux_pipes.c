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
PURPOSE: Put/Get ready buffers to/from NS-3 pipe.
*/

#include "zb_common.h"

#ifdef ZB_TRANSPORT_LINUX_PIPES

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>


#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_mac_transport.h"
#include "zb_nwk.h"

/*! \addtogroup ZB_MAC_TRANSPORT */
/*! @{ */

static zb_ret_t write_to_pipe();

#define DUMP_TRAF dump_traf2

static void dump_traf2(char *comment, zb_uint8_t *buf, zb_ushort_t len, zb_ushort_t total)
{
  zb_ushort_t i;
  char s[512];
  char *p = s;
  *p = 0;

  TRACE_MSG(TRACE_MAC3, "%s: len %d total %d", (FMT__D_D, comment, len, total+len));
  for (i = 0 ; i < len ; ++i)
  {
    sprintf(p, "%02X ", buf[i]);
    p += 3;
    if (i % 16 == 15)
    {
      TRACE_MSG(TRACE_MAC3, "%s", (FMT__0, s));
      p = s;
      *p = 0;
    }
  }
  if (p != s)
  {
    TRACE_MSG(TRACE_MAC3, "%s", (FMT__0, s));
  }
}


/**
   \par mac linux transport

   These functions are used only on linux build. From the first side this
   transport uses 2 pipes, one for input the other for output. Simple protocol
   is used, where the first byte is the packet length (not including length byte
   itself). From the other side this transport uses mac layer via
   zb_mac_main_loop function. Transport calls it when there is something to
   read from pipe, gets buffer zb_buf_t and calls mac again when packet is
   received. Also mac is called when output packet is sent.
*/

void zb_mac_transport_init(zb_char_t *rpipe_path, zb_char_t *wpipe_path)
{
  ZIG->ioctx.rpipe = -1;
  ZIG->ioctx.wpipe = -1;

  ZIG->ioctx.send_data_buf   = NULL;
  ZIG->ioctx.recv_data_buf   = NULL;
  ZIG->ioctx.out_buf_written = 0;
  ZIG->ioctx.in_buf_read     = 0;

  ZIG->ioctx.timeout = ZB_LINUX_PIPE_TRANSPORT_TIMEOUT;

  ZIG->ioctx.rpipe_path = rpipe_path ? rpipe_path : ZB_MAC_TRANSPORT_DEFAULT_RPIPE_PATH;
  ZIG->ioctx.wpipe_path = wpipe_path ? wpipe_path : ZB_MAC_TRANSPORT_DEFAULT_WPIPE_PATH;

  mkfifo(ZIG->ioctx.rpipe_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  mkfifo(ZIG->ioctx.wpipe_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

  ZB_TIMER_INIT();
}


#ifdef ZB_TRAFFIC_DUMP_ON
/**
   Dump traffic to the dump file.

   Dump file format is same as pipe_data_router produce: zb_mac_transport_hdr_t
   before packet.

   @param buf    - output buffer.
   @param is_w   - if 1, this is output, else input

   @return nothing.
 */
void zb_mac_traffic_dump(zb_buf_t *buf, zb_bool_t is_w)
{
  static int cnt = 0;
  cnt++;
  TRACE_MSG(TRACE_MAC1, ">>zb_mac_traffic_dump #%d buf %p / %hd in_b %hd (size %d) is_w %d",
            (FMT__D_P_H_H_D_D, cnt, buf, ZB_REF_FROM_BUF(buf), buf->u.hdr.is_in_buf, ZB_BUF_LEN(buf), is_w));

  if (!ZIG->ioctx.dump_file)
  {
    zb_char_t namefile[255];
    sprintf(namefile, "%ld.dump", (long)getpid());
    ZIG->ioctx.dump_file = fopen(namefile, "a+");
    if (!ZIG->ioctx.dump_file)
    {
      TRACE_MSG(TRACE_ERROR, "Can't open dump file %s", (FMT__0, namefile));
    }
  }

  if (ZIG->ioctx.dump_file)
  {
    zb_mac_transport_hdr_t hdr;
    hdr.len = ZB_BUF_LEN(buf) + sizeof(zb_mac_transport_hdr_t);
#ifdef ZB_NS_BUILD
    /* cut extra 9 bytes which imitates UZ trailer: let's be compatible with
     * dump produced by ns-3 */
    hdr.len -= (!is_w && buf->u.hdr.len > ZB_MAC_EXTRA_DATA_SIZE ? ZB_MAC_EXTRA_DATA_SIZE : 0);
#endif
    hdr.type = ZB_MAC_TRANSPORT_TYPE_DUMP | (is_w ? 0x80 : 0);
    {
      static zb_uint_t s_start_sec;

      struct timeval t;
      gettimeofday(&t, NULL);
      if (s_start_sec == 0)
      {
        s_start_sec = t.tv_sec;
      }
      hdr.time = ZB_MILLISECONDS_TO_BEACON_INTERVAL((zb_int_t)((t.tv_sec - s_start_sec) * 1000 + t.tv_usec / 1000));
      TRACE_MSG(TRACE_MAC3, "t: start %u; t %u %u, time %d", (FMT__D_D_D_D, s_start_sec, t.tv_sec, t.tv_usec, (int)hdr.time));
    }
    if (fwrite(&hdr, sizeof(hdr), 1, ZIG->ioctx.dump_file) != 1
        || fwrite(ZB_BUF_BEGIN(buf), hdr.len - sizeof(hdr), 1, ZIG->ioctx.dump_file) != 1)
    {
      TRACE_MSG(TRACE_ERROR, "Dump file write error %d", (FMT__D, errno));
    }
    fflush(ZIG->ioctx.dump_file);
  }

  TRACE_MSG(TRACE_MAC1, "<<zb_mac_traffic_dump", (FMT__0));
}
#endif


void zb_mac_transport_put_data(zb_buf_t *buf)
{
  TRACE_MSG(TRACE_MAC1, ">>zb_mac_put_data buf %p", (FMT__P, buf));

  /* check if we already have buffer for output */
  if ( ZIG->ioctx.send_data_buf )
  {
    TRACE_MSG(TRACE_ERROR, "output buffer is already in progress", (FMT__0));
    ZB_ASSERT(0);
  }
  else
  {
    zb_uint8_t *ptr;

    ZIG->ioctx.send_data_buf = buf;

    /* prefix data by length for ns-3 plugin module */
    ZB_BUF_ALLOC_LEFT(ZIG->ioctx.send_data_buf, 1, ptr);
    *ptr = ZB_BUF_LEN(ZIG->ioctx.send_data_buf);

    ZB_DUMP_OUTGOING_DATA(buf);

    /* call to write data here - it will work like 8051 sync logic for send */
    write_to_pipe();

    MAC_CTX().tx_cnt++;
  }

  TRACE_MSG(TRACE_MAC1, "<<zb_mac_put_data ret", (FMT__0));
}


void zb_mac_transport_start_recv(zb_buf_t *buf, zb_short_t bytes_to_recv)
{
  zb_ret_t ret = 0;

  (void)bytes_to_recv;

  TRACE_MSG(TRACE_MAC1, ">>zb_mac_start_recv buf %p", (FMT__P, buf));

  /* check if we already have buffer for input */
  if ( ZIG->ioctx.recv_data_buf )
  {
    TRACE_MSG(TRACE_ERROR, "incoming buffer is already in progress", (FMT__0));
    ret = RET_ALREADY_EXISTS;
  }
  else
  {
    ZIG->ioctx.recv_data_buf = buf;
    ZIG->ioctx.recv_data_buf->u.hdr.len = 0;
  }

  TRACE_MSG(TRACE_MAC1, "<<zb_mac_start_recv ret %d", (FMT__D, ret));
}


/**
   Read data from the pipe.

   Called when we have something to read from the pipe.

   @return Return number of bytes read from pipe, or error.
 */
zb_ret_t read_from_pipe()
{
  zb_ret_t ret = 0;
  zb_uint_t bts_read = 0;

  TRACE_MSG(TRACE_MAC1, ">>read_from_pipe", (FMT__0));

  if ( !ZIG->ioctx.recv_data_buf )
  {
    TRACE_MSG(TRACE_MAC1, "no buffer available, call mac to get incoming buffer", (FMT__0));
    /* call mac to get incoming buffer. exit */
    ZG->sched.mac_receive_pending = 1;
    ret = 1;
    TRACE_MSG(TRACE_MAC1, "<<read_from_pipe %d", (FMT__D, ret));
    return ret;
  }

  /* read packet length if it's not read already */
  if ( !ZB_BUF_LEN(ZIG->ioctx.recv_data_buf) )
  {
    zb_uint8_t len;
    zb_uint8_t *ptr = NULL;

    /* read size first */
    do
    {
      ret = read(ZIG->ioctx.rpipe, &len, sizeof(len));
      if ( ret > 0 )
      {
        bts_read += ret;
        DUMP_TRAF("read", &len, ret, bts_read);
      }
    } while ( bts_read < sizeof(len)
              && (ret > 0 || errno == EINTR || errno == EAGAIN) );

    if ( bts_read
         && len > 0 )
    {
      ZB_BUF_INITIAL_ALLOC(ZIG->ioctx.recv_data_buf, len, ptr);
      *ptr = len;
      ZIG->ioctx.in_buf_read = 1;
    }
    else
    {
      TRACE_MSG(TRACE_MAC1, "error reading packet length bts_read %d len %d ret %d errno %d", (FMT__D_D_D_D,
                             bts_read, len, ret, errno));
    }
  }

  if ( ZB_BUF_LEN(ZIG->ioctx.recv_data_buf) )
  {
    /* read rest */
    do
    {
      ret = read(ZIG->ioctx.rpipe,
                 ZB_BUF_BEGIN(ZIG->ioctx.recv_data_buf) + ZIG->ioctx.in_buf_read,
                 ZB_BUF_LEN(ZIG->ioctx.recv_data_buf) - ZIG->ioctx.in_buf_read);
      if ( ret > 0 )
      {
        DUMP_TRAF("read", ZB_BUF_BEGIN(ZIG->ioctx.recv_data_buf) + ZIG->ioctx.in_buf_read, ret, bts_read);

        ZIG->ioctx.in_buf_read += ret;
        bts_read += ret;
      }
    }
    while ( ZIG->ioctx.in_buf_read < ZB_BUF_LEN(ZIG->ioctx.recv_data_buf)
            && (ret > 0 || errno == EINTR) );
  }

  /* got full packet? */
  if ( ZIG->ioctx.in_buf_read
       && ZB_BUF_LEN(ZIG->ioctx.recv_data_buf) == ZIG->ioctx.in_buf_read )
  {
    TRACE_MSG(TRACE_MAC1, "receive full buffer %hd len %d", (FMT__H_D, ZB_REF_FROM_BUF(ZIG->ioctx.recv_data_buf), ZIG->ioctx.in_buf_read));

#ifdef ZB_NS_BUILD
    {
      zb_uint8_t *p;
      /* If got data from ns-3, imitate UZ trailer (9b) */
      ZB_BUF_ALLOC_RIGHT(ZIG->ioctx.recv_data_buf, ZB_MAC_EXTRA_DATA_SIZE, p);

      /* imitate 9b of LQI, RSSI, Frame timer, Superframe counter */
      ZB_MEMSET(p, 0, ZB_MAC_EXTRA_DATA_SIZE);
    }
#endif

    /* packet is received - call mac */
    ZIG->ioctx.in_buf_read = 0;
    ZIG->ioctx.recv_data_buf = NULL;
    ZB_MAC_STOP_IO();
  }

  if ( bts_read )
  {
    ret = bts_read;
  }
  TRACE_MSG(TRACE_MAC1, "<<read_from_pipe %d", (FMT__D, ret));
  return ret;
}


/**
   Write data to the pipe.

   Open write pipe if it's not opened and write data.

   @return Return number of written bytes or error.
 */
static zb_ret_t write_to_pipe()
{
  zb_ret_t ret = 0;
  int bts_written = 0;

  TRACE_MSG(TRACE_MAC1, ">>write_to_pipe", (FMT__0));

  if ( ZIG->ioctx.send_data_buf )
  {
    /* open write pipe if it's not openned, wait if necessary */
    if ( ZIG->ioctx.wpipe == -1
         && ZIG->ioctx.send_data_buf )
    {
      TRACE_MSG(TRACE_MAC1, "open wpipe %s. wait if nessesary", (FMT__0, ZIG->ioctx.wpipe_path));
      while ( (ZIG->ioctx.wpipe = open(ZIG->ioctx.wpipe_path, O_WRONLY | O_NONBLOCK)) == -1 )
      {
        sleep(1);
      }
    }

    /* write full buffer to pipe */
    while ( ZIG->ioctx.out_buf_written < ZB_BUF_LEN(ZIG->ioctx.send_data_buf) )
    {
      static int select_timeout = 1;
      struct timeval tv;
      fd_set writeSet;
      int maxfd = ZIG->ioctx.wpipe;

      FD_ZERO(&writeSet);
      FD_SET(ZIG->ioctx.wpipe, &writeSet);

      tv.tv_sec = select_timeout;
      tv.tv_usec = 0;

      ret = select(maxfd + 1, NULL, &writeSet, NULL, &tv);
      if ( ret >= 0 )
      {
        if ( FD_ISSET(ZIG->ioctx.wpipe, &writeSet) )
        {
          do
          {
            ret = write(ZIG->ioctx.wpipe,
                        &ZIG->ioctx.send_data_buf->buf[ ZIG->ioctx.send_data_buf->u.hdr.data_offset + ZIG->ioctx.out_buf_written ],
                        ZIG->ioctx.send_data_buf->u.hdr.len - ZIG->ioctx.out_buf_written);
            if ( ret > 0 )
            {
              DUMP_TRAF("written", &ZIG->ioctx.send_data_buf->buf[ ZIG->ioctx.send_data_buf->u.hdr.data_offset + ZIG->ioctx.out_buf_written ],
                           ret, bts_written);

              bts_written += ret;
              ZIG->ioctx.out_buf_written += ret;
            }
            else
            {
              TRACE_MSG(TRACE_MAC3, "ret %d errno %d", (FMT__D_D , ret, errno));
            }
          }
          while ( (ZIG->ioctx.out_buf_written < ZB_BUF_LEN(ZIG->ioctx.send_data_buf))
                  && (ret >= 0 || errno == EINTR) );
        }
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "select error %s", (FMT__0, strerror(errno)));
        sleep(1);
        continue;
      }
    }

    /* notify mac if whole buffer is sent */
    if ( ZIG->ioctx.out_buf_written == ZB_BUF_LEN(ZIG->ioctx.send_data_buf) )
    {
      zb_uint8_t *dummy;
      /* Buffer can be used for resend. Put it into initial state: no
       * length byte - see zb_mac_transport_put_data().
       * Not need to free buffer here - MAC logic does it.
       */
      ZB_BUF_CUT_LEFT(ZIG->ioctx.send_data_buf, 1, dummy);
      TRACE_MSG(TRACE_MAC1, "out buffer is sent", (FMT__0));
      ZB_SET_SEND_STATUS(ZB_SEND_FINISHED);
      ZIG->ioctx.out_buf_written = 0;
      ZIG->ioctx.send_data_buf = NULL;
    }
  }

  if ( ret >= 0
       && bts_written )
  {
    ret = bts_written;
  }
  TRACE_MSG(TRACE_MAC1, "<<write_to_pipe %d", (FMT__D, bts_written));
  return ret;
}




/**
   Emulates transiver interrupts under unix.

   This function waits for incoming data from pipe and send data if we have
   smth to send

   @return >= 0 - success, otherwise error code.
*/
void zb_mac_wait_for_ext_event()
{
  zb_ret_t ret = 0;
  zb_uint8_t done_smsng;
  TRACE_MSG(TRACE_MAC1, ">>zb_mac_wait_for_ext_event", (FMT__0));


  /* Open read pipe if they are not alreaady opened */
  if ( ZIG->ioctx.rpipe == -1 )
  {
    errno = 0;
    ZIG->ioctx.rpipe = open(ZIG->ioctx.rpipe_path, O_RDONLY | O_NONBLOCK);
    TRACE_MSG(TRACE_MAC1, "open rpipe %d. ret %d", (FMT__D_D, ZIG->ioctx.rpipe, errno));
  }

 /* Write data to pipe if we have smth to write. If written something, no need
  * to read for recv - continue processing in the main loop. */
  if ((done_smsng = write_to_pipe()) == 0
      && ZIG->ioctx.rpipe != -1 )
  {
    struct timeval tv;
    static struct timeval start_t; /* static to take into account time we spent
                                    * not here */
    fd_set read_set;
    int maxfd;
    zb_time_t tmo = (zb_time_t)~0;


    /* fill strucutes for select */
    FD_ZERO(&read_set);
    FD_SET(ZIG->ioctx.rpipe, &read_set);
    maxfd = ZIG->ioctx.rpipe;

    /* Implement stack timer: track time we spent sleepint inside select() */
    TRACE_MSG(TRACE_MAC3, "timer started = %d", (FMT__D, ZB_TIMER_CTX().started));
    if (ZB_TIMER_CTX().started)
    {
      if (ZB_TIME_GE(ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer) && ZB_TIMER_CTX().timer_stop != ZB_TIMER_CTX().timer)
      {
        tmo = ZB_TIME_SUBTRACT(ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer);
        TRACE_MSG(TRACE_MAC3, "timer stop %d timer %d  tmo %d", (FMT__D_D_D, ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer, tmo));
      }
    }

    if (tmo != (zb_time_t)~0)
    {
      tv.tv_sec = ZB_TIME_BEACON_INTERVAL_TO_MSEC(tmo) / 1000;
      tv.tv_usec = (ZB_TIME_BEACON_INTERVAL_TO_MSEC(tmo) % 1000) * 1000;
    }
    else
    {
      TRACE_MSG(TRACE_MAC3, "default timeout %d", (FMT__D, ZIG->ioctx.timeout));
      tv.tv_sec = ZIG->ioctx.timeout;
      tv.tv_usec = 0;
    }

    TRACE_MSG(TRACE_MAC3, "select() timeout %d.%d", (FMT__D_D, tv.tv_sec, tv.tv_usec));

    /* start time initialization - do it only once */
    if (start_t.tv_sec == 0)
    {
      gettimeofday(&start_t, NULL);
    }

    ret = select(maxfd + 1, &read_set, NULL, NULL, &tv);

    /* deal with time */
    {
      zb_time_t delta;
      struct timeval end_t;

      gettimeofday(&end_t, NULL);
      /* remember time we spent in select(), msec */
      delta = (((zb_int_t)(end_t.tv_sec - start_t.tv_sec)) * 1000 +
               ((zb_int_t)(end_t.tv_usec - start_t.tv_usec)) / 1000);
      TRACE_MSG(TRACE_MAC3, "delta %d (%d)", (FMT__D_D, delta, ZB_MILLISECONDS_TO_BEACON_INTERVAL(delta)));
      if (ZB_TIMER_CTX().started)
      {
        /* imitate 8051: move timer if it started only */
        ZB_TIMER_CTX().timer = ZB_TIME_ADD(ZB_TIMER_CTX().timer,
           ZB_MILLISECONDS_TO_BEACON_INTERVAL(delta + ZIG->ioctx.time_delta_rest_ms + ZIG->ioctx.time_delta_rest_us / 1000));
        if (ZB_MILLISECONDS_TO_BEACON_INTERVAL(delta + ZIG->ioctx.time_delta_rest_ms) == 0)
        {
          ZIG->ioctx.time_delta_rest_ms = delta + ZIG->ioctx.time_delta_rest_ms;
          ZIG->ioctx.time_delta_rest_us += abs((zb_int_t)(end_t.tv_usec - start_t.tv_usec));
        }
        else
        {
          ZIG->ioctx.time_delta_rest_ms = 0;
          ZIG->ioctx.time_delta_rest_us = 0;
        }
      }
      else
      {
        TRACE_MSG(TRACE_MAC3, "timer is not started", (FMT__0));
      }
      TRACE_MSG(TRACE_MAC3, "timer %d, delta_rest %d", (FMT__D_D, ZB_TIMER_CTX().timer, ZIG->ioctx.time_delta_rest_ms));
      /* remember current time for the next run */
      start_t = end_t;
    }

    if ( ret >= 0 )
    {
      if ( FD_ISSET(ZIG->ioctx.rpipe, &read_set) )
      {
        done_smsng = 1;
        ret = read_from_pipe();
        if ( ret == 0 )
        {
          close(ZIG->ioctx.rpipe);
          TRACE_MSG(TRACE_MAC1, "seems rpipe is closed from the other side, reopen it", (FMT__0));
          ZIG->ioctx.rpipe = open(ZIG->ioctx.rpipe_path, O_RDONLY | O_NONBLOCK);
          TRACE_MSG(TRACE_MAC1, "open rpipe %d. %s", (FMT__D, ZIG->ioctx.rpipe, strerror(errno)));
        }
      }
      else if ( ret == 0 )
      {
        TRACE_MSG(TRACE_MAC1, "timeout", (FMT__0));
      }
    }
    else
    {
      TRACE_MSG(TRACE_MAC1, "select error %d. %s", (FMT__D, ret, strerror(errno)));
    }
  }
  if (done_smsng)
  {
    TRACE_MSG(TRACE_MAC1, "Schedule mac main loop 2", (FMT__0));
  }

  TRACE_MSG(TRACE_MAC1, "<<zb_mac_wait_for_ext_event %d", (FMT__D, ret));
}

void zb_mac_transport_deinit()
{
  if ( ZIG->ioctx.rpipe != -1 )
  {
    close(ZIG->ioctx.rpipe);
    ZIG->ioctx.rpipe = -1;
  }

  if ( ZIG->ioctx.wpipe != -1 )
  {
    close(ZIG->ioctx.wpipe);
    ZIG->ioctx.wpipe = -1;
  }
}

#endif  /* ZB_TRANSPORT_LINUX_PIPES */

/*! @} */
