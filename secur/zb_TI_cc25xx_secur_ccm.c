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
PURPOSE: CCM* routines - to be used when HW is not available - that is, in Linux/ns-3
*/


/*! \addtogroup ZB_SECUR */
/*! @{ */


/**************************************************************
***************************************************************/


/* constants are hard-coded for Standard security */

#include "zb_common.h"
#include "zb_types.h"
#include "zb_config.h"
#include "zb_secur.h"
#ifdef ZB_CC25XX
#include "zb_bank_common.h"

#ifdef ZB_SECURITY


//  #include <ioCC2530.h>

//  #include "zb_types.h"
//  #include "zb_errors.h"
//  #include "zb_osif_8051.h"

  #define AES_START()           do { ENCCS |= 0x01; } while (0)                 /* Staring AES operation */
  #define AES_RDY()             ( ENCCS & 8)                                    /* Poll the AES ready bit */
  #define AES_SET_MODE(mode)    do { ENCCS &= ~0x70; ENCCS |= mode; } while (0) /* Setting the mode of the AES operation */
  #define AES_SET_OPERATION(op) do { ENCCS  = (ENCCS & ~0x07) | op; } while (0) /* Setting the AES operation */

/* Encryption mode */
  #define AES_MODE_CBC            0x00
  #define AES_MODE_CFB            0x10
  #define AES_MODE_OFB            0x20
  #define AES_MODE_CTR            0x30
  #define AES_MODE_ECB            0x40
  #define AES_MODE_CBCMAC         0x50

/* Operations */
  #define AES_ENCRYPT             0x00
  #define AES_DECRYPT             0x02
  #define AES_LOAD_KEY            0x04
  #define AES_LOAD_IV             0x06

  #define AES_BLOCK_LEN           0x10

  #define MIC_2_MICLEN(m)        (1 << (n((m & 3) + 1) & ~3))   /* Convert MIC to MIC length */


  #pragma optimize = none
void zb_wait_us(zb_uint16_t uSec) {
  uSec >>= 1;   /* /2 */
  while( uSec-- ) {
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }
}



/******************************************************************************
 * @fn      zb_AESLdBlck
 *
 * @brief   Writes data into the AES coprocessor
 * @param   Pointer to data.
 * @param   command () - AES_ENCRYPT or AES_LOAD_KEY or AES_LOAD_IV
 *
 * @return  None
 */
void zb_AESLdBlck( zb_uint8_t *pData, zb_uint8_t op) {

  zb_uint8_t i;

  AES_SET_OPERATION(op);      /* Set operation */
  AES_START();                /* Starting loading of key or vector. */

  for( i = 0; i < AES_BLOCK_LEN; i++ ) { /* loading the data (key or vector) */
    ENCDI = pData[i];
  }
  return;
}

/***********************************************************************************
* @fn          zb_RfSecurityInit
*
* @brief       Initialise key and write to radio
* @return      none
*/
void zb_RfSecurityInit(zb_uint8_t* pKey) {
  zb_uint8_t i;

  AES_SET_OPERATION(AES_LOAD_KEY);
  AES_START();    /* Starting loading of key or vector */
  for( i = 0; i < ZB_KEY_LENGTH; i++ ) {    /* Load key */
    ENCDI = pKey[i];
  }
}

  #if 1
/******************************************************************************
 * @fn      zb_AES_rw4x4
 *
 * @brief   Writes data into the AES coprocessor and reading result for
 * CFB, OFB and CTR  modes
 *
 * @return  None
 */
inline void zb_AES_rw4x4( zb_uint8_t*  pInData,    /* Input data pointer     */
                          zb_uint8_t*  pOutData,   /* Output data pointer    */
                          zb_uint8_t   nBlcks) {   /* AES block's number     */

  zb_uint8_t i, j, cb, bshi = 0, bsho = 0;


  for( cb = 0; cb < nBlcks; cb++ ) {
    for( j = 0; j < 4; j++ ) {
      for( i = 0; i < 4; i++ ) {    /* Writing the input data */
        ENCDI = pInData[bshi++];
      }
      for( i = 0; i < 4; i++ ) {    /* Reading the encrypted data */
        pOutData[bsho++] = ENCDO;
      }
    }
  }
}

/******************************************************************************
 * @fn      zb_AES_rw16
 *
 * @brief   Writes data into the AES coprocessor and reading result for
 * ECB, CBC and CBC-MAC  modes
 *
 * @return  None
 */
