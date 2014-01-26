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
PURPOSE: MAC transport for Linux/ARM. Transiver accessed via SPIDEV driver,
UZ2400 interrupt accessed using special driver.
Most logic in the userspace to achive short port time.
*/


#include "zb_common.h"

#ifdef ZB_TRANSPORT_LINUX_SPIDEV

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>


#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_mac_transport.h"
#include "zb_nwk.h"

/*! \addtogroup ZB_MAC_TRANSPORT */
/*! @{ */

#ifdef ZB_TRAFFIC_DUMP_ON
static void zb_spidev_dump_tx(zb_uint8_t *buf, zb_ushort_t len);
static void zb_spidev_dump_tx_tx(zb_uint8_t *buf1, zb_ushort_t len1, zb_uint8_t *buf2, zb_ushort_t len2);
static void zb_spidev_dump_rx(zb_uint8_t *buf, zb_ushort_t len);

#define DUMP_TX(buf, len) zb_spidev_dump_tx((buf), (len))
#define DUMP_TX_TX(buf1, len1, buf2, len2) zb_spidev_dump_tx_tx((buf1), (len1), (buf2), (len2))
#define DUMP_RX(buf, len) zb_spidev_dump_rx((buf), (len))

static zb_uint_t zb_packet_cnt;

#else

#define DUMP_TX(buf, len)
#define DUMP_TX_TX(buf1, len1, buf2, len2)
#define DUMP_RX(buf, len)

#endif  /* traffic dump */

void zb_mac_transport_init(zb_char_t *rpipe_path, zb_char_t *wpipe_path)
{
  (void)rpipe_path;
  (void)wpipe_path;

  TRACE_MSG(TRACE_MAC1, ">>zb_mac_transport_init", (FMT__0));

  ZIG->ioctx.intr_fd = -1;
  ZIG->ioctx.spidev_fd = open(ZB_MAC_TRANSPORT_SPIDEV_PATH, O_RDWR, 0);
  if (ZIG->ioctx.spidev_fd == -1)
  {
    TRACE_MSG(TRACE_ERROR, "error open spidev %d", (FMT__D, errno));
    return;
  }
  ZIG->ioctx.intr_fd = open(ZB_MAC_TRANSPORT_INTR_PATH, O_RDONLY, 0);
  if (ZIG->ioctx.intr_fd == -1)
  {
    TRACE_MSG(TRACE_ERROR, "error open zbintr dev %d", (FMT__D, errno));
    return;
  }

  if (!ZIG->ioctx.dump_file)
  {
    zb_char_t namefile[255];
    sprintf(namefile, "%ld.dump", (long)getpid());
    ZIG->ioctx.dump_file = fopen(namefile, "ab+");
    setvbuf(ZIG->ioctx.dump_file, NULL, _IONBF, 0);
    if (!ZIG->ioctx.dump_file)
    {
      TRACE_MSG(TRACE_ERROR, "Can't open dump file %s %d", (FMT__0, namefile, errno));
    }
  }
}


zb_ret_t zb_spidev_short_read(zb_uint8_t short_addr, zb_uint8_t *v)
{
  struct spi_ioc_transfer ioc;
  zb_uint8_t txb[2];
  zb_uint8_t rxb[2];
  zb_ret_t ret;

  ZB_MEMSET(&ioc, 0, sizeof(ioc));
  ioc.tx_buf = (unsigned long)txb;
  ioc.rx_buf = (unsigned long)rxb;
  /*
    Driver can do read-write operation only.
    It is impossible to read without write: driver crashes.
    Must supply both rx and tx buffers any time reading, but can supply tx only
    when writing only.
   */

  /* read/write (write address), then read/write (read value). */
  ioc.len = 2;
  ioc.bits_per_word = 8;
  txb[0] = (short_addr << 1);
  txb[1] = 0;

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(1), &ioc);
  *v = rxb[1];

  DUMP_TX(txb, 1);
  DUMP_RX(rxb+1, 1);
  return ret == 2 ? RET_OK : RET_ERROR;
}


