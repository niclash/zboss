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
PURPOSE: MAC layer globals definition
*/

#ifndef ZB_MAC_GLOBALS_H
#define ZB_MAC_GLOBALS_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */

#include "zb_bufpool.h"
#include "zb_mac.h"

typedef struct
{
  zb_uint8_t  type;         // value from zb_mac_request_type_e
  zb_buf_t*   buf;
} zb_mac_request_t;

/* defines outgoing requests queue. Outgoing means the
   following direction nwk->mac->transport               */

ZB_RING_BUFFER_DECLARE( zb_mac_out_request, zb_mac_request_t, ZB_MAC_MAX_REQUESTS );

typedef struct
{
  union
  {
    zb_uint8_t   short_addr;
    zb_uint16_t  long_addr;
  } address;
  union
  {
    zb_uint8_t   byte_value;
    zb_uint16_t  word_value;
  } value;
  struct data_s
  {
    zb_uint8_t*  data_ptr;
    zb_uint8_t   length;
  } dt;
} zb_mac_rw_reg_t;


#define MAC_MAX_STACK_LEVEL 5

#define ZB_MAC_SET_DEVICE_TYPE(device_type) (ZG->mac.mac_ctx.dev_type = (device_type))

#if 0
#define ZB_SET_MAC_SYNC_IO() (ZG->mac.mac_ctx.mac_io_sync_mode = 1)
#define ZB_SET_MAC_ASYNC_IO() (ZG->mac.mac_ctx.mac_io_sync_mode = 0)
#define ZB_CHECK_MAC_SYNC_IO() (ZG->mac.mac_ctx.mac_io_sync_mode)
#else

#define ZB_SET_MAC_SYNC_IO()
#define ZB_SET_MAC_ASYNC_IO()
//#define ZB_CHECK_MAC_SYNC_IO() 1

#endif

#define ZB_SET_MAC_STATUS(status) (ZG->mac.mac_ctx.mac_status = (status))
#define ZB_GET_MAC_STATUS() (zb_mac_status_t)((zb_uint8_t)ZG->mac.mac_ctx.mac_status + 0)

#define ZB_MAC_ACK_TIMEOUT_MASK   0x1
#define ZB_MAC_ACK_OK_MASK        0x2
#define ZB_MAC_ACK_FAIL_MASK      0x4
#define ZB_MAC_RESP_TIMEOUT_MASK  0x8
#define ZB_MAC_PEND_DATA_MASK     0x10
#define ZB_MAC_ASS_REQUEST_MASK   0x20
#define ZB_MAC_CHANNEL_ERROR_TEST_MASK 0x40
#define ZB_MAC_INDIRECT_DATA_REQUEST_MASK 0x80

#define ZB_MAC_ACK_NEEDED_MASK    0x1
#define ZB_MAC_IO_IN_USE_MASK     0x2
#define ZB_MAC_BEACON_REQ_MASK    0x4
#define ZB_MAC_SCAN_TIMEOUT_MASK  0x8
#define ZB_MAC_POLL_REQUEST_MASK  0x10
#define ZB_MAC_TRANS_SLP_MASK     0x20
#define ZB_MAC_SECURITY_PROCESS   0x40

#define ZB_MAC_SET_SECURITY_PROCESS()  (MAC_CTX().mac_flags2 |= ZB_MAC_SECURITY_PROCESS)
#define ZB_MAC_GET_SECURITY_PROCESS()  (MAC_CTX().mac_flags2 & ZB_MAC_SECURITY_PROCESS)
#define ZB_MAC_CLEAR_SECURITY_PROCESS()  (MAC_CTX().mac_flags2 &= ~ZB_MAC_SECURITY_PROCESS)




