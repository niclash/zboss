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
PURPOSE: ZigBee small part from MAC packet format. Used to extract destination
address from the MAC packet.
*/

#ifndef __MAC_FRAME_H__
#define __MAC_FRAME_H__

#ifdef __GNUC__
  #define PACKED_STRUCT __attribute__ ((packed))
#else
  #define PACKED_STRUCT
#endif

#include <inttypes.h>

#if defined SOLARIS

#undef ZB_LITTLE_ENDIAN
#define ZB_BIG_ENDIAN

#define ZB_HTOBE16(ptr, val)                      \
  (((uint8_t *)(ptr))[0] = ((uint8_t *)(val))[0], \
   ((uint8_t *)(ptr))[1] = ((uint8_t *)(val))[1]  \
  )

#define FROM_LITTLE_ENDIAN(ptr)                 \
  ( ((ptr)[0]) | ((ptr)[1] << 8) )

#define FROM_LITTLE_ENDIAN32(ptr)                 \
  ( ((ptr)[0]) | ((ptr)[1] << 8) | ((ptr)[2] << 16) | ((ptr)[3] << 24) )

#else

#if !defined ZB_LITTLE_ENDIAN
#define ZB_LITTLE_ENDIAN
#endif

#define FROM_LITTLE_ENDIAN(ptr)                 \
  ( *(uint16_t*)(ptr))

#define FROM_LITTLE_ENDIAN32(ptr)                 \
  ( *(uint32_t*)(ptr))

#define ZB_HTOBE16(ptr, val)                      \
  (((uint8_t *)(ptr))[0] = ((uint8_t *)(val))[1], \
   ((uint8_t *)(ptr))[1] = ((uint8_t *)(val))[0]  \
  )

#endif

/* MAC address mode. */
typedef enum mac_addr_mode_e
{
  MAC_ADDR_MODE_NO_ADDR  = 0x00,
  MAC_ADDR_MODE_RESERVED = 0x01,
  MAC_ADDR_MODE_16BIT    = 0x02,
  MAC_ADDR_MODE_64BIT    = 0x03
} mac_addr_mode_t;

/* MAC 16 bit address. */
typedef struct mac_addr_16bit_addr_fileds_s
{
  uint16_t dest_pan_id;
  uint16_t dest_addr;
  uint16_t source_pan_id;
  uint16_t source_addr;
} PACKED_STRUCT mac_addr_16bit_addr_fileds_t;

/* MAC 64 bit address. */
typedef struct mac_addr_64bit_addr_fileds_s
{
  uint16_t dest_pan_id;
  char  dest_addr[8];
  uint16_t source_pan_id;
  char  source_addr[8];
} PACKED_STRUCT mac_addr_64bit_addr_fileds_t;

/* MAC Frame control field. */
#ifdef ZB_LITTLE_ENDIAN

typedef struct mac_frame_control_s
{
  unsigned frame_type:3;
  unsigned security_enabled:1;
  unsigned frame_pending:1;
  unsigned ack_request:1;
  unsigned pan_id_compression:1;
  unsigned reserved:3;
  unsigned dest_addr_mode:2;
  unsigned frame_version:2;
  unsigned source_addr_mode:2;
} PACKED_STRUCT mac_frame_control_t;

#else /* ZB_BIG_ENDIAN */

typedef struct mac_frame_control_s
{
  unsigned source_addr_mode:2;
  unsigned frame_version:2;
  unsigned dest_addr_mode:2;
  unsigned reserved:3;
  unsigned pan_id_compression:1;
  unsigned ack_request:1;
  unsigned frame_pending:1;
  unsigned security_enabled:1;
  unsigned frame_type:3;
} PACKED_STRUCT mac_frame_control_t;

#endif

/* MAC 16 bit address frame format */
typedef struct mac_frame_format_16bit_addr_s
{
  mac_frame_control_t frame_control;
  unsigned char sequence_number;
  mac_addr_16bit_addr_fileds_t addr;
} PACKED_STRUCT mac_frame_format_16bit_addr_t;