inline void zb_AES_rw16( zb_uint8_t*  pInData,    /* Input data pointer     */
                         zb_uint8_t*  pOutData,   /* Output data pointer    */
                         zb_uint8_t   nBlcks) {   /* AES block's number     */

  zb_uint8_t i, cb, bshi = 0, bsho = 0;


  for( cb = 0; cb < nBlcks; cb++ ) {
    for( i = 0; i < AES_BLOCK_LEN; i++ ) {    /* Writing the input data */
      ENCDI = pInData[bshi++];
    }
    for( i = 0; i < AES_BLOCK_LEN; i++ ) {    /* Reading the encrypted data */
      pOutData[bsho++] = ENCDO;
    }
  }
}
  #endif
/******************************************************************************
 * @fn      auth_trans
 *
 * @brief   Generates CCM Authentication tag U.
 *
 * @return  None
 *
 */
static void auth_trans( /* Input */
                        zb_uint8_t  ccm_m,       /* Mval - Length of authentication field in octets [0,2,4,6,8,10,12,14 or 16] */
                        zb_uint8_t  *pKey,       /* Pointer to AES Key or Pointer to Key Expansion buffer. */
                        zb_uint8_t  *pNonce,     /* Pointer to 13-byte Nonce     */
                        zb_uint8_t  *pString_a,  /* Pointer to octet string 'a'  */
                        zb_uint16_t lenString_a, /* Length of A[] in octets      */
                        zb_uint8_t  *pString_m,  /* Pointer to octet string 'm'  */
                        zb_uint16_t lenString_m, /* Length of M[] in octets */
                        /* Output */
                        zb_uint8_t  *pT) {


  zb_uint8_t    buf[AES_BLOCK_LEN], *pbuf, *msg_in;
  zb_uint8_t    i, j;
  zb_uint16_t   blocks, msg_len;

  zb_buf_t*     ptbuf;

  ptbuf = zb_get_in_buf();

  /* Check if authentication is even requested.  If not, exit. */
  if( !ccm_m ) return;

  /*
   *  Build B0
   */
  buf[0] = (((ccm_m - 2)/2) << 3) | (ZB_CCM_L - 1);  /* (M - 2)/2 || L- 1 */

  if( lenString_a != 0 ) {
    buf[0] |= 0x40;         /* A_Data present */
  }

  ZB_MEMCPY(&buf[1], pNonce, ZB_CCM_NONCE_LEN);    /* append Nonce */

  buf[ZB_CCM_NONCE_LEN + 1] = ZB_GET_HI_BYTE(lenString_m);  /* append l(m) */
  buf[ZB_CCM_NONCE_LEN + 2] = ZB_GET_LOW_BYTE(lenString_m);

  /*
   *  Build B0
   */
  /* Calculate msg length */
  msg_len = 48 + lenString_m - (lenString_m & 0x0f);
  msg_len += (lenString_a != 0) ? ((lenString_a + 2) - ((lenString_a + 2) & 0x0f)) : 0;

  ZB_BUF_INITIAL_ALLOC(ptbuf, msg_len, pbuf);         /* Allocate buffer */
  msg_in = pbuf;

  if( !pbuf ) return;

  /* Move B0 into position */
  ZB_MEMCPY( pbuf, buf, AES_BLOCK_LEN );
  pbuf += AES_BLOCK_LEN;

  /*
   *  Encode l(a) and move l(a) into position
   */
  if( lenString_a != 0 ) {
    *pbuf++ = ZB_GET_HI_BYTE(lenString_a);    /* append l(a) */
    *pbuf++ = ZB_GET_LOW_BYTE(lenString_a);

    ZB_MEMCPY(pbuf, pString_a, lenString_a);
    pbuf += lenString_a;

    /* Fill appendix with zeros if len_a > 0 */
    i = AES_BLOCK_LEN - ((lenString_a + 2) & 0x0f);
    ZB_MEMSET(pbuf, 0, i);
    pbuf += i;
  }

  /* Move M into position */
  ZB_MEMCPY(pbuf, pString_m, lenString_m);
  pbuf += lenString_m;

  /* Fill appendix with zeros */
  if( lenString_m ) {
    i = AES_BLOCK_LEN - (lenString_m & 0x0f);
    ZB_MEMSET( pbuf, 0, i );
    pbuf += i;
  }

  /* Prepare CBC-MAC */
  AES_SET_MODE(AES_MODE_CBCMAC);

  ZB_MEMSET(pT, 0, AES_BLOCK_LEN);   /* X0 = 0 */
  zb_AESLdBlck(pT, AES_LOAD_IV);

  AES_SET_OPERATION(AES_ENCRYPT);
  zb_wait_us(1);                /* wait for data ready */
  pbuf = msg_in;

  for( blocks = 0; blocks < ((msg_len >> 4) - 1); blocks++ ) {  /* CBC-MAC does not generate output until the last block */
    AES_START();
    for( i = 0; i < AES_BLOCK_LEN; i++ ) {    /* Writing the input data with zero-padding */
      ENCDI =  *pbuf++;
    }
    while( !AES_RDY() );
  }

  AES_SET_MODE(AES_MODE_CBC);       /* Switch to CBC mode for the last block and kick it off */
  zb_wait_us(1);                  /* wait for data ready */

  AES_START();

  for( i = 0; i < AES_BLOCK_LEN; i++ ) {    /* Writing the input data*/
    ENCDI =  *pbuf++;
  }

  zb_wait_us(1);                  /* wait for data ready */

  ZB_MEMSET(buf, 0, AES_BLOCK_LEN);
  for( i = 0; i < AES_BLOCK_LEN; i++ ) {    /* Reading the output data */
    *(pT + i) = buf[i] = ENCDO;
  }

  zb_free_buf(ptbuf);
}


