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
PURPOSE: Mac layer API
*/

#ifndef ZB_MAC_API_INCLUDED
#define ZB_MAC_API_INCLUDED

#include "zb_types.h"
#include "zb_debug.h"
#include "zb_bufpool.h"
#include "zb_ubec24xx.h"
#include "zb_ns3.h"
#ifdef ZB_CC25XX
#include "zb_cc25xx.h"
#endif




/*! \cond internals_doc */
/**
   @addtogroup ZB_MAC
   @{
*/

/**
   Get MAC context
 */
#define MAC_CTX() (ZG->mac.mac_ctx)

/**
   Return maximal mac header size
*/
#define ZB_MAC_MAX_HEADER_SIZE(dst_short_addr, src_short_addr)          \
  2  /* fcf */ + 1 /* seq num */ + (dst_short_addr) ? 2 : 8 /* dst addr */ + \
  2  /* pan */ + (src_short_addr) ? 2 : 8 /* src addr */ /* TODO: aux security fields */

/**
   Address modes for different protocol layers

   Hope can use same constants without re-assign. TODO: check it!
 */
typedef enum zb_addr_mode_e
{
  ZB_ADDR_NO_ADDR = 0,          /*!< 802.15: 0x00 = no address (addressing fields omitted, see 7.2.1.1.8). */

  ZB_ADDR_16BIT_MULTICAST = 1,  /*!< 802.15: 0x1 = reserved.
                                     NWK:    0x01 = 16-bit multicast group address
                                     APS:    0x01 = 16-bit group address for DstAddress;
                                 */
#define ZB_ADDR_16BIT_GROUP  ZB_ADDR_16BIT_MULTICAST

  ZB_ADDR_16BIT_DEV_OR_BROADCAST = 2, /*!< 802.15: 0x02 = 16-bit short address.
                                           NWK:    0x02=16-bit network address of a device or a 16-bit broadcast address
                                      */

  ZB_ADDR_64BIT_DEV = 3         /*!< 802.15: 0x03 = 64-bit extended address.  */
} ZB_PACKED_STRUCT
zb_addr_mode_t;

/**
   offset in bytes from the start of the mac frame to sequence number
   field
*/
#define SEQ_NUMBER_OFFSET           2
           /* check specification IEEE 802.15 item 7.2.1 */

/**
   offset in bytes from the start of the mac frame to destination PAN
   identifier field
*/
#define ZB_MAC_DST_PANID_OFFSET     3
           /* check specification IEEE 802.15 item 7.2.1 */

/**
   offset in bytes from the start of the mac frame to destination address
   field if field destination PAN identifier is presented in given mac
   frame
*/
#define DST_ADDR_OFFSET             5
          /* check specification IEEE 802.15 item 7.2.1 */

/**
   length in bytes Sequence Number
*/
#define SEQUENCE_NUMBER_LENGTH      2
           /* check specification IEEE 802.15 item 7.2.2.1 */

/**
   length in bytes SuperFrame Specification
*/
#define SUREPFRAME_SPEC_LENGTH      2
           /* check specification IEEE 802.15 item 7.2.2.1 */

/**
   length in bytes of and field containg PAN id
*/
#define PAN_ID_LENGTH               2
           /* check specification IEEE 802.15 item 7.2.1 */

/**
   Number of the first channel
*/
#define ZB_MAC_START_CHANNEL_NUMBER ZB_TRANSCEIVER_START_CHANNEL_NUMBER
/**
   Maximal number of the channels
 */
#define ZB_MAC_MAX_CHANNEL_NUMBER   ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER
/**
   Total number of supported channels
 */
#define ZB_MAC_SUPPORTED_CHANNELS   (ZB_MAC_MAX_CHANNEL_NUMBER - ZB_MAC_START_CHANNEL_NUMBER + 1)


/**
   All supported channels mask
*/
#define ZB_MAC_ALL_CHANNELS_MASK    ZB_TRANSCEIVER_ALL_CHANNELS_MASK

/**
   Maximal pending addresses number
 */
#define MAX_PENDING_ADDRESSES          7
/**
   Maximal frame size
 */
#define MAX_PHY_FRM_SIZE              127
/**
   Maximal beacon overhead
 */
#define MAX_BCN_OVERHEAD              75
/**
   Maximal beacon payload size
 */
#define MAX_BCN_PAYLOAD               (MAX_PHY_FRM_SIZE-MAX_BCN_OVERHEAD)
/**
   Minimal CAP length
 */
#define MAC_MIN_CAP_LENGTH 440  /* mac spec 7.4.1 MAC constants */

/**
  Definitions for extra information size in RX fifo packet. Packet format:
  1 byte packet length + N bytes of RX fifo data + 9 bytes of extra information
  (LQI, RSSI, Frame timer, Superframe counter)
  See 3.2.3 RXMAC.
*/
#define ZB_MAC_PACKET_LENGTH_SIZE 1
#define ZB_MAC_EXTRA_DATA_SIZE    9

/**
    Is used for macBeaconOrder and macSuperframeOrder
*/
#define ZB_TURN_OFF_ORDER  15
/**
   Short address value
 */
#define ZB_MAC_SHORT_ADDR_NO_VALUE      0xFFFF
/**
   Not allocated short address value
 */
#define ZB_MAC_SHORT_ADDR_NOT_ALLOCATED 0xFFFE
/**
   Broadcast pan id value
 */
#define ZB_BROADCAST_PAN_ID 0xFFFF

/* 7.4 MAC constants and PIB attributes */
/**
   MAC base slot duration value
 */
#define ZB_MAC_BASE_SLOT_DURATION 60
/**
   Superframe slots number
 */
#define ZB_MAC_NUM_SUPERFRAME_SLOTS 16
/**
   MAC base superframe duration value
 */
#define ZB_MAC_BASE_SUPERFRAME_DURATION ZB_MAC_BASE_SLOT_DURATION * ZB_MAC_NUM_SUPERFRAME_SLOTS /* = 960 */

/*
NOTE: 1 beacon interval = ZB_MAC_BASE_SUPERFRAME_DURATION * 1 symbol (16e-6 sec) = 15.36 milliseconds
Beacon interval is used to measure MAC timeouts
*/

/**
   Wait synchronously for packet transmit
 */
#define ZB_SYNC_WAIT_FOR_SEND_COMPLETION() \
  ZB_SCHED_WAIT_COND(ZB_GET_SEND_STATUS() == ZB_SEND_FINISHED); \
  ZB_SET_SEND_STATUS(ZB_NO_IO);

/**
   Waiting for packet receive complete
 */
#define ZB_SYNC_WAIT_FOR_RECV_COMPLETION()                      \
  ZB_SCHED_WAIT_COND(ZB_GET_RECV_STATUS() == ZB_RECV_FINISHED); \
  ZB_SET_RECV_STATUS(ZB_NO_IO);


/* TRICKY: set MAC_CTX().ack_dsn value before calling this macro!!! */
/**
   Starts acknowledge wait (used in NS build)
 */

#define ZB_MAC_START_ACK_WAITING() \
{                                                                       \
  ZB_MAC_CLEAR_ACK_TIMEOUT();                                           \
  ZB_MAC_CLEAR_ACK_FAIL();                                              \
  ZB_MAC_CLEAR_ACK_OK();                                                \
                                                                        \
  ZB_SCHEDULE_ALARM_CANCEL(zb_mac_ack_timeout, ZB_ALARM_ALL_CB);        \
  ZB_SCHEDULE_ALARM(zb_mac_ack_timeout, 0, MAC_PIB().mac_ack_wait_duration); \
}


/**
   Enum defines transmission flags described in IEEE standart 802.15

   @param MAC_TX_OPTION_ACKNOWLEDGED_BIT: 1 - acknowledged transmission
            0 - unacknowledged transmission

   @param MAC_TX_OPTION_GTS_CAP_BIT:    1 - GTS transmission
            0 - CAP transmission

   @param MAC_TX_OPTION_TRANSMISSION_BIT: 1 - indirect transmission
            0 - direct transmission
*/

enum mac_frame_version_e
{
  MAC_FRAME_IEEE_802_15_4_2003 = 0,
  MAC_FRAME_IEEE_802_15_4      = 1
};

/**
   Device types
 */
enum zb_mac_dev_type_e
{
  MAC_DEV_UNDEFINED,
  MAC_DEV_FFD,
  MAC_DEV_RFD
};

/**
   MAC options bits
 */
enum mac_tx_options_bits_e
{
  MAC_TX_OPTION_ACKNOWLEDGED_BIT          = 1,
  MAC_TX_OPTION_GTS_CAP_BIT               = 2,
  MAC_TX_OPTION_INDIRECT_TRANSMISSION_BIT = 4
};

/**
   MAC frame types
 */
enum mac_frame_type_e
{
  MAC_FRAME_BEACON         = 0,
  MAC_FRAME_DATA           = 1,
  MAC_FRAME_ACKNOWLEDGMENT = 2,
  MAC_FRAME_COMMAND        = 3,
  MAC_FRAME_RESERVED1      = 4,
  MAC_FRAME_RESERVED2      = 5,
  MAC_FRAME_RESERVED3      = 6,
  MAC_FRAME_RESERVED4      = 7
};

/**
   MAC command frame id
 */
enum mac_command_frame_id
{
  MAC_CMD_ASSOCIATION_REQUEST = 1,
  MAC_CMD_ASSOCIATION_RESPONSE,
  MAC_CMD_DISASSOCIATION_NOTIFICATION,
  MAC_CMD_DATA_REQUEST,
  MAC_CMD_PAN_ID_CONFLICT_NOTIFICATION,
  MAC_CMD_ORPHAN_NOTIFICATION,
  MAC_CMD_BEACON_REQUEST,          /* 7 */
  MAC_CMD_COORDINATOR_REALIGNMENT,
  MAC_CMD_GTS_REQUEST
};

/* MAC security */
#ifdef ZB_MAC_SECURITY
#define MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE 6
#else
#define MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE 0
#endif

#define ZB_MAC_SECURITY_LEVEL 5
#define ZB_MAC_KEY_ID_MODE    1




/* Frame control field macroses */

/**
   Sets frame type subfield in frame control field ( FCF )
   Valid values are defined in mac_frame_type_e enum.


   @param p_fcf - pointer to 16bit FCF field.
   @param frame_type - mac_frame_type_e enum value.
*/


#define ZB_FCF_SET_FRAME_TYPE( p_fcf, frame_type )                                 \
do                                                                                 \
{                                                                                  \
  ZB_ASSERT( (zb_uchar_t) frame_type < MAC_FRAME_RESERVED1 );                      \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) &= 0xf8;                        \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) |= (zb_uchar_t) ( frame_type ); \
} while( 0 )


/**
   Gets frame type subfield in frame control field ( FCF )
   Return values are in range of \see mac_frame_type_e enum.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_FRAME_TYPE( p_fcf ) ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) & 0x07 ))


/**
   Gets security bit subfield in frame control field ( FCF )
   Return values can be 0 or 1.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_SECURITY_BIT( p_fcf ) ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) & 0x08 ))


/**
   Sets security bit subfield in frame control field ( FCF )
   Input values can be 0 or 1.

   @param p_fcf     - pointer to 16bit FCF field.
   @param bit_value - 0 or 1.
*/

#define ZB_FCF_SET_SECURITY_BIT( p_fcf, bit_value )                     \
  do                                                                    \
{                                                                       \
  ZB_ASSERT( ( bit_value ) == 0 || ( bit_value ) == 1 );                \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) &= 0xF7;             \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) |= (bit_value) << 3; \
} while( 0 )

/**
   Gets frame pending bit subfield in frame control field ( FCF )
   Return values can be 0 or 1.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_FRAME_PENDING_BIT( p_fcf ) ((zb_uint_t)(( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) & 0x10 ))


/**
   Sets security bit subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param bit_value - 0 or 1.
*/

#define ZB_FCF_SET_FRAME_PENDING_BIT( p_fcf, bit_value )                \
  do                                                                    \
{                                                                       \
  ZB_ASSERT( ( bit_value ) == 0 || ( bit_value ) == 1 );                \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) &= 0xEF;             \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) |= (bit_value) << 4; \
} while( 0 )

/**
   Gets ack request bit subfield in frame control field ( FCF )
   Return values can be 0 or 1.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_ACK_REQUEST_BIT( p_fcf ) ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) & 0x20 ))


/**
   Sets ack request bit subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param bit_value - 0 or 1.
*/

