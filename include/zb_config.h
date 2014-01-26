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
PURPOSE: Configuration file: configuration- and platform-specific definitions,
constants etc.
*/

#ifndef ZB_CONFIG_H
#define ZB_CONFIG_H 1


/*! \addtogroup ZB_CONFIG */
/*! @{ */

/**
   Define to let us work properly with Ember stack
*/

/*#define ZB_EMBER_GOLDEN_UNIT */

/**
   If defined, NVRAM not compiled

   To be used near always to prevent flash damage (flash can do ~1000 rewrites only)
 */
#define NO_NVRAM

#ifdef ZB_PLATFORM_LINUX_PC32
/**
Some additional run-time checks.
*/
#ifdef DEBUG
 /**
  Check arrays to be verified by valgring. Useful for Linux/PC build only. Slows down execution.
 */
 //#define ZB_ARRAYS_CHECK
 #endif




#define ZB_INIT_HAS_ARGS

#ifdef DEBUG
/**
  Check arrays to be verified by valgring
*/
//#define ZB_ARRAYS_CHECK
#endif

/**
   If defined, security is compiled
 */
#define ZB_SECURITY

/**
   If defined, switch on traffic dump
 */
#define ZB_TRAFFIC_DUMP_ON

#ifndef UNIX
#define UNIX
#endif
#ifndef LINUX
#define LINUX
#endif

/**
   In Linux work size 4 bytes, at 8051 1 byte
 */
#define ZB_WORD_SIZE_4
/**
   If defined, we run on little-endian  machine
 */
#define ZB_LITTLE_ENDIAN

/* Hardcode transport now. In the future there will be real transiver in Linux,
 * its connection can differ. */
/**
   If defined, transport is named pipes in Linux
 */
#define ZB_TRANSPORT_LINUX_PIPES
/**
   Linux named pipes transport timeout: wait in select() for this number of seconds
 */
#define ZB_LINUX_PIPE_TRANSPORT_TIMEOUT 1
/**
   If defined, this is special build to work with ns-3 network simulator.
 */
#define ZB_NS_BUILD
/**
   If defined (for NS build),  ack is sent and checked manually
*/
#define ZB_MANUAL_ACK

#elif defined ZB_PLATFORM_LINUX_ARM_2400
/**
 If defined, need some tricks for the work alignment (useful at ARM devices)
*/
#define ZB_NEED_ALIGN
/**
 Use UZ2400 transiever
*/
#define ZB_UZ2400

/**
 Timer functuionality is inside idle function  - Linux/ARM only
*/

#define ZB_TIMER_INSIDE_IDLE

/**
   If defined, security swithced on
 */
#define ZB_SECURITY

/**
   If defined, switch on traffic dump
 */
#define ZB_TRAFFIC_DUMP_ON

/**
 If defined, transport calls traffic dump by itself, generic MAC not need to do it
*/
#define ZB_TRANSPORT_OWN_TRAFFIC_DUMP_ON

#ifndef UNIX
/**
 If defined, compiled under some UNIX
*/
#define UNIX
#endif
#ifndef LINUX
/**
 If defined, compiled under Linux
*/
#define LINUX
#endif

/**
   In Linux work size 4 bytes, at 8051 1 byte
 */
#define ZB_WORD_SIZE_4
/**
   If defined, we run on little-endian  machine
 */
#define ZB_LITTLE_ENDIAN

/**
   If defined, transport is spidev and zbintr drivers - Linux/ARM
 */
#define ZB_TRANSPORT_LINUX_SPIDEV

/**
   Linux named pipes transport timeout: wait in select() for this number of seconds
 */
#define ZB_LINUX_SPIDEV_INTR_TIMEOUT 1

/**
   If defined, security swithced on
 */
#define ZB_SECURITY

/**
   Our Linux/ARM devices has bas antennas - try to send 3 beacons
 */
//#define ZB_MULTIPLE_BEACONS 3

#elif defined ZB_PLATFORM_8051_SIM  /* ZB_PLATFORM_LINUX_PC32 */

#ifdef ZB_IAR
#define ZB_LITTLE_ENDIAN
#pragma diag_suppress=Pa050
#else
/**
   If defined, host is big endian (8051)
 */
#define ZB_BIG_ENDIAN
#endif

#ifndef ZB8051
/**
If defined, platform is some 8051 clone
*/
#define ZB8051
#endif

/**
   Need to play with code banks
 */
