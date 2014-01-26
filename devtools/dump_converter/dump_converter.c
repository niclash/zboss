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
PURPOSE: Convert .dump file with traffic dump produced by the stack to .pcap
file to be read by WireShark.
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <winsock.h>
typedef	unsigned short int u_int16_t;
#else
#include <sys/socket.h>
#endif

#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pcap/pcap.h>

#include "zb_common.h"
#include "zb_mac_transport.h"

// #define ETH_AND_UDP_HDR_OFFSETS (sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr))


#define ETH_AND_UDP_HDR_OFFSETS 0

zb_int_t g_udp_port = ZB_UDP_PORT_REAL;
char *g_in_file;
char *g_out_file;
int custom_ns_header_offset;

static void create_eth_hdr(char *buf, int len, int is_w);
static void parse_cmd(int argc, char **argv);
static void fcs_add(char *buf, int len);

int main(int argc, char **argv)
{
  FILE *in_f;
  FILE *out_f;
  char iobuf[1024];
  char prev_reg_rx[3];
  int prev_len = 0;
  zb_mac_transport_hdr_t hdr;
  struct pcap_pkthdr cap_hdr;
  pcap_t *pcap = NULL;
  pcap_dumper_t *dumper = NULL;
  int real_mode;
  int pkt_n = 0;
  unsigned running_time = 0;
  unsigned prev_timestamp = (unsigned)~0;

  parse_cmd(argc, argv);
  in_f = fopen(g_in_file, "rb");
  if (!in_f)
  {
    fprintf(stderr, "Can't open in file %s - error %s\n", g_in_file, strerror(errno));
    return -1;
  }
  /* open out file, create if not exist */
  out_f = fopen(g_out_file, "wb+");
  if (!out_f)
  {
    fprintf(stderr, "Can't open out file %s - error %s\n", g_out_file, strerror(errno));
    return -1;
  }
  /* truncate if file exist */
  ftruncate(fileno(out_f), 0);
  fseek(out_f, 0, SEEK_SET);

  /* open file for pcap dump */
  // pcap = pcap_open_dead(DLT_EN10MB, 4096);

  pcap = pcap_open_dead(DLT_IEEE802_15_4, 4096);

  if (!pcap)
  {
    fprintf(stderr, "pcap_fopen_offline error\n");
  }
  else
  {
    dumper = pcap_dump_fopen(pcap, out_f);
  }
  if (!dumper && pcap)
  {
    fprintf(stderr, "pcap_dump_fopen error: %s\n", pcap_geterr(pcap));
  }
  if (!dumper)
  {
    return -1;
  }

  /* use same ethernet & udb headers for all packets */
  memset(&cap_hdr, 0, sizeof(cap_hdr)); /* clear ts */
  real_mode = (g_udp_port != ZB_UDP_PORT_NS);
  custom_ns_header_offset = real_mode ? 0 : 3;

  while (fread(&hdr, 1, 1, in_f) == 1)
  {
    int r;

    pkt_n++;

//    printf("%d: l %d\n", pkt_n, hdr.len);
    if (hdr.len == 0)
    {
      fprintf(stderr, "at pkt %d skip zero byte - hmm? Was it 0a -> 0d 0a translation??\n", pkt_n);
      pkt_n--;
      continue;
    }

    if (fread(&hdr.type, sizeof(hdr)-1, 1, in_f) != 1)
    {
      fprintf(stderr, "packet #%d: error reading hdr\n", pkt_n);
      return -1;
    }

    if (prev_timestamp == (unsigned)~0)
    {
      running_time = hdr.time;
    }
    else
    {
      running_time += ZB_TIME_SUBTRACT(hdr.time, prev_timestamp);
    }
    prev_timestamp = hdr.time;

    hdr.len -= sizeof(hdr);

    memset(iobuf, 0, sizeof(iobuf));

#define DATA_BUF (iobuf + ETH_AND_UDP_HDR_OFFSETS + real_mode + custom_ns_header_offset)

    if ((r = fread(DATA_BUF, hdr.len, 1, in_f)) != 1)
    {
      fprintf(stderr, "packet #%d: error reading %d bytes from the input file\n", pkt_n, hdr.len);
      return -1;
    }
    if (real_mode && hdr.len == 1 && hdr.type == 1 && prev_len && prev_len < 3)
    {
      /* this is register rx. Put address (it was write) here to be able to
       * decode register contents by wireshark */
      memmove(DATA_BUF + prev_len, DATA_BUF, hdr.len);
      memcpy(DATA_BUF, prev_reg_rx, prev_len);
      hdr.len += prev_len;
    }
    else if (real_mode && hdr.len > 3)
    {
      /* FIFO tx. Add FCS. */
      if (hdr.type != 1 && hdr.len > 4)
      {
        /* format: 1b hdr len, 1b frame len, data. Add FCS after buffer */
        fcs_add(DATA_BUF + 4, hdr.len - 4);
        hdr.len += 2;
      }
    }

    memcpy(prev_reg_rx, DATA_BUF, 2);
    prev_len = hdr.len;

    if ((hdr.type & 0x7f) != ZB_MAC_TRANSPORT_TYPE_DUMP && hdr.type != ZB_MAC_TRANSPORT_TYPE_DATA)
    {
      continue;
    }

    // create_eth_hdr(iobuf, hdr.len + real_mode, (hdr.type != 1));
    cap_hdr.caplen = cap_hdr.len = ETH_AND_UDP_HDR_OFFSETS + hdr.len + real_mode + custom_ns_header_offset;
    if (g_udp_port != ZB_UDP_PORT_NS)
    {
      /* put header type before body to distinguish between IN and OUT */
      iobuf[ETH_AND_UDP_HDR_OFFSETS] = hdr.type;
    }
    else
    {
      iobuf[ETH_AND_UDP_HDR_OFFSETS] = hdr.type + 1;
      iobuf[ETH_AND_UDP_HDR_OFFSETS + 1] = 0xDE;
      iobuf[ETH_AND_UDP_HDR_OFFSETS + 2] = 0x77;
      iobuf[ETH_AND_UDP_HDR_OFFSETS + 3] = 0xFF;
    }

    /* cap_hdr.ts struct timeval */
    {
      unsigned ms = ZB_TIME_BEACON_INTERVAL_TO_MSEC(running_time);
      cap_hdr.ts.tv_sec = ms / 1000;
      cap_hdr.ts.tv_usec = (ms % 1000) * 1000;
    }
    pcap_dump((u_char *)dumper, &cap_hdr, (u_char *)iobuf);
  }

  pcap_dump_flush(dumper);
  pcap_dump_close(dumper);
  fclose(in_f);

  return 0;
}

