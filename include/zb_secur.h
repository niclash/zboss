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
PURPOSE: Security services and routines - internals
*/

#ifndef ZB_SECUR_H
#define ZB_SECUR_H 1

#ifdef ZB_SECURITY

/*! \cond internals_doc */
/*! \addtogroup ZB_SECUR */
/*! @{ */


#define ZB_NONCE_LENGTH 13
#define ZB_KEY_LENGTH 16


/**
   Network key types
 */
typedef enum zb_secur_key_types_e
{
  ZB_TC_MASTER_KEY        = 0,
  ZB_STANDARD_NETWORK_KEY = 1,
  ZB_APP_MASTER_KEY       = 2,
  ZB_APP_LINK_KEY         = 3,
  ZB_TC_LINK_KEY          = 4,
  ZB_HIGH_SECUR_NETWORK_KEY = 5
} zb_secur_key_types_t;


/**
   Key id - see 4.5.1.1.2
 */
enum zb_secur_key_id_e
{
  ZB_SECUR_DATA_KEY,
  ZB_SECUR_NWK_KEY,
  ZB_SECUR_KEY_TRANSPORT_KEY,
  ZB_SECUR_KEY_LOAD_KEY
};


/**
   Security/rejoin states os the 'status' field of apsme-update-device as described in table 4.40
 */
enum zb_secur_upd_device_status_e
{
  ZB_STD_SEQ_SECURED_REJOIN     = 0,
  ZB_STD_SEQ_UNSECURED_JOIN     = 1,
  ZB_DEVICE_LEFT                = 2,
  ZB_STD_SEQ_UNSECURED_REJOIN   = 3,
  ZB_HIGH_SEQ_SECURED_REJOIN    = 4,
  ZB_HIGH_SEQ_UNSECURED_JOIN    = 5,
  ZB_HIGH_SEQ_UNSECURED_REJOIN  = 7
};


enum zb_secur_buf_encr_type_e
{
  ZB_SECUR_NO_ENCR,             /*!< no encryption at all  */
  ZB_SECUR_NWK_ENCR,            /*!< NWK frame encryption  */
  ZB_SECUR_APS_ENCR,            /*!< APS encryption. Analyze APS header to
                                 * define which key to use  */
  ZB_SECUR_MAC_ENCR             /*!< MAC encryption - for 802.15.4 certification
                                 * only */
};



/**
   Auxiliary frame header (4.5.1) for NWK frame and NWK key

   Extended nonce subfield set to 1 (4.3.1.1).
   source_address amd key_seq_number exist.
 */
typedef struct zb_nwk_aux_frame_hdr_s
{
  zb_uint8_t     secur_control;
  zb_uint32_t    frame_counter;
  zb_ieee_addr_t source_address;
  zb_uint8_t     key_seq_number;
} ZB_PACKED_STRUCT zb_nwk_aux_frame_hdr_t;


/**
   Auxiliary frame header (4.5.1) for APS frame encrypted by NWK key

   Extended nonce subfield set to 0 (4.4.1.1).
   source_address absent, key_seq_number exists.
 */
typedef struct zb_aps_nwk_aux_frame_hdr_s
{
  zb_uint8_t     secur_control;
  zb_uint32_t    frame_counter;
  zb_uint8_t     key_seq_number;
} ZB_PACKED_STRUCT zb_aps_nwk_aux_frame_hdr_t;

/**
   Auxiliary frame header (4.5.1) for APS frame encrypted by Data key

   Extended nonce subfield set to 0 (4.4.1.1).
   source_address and key_seq_number are absent.
 */
typedef struct zb_aps_data_aux_frame_hdr_s
{
  zb_uint8_t     secur_control;
  zb_uint32_t    frame_counter;
} ZB_PACKED_STRUCT zb_aps_data_aux_frame_hdr_t;


/**
   CCM nonce (see 4.5.2.2)
 */