#ifdef SDCC
/* s51 does not support >64k ROM, so don't use banks */
//#define ZB_BANKED_BUILD
#endif

#if !defined(SDCC) && !defined(KEIL) && !defined(ZB_IAR)
#error 8051 platform but compiler is not SDCC and KEIL!
#endif

/* Transport types:
   - ZB_TRANSPORT_8051_DATA_UART, if specified zigbee data is sent over uart transport
   - ZB_TRANSPORT_8051_DATA_SPI, if specified zigbee data is sent over spi transport
   - ZB_TRANSPORT_8051_UART is used for zigbee data, trace or dump
   - ZB_TRANSPORT_8051 is used to choose general transport type for MAC, trace and dump
*/
/* TODO: choose data uart/spi transport type based on some logic */
#ifdef ZB_NS_BUILD
/**
   If defined (ns build, 8051), data transport is UART
 */
#define ZB_TRANSPORT_8051_DATA_UART
#define ZB_MANUAL_ACK
#define ZB_INIT_HAS_ARGS


#elif !defined ZB_UZ2410 && !defined ZB_CC25XX
/* non-ns, uz2400 */

/**
   If defined (for UZ2400), data transport is SPI
 */
#define ZB_TRANSPORT_8051_DATA_SPI
#define ZB_UZ2400
#endif

#if defined C8051F120 && defined ZB_UZ2410
#error Only one MCU can be chosen!
#endif

#if defined ZB_SNIFFER && defined ZB_SNIFFER_SERIAL_TRACE
#define ZB_TRANSPORT_8051_UART
#endif


#if defined(ZB_TRANSPORT_8051_DATA_UART) || defined(ZB_TRACE_LEVEL) || defined(ZB_TRAFFIC_DUMP_ON)
/**
   If defined, use UART for either data or trace
 */
#define ZB_TRANSPORT_8051_UART
#endif

/**
   If defined, implemented some transport at 8051
 */
#define ZB_TRANSPORT_8051 /* MAC transport type */

#if defined(ZB_TRANSPORT_8051_DATA_UART) && defined(ZB_TRANSPORT_8051_DATA_SPI)
#error Only one transport for ZB data could be specified!
#endif

/**
   If defined, use hw 8051 timer
 */
#define ZB_8051_TIMER

#endif

/**
   Port to be used for zb-over-udp when converting traffic dump into .pcap for WireShark

   This is for real transiver case - that is, dump contains all transiver registers access.
*/
#define ZB_UDP_PORT_REAL 9998
/**
   Port to be used for zb-over-udp when converting traffic dump into .pcap for WireShark

   This is for ns-3 build case - that is, dump contains MAC packets.
*/
#define ZB_UDP_PORT_NS 9999

//#define ZB_TRAFFIC_DUMP_ON


#ifndef ZB_ED_ROLE
/**
   If defined, ZC functionality is compiled
   Implies ZR role as well
 */
#define ZB_COORDINATOR_ROLE
#endif
#if defined ZB_COORDINATOR_ROLE && !defined ZB_ROUTER_ROLE
/* coordinator supposes router */
/**
   If defined, ZR role compiled.
 */
#define ZB_ROUTER_ROLE
#endif


/**
   Stack profile constant
   1 means 2007, 2 means PRO, 0 means network select
*/
#ifdef ZB_EMBER_GOLDEN_UNIT
  #define ZB_STACK_PROFILE 0
#else
  #define ZB_STACK_PROFILE 1
#endif
/**
   If defined, 2007 stack profile is implemented
 */
#define ZB_STACK_PROFILE_2007

/**
   Protocol version: table 1.1 - current (2006 compatible)
*/
#define ZB_PROTOCOL_VERSION 2

/**
   Scheduler callbacks queue size. Ususlly not need to change it.
 */
#define ZB_SCHEDULER_Q_SIZE 16
#define ZB_MAC_QUEUE_SIZE 4

/**
   Size of queue for wait for free packet buffer
 */
#define ZB_BUF_Q_SIZE 16

/**
   Size, in bytes, of the packet buffer

   Be sure keep it multiple of 4 to exclude alignment problems at ARM
 */
#define ZB_IO_BUF_SIZE 148

/**
   Number of packet buffers. More buffers - more memory. Less buffers - risk to be blocked due to buffer absence.
 */
#define ZB_IOBUF_POOL_SIZE 16

/* MAC */

/**
   MAC transaction queue size
 */
#define ZB_MAC_MAX_REQUESTS 10