#define ZB_FCF_SET_ACK_REQUEST_BIT( p_fcf, bit_value )                  \
  do                                                                    \
{                                                                       \
  ZB_ASSERT( ( bit_value ) == 0 || ( bit_value ) == 1 );                \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) &= 0xDF;             \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) |= (bit_value) << 5; \
} while( 0 )


/**
   Gets PAN ID compression bit subfield in frame control field ( FCF )
   Return values can be 0 or 1.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_PANID_COMPRESSION_BIT( p_fcf ) ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) & 0x40 ))


/**
   Sets ack request bit subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param bit_value - 0 or 1.
*/

#define ZB_FCF_SET_PANID_COMPRESSION_BIT( p_fcf, bit_value )            \
  do                                                                    \
{                                                                       \
  ZB_ASSERT( ( bit_value ) == 0 || ( bit_value ) == 1 );                \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) &= 0xBF;             \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_ZERO_BYTE] ) |= (bit_value) << 6; \
} while( 0 )


/**
   Gets destination addressing mode subfield in frame control field ( FCF )
   Return values is one value from zb_addr_mode_e enum.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_FRAME_VERSION( p_fcf )  ( ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE] ) & 0x30 )) >> 4 )


/**
   Sets frame version subfield in frame control field ( FCF )
   Return values is one value defined in mac_frame_version_e.

   @param p_fcf     - pointer to 16bit FCF field.
   @param frame_version -
*/

#define ZB_FCF_SET_FRAME_VERSION( p_fcf, frame_version )        \
do                                                              \
{                                                               \
  ZB_ASSERT( ( frame_version ) == 0 ||                          \
             ( frame_version ) == 1    );                       \
                                                                \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE] ) &= 0xCF;                    \
  ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE] ) |= ( frame_version ) << 4;  \
} while( 0 )

/**
   Gets source addressing mode subfield in frame control field ( FCF )
   Return values is one value from zb_addr_mode_e enum.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_SRC_ADDRESSING_MODE( p_fcf )  ( ((zb_uint_t)( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE] ) & 0xC0 )) >> 6 )



#define ZB_CLEAR_PENDING_QUEUE_SLOT(num) \
MAC_CTX().pending_data_queue[(num)].pending_data = NULL;

/**
   Sets src addressing subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param addr_mode - 0 or 1.
*/
/* implemented as function zb_fcf_set_src_addressing_mode()
#define ZB_FCF_SET_SRC_ADDRESSING_MODE( p_fcf, addr_mode )                \
do                                                                        \
{                                                                         \
  ZB_ASSERT( ( addr_mode ) == 0 ||                                        \
             ( addr_mode ) == 1 ||                                        \
             ( addr_mode ) == 2 ||                                        \
             ( addr_mode ) == 3    );                                     \
                                                                          \
  ( ( ( zb_uint8_t* ) ( p_fcf ))[ZB_PKT_16B_FIRST_BYTE] ) &= 0x3F;               \
  ( ( ( zb_uint8_t* ) ( p_fcf ))[ZB_PKT_16B_FIRST_BYTE] ) |= ( addr_mode ) << 6; \
} while( 0 )
*/
zb_void_t zb_fcf_set_src_addressing_mode(zb_uint8_t *p_fcf, zb_uint8_t addr_mode);
#define ZB_FCF_SET_SRC_ADDRESSING_MODE zb_fcf_set_src_addressing_mode

/**
   Gets source addressing mode subfield in frame control field ( FCF )
   Return values is one value from zb_addr_mode_e enum.

   @param p_fcf - pointer to 16bit FCF field.
*/

#define ZB_FCF_GET_DST_ADDRESSING_MODE( p_fcf )  (( (zb_uint_t)(( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE] ) & 0x0C) ) >> 2 )


/**
   Sets dst addressing subfield in frame control field ( FCF )

   @param p_fcf     - pointer to 16bit FCF field.
   @param addr_mode - 0 or 1.
*/
/* implemented as function zb_fcf_set_dst_addressing_mode()
#define ZB_FCF_SET_DST_ADDRESSING_MODE( p_fcf, addr_mode )                    \
do                                                                            \
{                                                                             \
  ZB_ASSERT( ( addr_mode ) == 0 ||                                            \
             ( addr_mode ) == 1 ||                                            \
             ( addr_mode ) == 2 ||                                            \
             ( addr_mode ) == 3    );                                         \
                                                                              \
  ( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE]) ) &= 0xF3;               \
  ( ( ( ( zb_uint8_t* ) ( p_fcf ) )[ZB_PKT_16B_FIRST_BYTE]) ) |= ( addr_mode ) << 2; \
} while( 0 )
*/
zb_void_t zb_fcf_set_dst_addressing_mode(zb_uint8_t *p_fcf, zb_uint8_t addr_mode);
#define ZB_FCF_SET_DST_ADDRESSING_MODE zb_fcf_set_dst_addressing_mode

/**
   Gets memory address of FCF field inside MAC frame

   @param p_buf     - pointer to beginning of the MAC header r
   @return            - pointer to 16 bit FCF field
*/

#define ZB_MAC_GET_FCF_PTR( p_buf ) ( ( zb_uint8_t* ) ( p_buf ) )


/**
   Gets length in bytes of source address field inside MAC frame

   @param p_buf     - pointer to beginning of the MAC header
   @param out_len     - name of variable of any int type which has been declared before.
      stored value is in bytes, so following values are possible [ 0, 2, 8 ]

   @b Example
@code
        int z;
        ZB_FCF_GET_DST_ADDR_LENGTH( p_buf, z );
@endcode
*/

/**
   Tail size for mac packet
 */
#ifndef ZB_CC25XX
#define ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME (2/*fcs*/ + ZB_MAC_EXTRA_DATA_SIZE)
#else
#define ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME (1/*LQI */)
#endif

/**
   Get LQI value
   @param p_buf - pointer to buffer
 */
#ifndef ZB_CC25XX
#define ZB_MAC_GET_LQI(packet) *((zb_uint8_t*)ZB_BUF_BEGIN(packet) + ZB_BUF_LEN(packet) - ZB_MAC_EXTRA_DATA_SIZE)
#else
#define ZB_MAC_GET_LQI(packet) 255 /* TI devices can't directly get an LQI, it should be calculated */
                                   /* empirically from RSSI */
#endif
/**
   Get RSSI value
   @param p_buf - pointer to buffer
 */
#define ZB_MAC_GET_RSSI(p_buf) *((zb_uint8_t*)ZB_BUF_BEGIN(packet) + ZB_BUF_LEN(packet) - ZB_MAC_EXTRA_DATA_SIZE + 1)


#ifdef ZB_NS_BUILD
/**
   Adds FCS to mac packet
 */
void zb_mac_fcs_add(zb_buf_t *buf);
#define MAC_ADD_FCS(buf) zb_mac_fcs_add(buf)
#else
#define MAC_ADD_FCS(buf)
#endif


/**
   Parameters for data request
 */
typedef struct zb_mcps_data_req_params_s
{
#ifndef ZB_MAC_EXT_DATA_REQ
  zb_uint16_t src_addr;
  zb_uint16_t dst_addr;
#else
  /* In ZigBee never need to send data to/from long address, but need it for
   * MAC certification testing. */
  union zb_addr_u src_addr;
  zb_uint8_t      src_addr_mode;
  union zb_addr_u dst_addr;
  zb_uint8_t      dst_addr_mode;
  zb_uint16_t     dst_pan_id;
#endif  /* ZB_MAC_TESTING_MODE */
  zb_uint8_t tx_options;
  zb_uint8_t mhr_len;  /* mhr length, stored while filling data
                        * request, stored for further processing */
  zb_uint8_t msdu_handle;
#ifdef ZB_MAC_SECURITY
  zb_uint8_t      security_level;
  zb_uint8_t      key_id_mode;
  zb_uint8_t      key_source[8];
  zb_uint8_t      key_index;
#endif
}
zb_mcps_data_req_params_t;

/**
   Parameters for data confirm call
 */
typedef struct zb_mcps_data_confirm_params_s
{
  zb_uint8_t msdu_handle;
}
zb_mcps_data_confirm_params_t;

/**
   fills parameters structure for data request
*/
#define ZB_MCPS_BUILD_DATA_REQUEST(buf, src_addr_param, dst_addr_param, tx_options_param, msdu_hande_param) \
  {                                                                     \
    zb_mcps_data_req_params_t *_p = ZB_GET_BUF_PARAM((buf), zb_mcps_data_req_params_t); \
    _p->src_addr = (src_addr_param);                                    \
    _p->dst_addr = (dst_addr_param);                                    \
    _p->tx_options = (tx_options_param);                                \
    _p->msdu_handle = (msdu_hande_param);                               \
  }



/**
   Remove MAC header and trailer from the packet.

   To be used when passing packet up in NWK.
   Save hdr offset in internal buf structure to be able to get mac hdr data from
   upper layers, i.e. from nwk route discovery

   @param packet - packet to proceed
   @param ptr    - pointer to the NWK begin
 */
#define ZB_MAC_CUT_HDR(packet, hlen, ptr)                         \
do                                                                \
{                                                                 \
  /* Save hdr offset */                                           \
  packet->u.hdr.mac_hdr_offset = ZB_BUF_OFFSET(packet);           \
  /* Remove RSSI etc. */                                          \
  ZB_BUF_CUT_RIGHT(packet, ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME);  \
  ZB_BUF_CUT_LEFT(packet, hlen, ptr);                             \
} while (0)


/*
  NS build adds FCS to the packet end while HW build does not
 */
#ifdef ZB_NS_BUILD
#define ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME (2/*fcs*/)
#define ZB_MAC_CUT_FCS(packet) ZB_BUF_CUT_RIGHT(packet, ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME);
#else
#define ZB_MAC_CUT_FCS(packet)
#define ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME 0
#endif

#define ZB_BEACON_PAYLOAD_OFFSET(mhr_len) ((mhr_len) + SUREPFRAME_SPEC_LENGTH + 1 /*GTS*/ + 1/*pending address specification*/ )
#define ZB_BEACON_PAYLOAD_TAIL (ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME + ZB_MAC_EXTRA_DATA_SIZE)


/**
   Remove MAC header from the packet.

   To be used when passing packet up in NWK.
   Save hdr offset in internal buf structure to be able to get mac hdr data from
   upper layers, i.e. from nwk route discovery

   @param packet - packet to proceed
   @param ptr    - pointer to the NWK begin
 */

#if 0
#define ZB_MAC_CUT_HDR_WITHOUT_TRAILER(_packet, _ptr)               \
do                                                                \
{                                                                 \
  zb_ushort_t hlen;                                               \
  void *mac_hdr = ZB_BUF_BEGIN(_packet);                           \
                                                                  \
  /* Save hdr offset */                                           \
  _packet->u.hdr.mac_hdr_offset = ZB_BUF_OFFSET(_packet);           \
  ZB_MAC_CUT_FCS(_packet);                                         \
  hlen = zb_mac_calculate_mhr_length(                             \
    ZB_FCF_GET_SRC_ADDRESSING_MODE(mac_hdr),                      \
    ZB_FCF_GET_DST_ADDRESSING_MODE(mac_hdr),                      \
    ZB_FCF_GET_PANID_COMPRESSION_BIT(mac_hdr));                   \
                                                                  \
  ZB_BUF_CUT_LEFT(_packet,                                         \
    hlen + MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE * ZB_FCF_SET_SECURITY_BIT(mac_hdr),\
    _ptr);                                                         \
} while (0)
#endif

#define ZB_MAC_CUT_HDR_WITHOUT_TRAILER(_packet, _ptr)               \
do                                                                \
{                                                                 \
  zb_ushort_t hlen;                                               \
  void *mac_hdr = ZB_BUF_BEGIN(_packet);                           \
                                                                  \
  /* Save hdr offset */                                           \
  _packet->u.hdr.mac_hdr_offset = ZB_BUF_OFFSET(_packet);           \
  ZB_MAC_CUT_FCS(_packet);                                         \
  hlen = zb_mac_calculate_mhr_length(                             \
    ZB_FCF_GET_SRC_ADDRESSING_MODE(mac_hdr),                      \
    ZB_FCF_GET_DST_ADDRESSING_MODE(mac_hdr),                      \
    ZB_FCF_GET_PANID_COMPRESSION_BIT(mac_hdr));                   \
  ZB_BUF_CUT_LEFT(_packet, hlen + MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE * ZB_FCF_GET_SECURITY_BIT(mac_hdr) , _ptr);                                        \
                                        \
                                                                  \
} while (0)