/******************************************************************************
 * @fn      encrypt_trans
 *
 * @brief   Performs CCM encryption.
 *
 * @return  None
 *
 */

static void encrypt_trans(  /* Input */
                            zb_uint8_t   ccm_m,          /* Mval - Length of authentication field in octets [0,2,4,6,8,10,12,14 or 16] */
                            zb_uint8_t*  pKey,           /* Pointer to AES Key or Pointer to Key Expansion buffer */
                            zb_uint8_t*  pNonce,         /* Pointer to 13-byte Nonce */
                            zb_uint8_t*  pString_m,      /* Pointer to - Input: octet string 'm', Output: Encrypted octet string 'm' */
                            zb_uint16_t  string_m_len,   /* Length of M[] in octets */
                            /* Output */
                            zb_uint8_t*  tU) {           /* The first Mval bytes contain Encrypted Authentication Tag U */

  zb_uint8_t    A[AES_BLOCK_LEN], T[AES_BLOCK_LEN], *pbuf;
  zb_uint8_t    appendix, i, j, cb;
  zb_uint16_t   blocks;
  zb_buf_t*     ptbuf;

  ptbuf = zb_get_in_buf();

  ZB_MEMCPY(T, tU, ccm_m);

  A[0] = 1;              /* Flag: L=2, L-encoding = L-1 = 1 */
  ZB_MEMCPY((A + 1), pNonce, ZB_CCM_NONCE_LEN);   /* append Nonce */
  A[14] = A[15] = 0;     /* clear the CTR field */

  /* Calculate block sizes */
  blocks    = string_m_len >> 4;
  appendix  = string_m_len & 0x0f;
  if( appendix ) blocks++;

  /* Allocate memory for message buffer */
  ZB_BUF_INITIAL_ALLOC(ptbuf, (blocks * AES_BLOCK_LEN), pbuf);  /* Allocate buffer */

  if( !pbuf ) return;                             /* No free space available for operation */

  /* Move message into position and pad with zeros */
  ZB_MEMCPY(pbuf, pString_m, string_m_len);
  ZB_MEMSET((pbuf + string_m_len), 0, (AES_BLOCK_LEN - appendix));

  /* Set OFB mode and encrypt T to U */
  AES_SET_MODE(AES_MODE_OFB);
  zb_AESLdBlck(A, AES_LOAD_IV);   /* Load A0 as IV */

  AES_SET_OPERATION(AES_ENCRYPT);
  AES_START();

  for( j = 0; j < 4; j++ ) {
    for( i = 0; i < 4; i++ ) {    /* Writing the input data */
      ENCDI = T[i + (j * 4)];
    }
    for( i = 0; i < 4; i++ ) {    /* Reading the encrypted data */
      tU[i + (j * 4)] = ENCDO;
    }
  }
  zb_wait_us(1);                  /* wait for data ready */

  /* Switch to CTR mode to encrypt message. CTR field must be greater than zero */
  AES_SET_MODE(AES_MODE_CTR);
  A[15] = 1;

  zb_AESLdBlck(A, AES_LOAD_IV);
  AES_SET_OPERATION(AES_ENCRYPT);

  for( cb = 0; cb < blocks; cb++ ) {
    AES_START();
    for( j = 0; j < 4; j++ ) {
      for( i = 0; i < 4; i++ ) {    /* Writing the input data */
        ENCDI = *(pbuf + i + (j * 4) + (cb * AES_BLOCK_LEN));
      }

      for( i = 0; i < 4; i++ ) {    /* Reading the encrypted data */
        *(pbuf + i + (j * 4) + (cb * AES_BLOCK_LEN)) = ENCDO;
      }
    }
    zb_wait_us(1);                /* wait for data ready */
    while(!AES_RDY());
  }

  ZB_MEMCPY(pString_m, pbuf, string_m_len); /* Copy the encrypted message back to M */
  zb_free_buf(ptbuf);
}