/* ZB_DEBUG_ENLARGE_TIMEOUT is used as multiplier for debug build */
#ifdef ZB_NS_BUILD
//#define ZB_DEBUG_ENLARGE_TIMEOUT 3
#define ZB_DEBUG_ENLARGE_TIMEOUT 1
#else
#define ZB_DEBUG_ENLARGE_TIMEOUT 1
#endif  /* ZB_NS_BUILD */


/**
   MAC: max time to wait for a response command frame, range 2-64
   Default is 32, 64 set for better compatibility
*/
#define ZB_MAC_RESPONSE_WAIT_TIME 64

/* Make all MAC PIB attributes configurable */
//#define ZB_CONFIGURABLE_MAC_PIB
/**
 MAC: max time to wait for indirect data
*/
#define ZB_MAX_FRAME_TOTAL_WAIT_TIME 800

/**
   MAC: The maximum number of retries allowed after a transmission failure 0-7
*/
#define ZB_MAC_MAX_FRAME_RETRIES 3

#ifdef ZB_ED_ROLE

/**
 End device idle time-out
*/
#define ZB_TIME_ED_IDLE ZB_MILLISECONDS_TO_BEACON_INTERVAL(7500)
#endif


/* APS */

/**
   APS: dup check timeout.

   APS dup checks resolution is 1s, timer entry size in the address
   translation table is 2b, so dup timeout is 4s.
*/
/* MERGED: check me! */
#define ZB_APS_DUP_CHECK_TIMEOUT ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000)

/**
   After send APS packet, if waiting for ACK, call POLL after this timeout.
 */
#ifdef ZB_TRACE_LEVEL
#define ZB_APS_POLL_AFTER_REQ_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(700)
#else
#define ZB_APS_POLL_AFTER_REQ_TMO ZB_MILLISECONDS_TO_BEACON_INTERVAL(200)
#endif

/* APS binding */

/**
  APS: SRC binding tble size
 */
#define ZB_APS_SRC_BINDING_TABLE_SIZE 32

/**
   APS: DST binding tble size
 */
#define ZB_APS_DST_BINDING_TABLE_SIZE 32

/**
 APS: man number of groups in the system
*/
#define ZB_APS_GROUP_TABLE_SIZE       16

/**
 APS: max number of endpoints per group table entry
*/
#define ZB_APS_ENDPOINTS_IN_GROUP_TABLE 8

/**
 APS: size of queue to be used to pass
      incoming group addresses packets up
*/
#define ZB_APS_GROUP_UP_Q_SIZE 8

/**
 APS: size of the APS queue of buffers waiting for sending ACK from our side
*/
#define ZB_APS_RETRANS_ACK_Q_SIZE 4

/**
   APS retransmissions
 */

/**
 APS: max number of packets waiting for APS ACK
*/
#define ZB_N_APS_RETRANS_ENTRIES   10

/**
 APS maximum of apscMaxFrameRetries times
*/
#define ZB_N_APS_MAX_FRAME_ENTRIES 3


/**
 APS: APS ACK wait time. After this timeout resend APS packet
*/
#define ZB_N_APS_ACK_WAIT_DURATION 2*ZB_ZDO_INDIRECT_POLL_TIMER

/* NWK */
/**
 NWK: size of the long-short address translation table
*/
#define ZB_IEEE_ADDR_TABLE_SIZE 101

#ifdef ZB_ED_ROLE
/* Only parent is meangful for ED */
/**
 NWK: size of the neighbor table
*/
  #define ZB_NEIGHBOR_TABLE_SIZE 1

/**
 NWK: size of the neighbor table used during discovery phase of join
*/
  #define ZB_EXT_NEIGHBOR_TABLE_SIZE 8
#else
/**
 NWK: size of the neighbor table
*/
  #define ZB_NEIGHBOR_TABLE_SIZE 32
#endif

/**
 NWK: size os the long-short panid translation table
*/
#define ZB_PANID_TABLE_SIZE 8

/**
 NWK: If defined, use distributed address assing for tree and for mesh routing (ZigBee 2007).
 */
#define ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN
#ifndef ZB_NWK_DISTRIBUTED_ADDRESS_ASSIGN

/**
 NWK: If defined, use stochastic address assing (ZigBee PRO).
 */
  #define ZB_NWK_STOCHASTIC_ADDRESS_ASSING
#endif

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE

/**
 NWK: If defined, enable routing functionality
*/
#define ZB_NWK_ROUTING