#define ZB_MAC_SET_ACK_TIMEOUT()   (MAC_CTX().mac_flags |= ZB_MAC_ACK_TIMEOUT_MASK)
#define ZB_MAC_GET_ACK_TIMEOUT()   (MAC_CTX().mac_flags & ZB_MAC_ACK_TIMEOUT_MASK)
#define ZB_MAC_CLEAR_ACK_TIMEOUT() (MAC_CTX().mac_flags &= ~ZB_MAC_ACK_TIMEOUT_MASK)
#define ZB_MAC_SET_ACK_OK()        (MAC_CTX().mac_flags |= ZB_MAC_ACK_OK_MASK)
#define ZB_MAC_GET_ACK_OK()        (MAC_CTX().mac_flags & ZB_MAC_ACK_OK_MASK)
#define ZB_MAC_CLEAR_ACK_OK()      (MAC_CTX().mac_flags &= ~ZB_MAC_ACK_OK_MASK)
#define ZB_MAC_SET_ACK_FAIL()      (MAC_CTX().mac_flags |= ZB_MAC_ACK_FAIL_MASK)
#define ZB_MAC_GET_ACK_FAIL()      (MAC_CTX().mac_flags & ZB_MAC_ACK_FAIL_MASK)
#define ZB_MAC_CLEAR_ACK_FAIL()    (MAC_CTX().mac_flags &= ~ZB_MAC_ACK_FAIL_MASK)
#define ZB_MAC_CLEAR_ACK_STATUS()  (MAC_CTX().mac_flags &= ~(ZB_MAC_ACK_OK_MASK | ZB_MAC_ACK_FAIL_MASK))
#define ZB_MAC_SET_PENDING_DATA()  (MAC_CTX().mac_flags |= ZB_MAC_PEND_DATA_MASK)
#define ZB_MAC_GET_PENDING_DATA()  (MAC_CTX().mac_flags & ZB_MAC_PEND_DATA_MASK)
#define ZB_MAC_CLEAR_PENDING_DATA() (MAC_CTX().mac_flags &= ~ZB_MAC_PEND_DATA_MASK)
//#define ZB_MAC_SET_RESP_TIMEOUT()   (MAC_CTX().mac_flags |= ZB_MAC_RESP_TIMEOUT_MASK)
//#define ZB_MAC_GET_RESP_TIMEOUT()   (MAC_CTX().mac_flags & ZB_MAC_RESP_TIMEOUT_MASK)
//#define ZB_MAC_CLEAR_RESP_TIMEOUT() (MAC_CTX().mac_flags &= ~ZB_MAC_RESP_TIMEOUT_MASK)

#define ZB_MAC_SET_ASS_REQUEST()   (MAC_CTX().mac_flags |= ZB_MAC_ASS_REQUEST_MASK)
#define ZB_MAC_GET_ASS_REQUEST()   (MAC_CTX().mac_flags & ZB_MAC_ASS_REQUEST_MASK)
#define ZB_MAC_CLEAR_ASS_REQUEST() (MAC_CTX().mac_flags &= ~ZB_MAC_ASS_REQUEST_MASK)

#define ZB_MAC_SET_ACK_NEEDED()              (MAC_CTX().mac_flags2 |= ZB_MAC_ACK_NEEDED_MASK)
#define ZB_MAC_GET_ACK_NEEDED()              (MAC_CTX().mac_flags2 & ZB_MAC_ACK_NEEDED_MASK)
#define ZB_MAC_CLEAR_ACK_NEEDED()            (MAC_CTX().mac_flags2 &= ~ZB_MAC_ACK_NEEDED_MASK)

#define ZB_MAC_SET_INDIRECT_DATA_REQUEST()   (MAC_CTX().mac_flags |= ZB_MAC_INDIRECT_DATA_REQUEST_MASK)
#define ZB_MAC_GET_INDIRECT_DATA_REQUEST()   (MAC_CTX().mac_flags & ZB_MAC_INDIRECT_DATA_REQUEST_MASK)
#define ZB_MAC_CLEAR_INDIRECT_DATA_REQUEST() (MAC_CTX().mac_flags &= ~ZB_MAC_INDIRECT_DATA_REQUEST_MASK)