/**
   Holds status of purge operation.
   NOTE: this struct does not contain msduHandle, because it must be in NWK
   header
   @param status - The status of the request to be purged an MSDU
                   from the transaction queue ( one of values of mac_status_e )
 */

typedef struct zb_mac_purge_confirm_s
{
  zb_uint8_t status;
} ZB_PACKED_STRUCT
zb_mac_purge_confirm_t;

/**
    Returns pointer to zb_mac_purge_confirm_t structure
    located somewhere in buf.

    @param buf    - pointer ot zb_buf_t container
    @param outptr - out pointer to zb_mac_purge_confirm_t struct located
                    somewhere inside buf
*/

#define ZB_MCPS_GET_PURGE_CONFIRM_PTR( buf, outptr )  \
do                                                    \
{                                                     \
} while( 0 )

/**
    Builds request to MAC layer to  purge a MSDU from the
    transaction queue
    @param buf - pointer to zb_buf_t container
    @param outlen -
 */

#define ZB_MCPS_BUILD_PURGE_REQUEST( buf, outlen ) \
do                                                 \
{                                                  \
} while( 0 )


/*
    Defines capability information field as a set of bitfields.

    bit 0 alternate_PAN_coordinator - the bit is set if the device is capable
           of becoming the PAN coordinator. Otherwise the subfield is set to
           zero.

    bit 1 device_type - the bit is set if the device is FFD. The zero value
           indicates that device is RFD.

    bit 2 power_source - the bit is set if the device is receiving power from
           the alternating current mains. Otherwise, the subfield is set to
           zero.

    bit 3 receive_on_whenidle - the bit is set if the device does not disable
           its receiver to conserve power during idle periods. Otherwise the
           bit shall be set to zero.

    bit 6 security_capability - the bit is set if the device is capable of
           sending and receiving cryptographically proteced MAC frames,
           Otherwise shall be set to zero .

    bit 7 allocate_address - the bit is set if the device wishes the
           coordinator to allocate 16bit short aaddress as a result of the
           assciation procedure. Otherwise it shall be set  to zero
*/
typedef zb_uint8_t zb_mac_capability_info_t;

#define ZB_MAC_CAP_GET_ALTERNATE_PAN_COORDINATOR(cap) ((cap) & 1)
#define ZB_MAC_CAP_SET_ALTERNATE_PAN_COORDINATOR(cap, v) (cap) |= (v)

#define ZB_MAC_CAP_GET_DEVICE_TYPE(cap) (((cap) >> 1) & 1)
#define ZB_MAC_CAP_SET_DEVICE_TYPE(cap, v)        \
  (                                               \
  (cap) &= 0xFD, /* clear bit 1 */                \
  (cap) |= (v << 1) /* set bit if it is == 1 */   \
)

#define ZB_MAC_CAP_GET_POWER_SOURCE(cap) (((cap) >> 2) & 1)
#define ZB_MAC_CAP_SET_POWER_SOURCE(cap, v) ((cap) |= (v << 2))

#define ZB_MAC_CAP_GET_RX_ON_WHEN_IDLE(cap) (((cap) >> 3) & 1)
#define ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(cap, v) ((cap) |= (v << 3))

/**
   Set capabilities typical for router: device type - R, power source - main, rx
   on when idle.
 */
#define ZB_MAC_CAP_SET_ROUTER_CAPS(cap) ((cap) |= (1 << 1) | (1 << 2) | (1 << 3))

#define ZB_MAC_CAP_GET_SECURITY(cap) (((cap) >> 6) & 1)
#define ZB_MAC_CAP_SET_SECURITY(cap, v) ((cap) |= (v << 6))

#define ZB_MAC_CAP_GET_ALLOCATE_ADDRESS(cap) (((cap) >> 7) & 1)
#define ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(cap, v) ((cap) |= (v << 7))


/**
   Data structure for mlme_associate_request

   Some fields are not set - (set explicitly by MAC):
   ChannelPage   - 0
   CoordAddrMode - 3 (64-bit address)
   SecurityLevel - ?
   KeyIdMode     - ?
   KeySource     - ?
   KeyIndex      - ?
*/
typedef struct zb_mlme_associate_params_s
{
  zb_uint16_t     pan_id;
  union zb_addr_u coord_addr;
  zb_uint8_t      coord_addr_mode;
  zb_uint8_t      logical_channel;
  zb_mac_capability_info_t capability;
}
zb_mlme_associate_params_t;

/**
    Builds MLME-ASSOCIATE request. This request allows a device to request an
    association  with a coordinator

    @param buf - pointer to zb_buf_t

    @param logical_channel - selected from the available logical channels
           supported by the PHY

    @param coord_PAN_id - the identifier of the PAN with which to associate

    @param addr_mode - ZB_ADDR_16BIT_DEV_OR_BROADCAST or ZB_ADDR_64BIT_DEV

    @param p_address - pointer of the address of the coordinator with which to
           associate, Suppose 64-bit address mode.

    @param capability - zb_mac_capability_info_t struct. Specifies the
                        operational capabilities of the associating device.
 */

#define ZB_MLME_BUILD_ASSOCIATE_REQUEST( buf,                           \
                                         _logical_channel,              \
                                         coord_PAN_id,                  \
                                         addr_mode,                     \
                                         p_address,                     \
                                         _capability)                   \
do                                                                      \
{                                                                       \
  zb_mlme_associate_params_t *_p = (zb_mlme_associate_params_t *)ZB_GET_BUF_PARAM((buf), zb_mlme_associate_params_t); \
  _p->logical_channel = (_logical_channel);                             \
  _p->pan_id = (coord_PAN_id);                                          \
  _p->coord_addr_mode = addr_mode;                                      \
  if (addr_mode == ZB_ADDR_64BIT_DEV)                                   \
  {                                                                     \
    ZB_IEEE_ADDR_COPY(_p->coord_addr.addr_long, (p_address));           \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    ZB_MEMCPY(&_p->coord_addr.addr_short, (p_address), sizeof(zb_uint16_t)); \
  }                                                                     \
  _p->capability = _capability;                                         \
} while( 0 )

/**
    Defines struct for indication of associate request.

    @param device_address - IEEE 64bit address

    @param capability - zb_mac_capability_info_t struct. Specifies the
                        operational capabilities ofthe associating device.
 */


typedef struct zb_mlme_associate_indication_s
{
  zb_ieee_addr_t           device_address;
  zb_mac_capability_info_t capability;
  zb_uint8_t               lqi; /* non-standard, but MAC has it and we really
                                 * need it */
} ZB_PACKED_STRUCT
zb_mlme_associate_indication_t;


/**
   MLME-COMM-STATUS.indication data structure

   Not all fields included.
   Currently used after MLME-ASSOCIATE.response only.
   When will be used for MLME-ORPHAN.response, maybe, will add some fields.
 */
typedef struct zb_mlme_comm_status_indication_s
{
  union zb_addr_u src_addr;   /*!< The individual device address of the
                                  entity from which the frame causing the
                                  error originated.  */
  zb_uint8_t src_addr_mode;
  union zb_addr_u dst_addr;   /*!< The individual device address of the
                                device for which the frame was intended */
  zb_uint8_t dst_addr_mode;
  zb_uint8_t  status;           /*!< The communications status.
                                  SUCCESS,
                                  TRANSACTION_OVERFLOW,
                                  TRANSACTION_EXPIRED,
                                  CHANNEL_ACCESS_FAILURE,
                                  NO_ACK, COUNTER_ERROR,
                                  FRAME_TOO_LONG,
                                  IMPROPER_KEY_TYPE,
                                  IMPROPER_SECURITY_LEVEL,
                                  SECURITY_ERROR,
                                  UNAVAILABLE_KEY,
                                  UNSUPPORTED_LEGACY,
                                  UNSUPPORTED_SECURITY or
                                  INVALID_PARAMETER
                                 */
} ZB_PACKED_STRUCT
zb_mlme_comm_status_indication_t;




/**
   MLME-COMM-STATUS.indication data structure

   Not all foelds included.
   Currently used after MLME-ASSOCIATE.response only.
   When will be used for MLME-ORPHAN.response, maybe, will add some fields.
 */
typedef struct zb_mlme_associate_response_s
{
  zb_ieee_addr_t device_address;
  zb_uint16_t    short_address;
  zb_uint8_t     status;
} ZB_PACKED_STRUCT
zb_mlme_associate_response_t;



/**
    Builds MLME-ASSOCIATE response.

    @param buf - pointer to zb_buf_t

    @param p_address - pointer to the extended 64 bit IEEE  address of the
           coordinator with which to associate, size of address depends on
           coord_addr_mode parameter.

    @param short_address - 16-bit short device address
           allocated by the coordinator on successful association. This
           parameter is set to 0xffff if the association was unsuccessful.

    @param status - one of values of mac_status_e enum

    @param outlen - out integer variable to receive length

 */


#define ZB_MLME_BUILD_ASSOCIATE_RESPONSE( buf,                          \
                                          p_address,                    \
                                          short_addr,                   \
                                          st)                           \
do                                                                      \
{                                                                       \
  zb_mlme_associate_response_t *p = (zb_mlme_associate_response_t *)ZB_GET_BUF_PARAM((buf), zb_mlme_associate_response_t); \
  ZB_IEEE_ADDR_COPY(p->device_address, (p_address));                    \
  p->short_address = (short_addr);                                      \
  p->status = (st);                                                     \
}                                                                       \
while( 0 )

/**
   Defines structure for MLME-ASSOCIATE confirm primitive.

   @param assoc_short_addres - the short device address allocated by
          the coordinator on successful association. This parameter will be
          equal to 0xffff if the association attempt was unsuccessful.

   @param status - the status of the association attempt.
          ( value from   mac_status_e enum )

 */

typedef struct zb_mlme_associate_confirm_s
{
  zb_uint16_t assoc_short_address;
  zb_uint8_t  status;
  /* more fields (will it be used?):
     SecurityLevel
     KeyIdMode
     KeySource
     KeyIndex
   */
  zb_ieee_addr_t parent_address; /* non-standard field, but we really need it */
} ZB_PACKED_STRUCT
 zb_mlme_associate_confirm_t;

/**
    Returns pointer to zb_mac_associate_confirm_t structure
    located somewhere in buf.

    @param buf    - pointer ot zb_buf_t container
    @param outptr - out pointer to zb_mac_associate_confirm_t struct located
                    somewhere inside buf
 */

#define ZB_MLME_GET_ASSOCIATE_CONFIRM_PTR( buf, outptr ) \
do                                                       \
 {                                                       \
} while( 0 )

/**
   Builds MLME disassociate request. The MLME-DISASSOCIATE.request primitive is
   used by an associated device to notify the coordinator of its intent to leave
   the PAN. It is also used by the coordinator to instruct an associated device to leave the
   PAN.

   @param buf - pointer ot zb_buf_t container

   @param device_addr_mode - ( valid values  ZB_ADDR_16BIT_DEV_OR_BROADCAST or
          ZB_ADDR_64BIT_DEV ). The coordinator  addressing mode for this  primitive
          and subsequent MPDU.

   @param device_pan_id - the PAN identifier of the device to which to send the
          disassociation notification command.

   @param device_addr - pointer to the address of the device to which to send the
          disassociation notification command.

   @param disassociate_reason - one of values of mac_status_e enum

   @param tx_indirect - 1 if the disassociation notification command is to be
          sent indirectly, otherwise must be 0.

   @param outlen - out integer variable to receive length

*/

#define ZB_MLME_BUILD_DISASSOCIATE_REQUEST( buf,                  \
                                            device_addr_mode,     \
                                            device_pan_id,        \
                                            device_addr,          \
                                            disassociate_reason,  \
                                            tx_indirect,          \
                                            out_len )             \
do                                                                \
{                                                                 \
} while( 0 );


/**
   Defines struct for MLME disassociate indication primitive.

   @param device_address - the address of the device requesting disassociation.

   @param disassociate_reason - the status of the association attempt. ( value from   mac_status_e enum )

 */
typedef struct zb_mac_disassociate_indication_s
{
  zb_ieee_addr_t device_address;
  zb_uint8_t     disassociate_reason;
} ZB_PACKED_STRUCT
 zb_mac_disassociate_indication_t;

/**
    Returns pointer to zb_mac_disassociate_indication_t structure
    located somewhere in buf.

    @param buf    - pointer ot zb_buf_t container
    @param outptr - out pointer to zb_mac_associate_confirm_t struct located
                    somewhere inside buf
 */