/**
 Number of secure materials to store
*/
#define N_SECUR_MATERIAL 3

#else

#define N_SECUR_MATERIAL 1

#endif  /* coordinator or router */


/* None-Pro stack MUST support tree routing. Use Both routing algorithms */
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE

/**
 NWK: if defined, implement tree routing
*/
#define ZB_NWK_TREE_ROUTING
#define ZB_NWK_MESH_ROUTING
/**
 NWK: if defined, implement mesh routing
*/
#endif

/**
 NWK: Max number of children per node
*/
#define ZB_NWK_MAX_CHILDREN 4

/**
 NWK: Max number of routers per node
*/
#define ZB_NWK_MAX_ROUTERS  4

/**
 NWK: max network depth
*/
#define ZB_NWK_MAX_DEPTH    5

/**
   NWK Mesh route stuff: routing table size
*/
#define ZB_NWK_ROUTING_TABLE_SIZE 5

/**
   NWK Mesh route stuff: route discovery table size
*/
#define ZB_NWK_ROUTE_DISCOVERY_TABLE_SIZE 5

/* FIXME: just for test purpose to generate less logs, correct value is 1
 * second */
#define ZB_NWK_EXPIRY_ROUTE_DISCOVERY 2*ZB_TIME_ONE_SECOND
/* nwkcRouteDiscoveryTime == 0x2710 ms == 10 sec. Expiry function called once
 * per second */
#define ZB_NWK_ROUTE_DISCOVERY_EXPIRY 10
#if 0
#define ZB_NWK_RREQ_TABLE_SIZE 5
#endif
#define ZB_MWK_INITIAL_RREQ_RETRIES 3
#define ZB_MWK_RREQ_RETRIES 2
#define ZB_NWK_PENDING_TABLE_SIZE 5
#define ZB_NWK_PENDING_ENTRY_EXPIRY 20
#define ZB_NWK_STATIC_PATH_COST 7

#define ZB_NWK_BTR_TABLE_SIZE 16
#define ZB_NWK_BRR_TABLE_SIZE 8

#define ZB_NWK_WAIT_ALLOC_TABLE_SIZE 5

#define ZB_NWK_MAX_BROADCAST_JITTER_INTERVAL ZB_MILLISECONDS_TO_BEACON_INTERVAL(0x40)
#define ZB_NWK_RREQ_RETRY_INTERVAL ZB_MILLISECONDS_TO_BEACON_INTERVAL(0xFE)
#define ZB_NWK_EXPIRY_PENDING 5*ZB_TIME_ONE_SECOND


/* The maximum broadcast jitter time measured in milliseconds. */
#define ZB_NWK_MAX_BROADCAST_JITTER 0x40*ZB_TIME_ONE_SECOND

/* The maximum number of retries allowed after a broadcast transmission failure. */
#define ZB_NWK_MAX_BROADCAST_RETRIES 0x02

/* The maximum time duration in milliseconds allowed for the parent and all
 * child devices to retransmit a broadcast message */
/*0x2710 is too long, ZB_NWK_BROADCAST_DELIVERY_TIME becoms huge */
#define ZB_NWK_PASSIVE_ACK_TIMEOUT 100

/**
 Maximum number of rejoin requests in progress
*/
#define ZB_NWK_REJOIN_REQUEST_TABLE_SIZE 3

#define ZB_NWK_REJOIN_TIMEOUT ZB_MAC_PIB_RESPONSE_WAIT_TIME * 5
/* Some defaults for ZDO startup */

#ifndef ZB_TRACE_LEVEL
/**
 NWK: default energy/active scan duration
*/
#define ZB_DEFAULT_SCAN_DURATION 3
#else
#define ZB_DEFAULT_SCAN_DURATION 8
#endif

/* #define ZB_DEFAULT_SCAN_DURATION 4 */
/* #define ZB_TRANSCEIVER_ALL_CHANNELS_MASK   (0xffff << 11) */ /* 0000.0111 1111.1111 1111.1000 0000.0000*/
/* C51 dosn't like long shifts, it just cut last two bytes. */
#define ZB_TRANSCEIVER_ALL_CHANNELS_MASK   0x07FFF800 /* 0000.0111 1111.1111 1111.1000 0000.0000*/

#ifndef ZB_NS_BUILD