/******************************************************************************
 * @fn      SSP_CCM_Decrypt
 *
 * @brief   Performs CCM decryption.
 *
 * @return  None
 *
 */

static void decrypt_trans( /* Input  */
                           zb_uint8_t   ccm_m,          /* Length of authentication field in octets [0,2,4,6,8,10,12,14 or 16] */
                           zb_uint8_t   *pKey,          /* Pointer to AES Key */
                           zb_uint8_t   *pNonce,        /* Pointer to Nonce  */
                           zb_uint16_t  lenString_c,    /* Length of C[] in octets   */
                           /* Output */
                           zb_uint8_t   *pString_c,     /* Pointer to octet string 'c', where 'c' = encrypted 'm' || encrypted auth tag U */
                           zb_uint8_t   *Cstate) {      /* Pointer AES state buffer (cannot be part of C[]) */

  zb_uint8_t   *pbuf;
  zb_uint8_t   A[AES_BLOCK_LEN], U[AES_BLOCK_LEN];
  zb_uint8_t   i, j, cb, lenString_m;
  zb_uint16_t  blocks;

  zb_buf_t*     ptbuf;

  ptbuf = zb_get_in_buf();

  A[0] = 1;                                   /* Flag: L = 2, L-encoding = L-1 = 1 */
  ZB_MEMCPY(A + 1, pNonce, ZB_CCM_NONCE_LEN); /* add Nonce */
  A[AES_BLOCK_LEN - 2] = A[AES_BLOCK_LEN - 1] = 0;  /* CTR = 0 */

  /* Separate M from C */
  lenString_m = lenString_c - ccm_m;
  blocks = lenString_m >> 4;

  if( lenString_m & 0x0f ) blocks++;    /* Have appendix */

  /* Extract U and pad it with zeros */
  ZB_MEMSET(U, 0, AES_BLOCK_LEN);
  ZB_MEMCPY(U, pString_c + lenString_m, ccm_m);

  /* Set OFB mode to encrypt U to T */
  AES_SET_MODE(AES_MODE_OFB);
  zb_AESLdBlck(A, AES_LOAD_IV);

  AES_SET_OPERATION(AES_ENCRYPT);

  /* Kick it off */
  AES_START();

  for( j = 0; j < (AES_BLOCK_LEN/4); j++ ) {
    for( i = 0; i < 4; i++ ) {    /* Writing the input data */
      ENCDI = U[i + (j * 4)];
    }

    for( i = 0; i < 4; i++ ) {    /* Reading the decrypted data */
      Cstate[i + (j * 4)] = ENCDO;
    }
  }
  zb_wait_us(1);          /* wait for data ready */

  /* Allocate memory for message buffer */
  ZB_BUF_INITIAL_ALLOC(ptbuf, (blocks * AES_BLOCK_LEN), pbuf);         /* Allocate buffer */
  if( !pbuf ) return;                             /* No free space available for operation */

  /* Place message to position and pad with zeros */
  ZB_MEMSET(pbuf, 0, blocks * AES_BLOCK_LEN);
  ZB_MEMCPY(pbuf, pString_c, lenString_m);

  /* Switch to CTR mode for decrypt message */
  AES_SET_MODE(AES_MODE_CTR);
  A[15] = 1;                /* CTR field must be none zero */

  zb_AESLdBlck(A, AES_LOAD_IV);
  AES_SET_OPERATION(AES_DECRYPT);

  for( cb = 0; cb < blocks; cb++ ) {
    AES_START();
    for( j = 0; j < (AES_BLOCK_LEN/4); j++ ) {
      for( i = 0; i < 4; i++ ) {    /* Writing the input data */
        ENCDI = *(pbuf + i + (j * 4) + (cb * AES_BLOCK_LEN));
      }

      for( i = 0; i < 4; i++ ) {    /* Reading the decrypted data */
        *(pbuf + i + (j * 4) + (cb * AES_BLOCK_LEN)) = ENCDO;
      }
    }
    zb_wait_us(1);                /* wait 1uS */
    while( !AES_RDY() );
  }

  /* Copy the decrypted message back to M and return allocated memory */
  ZB_MEMCPY(pString_c, pbuf, lenString_m);
  zb_free_buf(ptbuf);

  /* Copy T to where U used to be */
  ZB_MEMCPY(pString_c + lenString_m, Cstate, ccm_m);
}