#define ZB_MLME_GET_DISASSOCIATE_INDICATION_PTR( buf, outptr )  \
do                                                              \
{                                                               \
} while( 0 );

/**
 Defines struct which describes MLME-DISASSOCIATE.confirm primitive.

 @param status - the status of the disassociation attempt. ( one of the
        values list from the mac_status_e enum )

 @param device_addr_mode - ( valid values  ZB_ADDR_16BIT_DEV_OR_BROADCAST or
        ZB_ADDR_64BIT_DEV ). The addressing mode of the device that has either requested
        disassociation or been instructed to disassociate by its coordinator.

 @param device_pan_id - the PAN identifier of the device that has either requested disassociation or
        been instructed to disassociate by its coordinator.

 @param device_addr - the address of the device that has either requested disassociation or
        been instructed to disassociate by its coordinator.

 */

typedef struct zb_mac_disassociate_confirm_s
{
  zb_uint8_t      status;
  zb_uint8_t      device_addr_mode;
  zb_uint16_t     device_pan_id;
  union zb_addr_u device_addr;
} ZB_PACKED_STRUCT
zb_mac_disassociate_confirm_t;


/**
    Returns pointer to zb_mac_disassociate_confirm_t structure
    located somewhere in buf.

    @param buf    - pointer ot zb_buf_t container
    @param outptr - out pointer to zb_mac_associate_confirm_t struct located
                    somewhere inside buf
 */

#define ZB_MAC_GET_DISASSOCIATE_CONFIRM_PTR( buf, outptr )  \
do                                                          \
{                                                           \
} while( 0 )

/**
   Define the Superframe Specification field

  @param beacon_order - shall specify the transmission interval of the beacon.
                        ( see 7.5.1.1 )

  @param superframe_order - shall specify the length of time during which the
                            superframe is active (i.e., receiver enabled),
                            including the beacon frame transmission time. See
                            7.5.1.1 for explanation of the relationship between
                            the superframe order and the superframe duration.

  @param final_cap_slot - specifies the final superframe slot utilized by the CAP.
                          The duration of the CAP, as implied by this subfield,
                          shall be greater than or equal to the value specified by
                          aMinCAPLength. However, an exception is allowed for
                          the accommodation of the temporary increase in the
                          beacon frame length needed to perform GTS maintenance
                          (see 7.2.2.1.3).

  @param battery_life_extension - shall be set to one if frames transmitted to
                                  the beaconing device during its CAP are
                                  required to start in or before
                                  macBattLifeExtPeriods full backoff periods
                                  after the IFS period following the beacon.
                                  Otherwise, the BLE subfield shall be set to zero.

  @param reserved - reserved for future needs.

  @param pan_coordinator - shall be set to one if the beacon frame is being
                           transmitted by the PAN coordinator. Otherwise, the
                           PAN Coordinator subfield shall be set to zero.

  @param associate_permit - shall be set to one if macAssociationPermit is set
                            to TRUE ( i.e., the coordinator is accepting
                            association to the PAN ). The association permit bit
                            shall be set to zero if the coordinator is currently
                            not accepting association requests on its network.

 */

typedef struct zb_super_frame_spec_s
{
#ifdef ZB_LITTLE_ENDIAN
  zb_bitfield_t beacon_order:4;
  zb_bitfield_t superframe_order:4;

  zb_bitfield_t final_cap_slot:4;
  zb_bitfield_t battery_life_extension:1;
  zb_bitfield_t reserved:1;
  zb_bitfield_t pan_coordinator:1;
  zb_bitfield_t associate_permit:1;
#else
  zb_bitfield_t final_cap_slot:4;
  zb_bitfield_t battery_life_extension:1;
  zb_bitfield_t reserved:1;
  zb_bitfield_t pan_coordinator:1;
  zb_bitfield_t associate_permit:1;

  zb_bitfield_t beacon_order:4;
  zb_bitfield_t superframe_order:4;
#endif
} ZB_PACKED_STRUCT
zb_super_frame_spec_t;

/* zb_super_frame_spec_t structure is MSBF/LSBF specific, use the
 * following macro to make code MSBF/LSBF portable */

/**
   Sets superframe beacon order
   @param superframe - pointer to superframe
   @param beacon_order - beacon order value
 */
#define ZB_SUPERFRAME_SET_BEACON_ORDER(superframe, beacon_order)    \
  do                                                                \
  {                                                                 \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_ZERO_BYTE] &= 0xF0;             \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_ZERO_BYTE] |= (beacon_order);   \
  } while(0)

/**
   Sets superframe order
   @param superframe - pointer to superframe
   @param superframe_order - superframe order value
 */
#define ZB_SUPERFRAME_SET_SUPERFRAME_ORDER(superframe, superframe_order) \
  do                                                                    \
  {                                                                     \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_ZERO_BYTE] &= 0x0F;                 \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_ZERO_BYTE] |= (superframe_order) << 4; \
  } while(0)

/**
   Sets superframe fcs
   @param superframe - pointer to superframe
   @param fcs - fcs value
 */
#define ZB_SUPERFRAME_SET_FINAL_CAP_SLOT(superframe, fcs)               \
  do                                                                    \
  {                                                                     \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] &= 0xF0; /*1111.0000*/  \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] |= (fcs);               \
  } while(0)

/**
   Sets superframe ble
   @param superframe - pointer to superframe
   @param ble - superframe ble value
 */
#define ZB_SUPERFRAME_SET_BLE(superframe, ble)                          \
  do                                                                    \
  {                                                                     \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] &= 0xEF; /*1110.1111*/  \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] |= (ble) << 4;          \
  } while(0)

/**
   Sets superframe pan coordinatir flag
   @param superframe - pointer to superframe
   @param superframe_order - pan coordinatir flag value
 */
#define ZB_SUPERFRAME_SET_PAN_COORD(superframe, pan_coord)              \
  do                                                                    \
  {                                                                     \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] &= 0xBF; /*1011.1111*/  \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] |= (pan_coord) << 6;    \
  } while(0)

/**
   Sets superframe association permit flag
   @param superframe - pointer to superframe
   @param superframe_order - association permit flag value
 */
#define ZB_SUPERFRAME_SET_ASS_PERMIT(superframe, ass_permit)            \
  do                                                                    \
  {                                                                     \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] &= 0x7F; /*0111.1111*/  \
    ((zb_uint8_t*)(superframe))[ZB_PKT_16B_FIRST_BYTE] |= (ass_permit) << 7;   \
  } while(0)


/**
   Define the Beacon Payload field

   see figure 3.49

  @param protocol_id - This field identifies the network layer protocols in use and,
                       for purposes of this specification, shall always be set to 0,
                       indicating the ZigBee protocols.
                       The value 0xff shall also be reserved for future use by
                       the ZigBee Alliance.
                       ( see zigbee 3.6.7 )

  @param stack_profile - ZigBee stack profile identifier. ( see zigbee 3.6.7 )

  @param protocol_version - The version of the ZigBee protocol. ( see zigbee 3.6.7 )

  @param reserved - reserved for future needs.

  @param router_capacity - This value is set to TRUE if this device is capable
                           of accepting join requests from router-capable devices
                           and is set to FALSE otherwise. ( see zigbee 3.6.7 )

  @param device_depth - The network depth of this device. A value of 0x00 indicates
                        that this device is the ZigBee coordinator for the network.
                        ( see zigbee 3.6.7 )

  @param end_device_capacity - This value is set to TRUE if the device is capable
                               of accepting join requests from end devices seeking
                               to join the network and is set to FALSE otherwise.
                               ( see zigbee 3.6.7 )

  @param nwkExtendedPANId - The globally unique ID for the PAN of which the beaconing
                            device is a member. By default, this is the 64-bit IEEE address
                            of the ZigBee coordinator that formed the network,
                            but other values are possible and
                            there is no required structure to the address.
                            ( see zigbee 3.6.7 )

  @param txoffset - This value indicates the difference in time, measured in symbols,
                    between the beacon transmission time of the device and
                    the beacon transmission time of its parent;
                    This offset may be subtracted from the beacon transmission time
                    of the device to calculate the beacon transmission time of the parent.
                    This parameter is set to the default value of 0xFFFFFF
                    in beaconless networks.
                    ( see zigbee 3.6.7 )

  @param nwkUpdateId - This field reflects the value of nwkUpdateId from the NIB.
                       ( see zigbee 3.6.7 )

 */
typedef struct  zb_mac_beacon_payload_s
{
  /* TODO: verify order in which bitfields will be put to air */
  zb_uint8_t protocol_id;       /*!< for purposes of this specification,
                                  shall always be set to 0,
                                  indicating the ZigBee protocols.
                                */
#ifdef ZB_LITTLE_ENDIAN
  zb_bitfield_t stack_profile:4;
  zb_bitfield_t protocol_version:4;

  zb_bitfield_t reserved:2;
  zb_bitfield_t router_capacity:1;
  zb_bitfield_t device_depth:4;
  zb_bitfield_t end_device_capacity:1;
#else
  zb_bitfield_t reserved:2;
  zb_bitfield_t router_capacity:1;
  zb_bitfield_t device_depth:4;
  zb_bitfield_t end_device_capacity:1;

  zb_bitfield_t stack_profile:4;
  zb_bitfield_t protocol_version:4;
#endif
  zb_ext_pan_id_t extended_panid;
  zb_uint8_t txoffset[3];       /*!< This parameter is set to the
                                  default value of 0xFFFFFF in
                                  a beaconless networks.  */
  zb_uint8_t nwk_update_id;       /*<! This field reflects the value of
                                  nwkUpdateId from the NIB. */
} ZB_PACKED_STRUCT
zb_mac_beacon_payload_t;


/**
   Gets Beacon Payload from beacon frame

   @param p_buf       - pointer to beginning of the MAC header
   @param mhr_len     - MAC header length (must be already known - use \see zb_parse_mhr
   @param beacon_payload_p  - out pointer to Beacon Payload

   @b Example
@code
        zb_mac_beacon_payload_t *beacon_payload;
        ZB_MAC_GET_BEACON_PAYLOAD( p_buf, mhr_len, beacon_payload );
@endcode
*/
#define ZB_MAC_GET_BEACON_PAYLOAD(mhr, mhr_len, beacon_payload_p )        \
  beacon_payload_p = (void *)(((zb_uint8_t *)mhr) + mhr_len + SUREPFRAME_SPEC_LENGTH \
                              + 1 /*GTS*/ + 1/*pending address specification*/)


/**
   Gets Superframe specification from beacon frame

   @param mhr         - pointer to beginning of the MAC header
   @param mhr_len     - MAC header length (must be already known - use \see zb_parse_mhr
   @param superframe  - out pointer to superframe specification

   @b Example
@code
        zb_uint8_t *mac_hdr;
        ZB_GET_SUPERFRAME( mhr, mhr_len, superframe );
@endcode
*/
#define ZB_GET_SUPERFRAME(mhr, mhr_len, superframe)                     \
  ZB_LETOH16(superframe, (zb_uint16_t*)((zb_uint8_t*)(mhr) + (mhr_len)))


/**
   Gets GTS fields from beacon frame

   @param p_buf       - pointer to beginning of the MAC header
   @param mhr_len     - MAC header length (must be already known - use \see zb_parse_mhr
   @param gts_fields_p  - out pointer to GTS fields

   @b Example
@code
        zb_mac_gts_fields_t *beacon_payload;
        ZB_MAC_GET_GTS_FIELDS( p_buf, mhr_len, gts_fields );
@endcode
*/
#define ZB_MAC_GET_GTS_FIELDS(mhr, mhr_len, gts_fields_p )        \
  (gts_fields_p) = (void *)(((zb_uint8_t *)mhr) + mhr_len + SUREPFRAME_SPEC_LENGTH)


/**
   Get superframe spec from the Beacon packet.

   @param p_buf       - pointer to beginning of the MAC header
   @param mhr_len     - MAC header length (must be already known - use \see zb_parse_mhr
   @param superframe_p - out pointer to Superframe

 */
#define ZB_MAC_GET_SUPERFRAME_SPEC(mhr, mhr_len, superframe_p) \
  superframe_p = (void *)(((zb_uint8_t *)mhr) + mhr_len);



/**
   Gets Beacon Order subfield in Superframe Specification field ( SFS )

   @param p_sfs - pointer to 16bit SFS field.
*/