static void zb_spidev_dump_tx(zb_uint8_t *buf, zb_ushort_t len)
{
  zb_mac_transport_hdr_t h;

  h.len = len + sizeof(zb_mac_transport_hdr_t);
  h.type = ZB_MAC_TRANSPORT_TYPE_DUMP | 0x80;
  ZB_HTOLE16(&h.time, &ZB_TIMER_GET());

  zb_packet_cnt++;
  TRACE_MSG(TRACE_MAC3, "tx #%d len %hd", (FMT__D_H, zb_packet_cnt, len));

  if (ZIG->ioctx.dump_file)
  {
    fwrite(&h, 1, sizeof(h), ZIG->ioctx.dump_file);
    fwrite(buf, 1, len, ZIG->ioctx.dump_file);
  }
}


static void zb_spidev_dump_tx_tx(zb_uint8_t *buf1, zb_ushort_t len1, zb_uint8_t *buf2, zb_ushort_t len2)
{
  zb_mac_transport_hdr_t h;

  h.len = len1 + len2 + sizeof(zb_mac_transport_hdr_t);
  h.type = ZB_MAC_TRANSPORT_TYPE_DUMP | 0x80;
  ZB_HTOLE16(&h.time, &ZB_TIMER_GET());

  zb_packet_cnt++;
  TRACE_MSG(TRACE_MAC3, "tx #%d len %hd", (FMT__D_H, zb_packet_cnt, len1 + len2));

  if (ZIG->ioctx.dump_file)
  {
    fwrite(&h, 1, sizeof(h), ZIG->ioctx.dump_file);
    fwrite(buf1, 1, len1, ZIG->ioctx.dump_file);
    fwrite(buf2, 1, len2, ZIG->ioctx.dump_file);
  }
}


static void zb_spidev_dump_rx(zb_uint8_t *buf, zb_ushort_t len)
{
  zb_mac_transport_hdr_t h;

  h.len = len + sizeof(zb_mac_transport_hdr_t);
  h.type = ZB_MAC_TRANSPORT_TYPE_DUMP;
  ZB_HTOLE16(&h.time, &ZB_TIMER_GET());

  zb_packet_cnt++;
  TRACE_MSG(TRACE_MAC3, "rx #%d len %hd", (FMT__D_H, zb_packet_cnt, len));

  if (ZIG->ioctx.dump_file)
  {
    fwrite(&h, 1, sizeof(h), ZIG->ioctx.dump_file);
    fwrite(buf, 1, len, ZIG->ioctx.dump_file);
  }
}


zb_ret_t zb_spidev_short_write(zb_ushort_t short_addr, zb_uint8_t val)
{
  struct spi_ioc_transfer ioc;
  zb_uint8_t txb[2];
  zb_ret_t ret;

  ZB_MEMSET(&ioc, 0, sizeof(ioc));
  ioc.tx_buf = (unsigned long)txb;
  ioc.len = 2;
  ioc.bits_per_word = 8;

  txb[0] = (short_addr << 1) | 1;
  txb[1] = val;

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(1), &ioc);

  DUMP_TX(txb, 2);
  return ret == 2 ? RET_OK : RET_ERROR;
}


zb_ret_t zb_spidev_long_write(zb_ushort_t long_addr, zb_uint8_t val)
{
  struct spi_ioc_transfer ioc;
  zb_uint8_t txb[3];
  zb_ret_t ret;

  memset(&ioc, 0, sizeof(ioc));
  ioc.tx_buf = (unsigned long)txb;
  ioc.len = 3;
  ioc.bits_per_word = 8;

  /* write 2 bytes, read 1 byte */
  ZB_HTOBE16_VAL(txb, (long_addr << 5 ) | 0x8010);
  txb[2] = val;

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(1), &ioc);
  DUMP_TX(txb, 3);
  return ret == 3 ? RET_OK : RET_ERROR;
}


zb_ret_t zb_spidev_long_read(zb_ushort_t long_addr, zb_uint8_t *val)
{
  struct spi_ioc_transfer ioc;
  zb_uint8_t txb[3];
  zb_uint8_t rxb[3];
  zb_ret_t ret;

  memset(&ioc, 0, sizeof(ioc));
  ioc.tx_buf = (unsigned long)txb;
  ioc.rx_buf = (unsigned long)rxb;
  ioc.len = 3;
  ioc.bits_per_word = 8;

  ZB_HTOBE16_VAL(txb, (long_addr << 5 ) | 0x8000);
  txb[2] = 0;

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(1), &ioc);
  *val = rxb[2];

  DUMP_TX(txb, 2);
  DUMP_RX(rxb+2, 1);
  return ret == 3 ? RET_OK : RET_ERROR;
}