typedef struct zb_secur_ccm_nonce_s
{
  zb_ieee_addr_t source_address;
  zb_uint32_t    frame_counter;
  zb_uint8_t     secur_control;
} ZB_PACKED_STRUCT zb_secur_ccm_nonce_t;


#define ZB_NWK_STD_SECUR_CONTROL \
  ( 5 /*security level */ | (1 << 3) /* key identifier - NWK key */ | 1 << 5 /* ext nonce */)

#define ZB_NWK_STD_SECUR_CONTROL_ZEROED_LEVEL \
  ( 0 /*security level */ | (1 << 3) /* key identifier */ | 1 << 5 /* ext nonce */)


#define ZB_APS_NWK_STD_SECUR_CONTROL \
  ( 5 /*security level */ | (1 << 3) /* key identifier - NWK key */ | 0 /* ext nonce */)

#define ZB_APS_NWK_STD_SECUR_CONTROL_ZEROED_LEVEL \
  ( 0 /*security level */ | (1 << 3) /* key identifier */ | 0 /* ext nonce */)

#define ZB_APS_DATA_STD_SECUR_CONTROL \
  ( 5 /*security level */ | (0 << 3) /* key identifier - APS data key */ | 0 /* ext nonce */)

#define ZB_APS_DATA_STD_SECUR_CONTROL_ZEROED_LEVEL \
  ( 0 /*security level */ | (0 << 3) /* key identifier */ | 0 /* ext nonce */)

#define ZB_SECUR_AUX_HDR_GET_KEY_TYPE(ctrl) (((ctrl) >> 3) & 0x3)

/**
   CCM* encryption and authentication procedure for Standard security

   This implementation may be not optimal. It done for debug only: be able to
   encrypt in Linux to be able to compare results with real HW.
   To be debugged using test vectors from the spec.


   @param ccm_m - 'M' parameter - see table 4.38 (4 for Standard security - mode 5)
   @param key   - 16-bytes key
   @param nonce - 16-bytes nonce - see 4.5.2.2 "CCM nonce" for details
   @param string_a - 'a' parameter - authentication string -
                     NwkHeader||AuxuluaryHeader for Standard security. See 4.3.1.1 for details
   @param string_a_len - l(a)
   @param string_m - 'm' parameter - text data -
                     Payload for Standard security. See 4.3.1.1 for details
   @param string_m_len - l(m)
   @param crypted_text - encryption result - user supplied buffer. Must have size
          string_m_len + ccm_m rounded to 16.
          Result len is always string_m_len + ccm_m.


   @return RET_OK if success, RET_ERROR in case of error
 */


zb_ret_t
zb_ccm_encrypt_n_auth(
  zb_uint8_t *key,
  zb_uint8_t *nonce,
  zb_uint8_t *string_a,
  zb_ushort_t string_a_len,
  zb_uint8_t *string_m,
  zb_ushort_t string_m_len,
  zb_buf_t *crypted_text);


/**
   CCM* decryption and authentication procedure for Standard security

   @param ccm_m - 'M' parameter - see table 4.38 (4 for Standard security - mode 5)
   @param key   - 16-bytes key
   @param nonce - 16-bytes nonce - see 4.5.2.2 "CCM nonce" for details
   @param string_a - 'a' parameter - authentication string -
                     NwkHeader||AuxuluaryHeader for Standard security. See 4.3.1.1 for details
   @param string_a_len - l(a)
   @param string_c - 'c' parameter - authentication string -
                     Payload for Standard security. See 4.3.1.1 for details
   @param string_c_len - l(c)
   @param decrypted_text - encryption result - user supplied buffer. Must have size
          string_m_len + ccm_m rounded to 16.
          Result len is always string_m_len - ccm_m.

   @return RET_OK if success, RET_ERROR in case of error (authentication failure)
 */

zb_ret_t
zb_ccm_decrypt_n_auth(
  zb_uint8_t *key,
  zb_uint8_t *nonce,
  zb_buf_t  *buf,
  zb_ushort_t string_a_len,
  zb_ushort_t string_c_len);