#define ZB_MAC_GET_BEACON_ORDER( p_sfs ) ((zb_uint_t)((((zb_uint8_t*)(p_sfs))[ZB_PKT_16B_ZERO_BYTE]) & 0x07))

/**
   Gets Association Permit subfield in Superframe Specification field ( SFS )

   @param p_sfs - pointer to 16bit SFS field.
*/

#define ZB_MAC_GET_ASSOCIATION_PERMIT( p_sfs ) ((zb_uint_t)((((zb_uint8_t*)(p_sfs))[ZB_PKT_16B_FIRST_BYTE] >> 7) & 1))

/**
   Gets Superframe Order subfield in Superframe Specification field ( SFS )

   @param p_sfs - pointer to 16bit SFS field.
*/

#define ZB_MAC_GET_SUPERFRAME_ORDER( p_sfs ) ((zb_uint_t)((((zb_uint8_t*)(p_sfs))[ZB_PKT_16B_ZERO_BYTE] >> 4) & 0xF))

/**
   Gets PAN Coordinator subfield in Superframe Specification field ( SFS )

   @param p_sfs - pointer to 16bit SFS field.
*/

#define ZB_MAC_GET_SUPERFRAME_PAN_COORDINATOR( p_sfs ) ((zb_uint_t)((((zb_uint8_t*) (p_sfs))[ZB_PKT_16B_FIRST_BYTE] >> 6) & 1))


/**
   Defines PAN descriptor structure

   @param coord_addr_mode - the coordinator addressing mode corresponding
          to the received beacon frame.
          ( valid values  ZB_ADDR_16BIT_DEV_OR_BROADCAST or
          ZB_ADDR_64BIT_DEV ).

   @param coord_pan_id - the PAN identifier of the coordinator as specified
          in the received beacon frame.

   @param coord_address - pointer to 16 bit or 64 bit IEEE address. This value
          depends on coord_addr_mode.

   @param logical_channel - the current logical channel occupied by the
          network.

   @param super_frame_spec - the superframe specification as specified in the
          received beacon frame.

   @param gts_permit - TRUE if the beacon is from the PAN coordinator that
          is accepting GTS requests.

   @param LinkQuality - the LQI at which the network beacon was received.
*/


typedef struct zb_mac_pan_descriptor_s    // 7.1.5.1.1 table-41
{
  zb_uint8_t            coord_addr_mode;
  zb_uint16_t           coord_pan_id;
  union zb_addr_u       coord_address;    /* pointer to 16 bit or 64 bit address, this
                                          * depends on value of coord_addr_mode  */
  zb_uint8_t            logical_channel;
  zb_super_frame_spec_t super_frame_spec;
  zb_uint8_t            gts_permit;
  zb_uint8_t            link_quality;
  /* zigbee does not use security and uses beaconless mode, so skip other pan descriptor
     fields - for timestamp and security  */
} ZB_PACKED_STRUCT
zb_pan_descriptor_t;

/**
   Defines pending address specification field.

   @param num_of_short_addr - indicates the number of 16-bit short addresses
          contained in the Address List field of the beacon frame.

   @param num_of_long_addr -

   @rvd0 - reserved bit.
   @
 */
typedef struct  zb_pending_address_spec_s
{
  zb_bitfield_t num_of_short_addr:3;
  zb_bitfield_t rvd0:1;
  zb_bitfield_t num_of_long_addr:3;
  zb_bitfield_t rvd1:1;
} ZB_PACKED_STRUCT
zb_pending_address_spec_t;

/**
   Enum defines types of possible scan operations

   @param ED_SCAN
   @param ACTIVE_SCAN
   @param PASSIVE_SCAN
   @param ORPHAN_SCAN
*/

enum zb_mac_scan_type_e
{
  ED_SCAN      = 0,
  ACTIVE_SCAN  = 1,
  PASSIVE_SCAN = 2,
  ORPHAN_SCAN  = 3
};

/**
   Defines scan duration
 */
#define ZB_MAX_SCAN_DURATION_VALUE 14

/**
   Table 67 MLME-SCAN.request parameters

   Table 67defines more parameters but NWK sends only 3 parameters.
   Others should be set by MAC to some constants.

 */
typedef struct
{
  zb_bitfield_t  scan_type:2;   /*!< Indicates the type of scan performed:
                                  0x00 = ED scan (optional for RFD).
                                  0x01 = active scan (optional for RFD).
                                  0x02 = passive scan.
                                  0x03 = orphan scan.  */
  zb_bitfield_t  scan_duration:4; /*!< A value used to calculate the length of time to
                                    spend scanning each channel for ED, active,
                                    and passive scans. This parameter is ignored for
                                    orphan scans.
                                    The time spent scanning each channel is
                                    [aBaseSuperframeDuration * (2**n + 1)]
                                    symbols, where n is the value of the
                                    ScanDuration parameter.  */
  zb_bitfield_t  reserved:10;
  zb_uint32_t    channels;   /*!< The 27bits (b , b ,... b ) indicate which
                                  channels are to be scanned (1=scan, 0=do not
                                  scan) for each of the 27 channels supported by
                                  the ChannelPage parameter.  */
} zb_mlme_scan_params_t;

/**

   Parameters for MLME-SCAN.request primitive

   @param buf      - pointer ot zb_buf_t container
   @param type     - one value of zb_mac_scan_type_e enum

   @param channels - bitmap. Only first 27 bits are used.
                     Bits (b0, b1,... b26) indicate which channels are to be scanned
                     ( 1 = scan, 0 = do not scan) for each of the 27 channels supported by hardware.
                     UBEK transciever maps 16 IEEE 802.15.4 2.4GHz band channels to following interval [ 11, 26 ].
                     So values of all others bits except bits from 11 to 26 are ignored

   @param duration - a value used to calculate the length of time to spend scanning each channel for ED,
                     active, and passive scans. This parameter is ignored for orphan scans. The time spent scanning
                     each channel is [ aBaseSuperframeDuration * ( 2n + 1 ) ] symbols, where n is the value of the
                     ScanDuration parameter.

*/

#define ZB_MLME_BUILD_SCAN_REQUEST( buf, _channels, type, duration)                                  \
do                                                                                                   \
{                                                                                                    \
  zb_mlme_scan_params_t *_p = (zb_mlme_scan_params_t *)ZB_GET_BUF_PARAM((buf), zb_mlme_scan_params_t); \
  _p->channels = _channels;                                                                          \
  _p->scan_type = type;                                                                              \
  _p->scan_duration = duration;                                                                      \
} while( 0 )

/**
   Defines structure for MAC-MLME.confirm primitive

   @param status - one of values of zb_mac_status_e

   @param scan_type - one of values of zb_mac_scan_type_e

   @param unscanned_channels - indicates which channels given in the
                               request were not scanned
                               ( 1 = not scanned, 0 = scanned or not requested).
                               This parameter is not valid for ED scans.

   @param result_list_size - the number of elements returned in the appropriate
                             result lists. This value is zero for the result of
                             an orphan scan.

   @param energy_detect - the array of energy measurements, one for each channel
                          searched during an ED scan. This parameter is null for
                          active, passive, and orphan scans.

   @param pan_descriptor - The list of PAN descriptors, one for each beacon
                           found during an active or passive scan if
                           macAutoRequest is set to TRUE. This parameter is null
                           for ED and orphan scans or when macAutoRequest is set
                           to FALSE during an active or passive scan.
 */


typedef struct zb_mac_scan_confirm_s
{
  zb_uint8_t  status;
  zb_uint8_t  scan_type;
  zb_uint32_t unscanned_channels;
  zb_uint8_t  result_list_size;

  union
  {
    zb_uint8_t           energy_detect[ ZB_MAC_SUPPORTED_CHANNELS ];
  } list;
} ZB_PACKED_STRUCT
zb_mac_scan_confirm_t;

/**
    Returns pointer to zb_mac_scan_confirm_t structure
    located somewhere in buf.

    @param buf    - pointer ot zb_buf_t container.
    @param outptr - out pointer to zb_mac_scan_confirm_t structure located
                    somewhere inside buf.
 */

#define ZB_MAC_GET_SCAN_CONFIRM_PTR( buf, outptr )  \
do                                                  \
{                                                   \
} while( 0 )

/**
   Defines MLME-BEACON-NOTIFY.indication.

   @param bsn - the beacon sequence number.
   @param pan_descriptor - the PANDescriptor for the received.
   @param pend_addr_spec - the beacon pending address specification.
   @param addr_list - the list of addresses of the devices for which the
            beacon source has data.

   @param sdu_length - the number of octets contained in the
                       beacon payload of the beacon frame received by the MAC
                       sublayer.

   @param sdu - the set of octets comprising the beacon payload to be transferred from the MAC
                sublayer entity to the next higher layer.
 */

typedef struct zb_mac_beacon_notify_indication_s
{
  zb_uint8_t                bsn;
  zb_pan_descriptor_t       pan_descriptor;
  zb_pending_address_spec_t pend_addr_spec;
  union zb_addr_u           addr_list[ MAX_PENDING_ADDRESSES ];
  zb_uint8_t                sdu_length;
  zb_uint8_t                sdu[ MAX_BCN_PAYLOAD ];
} ZB_PACKED_STRUCT
zb_mac_beacon_notify_indication_t;

/**
   Get mac current channel
 */
#define ZB_MAC_GET_CURRENT_LOGICAL_CHANNEL() MAC_CTX().current_channel


/*! @} */
/*! \endcond */

/*! \addtogroup mac_api */
/*! @{ */


/**
   MAC status
 */
typedef enum zb_mac_status_e
{
    // mac status
    MAC_SUCCESS                     = 0x0,      ///< Transaction was successful
    MAC_PAN_AT_CAPACITY             = 0x1,
    MAC_PAN_ACCESS_DENIED           = 0x2,
    MAC_BEACON_LOSS                 = 0xe0,     ///< Beacon was lost (used in beacon'd networks)
    MAC_CHANNEL_ACCESS_FAILURE      = 0xe1,     ///< Unable to transmit due to channel being busy
    MAC_COUNTER_ERROR               = 0xdB,     ///< Frame counter of received frame is invalid
    MAC_DENIED                      = 0xe2,     ///< GTS request denied
    MAC_DISABLE_TRX_FAILURE         = 0xe3,     ///< Failed to disable the transceiver
    MAC_SECURITY_ERROR              = 0xe4,     ///< Frame failed decryption
    MAC_FRAME_TOO_LONG              = 0xe5,     ///< Frame exceeded maximum size
    MAC_IMPROPER_KEY_TYPE           = 0xdc,     ///< Key not allowed to be used with this frame type
    MAC_IMPROPER_SECURITY_LEVEL     = 0xdd,     ///< Frame does not meet min security level expected
    MAC_INVALID_ADDRESS             = 0xf5,     ///< Data request failed because no src or dest address
    MAC_INVALID_GTS                 = 0xe6,     ///< Invalid timeslot requested (beacon'd networks)
    MAC_INVALID_HANDLE              = 0xe7,     ///< Invalid frame data handle
    MAC_INVALID_INDEX               = 0xf9,     ///< Invalid index when trying to write MAC PIB
    MAC_INVALID_PARAMETER           = 0xe8,     ///< Invalid parameter passed to service
    MAC_LIMIT_REACHED               = 0xfa,     ///< Scan terminated because max pan descriptors reached
    MAC_NO_ACK                      = 0xe9,     ///< ACK not received after tx with ack_req flag set
    MAC_NO_BEACON                   = 0xea,     ///< Beacon not returned after beacon request
    MAC_NO_DATA                     = 0xeb,     ///< Data frame not returned after data request (indirect poll)
    MAC_NO_SHORT_ADDRESS            = 0xec,     ///< No short address allocated to this device (due to lack of address space)
    MAC_ON_TIME_TOO_LONG            = 0xf6,     ///< Rx enable request failed. Spec'd number of symbols longer than beacon interval
    MAC_OUT_OF_CAP                  = 0xed,     ///< Association failed due to lack of capacity (no nbor tbl entry or no address)
    MAC_PAN_ID_CONFLICT             = 0xee,     ///< Different networks within listening range have identical PAN IDs
    MAC_PAST_TIME                   = 0xf7,     ///< Rx enable failed. Too late for current superframe and unable to be deferred
    MAC_READ_ONLY                   = 0xfb,     ///< PIB attribute is read only
    MAC_REALIGNMENT                 = 0xef,     ///< Coordinator realignment received
    MAC_SCAN_IN_PROGRESS            = 0xfc,     ///< Request to perform scan failed because scan already in progress
    MAC_SUPERFRAME_OVERLAP          = 0xfd,     ///< Start time of beacon overlapped transmission time of coordinator beacon
    MAC_TRACKING_OFF                = 0xf8,     ///< Device not tracking beacons but instructed to send beacons based on tracked beacons
    MAC_TRANSACTION_EXPIRED         = 0xf0,     ///< Frame buffered in indirect queue expired
    MAC_TRANSACTION_OVERFLOW        = 0xf1,     ///< Exceeded maximum amount of entries in indirect queue
    MAC_TX_ACTIVE                   = 0xf2,     ///< Transmission in progress
    MAC_UNAVAILABLE_KEY             = 0xf3,     ///< Security key unavailable
    MAC_UNSUPPORTED_ATTRIBUTE       = 0xf4,     ///< Requested PIB attribute is not supported
    MAC_UNSUPPORTED_LEGACY          = 0xde,     ///< 802.15.4 2003 security on frame, but not supported by device
    MAC_UNSUPPORTED_SECURITY        = 0xdf      ///< Security on received frame is not supported
} ZB_PACKED_STRUCT
zb_mac_status_t;