#define ZB_MAC_SET_IO_IN_USE()      (MAC_CTX().mac_flags2 |= ZB_MAC_IO_IN_USE_MASK)
#define ZB_MAC_GET_IO_IN_USE()      (MAC_CTX().mac_flags2 & ZB_MAC_IO_IN_USE_MASK)
#define ZB_MAC_CLEAR_IO_IN_USE()    (MAC_CTX().mac_flags2 &= ~ZB_MAC_IO_IN_USE_MASK)

#define ZB_MAC_SET_BEACON_REQ()     (MAC_CTX().mac_flags2 |= ZB_MAC_BEACON_REQ_MASK)
#define ZB_MAC_GET_BEACON_REQ()     (MAC_CTX().mac_flags2 & ZB_MAC_BEACON_REQ_MASK)
#define ZB_MAC_CLEAR_BEACON_REQ()   (MAC_CTX().mac_flags2 &= ~ZB_MAC_BEACON_REQ_MASK)

#define ZB_MAC_SET_SCAN_TIMEOUT()   (MAC_CTX().mac_flags2 |= ZB_MAC_SCAN_TIMEOUT_MASK)
#define ZB_MAC_GET_SCAN_TIMEOUT()   (MAC_CTX().mac_flags2 & ZB_MAC_SCAN_TIMEOUT_MASK)
#define ZB_MAC_CLEAR_SCAN_TIMEOUT() (MAC_CTX().mac_flags2 &= ~ZB_MAC_SCAN_TIMEOUT_MASK)

#define ZB_MAC_SET_POLL_REQUEST()   (MAC_CTX().mac_flags2 |= ZB_MAC_POLL_REQUEST_MASK)
#define ZB_MAC_GET_POLL_REQUEST()   (MAC_CTX().mac_flags2 & ZB_MAC_POLL_REQUEST_MASK)
#define ZB_MAC_CLEAR_POLL_REQUEST() (MAC_CTX().mac_flags2 &= ~ZB_MAC_POLL_REQUEST_MASK)

#define ZB_MAC_SET_CHANNEL_ERROR_TEST()   (MAC_CTX().mac_flags |= ZB_MAC_CHANNEL_ERROR_TEST_MASK)
#define ZB_MAC_GET_CHANNEL_ERROR_TEST()   (MAC_CTX().mac_flags & ZB_MAC_CHANNEL_ERROR_TEST_MASK)
#define ZB_MAC_CLEAR_CHANNEL_ERROR_TEST() (MAC_CTX().mac_flags &= ~ZB_MAC_CHANNEL_ERROR_TEST_MASK)

#define ZB_MAC_SET_TRANS_SPLEEPING()   (MAC_CTX().mac_flags2 |= ZB_MAC_TRANS_SLP_MASK)
#define ZB_MAC_GET_TRANS_SPLEEPING()   (MAC_CTX().mac_flags2 & ZB_MAC_TRANS_SLP_MASK)
#define ZB_MAC_CLEAR_TRANS_SPLEEPING() (MAC_CTX().mac_flags2 &= ~ZB_MAC_TRANS_SLP_MASK)


#define ZB_TX_TOTAL_THRESHOLD 20 /* Annex E, total transmissions attempted */
#define ZB_FAILS_PERCENTAGE   4  /* use it as divider, 25% */
#define ZB_CHANNEL_BUSY_ED_VALUE 0x60  /* TODO: check it */
#define ZB_CHANNEL_FREE_ED_VALUE 0x20  /* TODO: check it */

#define ZB_ACTIVE_SCAN_MAX_PAN_DESC_COUNT 2

typedef struct zb_mac_data_req_storage_s
{	
	zb_uint8_t	          data_found;
    zb_uint8_t            mhr_pend_len;
	zb_uint8_t            data_found_index;
	zb_uint8_t            *fcf;
    zb_uint8_t            *cmd_ptr;
	zb_mac_mhr_t          mhr_pend;
} zb_mac_data_req_storage_t;