/******************************************************************************
 * @fn      auth_check
 *
 * @brief   Verifies CCM authentication.
 *
 * @return  0 = Success, 1 = Failure
 *
 */

static zb_uint8_t auth_check( zb_uint8_t  ccm_m,      /* Length of authentication field in octets [0,2,4,6,8,10,12,14 or 16] */
                              zb_uint8_t* pNonce,     /* Pointer to 13-byte Nonce */
                              zb_uint8_t* pString_c,  /* Pointer to octet string 'c' = 'm' || auth tag T */
                              zb_uint16_t len_c,      /* Length of C[] in octets */
                              zb_uint8_t* A,          /* Pointer to octet string 'a' */
                              zb_uint16_t len_a,      /* Length of A[] in octets */
                              zb_uint8_t* pKey,       /* Pointer to AES Key or Pointer to Key Expansion buffer. */
                              zb_uint8_t* Cstate) {   /* Input:   Pointer to AES state buffer (cannot be part of C[])
                                                          Output:  The first Mval bytes contain computed Authentication Tag T */
  zb_uint8_t   i, t;
  zb_uint8_t   status = RET_OK;

  if( !ccm_m ) return RET_ERROR;  /* Check if authentication is even requested. */

  t = (len_c - ccm_m);

  auth_trans(ccm_m, pKey, pNonce, A, len_a, pString_c, t, Cstate);

  for( i = 0; i < ccm_m; i++ ) {
    if( Cstate[i] != pString_c[t++] ) {
      status = RET_ERROR;
      break;
    }
  }
  return(status);
}


zb_ret_t zb_ccm_encrypt( zb_uint8_t  ccm_m,            /* 'M' parameter - see table 4.38 (4 for Standard security - mode 5) */
                         zb_uint8_t* pKey,             /* 16-bytes key */
                         zb_uint8_t* pNonce,           /* nonce - see 4.5.2.2 "CCM nonce" for details */
                         zb_uint8_t* string_a,         /* 'a' parameter - authentication string -
                                                           NwkHeader||AuxuluaryHeader for Standard security.
                                                           See 4.3.1.1 for details */
                         zb_ushort_t string_a_len,     /* l(a) */
                         zb_uint8_t* string_m,         /* 'm' parameter - text data - Payload for
                                                           Standard security. See 4.3.1.1 for details */
                         zb_ushort_t string_m_len,     /* l(m) */
                         zb_buf_t*   crypted_text) {   /* encryption result - user supplied buffer. Must have size
                                                          string_m_len + ccm_m rounded to 16.
                                                          Result len is always string_m_len + ccm_m. */

  zb_uint8_t  t[16];
  zb_uint8_t *c_text;


  zb_RfSecurityInit(pKey);

  /* A.2.2 Authentication Transformation */
  /* First ccm_m bytes of t now holds tag T */
  auth_trans(ccm_m,
             pKey,
             pNonce,
             string_a,
             string_a_len,
             string_m,
             string_m_len,
             t);

  ZB_BUF_INITIAL_ALLOC(crypted_text, string_a_len + string_m_len + ccm_m, c_text);

#if 0
  /* A.2.3Encryption Transformation */
  encrypt_trans(ZB_CCM_M, key, nonce, string_m, string_m_len, t, c_text + string_a_len);
  ZB_MEMCPY(ZB_BUF_BEGIN(crypted_text), string_a, string_a_len);
#endif

  /* A.2.3 Encryption Transformation */
  encrypt_trans(ccm_m,
                pKey,
                pNonce,
                string_m,
                string_m_len,
                t);


  ZB_MEMCPY(c_text, string_a, string_a_len);                    /* a[]  */
  ZB_MEMCPY(c_text + string_a_len, string_m, string_m_len);     /* Encrypted message  */
  ZB_MEMCPY((c_text + string_a_len + string_m_len), t, ccm_m);  /* Authentication tag */

  return(RET_OK);
}