/* MAC 64 bit address frame format */
typedef struct mac_frame_format_64bit_addr_s
{
  mac_frame_control_t frame_control;
  unsigned char sequence_number;
  mac_addr_64bit_addr_fileds_t addr;
} PACKED_STRUCT mac_frame_format_64bit_addr_t;

/* NWK frame control  */
#ifdef ZB_LITTLE_ENDIAN

typedef struct nwk_frame_control_s
{
  unsigned frame_type:2;
  unsigned protocol_version:4;
  unsigned discover_route:2;
  unsigned multicast:1;
  unsigned security:1;
  unsigned source_route:1;
  unsigned dest_ieee_addr:1;
  unsigned src_ieee_addr:1;
  unsigned reserverd:3;
} PACKED_STRUCT
nwk_frame_control_t;

#else /* ZB_BIG_ENDIAN */

typedef struct nwk_frame_control_s
{
  unsigned reserverd:3;
  unsigned src_ieee_addr:1;
  unsigned dest_ieee_addr:1;
  unsigned source_route:1;
  unsigned security:1;
  unsigned multicast:1;
  unsigned discover_route:2;
  unsigned protocol_version:4;
  unsigned frame_type:2;
} PACKED_STRUCT
nwk_frame_control_t;

#endif

/* NWK frame */
typedef struct nwk_hdr_s
{
  nwk_frame_control_t frame_control;
  uint16_t            dst_addr;
  uint16_t            src_addr;
  uint8_t             radius;
  uint8_t             seq_num;
  uint8_t             dst_ieee_addr[8];
  uint8_t             src_ieee_addr[8];
  uint8_t             mcast_control;
} PACKED_STRUCT
nwk_hdr_t;

typedef struct nwk_aux_frame_hdr_s
{
  uint8_t     secur_control;
  uint32_t    frame_counter;
  uint8_t     source_address[8];
  uint8_t     key_seq_number;
}  __attribute__ ((packed)) nwk_aux_frame_hdr_t;


typedef struct secur_ccm_nonce_s
{
  uint8_t     source_address[8];
  uint32_t    frame_counter;
  uint8_t     secur_control;
}  __attribute__ ((packed))  secur_ccm_nonce_t;

/* APS frame control  */
#ifdef ZB_LITTLE_ENDIAN

typedef struct aps_frame_control_s
{
  unsigned frame_type:2;
  unsigned delivery_mode:2;
  unsigned reserverd:1;
  unsigned security:1;
  unsigned acknowlegment:1;
  unsigned extended_header:1;
} PACKED_STRUCT
aps_frame_control_t;

#else /* ZB_BIG_ENDIAN */

typedef struct aps_frame_control_s
{
  unsigned extended_header:1;
  unsigned acknowlegment:1;
  unsigned security:1;
  unsigned reserverd:1;
  unsigned delivery_mode:2;
  unsigned frame_type:2;
} PACKED_STRUCT
aps_frame_control_t;

#endif

void get_mac_addr_16bit_addr_fileds(uint8_t *ptr, mac_addr_16bit_addr_fileds_t *ret);
void get_mac_addr_64bit_addr_fileds(uint8_t *ptr, mac_addr_64bit_addr_fileds_t *ret);
void get_mac_frame_control(uint8_t *ptr, mac_frame_control_t *ret);
void get_mac_frame_format_16bit_addr(uint8_t *ptr, mac_frame_format_16bit_addr_t *ret);
void get_mac_frame_format_64bit_addr(uint8_t *ptr, mac_frame_format_64bit_addr_t *ret);
void get_nwk_frame_control(uint8_t *ptr, nwk_frame_control_t *ret);
void get_nwk_hdr(uint8_t *ptr, nwk_hdr_t *ret);
void get_nwk_aux_frame_hdr(uint8_t *ptr, nwk_aux_frame_hdr_t *ret);
void get_secur_ccm_nonce(uint8_t *ptr, secur_ccm_nonce_t *ret);
void get_aps_frame_control(uint8_t *ptr, aps_frame_control_t *ret);

#define ZB_CCM_NONCE_LEN 13
#define ZB_CCM_L 2

#endif /*  __MAC_FRAME_H__ */