/**
   Get MAC PIB
 */
#define MAC_PIB() (ZG->mac.pib)


/**
   Mac PIB attributes
 */
typedef enum
{
  /* PHY PIB */
  ZB_PHY_PIB_CURRENT_CHANNEL                    = 0x00,
  ZB_PHY_PIB_CURRENT_PAGE                       = 0x04,
  /* MAC PIB */
  ZB_PIB_ATTRIBUTE_ACK_WAIT_DURATION            = 0x40,
  ZB_PIB_ATTRIBUTE_ASSOCIATION_PERMIT           = 0x41,
  ZB_PIB_ATTRIBUTE_AUTO_REQUEST                 = 0x42,
  ZB_PIB_ATTRIBUTE_BATT_LIFE_EXT                = 0x43,
  ZB_PIB_ATTRIBUTE_BATT_LIFE_EXT_PERIODS        = 0x44,
  ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD               = 0x45,
  ZB_PIB_ATTRIBUTE_BEACON_PAYLOAD_LENGTH        = 0x46,
  ZB_PIB_ATTRIBUTE_BEACON_ORDER                 = 0x47,
  ZB_PIB_ATTRIBUTE_BEACON_TX_TIME               = 0x48,
  ZB_PIB_ATTRIBUTE_BSN                          = 0x49,
  ZB_PIB_ATTRIBUTE_COORD_EXTEND_ADDRESS         = 0x4a,
  ZB_PIB_ATTRIBUTE_COORD_SHORT_ADDRESS          = 0x4b,
  ZB_PIB_ATTRIBUTE_DSN                          = 0x4c,
  ZB_PIB_ATTRIBUTE_GTS_PERMIT                   = 0x4d,
  ZB_PIB_ATTRIBUTE_MAX_CSMA_BACKOFFS            = 0x4e,
  ZB_PIB_ATTRIBUTE_MIN_BE                       = 0x4f,
  ZB_PIB_ATTRIBUTE_PANID                        = 0x50,
  ZB_PIB_ATTRIBUTE_PROMISCUOUS_MODE             = 0x51,
  ZB_PIB_ATTRIBUTE_RX_ON_WHEN_IDLE              = 0x52,
  ZB_PIB_ATTRIBUTE_SHORT_ADDRESS                = 0x53,
  ZB_PIB_ATTRIBUTE_SUPER_FRAME_ORDER            = 0x54,
  ZB_PIB_ATTRIBUTE_TRANSACTION_PERSISTENCE_TIME = 0x55,
  ZB_PIB_ATTRIBUTE_ASSOCIATED_PAN_COORD         = 0x56,
  ZB_PIB_ATTRIBUTE_MAX_BE                       = 0x57,
  ZB_PIB_ATTRIBUTE_MAX_FRAME_TOTAL_WAIT_TIME    = 0x58,
  ZB_PIB_ATTRIBUTE_MAX_FRAME_RETRIES            = 0x59,
  ZB_PIB_ATTRIBUTE_RESPONSE_WAIT_TIME           = 0x5a,
  ZB_PIB_ATTRIBUTE_SYNC_SYMBOL_OFFSET           = 0x5b,
  ZB_PIB_ATTRIBUTE_TIMESTAMP_SUPPORTED          = 0x5c,
  ZB_PIB_ATTRIBUTE_SECURITY_ENABLED             = 0x5d
} zb_mac_pib_attr_t;


typedef struct zb_mac_device_table_s
{
  zb_ieee_addr_t long_address;
  zb_uint16_t    short_address;
  zb_uint32_t    frame_counter;
  zb_uint16_t    pan_id;
}
zb_mac_device_table_t;

/**
   MAC PIB
 */
typedef struct
{
#if defined(ZB_CONFIGURABLE_MAC_PIB) || defined(ZB_NS_BUILD) || defined(ZB_CC25XX)
  zb_uint16_t             mac_ack_wait_duration;         /*!< The maximum number of symbols to wait for an
                                                           acknowledgment frame to arrive following a
                                                           transmitted data frame.
                                                           The commencement time is described in 7.5.6.4.2.*/
#endif
  zb_uint8_t              mac_association_permit;        /*!< Indication of whether a coordinator is currently
                                                           allowing association. A value of TRUE indicates
                                                           that association is permitted. */
  zb_uint8_t              mac_auto_request;              /*!< Indication of whether a device automatically
                                                           sends a data request command if its address
                                                           is listed in the beacon frame.
                                                           indication primitive (see 7.1.5.1.2). */
  zb_uint8_t              mac_batt_life_ext;             /*!< Indication of whether BLE, through the reduction of
                                                           coordinator receiver operation time during the CAP,
                                                           is enabled. Also, see 7.5.1.4 for an explanation. */
  zb_mac_beacon_payload_t mac_beacon_payload;            /*!< The contents of the beacon payload. */
  zb_uint8_t              mac_beacon_payload_length;     /*!< The length, in octets, of the beacon payload. */
  zb_uint8_t              mac_beacon_order;              /*!< Specification of how often the coordinator
                                                           transmits its beacon. */
  zb_uint8_t              mac_bsn;                       /*!< The sequence number added to the transmitted
                                                           beacon frame. */
  zb_ieee_addr_t          mac_coord_extended_address;    /*!< The 64-bit address of the coordinator
                                                           through which the device is associated. */
  zb_uint16_t             mac_coord_short_address;       /*!< The 16-bit short address assigned to the coordinator
                                                           through which the device is associated. */
  zb_uint8_t              mac_dsn;                       /*!< The sequence number added to the transmitted data
                                                           or MAC command frame. */
  zb_uint16_t             mac_pan_id;                    /*!< The 16-bit identifier of the PAN on which
                                                           the device is operating. If this value is 0xffff,
                                                           the device is not associated. */
  zb_uint8_t              mac_rx_on_when_idle;           /*!< Indication of whether the MAC sublayer is to enable
                                                           its receiver during idle periods. */
  zb_uint16_t             mac_short_address;             /*!< The 16-bit address that the device uses
                                                           to communicate in the PAN. */
  zb_uint16_t             mac_superframe_order;          /*!< The length of the active portion of the outgoing
                                                           superframe, including the beacon frame. */
#if defined(ZB_CONFIGURABLE_MAC_PIB) || defined(ZB_NS_BUILD) || defined(ZB_CC25XX)
  zb_uint8_t              mac_max_frame_retries;         /*!< The maximum number of retries allowed after a
                                                           transmission failure. */
#endif
#ifdef ZB_CONFIGURABLE_MAC_PIB
  zb_uint16_t             mac_transaction_persistence_time; /*!< The maximum time (in unit periods) that
                                                              a transaction is stored by a coordinator and
                                                              indicated in its beacon. */
  zb_uint8_t              mac_response_wait_time;        /*!< The maximum time, in multiples of aBaseSuperframeDuration,
                                                           a device shall wait for a response command frame to
                                                           be available following a request command frame. */
  zb_uint16_t             mac_max_frame_total_wait_time; /*!< The maximum number of CAP symbols in a beaconenabled
                                                           PAN, or symbols in a nonbeacon-enabled PAN, to wait
                                                           either for a frame intended as a response to a
                                                           data request frame or for a broadcast frame following a
                                                           beacon with the Frame Pending subfield set to one. */
/* next parameters are also unused */
  zb_uint16_t             mac_base_superframe_duration;  /*!< The number of symbols forming a superframe when
                                                           the superframe order is equal to 0. */
  zb_uint8_t              mac_max_csma_backoffs;         /*!< The maximum number of backoffs the CSMA-CA algorithm
                                                           will attempt before declaring a channel access failure. */
  zb_uint8_t              mac_batt_life_ext_periods;     /*!< In BLE mode, the number of backoff periods
                                                           during which the receiver is enabled after
                                                           the IFS following a beacon. */
  zb_uint8_t              mac_gts_permit;                /*!< TRUE if the PAN coordinator is to accept
                                                           GTS requests. FALSE otherwise. */
  zb_uint8_t              mac_promiscuous_mode;          /*!< Indication of whether the MAC sublayer is in a
                                                           promiscuous (receive all) mode. */
  zb_uint8_t              mac_min_be;                    /*!< *The minimum value of the backoff exponent (BE)
                                                           in the CSMA-CA algorithm. See 7.5.1.4
                                                           for a detailed explanation */
#endif
  zb_uint8_t              phy_current_page;

  zb_uint8_t              phy_current_channel;

  zb_ieee_addr_t          mac_extended_address;          /*!< The 64-bit (IEEE) address assigned to the device. */
#ifdef ZB_MAC_SECURITY
  zb_mac_device_table_t mac_device_table[MAC_DEVICE_TABLE_SIZE];
  zb_uint8_t            mac_device_table_entries;
  zb_uint32_t           mac_frame_counter;
  zb_uint8_t            mac_key[16];
#endif

} ZB_PACKED_STRUCT
 zb_mac_pib_t;


/* PIB access macros */

/**
   Get mac pan id
 */
#define ZB_PIB_SHORT_PAN_ID() ZG->mac.pib.mac_pan_id
/**
   Get mac short address
 */
#define ZB_PIB_SHORT_ADDRESS() ZG->mac.pib.mac_short_address
/**
   Get mac extended address
 */
#define ZB_PIB_EXTENDED_ADDRESS() ZG->mac.pib.mac_extended_address
/**
   Get mac coord short address
 */
#define ZB_PIB_COORD_SHORT_ADDRESS() ZG->mac.pib.mac_coord_short_address

/**
   Get mac rx on when idle
 */
#define ZB_PIB_RX_ON_WHEN_IDLE() ZG->mac.pib.mac_rx_on_when_idle

/**
   Get mac DSN
 */
#define ZB_MAC_DSN() ZG->mac.pib.mac_dsn
/**
   Get mac pan BSN
 */
#define ZB_MAC_BSN() ZG->mac.pib.mac_bsn
/**
   Increment mac pan DSN
 */
#define ZB_INC_MAC_DSN() (ZG->mac.pib.mac_dsn++)
/**
   Increment mac pan BSN
 */
#define ZB_INC_MAC_BSN() (ZG->mac.pib.mac_bsn++)
/**
   Get mac beacon payload
 */
#define ZB_PIB_BEACON_PAYLOAD() ZG->mac.pib.mac_beacon_payload

/**
   Defines MLME-GET.request primitive

   @param buf - pointer to zb_buf_t
   @param pib_attr - one of possible values from zb_mac_pib_attr_t
   @param outlen - out integer variable to receive length
*/

#define ZB_MLME_BUILD_GET_REQ( buf, pib_attr, outlen )  \
do                                                      \
{                                                       \
} while( 0 )

/**
   Defines MLME-GET.request primitive
*/
typedef struct zb_mlme_get_request_s
{
  zb_mac_pib_attr_t  pib_attr;
  zb_uint8_t          pib_index;
} ZB_PACKED_STRUCT
zb_mlme_get_request_t;

/**
   Defines MLME-GET.confirm primitive
*/
typedef struct zb_mlme_get_confirm_s
{
  zb_mac_status_t    status;
  zb_mac_pib_attr_t  pib_attr;
  zb_uint8_t         pib_index;
  zb_uint8_t         pib_length;
} ZB_PACKED_STRUCT
zb_mlme_get_confirm_t;