static void usage(char **argv)
{
  printf("Usage: %s {-ns} dump_file pcap_file\n"
         " - ns - dump produced by ns stack build (no transiver registers access)\n",
         argv[0]);
  exit(1);
}

static void parse_cmd(int argc, char **argv)
{
  int i = 1;

  while ( i < argc )
  {
    if ( !strcmp(argv[i], "-ns") )
    {
      g_udp_port = ZB_UDP_PORT_NS;
      i++;
      break;
    }
    else if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "-help") )
    {
      usage(argv);
    }
    else
    {
      break;
    }
  }
  if (i < argc)
  {
    g_in_file = argv[i];
    i++;
  }
  else
  {
    usage(argv);
  }
  if (i < argc)
  {
    g_out_file = argv[i];
    i++;
  }
  else
  {
    usage(argv);
  }
}


static void create_eth_hdr(char *buf, int len, int is_w)
{
  struct ether_header *eh = (struct ether_header *)buf;
  struct iphdr *iph = (struct iphdr *)(eh + 1);
  struct udphdr *udph = (struct udphdr *)(iph + 1);
  zb_uint32_t sum = 0;
  zb_uchar_t *p;
  unsigned i;


  /* ethernet header. */
  /* fake Ethernet addresses 1 and 2 */
  if (is_w)
  {
    eh->ether_dhost[5] = 2;
    eh->ether_shost[5] = 1;
  }
  else
  {
    eh->ether_dhost[5] = 1;
    eh->ether_shost[5] = 2;
  }
  eh->ether_type = 0x08;

  /* IP header */
  iph->ihl = sizeof(*iph) / 4;
  iph->version = 4;
  iph->tot_len = htons(sizeof(*udph) + sizeof(*iph) + len);
  iph->protocol = 0x11;         /* UDP */
  iph->ttl = 64;                /* ?? */
  if (is_w)
  {
    iph->saddr = 0x0100000a;
    iph->daddr = 0xfefefe0a;
  }
  else
  {
    iph->saddr = 0xfefefe0a;
    iph->daddr = 0x0100000a;
  }

  p = (zb_uchar_t *)iph;
  for (i = 0 ; i < sizeof(*iph) ; i += 2)
  {
    sum += (zb_uint16_t)((p[i]<<8) & 0xff00)+(p[i + 1] & 0xff);
  }
	while (sum>>16)
  {
	  sum = (sum & 0xFFFF)+(sum >> 16);
  }
	iph->check = htons((zb_uint16_t)(~sum));

  /* UDP header */
  udph->source = htons(g_udp_port);
  udph->dest = htons(g_udp_port);
  udph->len = htons(len + sizeof(*udph));
}


static void fcs_add(char *buf, int len)
{
  static const zb_uint16_t table[256] =
    {
 0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
 0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
 0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
 0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
 0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
 0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
 0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
 0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
 0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
 0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
 0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
 0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
 0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
 0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
 0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
 0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
 0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
 0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
 0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
 0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
 0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
 0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
 0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
 0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
 0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
 0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
 0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
 0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
 0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
 0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
 0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
 0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
    };
  zb_uint16_t crc = 0;
  zb_int_t i;
  zb_uint8_t *p = (zb_uint8_t*)buf;

  for (i = 0 ; i < len ; ++i)
  {
    crc = table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
  }

  memcpy(p, &crc, 2);           /* little-endian only! */
}