#if defined ZB_UZ2410 || defined ZB_CC25XX
#define ZB_USE_RX_QUEUE
/*this is currently unused and broken feature, we need to dump saver registers from queue, when entering */
/* traffic dump function */
/* #define ZB_RESERVED_REGS_DUMP              */
#endif

#if 0
/**
 If defined, do not compile some features, even Mandatory, to fir into 64k ROM at 2410/64k device in Keil debug build.
*/
#define ZB_LIMITED_FEATURES
#endif

#ifndef ZB_PLATFORM_LINUX_ARM_2400
/* #define ZB_DEFAULT_APS_CHANNEL_MASK ZB_TRANSCEIVER_ALL_CHANNELS_MASK */
#define ZB_DEFAULT_APS_CHANNEL_MASK (1l<<14)
#else
/**
 Default value of ZB_AIB().aps_channel_mask
 Channels to be used for discovery and join. If here is single channel, not need to do energy scan.
 This definition often
*/
#define ZB_DEFAULT_APS_CHANNEL_MASK ((1l<<15))
#endif

#else
#define ZB_DEFAULT_APS_CHANNEL_MASK ((1l<<11)|(1l<<12))
#endif

/**
 Default duration to permit joining (currently infinite)
*/
#define ZB_DEFAULT_PRMIT_JOINING_DURATION 0xff

/**
 Default value of nib.max_children - max number of children which can join to this device
*/
#define ZB_DEFAULT_MAX_CHILDREN 32

/**
 NWK radius to be used when sending APS command
*/
#define ZB_APS_COMMAND_RADIUS 5


#ifdef ZB_SECURITY

/* security section */

/**
 SECUR: if defined, implement Standard security
*/
#define ZB_STANDARD_SECURITY

/**
 SECUR: If defined, generate random keys at Trust Center at start of pre-configured jey is not set.
*/
#define ZB_TC_GENERATES_KEYS
/**
 SECUR: If defined, trust Center is at ZC (currently - always)
*/
#define ZB_TC_AT_ZC

/**
 SECUR: CCM key size. Hard-coded
*/
#define ZB_CCM_KEY_SIZE 16

/* NLS5 - All devices shall maintain at least 2 NWK keys with the frame
   counters consistent with the security mode of the network (Standard or High).*/
#define ZB_SECUR_N_SECUR_MATERIAL 3


/* parameters for security level 5 - the only security level supported */
/**
 SECUR: security level. Now fixed to be 5
*/
#define ZB_SECURITY_LEVEL 5

/**
 SECUR: CCM L parameter. Fixed to 2 for security level 5
*/
#define ZB_CCM_L 2

/**
 SECUR: CCM nonce length. Now fixed.
*/
#define ZB_CCM_NONCE_LEN 13

/**
 SECUR: CCM M parameter. Fixed to 4 for security level 5
*/
#define ZB_CCM_M 4

/**
 Value of nwk packets counter which triggered nwk key switch
*/
#define ZB_SECUR_NWK_COUNTER_LIMIT (((zb_uint32_t)~0) - 128)

//#define ZB_DEFAULT_SECURE_ALL_FRAMES 0
/**
 Default value for nib.secure_all_frames
*/
#define ZB_DEFAULT_SECURE_ALL_FRAMES 1


#endif  /* ZB_SECURITY */

/**
   Maximum number of ZCL clusters
*/
#define ZB_ZCL_CLUSTER_NUM 8

/**
   ZDO Indirect poll timer
 */
#define ZB_ZDO_INDIRECT_POLL_TIMER (5*ZB_TIME_ONE_SECOND) /* ZB_TIME_ONE_SECOND*10 */

/**
   ZDO Max parent threshold retry
 */
#define ZB_ZDO_MAX_PARENT_THRESHOLD_RETRY 10

/**
 Min scan duration for mgmt_nwk_update_req
*/
#define ZB_ZDO_MIN_SCAN_DURATION 0
/**
 Max scan duration for mgmt_nwk_update_req
*/
#define ZB_ZDO_MAX_SCAN_DURATION 5
/**
 Special value of the scan duration for mgmt_nwk_update_req: change active channel (by number)
*/
#define ZB_ZDO_NEW_ACTIVE_CHANNEL 0xFE
/**
 Special value of the scan duration for mgmt_nwk_update_req: change channels mask
*/
#define ZB_ZDO_NEW_CHANNEL_MASK   0xFF

/**
 15 minutes timeout. KLUDGE: it is 2 bytes value, 15 minutes is
 nearly maximum value that can be stored
*/
#define ZB_ZDO_CHANNEL_CHECK_TIMEOUT (ZB_TIME_ONE_SECOND * 60 * 15)