/**
   Defines MLME-SET.request primitive
*/
typedef struct zb_mlme_set_request_s
{
  zb_mac_pib_attr_t  pib_attr;
  zb_uint8_t         pib_index;
  zb_uint8_t         pib_length;
} ZB_PACKED_STRUCT
zb_mlme_set_request_t;

/**
   Defines MLME-SET.confirm primitive
*/
typedef struct zb_mlme_set_confirm_s
{
  zb_mac_status_t    status;
  zb_mac_pib_attr_t  pib_attr;
  zb_uint8_t         pib_index;
} ZB_PACKED_STRUCT
zb_mlme_set_confirm_t;

/**
   MLME-GET.request primitive
 */
void zb_mlme_get_request(zb_uint8_t param) ZB_CALLBACK;

/**
   MLME-GET.confirm primitive
 */
void zb_mlme_get_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   MLME-SET.request primitive
 */
void zb_mlme_set_request(zb_uint8_t param) ZB_CALLBACK;

/**
   MLME-SET.confirm primitive
 */
void zb_mlme_set_confirm(zb_uint8_t param) ZB_CALLBACK;


/*! @} */


/*! \cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */


/**
   Parameters for start request
 */
typedef struct zb_mlme_start_req_s
{
  zb_uint16_t pan_id;
  zb_uint8_t  logical_channel;
  zb_uint8_t  channel_page;
  zb_uint8_t  beacon_order;
  zb_uint8_t  superframe_order;
  zb_bitfield_t  pan_coordinator:1;
  zb_bitfield_t  battery_life_extension:1;
  zb_bitfield_t  coord_realignment:1;
  zb_bitfield_t  reserved:5;
} ZB_PACKED_STRUCT
zb_mlme_start_req_t;

/* mlme_start_confirm - status only - in the buffer header */

/**
   Parameters for realignment command, mac spec 7.3.8 Coordinator
*/
typedef struct zb_coord_realignment_cmd_s
{
  /* MHR fields - are not described here */
  zb_uint8_t cmd_frame_id;  /* Command Frame Identifier == 0x08*/
  zb_uint16_t pan_id;       /* PAN Identifier */
  zb_uint16_t coord_short_addr; /* Coordinator Short Address */
  zb_int8_t logical_channel;    /* Logical Channel */
  zb_uint16_t short_addr;   /* Short Address */
  zb_uint8_t  channel_page; /* Channel page, may be omitted */
} ZB_PACKED_STRUCT
zb_coord_realignment_cmd_t;

/**
   mac spec 7.2.1 General MAC frame format
   NOTE: actually MHR is of variable length and this structure can't
   be used for physical data i/o, it is used only for function calls
*/
typedef struct zb_mac_mhr_s
{
  zb_uint8_t frame_control[2];
  zb_uint8_t seq_number;
  zb_uint16_t dst_pan_id;
  union zb_addr_u dst_addr;
  zb_uint16_t src_pan_id;
  union zb_addr_u src_addr;

#ifdef ZB_MAC_SECURITY
  zb_uint8_t      security_level;
  zb_uint8_t      key_id_mode;
  zb_uint8_t      key_source[8];
  zb_uint8_t      key_index;
  zb_uint32_t     frame_counter;
#endif

}
zb_mac_mhr_t;

/**
   Parameters for sending beacon frame
 */
typedef struct zb_beacon_frame_params_s
{
  zb_uint8_t src_addr_mode;
  zb_uint8_t security_enabled;
  zb_uint8_t frame_pending;
  zb_uint8_t ble;
  zb_uint8_t beacon_order;
  zb_uint8_t superframe_order;
}
zb_beacon_frame_params_t;

/**
   Parameters for sending data request
*/
typedef enum zb_callback_type_e
{
  MAC_ASS_CONFIRM_CALLBACK = 0,
  MAC_POLL_REQUEST_CALLBACK = 1
}
zb_callback_type_t;

typedef struct zb_mlme_data_req_params_s
{
  zb_uint8_t src_addr_mode;
  zb_uint8_t dst_addr_mode;
  union zb_addr_u src_addr;
  union zb_addr_u dst_addr;
  zb_callback_type_t cb_type;
}
zb_mlme_data_req_params_t;


/**
   Parameters for storing data in a pending queue
*/
typedef struct zb_mac_pending_data_s
{
  zb_buf_t *pending_data;
  zb_addr_mode_t dst_addr_mode;
  union zb_addr_u dst_addr;
}
zb_mac_pending_data_t;


/**
   Parameters for orphan indication
*/
typedef struct zb_mac_orphan_ind_s
{
  zb_ieee_addr_t orphan_addr; /*<! The 64 bits address of the orphaned device */
}
zb_mac_orphan_ind_t;

/**
   Parameters for orphan response
*/
typedef struct zb_mac_orphan_response_s
{
  zb_ieee_addr_t orphan_addr; /*<! The 64 bits address of the orphaned device */
  zb_uint16_t short_addr; /*<! The 16-bit short address allocated to the
                           * orphaned device if it is associated with this coordinator */
  zb_bool_t associated; /*<! TRUE if the orphaned device is associated with this
                          coordinator or FALSE otherwise */
}
zb_mac_orphan_response_t;

/**
   Parameters for storing data request command context
*/
typedef struct zb_mac_data_req_ctx_s
{
  zb_mac_mhr_t mhr;     /* parsed mhr of the received command */
  zb_int8_t data_index; /* pending data index in the pending queue */
}
zb_mac_data_req_ctx_t;


/**
   pending transactions queue size is implementation-dependent but
   must be at least one
*/
#define ZB_MAC_PENDING_QUEUE_SIZE (ZB_IOBUF_POOL_SIZE / 4)

/**
   Set mac i/o in progress flag
 */
#ifdef C8051F120
#define ZB_MAC_START_IO()  ZB_SET_NSS_UP()
#else
#define ZB_MAC_START_IO()
#endif
/**
   Clear mac i/o in progress flag
 */
#ifdef C8051F120
#define ZB_MAC_STOP_IO()  { ZB_SET_NSS_DOWN(); ZB_8051_DELAY(); }
#else  /* 2410 */
#define ZB_MAC_STOP_IO()
#endif
/**
   Initialize MAC layer for the work. Must be
   called during initialization of whole system.
*/
void zb_mac_init();


/**
  MCPS-DATA.request. Accepts data request
  @param param - reference to buffer, contains upper layer data to send.
*/
void zb_mcps_data_request(zb_uint8_t param) ZB_CALLBACK;


/**
   MCPS-DATA.indication primitive
   This function called via scheduler by the MAC layer to pass
   incoming data packet to the NWK layer.
   Note that zb_mcps_data_indication() must be defined in the NWK layer!
   MAC layer just calls this function.

   @param param - reference to buffer, conatains nsdu, the set of
   octets comprising the NSDU to be transferred (with length)

   Other fields got from MAC nsdu by macros
 */
void zb_mcps_data_indication(zb_uint8_t param) ZB_CALLBACK;

/**
   MCPS-DATA.confirm primitive

   This function called via scheduler by the MAC layer to pass
   incoming data packet to the NWK layer.
   Note that zb_mcps_data_indication() must be defined in the NWK layer!
   MAC layer just calls this function.

   @param param - reference to buffer, contains nsdu - The set of octets
   comprising the NSDU to be transferred (with length)
   Other fields got from MAC nsdu by macros

 */
void zb_mcps_data_confirm(zb_uint8_t param) ZB_CALLBACK;

zb_ret_t pending_queue_is_empty();

/**
   Parameter for the zb_mlme_reset_request()
 */
typedef struct zb_mlme_reset_request_s
{
  zb_uint8_t           set_default_pib; /*!< If TRUE, the MAC sublayer is reset, and all MAC
                                          PIB attributes are set to their default values. If
                                          FALSE, the MAC sublayer is reset, but all MAC PIB
                                          attributes retain their values prior to the generation of
                                          the MLME-RESET.request primitive.  */
} ZB_PACKED_STRUCT
zb_mlme_reset_request_t;

/**
   Handles MLME-RESET.request

   @param param - parameter (packet buffer), @see zb_mlme_reset_request_t is on its tail
 */
void zb_mlme_reset_request (zb_uint8_t param) ZB_CALLBACK;

/**
   MLME-RESET.confirm

   This function called by MAC layer via callback.

   @param param - packet buffer. Only its header.status is used.
 */
void zb_mlme_reset_confirm (zb_uint8_t param) ZB_CALLBACK;

/**
   Handles scan request.
   @param - reference to buffer, contains zb_mlme_scan_params_t
   parameters for scan.
 */
void zb_mlme_scan_request (zb_uint8_t param) ZB_CALLBACK;

/**
   Handles start request.
   @param - reference to buffer, contains zb_mlme_start_req_t
   parameters for start.
 */
void zb_mlme_start_request (zb_uint8_t param) ZB_CALLBACK;

/**
   Confirms scan procedure.
   @param - reference to buffer.
 */
void zb_mlme_scan_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   Confirms start procedure.
   @param - reference to buffer.
 */
void zb_mlme_start_confirm (zb_uint8_t param) ZB_CALLBACK;

/**
   Performs ED scan.
   @return Returns RET_OK on success and RET_ERROR on error.
 */
zb_ret_t zb_mlme_ed_scan();


/**
   Performs active scan.
   @return Returns RET_OK on success and RET_ERROR on error.
 */
zb_ret_t zb_mlme_active_scan() ZB_SDCC_REENTRANT;


/**
   Performs orphan scan.
   @return Returns RET_OK on success and RET_ERROR on error.
 */
zb_ret_t zb_mlme_orphan_scan() ZB_SDCC_REENTRANT;

/**
   performs processing of the MLME-Start.request.
   @return RET_OK, RET_BLOCKED, error code on error
 */
zb_ret_t zb_mac_process_mlme_start() ZB_SDCC_BANKED;

/**
  Fill packed mac header

  @param ptr - pointer to out data
  @param mhr - structure with mhr data
*/
void zb_mac_fill_mhr(zb_uint8_t *ptr, zb_mac_mhr_t *mhr);


/**
  Handle MLME-ASSOCIATION.request
  @param - reference to buffer, contains zb_mlme_associate_params_t
  parameters.
 */
void zb_mlme_associate_request (zb_uint8_t param) ZB_CALLBACK;

/**
  associate responce - coordinator side. send response to device.
  @param - reference to buffer
 */
void zb_mlme_associate_response (zb_uint8_t param) ZB_CALLBACK;


/**
  MLME-BEACON-NOTIFY.indication primitive
  This function called via scheduler by the MAC layer to pass
  incoming beacon packet to the NWK layer.
  Note that zb_mlme_beacon_notify_indication() must be defined in the NWK layer!
  MAC layer just calls this function.

  @param param - reference to buffer, contains packet (MAC header + beacon payload + trailes with RSSI
 */
void zb_mlme_beacon_notify_indication(zb_uint8_t param) ZB_CALLBACK;

/**
  Confirms scan request, MLME-ASSOCIATE.confirm
  @param param - reference to buffer
 */
void zb_mlme_scan_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
  Function parses incoming MAC command and executes it
  @param param - reference to buffer
*/
void zb_mlme_command_accept(zb_uint8_t param) ZB_CALLBACK;

/**
  Function checks received ack.
  @param param - reference to buffer
*/
void zb_mlme_ack_accept(zb_uint8_t param) ZB_CALLBACK;

/**
  Function is called on acknowledge timeout
  @param param is unused
 */
void zb_mac_ack_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
  Function is called on response timeout
  @param param is unused
 */
void zb_mac_resp_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
  Function is called on scan timeout
  @param param is unused
 */
void zb_mac_scan_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
  Function is called on transaction (pending data) timeout
  @param param is pending data index
 */
void zb_mac_pending_data_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
  Function is called on getting indirect data timeout
  @param param is unused
 */
void zb_mac_indirect_data_timeout(zb_uint8_t param) ZB_CALLBACK;

/**
  Accept data request function
  @param param - reference to buffer.
 */
void zb_accept_data_request_cmd(zb_uint8_t param) ZB_CALLBACK;


/**
   Coordinator side: get request command, say ACK to end device,
   signal to high level with associate.indication
   @param param - reference to buffer.
*/
void zb_accept_ass_request_cmd(zb_uint8_t param) ZB_CALLBACK;