zb_ret_t
zb_ccm_decrypt_n_auth_stdsecur(
  zb_uint8_t *key,
  zb_uint8_t *nonce,
  zb_buf_t *buf,
  zb_ushort_t string_a_len,
  zb_ushort_t string_m_len);

/**
   Secure frame at NWK level according to the current security mode
   This procedure secures frame 'on place' increasing its size.
   It is supposed that buffer tail has enough free space.

   @param src - nwk packet to secure
   @param mac_hdr_size - size of header placed before mac hdr
   @param dst - buffer for the secured frame


   @return RET_OK if ok, else error code
 */
zb_ret_t zb_nwk_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst);


/**
   Unsecure NWK frame at NWK level according to the current security mode
   This procedure secures frame 'on place' decreasing its size.

   @param param - ref to nwk packet to unsecure
   @param mhr - parsed mac hdr
   @param mhr_len - mhr raw length

   @return RET_OK if success or RET_ERROR if unsecure failed
   Attention: if frame unsecure failed, this function reuses packet buffer to
   send NWK status. Don't use the buffer if return code != RET_OK!
 */
zb_ret_t zb_nwk_unsecure_frame(zb_uint8_t param, zb_mac_mhr_t *mhr, zb_uint8_t mhr_len);


/**
   Fill auxiluary header in the APS data or command frame.

   @param p - pointer to the aux hdr
   @param nwk_key - true if use nwk key, false if use aps data key
 */
void zb_secur_aps_aux_hdr_fill(zb_uint8_t *p, zb_bool_t nwk_key);

/**
   Add security to the aps command frame. Allocate space for aps header.

   @param buf - buffer with command payload
   @param command_id - command id to assign into aps command header

   @return ZB_TRUE if need to secure this frame at nwk level.
 */
zb_bool_t zb_aps_command_add_secur(zb_buf_t *buf, zb_uint8_t command_id);

/**
   Secure APS frame before its send.

   Need to put secured frame into another buffer because original buffer will be
   passed back to APS in CONFIRM primitive and, potentially, will be
   retransmitter by APS layer, so must keep it unencrypted.

   @param src - source buffer with all headers filled, including MHR
   @param mac_hdr_size - size of MAC header
   @param dsr - destination buffer (for secure result)
 */
void zb_aps_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst);


/**
   Unsecure APS frame "on place"

   @param buf - (in/out) buffer holding frame

   @return RET_OK if successfully unsecured frame, error code otherwhise
 */
zb_ret_t zb_aps_unsecure_frame(zb_buf_t *buf);


/**
   Return size of APS aux secure header

   @param p - aux aps header pointer

   @return aux header size if ok or -1 if error (unknown key type)
 */
zb_ushort_t zb_aps_secur_aux_size(zb_uint8_t *p);


zb_ret_t zb_mac_secure_frame(zb_buf_t *src, zb_uint_t mac_hdr_size, zb_buf_t *dst);

zb_ret_t zb_mac_unsecure_frame(zb_uint8_t param);

/**
   Parameters for APSME-TRANSPORT-KEY.request primitive
 */
typedef struct zb_apsme_transport_key_req_s
{
  zb_uint8_t key_type;          /*!< @see zb_secur_key_types_t  */
  union zb_addr_u dest_address; /*!< destinationa address  */
  zb_uint8_t addr_mode;    /*!< The type of destination address supplied by
                             the DstAddr parameter - @see zb_addr_mode_e.
                             This field is non-standard: according to table 4.11
                             dest_address can be 64-bit only.
                             But, according to 4.6.3.4.1  Trust Center Operation,
                             TC must issue APSME-TRANSPORT-KEY.request with
                             broadcast address - means, need 16-bit address here!
                           */
  union {
    struct apsme_transport_key_nwk_s
    {
      zb_uint8_t key[ZB_CCM_KEY_SIZE];
      zb_uint8_t key_seq_number;
      zb_uint8_t use_parent;
      zb_ieee_addr_t parent_address;
    } nwk;
  } key;
} zb_apsme_transport_key_req_t;