/**
 A countdown timer (in minutes) indicating the time to the next
 permitted frequency agility channel change
*/
#define ZB_ZDO_APS_CHANEL_TIMER (1 * 60)

/**
 15 minutes timer to measure large timeouts
*/
#define ZB_ZDO_15_MIN_TIMEOUT (ZB_TIME_ONE_SECOND * 60 * 15)

/**
 1 minute timer to measure large timeouts
*/
#define ZB_ZDO_1_MIN_TIMEOUT (ZB_TIME_ONE_SECOND * 60)

/**
   Integer value representing the
   * number of scan attempts to make
   * before the NWK layer decides
   * which ZigBee coordinator or
   * router to associate with
*/
#if defined ZB_NS_BUILD || defined ZB_PLATFORM_LINUX_ARM_2400
#define ZB_ZDO_NWK_SCAN_ATTEMPTS 1
/* FIXME: Now this value could be only 1, because scan attempts routine work
 * wrong */
#else
/**
   Integer value representing the
   * number of scan attempts to make
   * before the NWK layer decides
   * which ZigBee coordinator or
   * router to associate with
*/
  #define ZB_ZDO_NWK_SCAN_ATTEMPTS 1
#endif

#if ZB_ZDO_NWK_SCAN_ATTEMPTS == 1
  #define ZB_NWK_ONE_SCAN_ATTEMPT
#endif


/**
   Integer value representing the
   time duration (in milliseconds)
*/
#define ZB_ZDO_NWK_TIME_BTWN_SCANS 30

/**
 Timeout value in seconds
 employed in End Device
 Binding */
#define ZB_ZDO_ENDDEV_BIND_TIMEOUT 30

/**
 ZDO: transactions table size
*/
#define ZDO_TRAN_TABLE_SIZE 16

/**
   Number of pending Mgmt_Leave requests allowed
 */
#define ZB_ZDO_PENDING_LEAVE_SIZE 4

/**
+  This define is for APS retransmissions test, do not use it for the normal work
  */
//#define APS_RETRANSMIT_TEST

/**
   This define turns on/off test profile
 */
#ifndef ZB_LIMITED_FEATURES


/**
  Comp[ile Test Profile feature
*/
/* #define ZB_TEST_PROFILE */
#endif
/**
   This difine turnes on/off channel error mode (set errors while data sending)
*/
//#define ZB_CHANNEL_ERROR_TEST

/**
   Number of times device failes to send packet to the parent before rejoin
*/
#define ZB_ZDO_PARENT_LINK_FAILURE_CNT 12


/* Definitions for 802.15.4 certification hacks */

//#define MAC_CERT_TEST_HACKS

#define ZB_PREDEFINED_ROUTER_ADDR 0x3344
#define ZB_PREDEFINED_ED_ADDR     0x3344

#define ZB_DISTURBER_PANID 0x0bad

/*! @} */

//#define TP_PRO_BV_31

//define ZB_MAC_TESTING_MODE

#ifdef ZB_MAC_TESTING_MODE
#define ZB_MAC_SECURITY
#endif
#define MAC_DEVICE_TABLE_SIZE 4

#ifdef ZB8051
/**
 If defined, switch ON visibility device limit functionality (drop packets from invisible devices at MAC level)
*/
#define ZB_LIMIT_VISIBILITY

/**
 Maximum number of addresses in the visibility limit arrays
*/
#define ZB_N_VIZIBLE_ADDRESSES 6
#endif

/* Certification defines */

/**
 Disabling of processing of bind_req and end_device_bind_req for certification purposes
*/
#define ZB_DISABLE_BIND_REQ
//#define ZB_DISABLE_ED_BIND_REQ

/*PRO STACK COMPATIBILITY, it's better to turn it on only when needed */
#define ZB_PRO_COMPATIBLE

/**
 Default value for mac frame version subfield
*/
#define MAC_FRAME_VERSION MAC_FRAME_IEEE_802_15_4_2003

/**
 Block sleepy end devices (devices with rx-on-when-idle == false) from receiving broadcasts (drop it at MAC level)
*/
#define ZB_BLOCK_BROADCASTS_SLEEPY_ED

#define ZB_DISABLE_APS_ACK_REQ

#define ZB_DISABLE_REJOIN_AFTER_SEC_FAIL



#endif /* ZB_CONFIG_H */