zb_ret_t zb_spidev_long_write_fifo(zb_ushort_t long_addr, zb_uint8_t *buf, zb_ushort_t len)
{
  struct spi_ioc_transfer ioc[24];
  zb_uint8_t txb[24][8];
  zb_ushort_t i = 0;
  zb_ushort_t n = len / 6;
  zb_ret_t ret;

  /*
    Number of bytes transferred in both directions by one ioctl call is limited
    to 8 - HW FIFO size. If put more bytes, driver returns length reporting all
    bytes are written (read) , but really does i/o for 8 bytes only (including 2
    bytes of reg address tx).
    So, divide transmission into 8-bytes portions (2 bytes write fifo address,
    then tx or rx data)
   */
  memset(&ioc, 0, sizeof(ioc));
  for (i = 0 ; i < n ; ++i)
  {
    ioc[i].tx_buf = (unsigned long)txb[i];
    ZB_HTOBE16_VAL(txb[i], ((long_addr + i * 6) << 5 ) | 0x8010);
    ioc[i].len = 8;
    memcpy(txb[i] + 2, buf + i * 6, 6);
    ioc[i].bits_per_word = 8;
  }
  if (len % 6)
  {
    ioc[i].tx_buf = (unsigned long)txb[i];
    ZB_HTOBE16_VAL(txb[i], ((long_addr + i * 6) << 5 ) | 0x8010);
    ioc[i].len = len % 6 + 2;
    memcpy(txb[i] + 2, buf + i * 6, len % 6);
    ioc[i].bits_per_word = 8;
    n++;
  }

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(n), ioc);
  DUMP_TX_TX(&txb[0][0], 2, buf, len);
  return ret > 0 ? RET_OK: RET_ERROR;
}


zb_ret_t zb_spidev_long_read_fifo(zb_ushort_t long_addr, zb_uint8_t *buf, zb_ushort_t len)
{
  struct spi_ioc_transfer ioc[24];
  zb_uint8_t txb[24][8];
  zb_uint8_t rxb[24][8];
  zb_ushort_t i = 0;
  zb_ushort_t n = len / 6;
  zb_ret_t ret;

  ZB_MEMSET(&ioc, 0, sizeof(ioc));
  ZB_MEMSET(&txb, 0, sizeof(txb));
  for (i = 0 ; i < n ; ++i)
  {
    ioc[i].tx_buf = (unsigned long)txb[i];
    ioc[i].rx_buf = (unsigned long)rxb[i];
    ZB_HTOBE16_VAL(txb[i], ((long_addr + i * 6) << 5 ) | 0x8000);
    ioc[i].len = 8;
    ioc[i].bits_per_word = 8;
  }
  if (len % 6)
  {
    ioc[i].tx_buf = (unsigned long)txb[i];
    ioc[i].rx_buf = (unsigned long)rxb[i];
    ZB_HTOBE16_VAL(txb[i], ((long_addr + i * 6) << 5 ) | 0x8000);
    ioc[i].len = len % 6 + 2;
    ioc[i].bits_per_word = 8;
    n++;
  }

  ret = ioctl(ZIG->ioctx.spidev_fd, SPI_IOC_MESSAGE(n), ioc);

  n = len / 6;
  for (i = 0 ; i < n ; ++i)
  {
    ZB_MEMCPY(buf + i * 6, rxb[i] + 2, 6);
  }
  ZB_MEMCPY(buf + i * 6, rxb[i] + 2, len % 6);

  DUMP_TX(txb[0], 2);
  DUMP_RX(buf, len);
  return ret > 0 ? RET_OK: RET_ERROR;
}