typedef struct zb_transport_key_nwk_key_dsc_pkt_s
{
  zb_uint8_t     key_type;       /* indeed, not part of the nwk key descriptor,
                                  * but it simplifies code  */

  zb_uint8_t     key[ZB_CCM_KEY_SIZE];
  zb_uint8_t     seq_number;
  zb_ieee_addr_t dest_address;
  zb_ieee_addr_t source_address;
} ZB_PACKED_STRUCT zb_transport_key_nwk_key_dsc_pkt_t;


typedef struct zb_apsme_update_device_pkt_s
{
  zb_ieee_addr_t device_address;
  zb_uint16_t    device_short_address;
  zb_uint8_t     status;        /*!< \see zb_secur_upd_device_status_e  */
} ZB_PACKED_STRUCT zb_apsme_update_device_pkt_t;


void zb_apsme_transport_key_request(zb_uint8_t param) ZB_CALLBACK;


/**
   Parameter for APSME-UPDATE-DEVICE.request
 */
typedef struct zb_apsme_update_device_req_s
{
  zb_ieee_addr_t dest_address;
  zb_ieee_addr_t device_address;
  zb_uint16_t    device_short_address;
  zb_uint8_t     status;
} zb_apsme_update_device_req_t;



/**
   Parameter for APSME-UPDATE-DEVICE.indication
 */
typedef struct zb_apsme_update_device_ind_s
{
  zb_ieee_addr_t src_address;
  zb_ieee_addr_t device_address;
  zb_uint16_t    device_short_address;
  zb_uint8_t     status;
} zb_apsme_update_device_ind_t;


/**
   Parameter for APSME-SWITCH-KEY.request

   Note about addr_mode: it is spec violation, but it necessary to satisfy TC.
 */
typedef struct zb_apsme_switch_key_req_s
{
  union zb_addr_u dest_address; /*!< destinationa address  */
  zb_uint8_t addr_mode;    /*!< The type of destination address supplied by
                             the DstAddr parameter - @see zb_addr_mode_e.
                             This field is non-standard: according to table 4.11
                             dest_address can be 64-bit only.
                             But, according to 4.6.3.4.1  Trust Center Operation,
                             TC must issue APSME-TRANSPORT-KEY.request with
                             broadcast address - means, need 16-bit address here!
                           */
  zb_uint8_t key_seq_number;
} zb_apsme_switch_key_req_t;


/**
   APSME-SWITCH-KEY.request primitive

   @param param - packet buffer filled bu \see zb_apsme_switch_key_req_t
 */
void zb_apsme_switch_key_request(zb_uint8_t param) ZB_CALLBACK;


/**
   APSME-SWITCH-KEY APS command body
 */
typedef struct zb_apsme_switch_key_pkt_s
{
  zb_uint8_t key_seq_number;
} ZB_PACKED_STRUCT zb_apsme_switch_key_pkt_t;


/**
   APSME-SWITCH-KEY.indication
 */
typedef struct zb_apsme_switch_key_ind_s
{
  zb_ieee_addr_t src_address;
  zb_uint8_t key_seq_number;
} zb_apsme_switch_key_ind_t;

void zb_aps_in_switch_key(zb_uint8_t param);


/**
   APSME-REMOVE-DEVICE.request primitive parameters structure
 */
typedef struct zb_apsme_remove_device_req_s
{
  zb_ieee_addr_t parent_address;
  zb_ieee_addr_t child_address;
} ZB_PACKED_STRUCT zb_apsme_remove_device_req_t;


typedef struct zb_apsme_remove_device_pkt_s
{
  zb_ieee_addr_t child_address;
} ZB_PACKED_STRUCT zb_apsme_remove_device_pkt_t;