typedef struct zb_mac_ctx_s
{
  zb_buf_t*             recv_buf;      /* current buffer for receiving ( mac<-transceiver ) */
  zb_buf_t*             pending_buf;   /* zb_buf_t* for handling any service requests */
  zb_uint8_t            tx_cnt; /* tx counter, just to save tx number */
  zb_uint8_t 			mlme_scan_in_progress; /* 1 if active, ed or orphan scan are processing */  
  zb_uint8_t            ed_scan_step_passed; /* to get rssi right before next channel change */
  zb_callback_t         tx_wait_cb;
  zb_uint8_t            tx_wait_cb_arg;
  union
  {
   zb_mac_data_req_storage_t      mac_data_req;
  }mac_storage;
   
  zb_uint8_t            rx_need_buf;
#ifdef ZB_USE_RX_QUEUE
  zb_uint8_t            recv_buf_full;
  zb_rx_queue_t         mac_rx_queue;
#ifdef ZB_RESERVED_REGS_DUMP
  zb_regs_queue_t   	regs_queue;
#endif
#endif
  zb_uint8_t            isrsts; /* TODO: check, is it needed? */
  /* TODO: it seems we can use buffer smaller operation send/recv buffers */
  zb_buf_t*             operation_buf;
  /* we should use separate buffer for receiving data, we need no set
   * recv buffer before sending address while reading registers; if
   * recv buffer is not set when byte is received, data will be lost */
  zb_buf_t*             operation_recv_buf;

#ifdef ZB_SECURITY
  zb_buf_t*             encryption_buf; /* buffer used for nwk encryption */
#endif

  zb_mac_out_request_t  out;

  zb_mac_rw_reg_t       rw_reg;
  zb_uint8_t            byte_result_1;
  zb_uint8_t            mac_io_sync_mode; /* MAC performs sync/async i/o*/ /* TODO: move it to mac_flags !!!! */
  zb_mac_status_t       mac_status; /* MAC status of the last operation. NOTE: if operation
                                     * is successful, status is not updated! */
  zb_uint8_t            dev_type;   /*!< my device type - \see enum zb_mac_dev_type_e */
  zb_uint8_t            current_channel;



  zb_uint8_t mac_flags; /* flags to store internal state */
  zb_uint8_t mac_flags2; /* flags to store internal state */
  zb_uint8_t ack_dsn;   /* dsn value, is used to check received acknowledgement */
  zb_uint8_t retry_counter; /* packet re-send counter */


#ifdef ZB_MULTIPLE_BEACONS
  zb_uint8_t beacons_sent;      /* number of beacons already sent */
#endif
  zb_uint32_t unscanned_channels;
  union
  {
    struct
    {      
      zb_uint8_t channel_number;
      zb_uint8_t beacon_found;
#ifdef ZB_MAC_TESTING_MODE
      zb_uint8_t pan_desc_buf_param;
      zb_uint8_t stop_scan;
#endif
    }
    active_scan;

    struct
    {     
      zb_uint8_t got_realignment;
    }
    orphan_scan;

    struct
    {
      zb_callback_type_t cb_type;
      zb_uint_t cb_status;
      zb_uint8_t buf_ref;
    }
    indirect_data;

    struct
    {
      zb_uint8_t scan_timeout;
      zb_uint8_t channel_number;
      zb_uint8_t save_channel;
      zb_uint8_t max_rssi_value;
    }
    ed_scan;
  }
  rt_ctx;

  zb_mac_pending_data_t pending_data_queue[ZB_MAC_PENDING_QUEUE_SIZE];

  zb_void_t (*beacon_sent)(void);
#ifdef ZB_LIMIT_VISIBILITY
  zb_uint8_t n_visible_addr;
  zb_ieee_addr_t visible_addresses[ZB_N_VIZIBLE_ADDRESSES];
  zb_uint8_t n_invisible_short_addr;
  zb_uint16_t invisible_short_addreesses[ZB_N_VIZIBLE_ADDRESSES];
#endif
} zb_mac_ctx_t;


typedef struct zb_mac_globals_s
{
  zb_mac_pib_t    pib;
  zb_mac_ctx_t    mac_ctx;
} zb_mac_globals_t;

/*! @} */
/*! \endcond */

#endif /* ZB_MAC_GLOBALS_H */