/**
   Calculates length of mac header (MHR) inside MAC frame

   @param src_addr_mode   - source addressing mode one of values of zb_addr_mode_e enum
   @param dst_addr_mode   - destination addressing mode one of values of zb_addr_mode_e enum
   @param security_enabled - if set to 1, Auxiliary Security Header is present
   @return mhr length
*/
zb_uint8_t zb_mac_calculate_mhr_length(zb_uint8_t src_addr_mode, zb_uint8_t dst_addr_mode, zb_uint8_t panid_compression);

/**
   Handles scan request.
   @return RET_OK, RET_BLOCKED, error code on error
 */
void  zb_handle_scan_request(zb_uint8_t param) ZB_CALLBACK;

/**
  Sends beacon request command, mac spec 7.3.7 Beacon request command
  @return RET_OK, RET_ERROR
*/
zb_ret_t zb_beacon_request_command();

/**
  Sends orphan notification command, mac spec 7.3.6
  @return RET_OK, RET_ERROR
*/
zb_ret_t zb_orphan_notification_command();

/**
  Parses packed mhr header, fills mhr structure
  @param1 mhr - out pointer to mhr structure
  @param2 ptr - pointer to packed mhr header buffer
  @return packed mhr buffer length
*/
zb_uint8_t zb_parse_mhr(zb_mac_mhr_t *mhr, zb_uint8_t *ptr);

/**
  sends association request command
  @return RET_OK, RET_ERROR
*/
void zb_mlme_send_association_req_cmd(zb_uint8_t param) ZB_CALLBACK;
void zb_mlme_send_association_req_continue(zb_uint8_t param) ZB_CALLBACK;
/**
  handle mac association response: forms association response cmd
  and puts it to pending queue, coordinator side
  @return RET_OK, RET_ERROR
*/
zb_ret_t zb_handle_associate_response();




/**
 Updates globals
*/
void zb_mac_update_superframe_and_pib();

/**
   Sends data request command
   @param params - parameters for sending data request,
   zb_mlme_data_req_params_t type
   @return RET_OK, error code on error
*/
zb_ret_t zb_mlme_send_data_req_cmd(zb_mlme_data_req_params_t *params) ZB_SDCC_REENTRANT;

/**
   Check command transmit status.
   @return RET_OK, RET_BLOCKED, error code on error
 */
zb_ret_t zb_check_cmd_tx_status();

#ifdef ZB_MANUAL_ACK
/**
  sends acknowledgement
  @param ack_dsn - frame sequence number to acknowledge
  @param data_pending - signal if there is more pending data
  @return RET_OK, RET_ERROR
*/
zb_ret_t zb_mac_send_ack(zb_uint8_t ack_dsn, zb_uint8_t data_pending);
#endif

/**
  gets pending data from a coordinator (get data indirectly)
  @param data_req_cmd_params - parameters for data request command
  @return RET_OK, RET_ERROR, RET_BLOCKED, RET_PENDING
  RET_PENDING is returned if there is more data available on
  coordinator
*/

zb_ret_t zb_mac_get_indirect_data(zb_mlme_data_req_params_t *data_req_cmd_params) ZB_CALLBACK;

/**
  Puts data to pending queue. It is used for indirect
  transmission. Coordinator side
  @return RET_PENDING on success, RET_ERROR on error
 */
zb_ret_t zb_mac_put_data_to_pending_queue(zb_mac_pending_data_t *pend_data) ZB_SDCC_BANKED;

/**
  Handles data request command, coordinator side. Finds pending data
  for the device if any and sends it. spec 7.3.4 Data request
  command, 7.5.6.3 Extracting pending data from a coordinator.
  @return RET_OK, RET_ERROR, RET_BLOCKED
*/
zb_ret_t zb_handle_data_request_cmd() ZB_SDCC_REENTRANT;

/**
   Handle mac data request, caller side
   @return RET_OK, RET_ERROR, RET_BLOCKED
 */
void zb_handle_mcps_data_req(zb_uint8_t param) ZB_CALLBACK;

/**
   Handles data frame. Now it's a callback.

 */
void zb_handle_data_frame(zb_uint8_t param) ZB_CALLBACK;

/**
   Handles beacon request
   @return RET_OK, RET_ERROR
 */
void zb_handle_beacon_req(zb_uint8_t param) ZB_CALLBACK;

/*
  Sends coordinator realignment command
  @param is_broadcast - if TRUE, cmd is broadcast over PAN, if FALSE
  cmd is directed to orphaned device
  @param orphaned_dev_ext_addr - orphaned device extended addres
  @param orphaned_dev_short_addr - orphaned device short address,
  maybe set to 0xFFFE

  @return RET_OK if ok, error code on error
*/
zb_ret_t zb_tx_coord_realignment_cmd(zb_bool_t is_broadcast, zb_ieee_addr_t orphaned_dev_ext_addr,
                                     zb_uint16_t orphaned_dev_short_addr);

/**
   Sends orphan request command
   @param params - parameters for sending data request,
   zb_mac_orphan_response_t
   @return noting
*/
void zb_mlme_orphan_response(zb_uint8_t param);

/*
  Sends MLME-COMM-STATUS.indication to higher level.
  @param pending_buf - parameters for indication are taken from  panding_buf.mhr
  @param mac_status - indication status
  @param buffer - is used to store parameters for indication in it
  @return - returns RET_OK
*/
zb_ret_t zb_mac_send_comm_status(zb_buf_t *pending_buf, zb_uint8_t mac_status, zb_buf_t *buffer) ZB_SDCC_REENTRANT;



/**
   Parameters for poll request
*/
typedef struct zb_mlme_poll_request_s
{
  zb_addr_mode_t  coord_addr_mode;
  union zb_addr_u coord_addr;
  zb_uint16_t     coord_pan_id;
}
zb_mlme_poll_request_t;

void zb_poll_request(zb_uint8_t param) ZB_CALLBACK;


/**
  Handle mac poll request
  @return RET_OK, RET_BLOCKED, error code on error
*/
void zb_handle_poll_request(zb_uint8_t param) ZB_CALLBACK;

/**
   Handles MLME-poll.confirm

   @param param - parameter (packet buffer), with poll status
 */
void zb_mlme_poll_confirm(zb_uint8_t param) ZB_CALLBACK;

/**
   Sync loss reasons

   The reason that synchronization was lost.
 */
typedef enum zb_sync_loss_reason_e
{
  ZB_SYNC_LOSS_REASON_PAN_ID_CONFLICT,
  ZB_SYNC_LOSS_REASON_REALIGNMENT,
  ZB_SYNC_LOSS_REASON_BEACON_LOST
}
zb_sync_loss_reason_t;

/**
   Parameters for sync loss indication
*/
typedef struct zb_mlme_sync_loss_ind_s
{
  zb_sync_loss_reason_t loss_reason; /* Lost syncronisation reason */
  zb_uint16_t pan_id; /* The PAN identifier with which the device lost
                       * synchronization or to which it was realigned */
  zb_uint8_t logical_channel; /* Logical channel */
  zb_uint8_t channel_page; /* Channel page */
}
zb_mlme_sync_loss_ind_t;

/**
   Handles MLME-sync-loss.indication

   @param param - parameter (packet buffer), @see zb_mlme_sync_loss_ind_t is on its tail
 */
void zb_mlme_sync_loss_indication(zb_uint8_t param) ZB_CALLBACK;

zb_ret_t zb_mac_setup_for_associate(zb_uint8_t logical_channel, zb_uint16_t pan_id,
                                    zb_uint16_t short_addr, zb_ieee_addr_t long_addr);


#ifdef MAC_CERT_TEST_HACKS

/**
   Start work as disturber device: flood ratio by bad packets.

   @param logical_channel - channel to use
 */
void zb_mac_disturber_loop(zb_uint8_t logical_channel);


/**
   MLME-purge.request parameter
 */
typedef struct zb_mlme_purge_request_s
{
  zb_uint8_t msdu_handle;
}
zb_mlme_purge_request_t;

typedef zb_mlme_purge_request_t zb_mlme_purge_confirm_t;


/**
   Handles MLME-purge.request

   @param param - parameter (packet buffer), @see zb_mlme_purge_request_t is on its tail
 */
void zb_mlme_purge_request(zb_uint8_t param) ZB_CALLBACK;

/**
   Handles MLME-purge.confirm

   @param param - parameter (packet buffer), with status and zb_mlme_purge_confirm_t
 */
void zb_mlme_purge_confirm(zb_uint8_t param) ZB_CALLBACK;


#endif


#ifdef ZB_LIMIT_VISIBILITY
void mac_add_visible_device(zb_ieee_addr_t long_addr);
void mac_add_invisible_short(zb_uint16_t addr);
#define MAC_ADD_VISIBLE_LONG(long_addr) mac_add_visible_device(long_addr)
#define MAC_ADD_INVISIBLE_SHORT(addr) mac_add_invisible_short(addr)
//zb_bool_t mac_is_frame_visible(zb_mac_mhr_t *mhr);
#else
#define MAC_ADD_VISIBLE_LONG(long_addr)
#define MAC_ADD_INVISIBLE_SHORT(addr)
#endif


/* waiting for tx, this routine wasn't planned, but seems  */
/* like it's needed for zb_realign_pan, coz it should not */
/* be interrupted */
#if !defined(ZB_TRANSPORT_8051_DATA_SPI) && !defined(ZB_TRANSPORT_LINUX_SPIDEV)
#define ZB_WAIT_FOR_TX() while (!MAC_CTX().tx_cnt) ZB_GO_IDLE()
#else
#define ZB_WAIT_FOR_TX()                  \
  while(!MAC_CTX().tx_cnt)          \
  {                                 \
          CHECK_INT_N_TIMER();            \
    if (ZB_GET_TRANS_INT())         \
    {                               \
      zb_ubec_check_int_status();   \
    }                               \
  }
#endif
/* the following macro used to skip tx check */
/* if fully synchronous output needed - just zero it */
#define ZB_SKIP_TX_CHK() (MAC_CTX().tx_cnt = 1);  /* disable waiting for the tx */



void zb_mac_main_loop();

/*! @} */
/*! \endcond */
/* receiving packet, suggested to be scheduled in check_int_status*/
/* switching to the next channel*/
void zb_mlme_scan_step(zb_uint8_t param) ZB_CALLBACK;

void zb_mac_recv_data(zb_uint8_t param) ZB_CALLBACK;

void zb_mac_parse_recv_data(zb_uint8_t param) ZB_CALLBACK;

void zb_handle_data_request_cmd_continue(zb_uint8_t param) ZB_CALLBACK;

void zb_mlme_handle_orphan_response_continue(zb_uint8_t param) ZB_CALLBACK;

void zb_mlme_handle_orphan_response(zb_uint8_t param) ZB_CALLBACK;

void zb_handle_mcps_data_req_continue(zb_uint8_t param) ZB_CALLBACK;

/* used, when we need no callback called after tx finished */
void zb_dummy_cb(zb_uint8_t param) ZB_CALLBACK;

/* supporting MLME_SET.request for 802.15.4 */
#ifndef ZB_CONFIGURABLE_MAC_PIB
#define ZB_MAC_PIB_RESPONSE_WAIT_TIME ZB_MAC_RESPONSE_WAIT_TIME * ZB_DEBUG_ENLARGE_TIMEOUT
#define ZB_MAC_PIB_ACK_WAIT_DURATION (ZB_UINT_BACKOFF_PERIOD + ZB_TURNAROUND_TIME + ZB_PHY_SHR_DURATION + (zb_uint16_t)(6 * ZB_PHY_SYMBOLS_PER_OCTET))
#define ZB_MAC_PIB_TRANSACTION_PERSISTENCE_TIME ZB_MAC_TRANSACTION_PERSISTENCE_TIME
#define ZB_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME ZB_MAX_FRAME_TOTAL_WAIT_TIME
#define ZB_MAC_PIB_MAX_FRAME_RETRIES ZB_MAC_MAX_FRAME_RETRIES
#else
#define ZB_MAC_PIB_RESPONSE_WAIT_TIME MAC_PIB().mac_response_wait_time
#define ZB_MAC_PIB_ACK_WAIT_DURATION MAC_PIB().mac_ack_wait_duration
#define ZB_MAC_PIB_TRANSACTION_PERSISTENCE_TIME MAC_PIB().mac_transaction_persistence_time
#define ZB_MAC_PIB_MAX_FRAME_TOTAL_WAIT_TIME MAC_PIB().mac_max_frame_total_wait_time
#define ZB_MAC_PIB_MAX_FRAME_RETRIES MAC_PIB().mac_max_frame_retries

#endif

#endif