/**
   APSME-REMOVE-DEVICE.indication primitive parameters structure
 */
typedef struct zb_apsme_remove_device_ind_s
{
  zb_ieee_addr_t src_address;
  zb_ieee_addr_t child_address;
} ZB_PACKED_STRUCT zb_apsme_remove_device_ind_t;

void zb_secur_apsme_remove_device(zb_uint8_t param) ZB_CALLBACK;
void zb_aps_in_remove_device(zb_uint8_t param);


/**
   APSME-REQUEST-KEY.request primitive parameters structure
 */
typedef struct zb_apsme_request_key_req_s
{
  zb_ieee_addr_t dest_address;
  zb_uint8_t     key_type;      /*<! \see zb_secur_key_types_e  */
  zb_ieee_addr_t partner_address;
} ZB_PACKED_STRUCT zb_apsme_request_key_req_t;


typedef struct zb_apsme_request_nwk_key_pkt_s
{
  zb_uint8_t     key_type;      /*<! \see zb_secur_key_types_e  */
} ZB_PACKED_STRUCT zb_apsme_request_nwk_key_pkt_t;

/**
   APSME-REQUEST-KEY.indication primitive parameters structure
 */
typedef struct zb_apsme_request_key_ind_s
{
  zb_ieee_addr_t src_address;
  zb_uint8_t     key_type;      /*<! \see zb_secur_key_types_e  */
  zb_ieee_addr_t partner_address;
} ZB_PACKED_STRUCT zb_apsme_request_key_ind_t;

void zb_secur_apsme_request_key(zb_uint8_t param) ZB_CALLBACK;
void zb_aps_in_request_key(zb_uint8_t param);


void zb_apsme_update_device_request(zb_uint8_t param) ZB_CALLBACK;

/**
   Initialize Trust Center functionality
 */
void secur_tc_init();


/**
   Generate keys as random numbers
 */

void secur_generate_keys();

/**
   Authenticate child after join
 */

void secur_authenticate_child(zb_uint8_t param) ZB_CALLBACK;


/**
   Gen NWK key by key sequence number
 */
zb_uint8_t *secur_nwk_key_by_seq(zb_ushort_t key_seq_number);



/**
   Parameters for APSME-TRANSPORT-KEY.indication primitive
 */
typedef struct zb_apsme_transport_key_indication_s
{
  zb_uint8_t key_type;          /*!< @see zb_secur_key_types_t  */
  zb_ieee_addr_t src_address;
  union {
    struct apsme_transport_key_nwk_ind_s
    {
      zb_uint8_t key[ZB_CCM_KEY_SIZE];
      zb_uint8_t key_seq_number;
    } nwk;
  } key;
} zb_apsme_transport_key_indication_t;



/**
   APSME-TRANSPORT-KEY.indication primitive
 */
void zb_apsme_transport_key_indication(zb_uint8_t param) ZB_CALLBACK;


void secur_nwk_key_switch(zb_uint8_t key_number);

/**
   Switch nwk key and send it via broadcast
 */
void zb_secur_switch_nwk_key_br(zb_uint8_t param) ZB_CALLBACK;

/**
   Return true if network key is empty (all zeroes).

   @param key - key
   @return ZB_TRUE if key is empty, ZB_FALSE otherwhise
 */
zb_bool_t secur_nwk_key_is_empty(zb_uint8_t *key);


/**
   Return TRUE is this device has preconfigured NWK key
 */
zb_bool_t secur_has_preconfigured_key();


void zb_apsme_remove_device_indication(zb_uint8_t param) ZB_CALLBACK;


void zb_apsme_request_key_indication(zb_uint8_t param) ZB_CALLBACK;


void zb_apsme_switch_key_indication(zb_uint8_t param) ZB_CALLBACK;

/*! @} */
/*! \endcond */

#endif  /* ZB_SECURITY */


#endif /* ZB_SECUR_H */