zb_ret_t
zb_ccm_encrypt_n_auth( zb_uint8_t*  key,
                       zb_uint8_t*  nonce,
                       zb_uint8_t*  string_a,
                       zb_ushort_t  string_a_len,
                       zb_uint8_t*  string_m,
                       zb_ushort_t  string_m_len,
                       zb_buf_t*    crypted_text) {
                         
  return( zb_ccm_encrypt( ZB_CCM_M,
                          key,
                          nonce,
                          string_a,
                          string_a_len,
                          string_m,
                          string_m_len,
                          crypted_text));
}



/**
   CCM* decryption and authentication procedure for Standard
   security
 */
/**
   CCM* decryption and authentication procedure for Standard security

   @param ccm_m     - 'M' parameter - see table 4.38 (4 for Standard security - mode 5)
   @param key       - 16-bytes key
   @param nonce     - 16-bytes nonce - see 4.5.2.2 "CCM nonce" for details
   @param string_a  - 'a' parameter - authentication string -
                     NwkHeader||AuxuluaryHeader for Standard security. See 4.3.1.1 for details
   @param string_a_len - l(a)
   @param string_c  - 'c' parameter - authentication string -
                      Payload for Standard security. See 4.3.1.1 for details
   @param string_c_len - l(c)
   @param decrypted_text - encryption result - user supplied buffer. Must have size
          string_m_len + ccm_m rounded to 16.
          Result len is always string_m_len - ccm_m.

   @return RET_OK if success, RET_ERROR in case of error (authentication failure)
 */

zb_ret_t zb_ccm_decrypt(zb_ushort_t ccm_m,           /* 'M' parameter - see table 4.38 (4 for Standard security - mode 5) */
                        zb_uint8_t*  pKey,           /* Pointer to AES Key */
                        zb_uint8_t*  pNonce,         /* Pointer to Nonce  */
                        zb_buf_t*    buf,            /*  */
                        zb_ushort_t  string_a_len,   /* l(a) */
                        zb_ushort_t  string_c_len) { /* Length of C[] in octets   */

  zb_uint8_t* c_text;
  zb_uint8_t  c[16];
  zb_uint8_t  ret;

  zb_RfSecurityInit(pKey);

  decrypt_trans(  ccm_m,
                  pKey,
                  pNonce,
                  string_c_len,
                  (ZB_BUF_BEGIN(buf) + string_a_len),   /* Pointer to octet string 'c', where 'c' = encrypted 'm' || encrypted auth tag U */
                  c);                                   /* Pointer AES state buffer (cannot be part of C[]) */

  ret =  auth_check( ccm_m,
                     pNonce,
                     (ZB_BUF_BEGIN(buf) + string_a_len),
                     string_c_len,
                     ZB_BUF_BEGIN(buf),
                     string_a_len,
                     pKey,
                     c);

  buf -> u.hdr.len -= ZB_CCM_M;

  return( ret );
}

zb_ret_t
zb_ccm_decrypt_n_auth(zb_uint8_t  *pKey,          /* Pointer to AES Key */
                      zb_uint8_t  *pNonce,        /* Pointer to Nonce  */
                      zb_buf_t    *buf,
                      zb_ushort_t string_a_len,
                      zb_ushort_t string_c_len) {  /* Length of C[] in octets   */
 
  return(zb_ccm_decrypt(ZB_CCM_M,
                        pKey,           /* Pointer to AES Key */
                        pNonce,         /* Pointer to Nonce  */
                        buf,
                        string_a_len,
                        string_c_len));
}

/**
   Decryption and authentication according to standard security requirements.

   Decrypt "on place". Use HW if possible,
 */

#if 0
zb_ret_t
zb_ccm_decrypt_n_auth_stdsecur(
  zb_uint8_t  *key,
  zb_uint8_t  *nonce,
  zb_buf_t    *buf,
  zb_ushort_t string_a_len,
  zb_ushort_t string_m_len)
{

  zb_ret_t ret;

  ret = zb_ccm_decrypt_n_auth(key,
                              nonce,
                              buf, string_a_len,
                              string_m_len);

  return ret;
}
#endif


#endif  /* ZB_SECURITY */
#endif /* ZB_CC25XX */
/*! @} */