/**
   Wait for transiver interrupts in Linux/ARM wth spidev transport.
*/
zb_ret_t zb_spidev_wait_for_intr(zb_uint8_t can_wait)
{
  zb_ret_t ret;
  struct timeval tv;
  static struct timeval start_t; /* static to take into account time we spent
                                  * not here */
  fd_set read_set;
  zb_time_t tmo = (zb_time_t)~0;

  TRACE_MSG(TRACE_MAC1, ">>zb_spidev_wait_for_intr can_wait %hd", (FMT__H, can_wait));


  /* fill strucutes for select */
  FD_ZERO(&read_set);
  FD_SET(ZIG->ioctx.intr_fd, &read_set);

  /* Implement stack timer: track time we spent sleeping inside select() */
  TRACE_MSG(TRACE_MAC3, "timer started = %d", (FMT__D, ZB_TIMER_CTX().started));

  if (!can_wait)
  {
    TRACE_MSG(TRACE_MAC3, "mac_main_loop on - zero timeout", (FMT__0));
    tv.tv_usec = 0;
    tv.tv_sec = 0;
  }
  else
  {
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
      TRACE_MSG(TRACE_MAC3, "default timeout %d", (FMT__D, ZB_LINUX_SPIDEV_INTR_TIMEOUT));
      tv.tv_sec = ZB_LINUX_SPIDEV_INTR_TIMEOUT;
      tv.tv_usec = 0;
    }
  }

  TRACE_MSG(TRACE_MAC3, "select() timeout %d.%d", (FMT__D_D, tv.tv_sec, tv.tv_usec));

  /* start time initialization - do it only once */
  if (start_t.tv_sec == 0)
  {
    gettimeofday(&start_t, NULL);
  }

  ret = select(ZIG->ioctx.intr_fd + 1, &read_set, NULL, NULL, &tv);

  TRACE_MSG(TRACE_MAC3, "select() ret %d", (FMT__D, ret));

  /* deal with time */
  {
    zb_time_t delta_b;
    unsigned long long delta_us;
    struct timeval end_t;

    TRACE_MSG(TRACE_MAC3, "s1", (FMT__0));
    /* remember time we spent in select(), msec */
    gettimeofday(&end_t, NULL);
    delta_us = end_t.tv_sec * 1000000ll + end_t.tv_usec - start_t.tv_sec * 1000000ll - start_t.tv_usec + ZIG->ioctx.time_delta_rest_us;
    delta_b = ZB_MILLISECONDS_TO_BEACON_INTERVAL(delta_us / 1000);
    TRACE_MSG(TRACE_MAC3, "delta %llu us / %d ticks", (FMT__D_D, delta_us, delta_b));
    if (ZB_TIMER_CTX().started)
    {
      /* imitate 8051: move timer if it started only */
      ZB_TIMER_CTX().timer = ZB_TIME_ADD(ZB_TIMER_CTX().timer, delta_b);
      ZIG->ioctx.time_delta_rest_us = delta_us - ZB_TIME_BEACON_INTERVAL_TO_MSEC(delta_b) * 1000;
    }
    else
    {
      TRACE_MSG(TRACE_MAC3, "timer is not started", (FMT__0));
    }
    TRACE_MSG(TRACE_MAC3, "timer %d, delta_rest_us %llu", (FMT__D_D, ZB_TIMER_CTX().timer, ZIG->ioctx.time_delta_rest_us));
    /* remember current time for the next run */
    start_t = end_t;
  }

  if ((ret >= 0
       && FD_ISSET(ZIG->ioctx.intr_fd, &read_set))
#if 0
      /* debug staff: read intr status in case int loss */
      ||
      (ZG->transceiver.int_status == 0
       && can_wait)
#endif
    )
  {
    TRACE_MSG(TRACE_MAC1, "Got INTR. Schedule mac main loop", (FMT__0));
    ret = RET_OK;
    ZB_SET_TRANS_INT();
  }
  else
  {
    ret = RET_BLOCKED;
  }

  TRACE_MSG(TRACE_MAC1, "<<zb_spidev_wait_for_intr %d", (FMT__D, ret));
  return ret;
}


void zb_mac_transport_deinit()
{
  if ( ZIG->ioctx.spidev_fd != -1 )
  {
    close(ZIG->ioctx.spidev_fd);
    ZIG->ioctx.spidev_fd = -1;
  }

  if ( ZIG->ioctx.intr_fd != -1 )
  {
    close(ZIG->ioctx.intr_fd);
    ZIG->ioctx.intr_fd = -1;
  }
}



/*! @} */

#endif  /* ZB_TRANSPORT_LINUX_SPIDEV */
