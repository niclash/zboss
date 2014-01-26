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
PURPOSE: Network simulator. Zigbee stuff.
*/

#include "mac_frame.h"
#include <string.h>
#include <map>
#include <iostream>


using namespace std;

/* Got from https://github.com/chrishulbert/crypto/blob/master/c/c_aes.c
   Original (c):

// Simple, thoroughly commented implementation of 128-bit AES / Rijndael using C
// Chris Hulbert - chris.hulbert@gmail.com - http://splinter.com.au/blog
// References:
// http://en.wikipedia.org/wiki/Advanced_Encryption_Standard
// http://en.wikipedia.org/wiki/Rijndael_key_schedule
// http://en.wikipeia.org/wiki/Rijndael_mix_columns
// http://en.wikipedia.org/wiki/Rijndael_S-box
// This code is public domain, or any OSI-approved license, your choice. No warranty.

*/

// Here are all the lookup tables for the row shifts, rcon, s-boxes, and galois field multiplications
static uint8_t shift_rows_table[] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
static uint8_t lookup_rcon[]={0x8d,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,0x6c,0xd8,0xab,0x4d,0x9a};
static uint8_t lookup_sbox[]={0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16};
static uint8_t lookup_g2 []={0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5};
static uint8_t lookup_g3 []={0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a};

// Xor's all elements in a n uint8_t array a by b
static void doxor(uint8_t *a, uint8_t *b, int n)
{
  int i;
  for (i=0;i<n;i++)
    a[i] ^= b[i];
}

// Xor the current cipher state by a specific round key
static void xor_round_key(uint8_t *state, uint8_t *keys, int round) {
  doxor(state,keys+round*16,16);
}

// Apply and reverse the rijndael s-box to all elements in an array
// http://en.wikipedia.org/wiki/Rijndael_S-box
static void sub_bytes(uint8_t *a,int n) {
  int i;
  for (i=0;i<n;i++)
    a[i] = lookup_sbox[a[i]];
}


// Perform the core key schedule transform on 4 bytes, as part of the key expansion process
// http://en.wikipedia.org/wiki/Rijndael_key_schedule#Key_schedule_core
static void key_schedule_core(uint8_t *a, int i) {
  uint8_t temp = a[0]; // Rotate the output eight bits to the left
  a[0]=a[1];
  a[1]=a[2];
  a[2]=a[3];
  a[3]=temp;
  sub_bytes(a,4); // Apply Rijndael's S-box on all four individual bytes in the output word
  a[0]^=lookup_rcon[i]; // On just the first (leftmost) uint8_t of the output word, perform the rcon operation with i
                        // as the input, and exclusive or the rcon output with the first uint8_t of the output word
}

// Expand the 16-uint8_t key to 11 round keys (176 bytes)
// http://en.wikipedia.org/wiki/Rijndael_key_schedule#The_key_schedule
static void expand_key(uint8_t *key, uint8_t *keys) {
  int bytes=16; // The count of how many bytes we've created so far
  int i=1; // The rcon iteration value i is set to 1
  int j; // For repeating the second stage 3 times
  uint8_t t[4]; // Temporary working area known as 't' in the Wiki article
  memcpy(keys,key,16); // The first 16 bytes of the expanded key are simply the encryption key

  while (bytes<176) { // Until we have 176 bytes of expanded key, we do the following:
    memcpy(t,keys+bytes-4,4); // We assign the value of the previous four bytes in the expanded key to t
    key_schedule_core(t, i); // We perform the key schedule core on t, with i as the rcon iteration value
    i++; // We increment i by 1
    doxor(t,keys+bytes-16,4); // We exclusive-or t with the four-uint8_t block 16 bytes before the new expanded key.
    memcpy(keys+bytes,t,4); // This becomes the next 4 bytes in the expanded key
    bytes+=4; // Keep track of how many expanded key bytes we've added

    // We then do the following three times to create the next twelve bytes
    for (j=0;j<3;j++) {
      memcpy(t,keys+bytes-4,4); // We assign the value of the previous 4 bytes in the expanded key to t
      doxor(t,keys+bytes-16,4); // We exclusive-or t with the four-uint8_t block n bytes before
      memcpy(keys+bytes,t,4); // This becomes the next 4 bytes in the expanded key
      bytes+=4; // Keep track of how many expanded key bytes we've added
    }
  }
}

// Apply / reverse the shift rows step on the 16 byte cipher state
// http://en.wikipedia.org/wiki/Advanced_Encryption_Standard#The_ShiftRows_step
static void shift_rows(uint8_t *state) {
  int i;
  uint8_t temp[16];
  memcpy(temp,state,16);
  for (i=0;i<16;i++)
    state[i]=temp[shift_rows_table[i]];
}

// Perform the mix columns matrix on one column of 4 bytes
// http://en.wikipedia.org/wiki/Rijndael_mix_columns
static void mix_col (uint8_t *state) {
  uint8_t a0 = state[0];
  uint8_t a1 = state[1];
  uint8_t a2 = state[2];
  uint8_t a3 = state[3];
  state[0] = lookup_g2[a0] ^ lookup_g3[a1] ^ a2 ^ a3;
  state[1] = lookup_g2[a1] ^ lookup_g3[a2] ^ a3 ^ a0;
  state[2] = lookup_g2[a2] ^ lookup_g3[a3] ^ a0 ^ a1;
  state[3] = lookup_g2[a3] ^ lookup_g3[a0] ^ a1 ^ a2;
}

// Perform the mix columns matrix on each column of the 16 bytes
static void mix_cols (uint8_t *state) {
  mix_col(state);
  mix_col(state+4);
  mix_col(state+8);
  mix_col(state+12);
}


static void xor16(uint8_t *v1, uint8_t *v2, uint8_t *result)
{
  int i;
  for (i = 0 ; i < 16 ; ++i)
  {
    result[i] = v1[i] ^ v2[i];
  }
}

// Encrypt a single 128 bit block by a 128 bit key using AES
// http://en.wikipedia.org/wiki/Advanced_Encryption_Standard
static void aes128(uint8_t *key, uint8_t *msg, uint8_t *c)
{
  int i; // To count the rounds

  // Key expansion
  uint8_t keys[176];
  expand_key(key,keys);

  // First Round
  memcpy(c, msg, 16);
  xor_round_key(c,keys,0);

  // Middle rounds
  for(i=0; i<9; i++) {
    sub_bytes(c,16);
    shift_rows(c);
    mix_cols(c);
    xor_round_key(c, keys, i+1);
  }

  // Final Round
  sub_bytes(c,16);
  shift_rows(c);
  xor_round_key(c, keys, 10);
}

/**
   A.2.3Encryption Transformation
*/
static void encrypt_trans(
  int ccm_m,
  uint8_t *key,
  uint8_t *nonce,
  uint8_t *string_m,
  uint16_t string_m_len,
  uint8_t *t,
  uint8_t *encrypted)
{
  uint8_t eai[16];
  uint8_t ai[16];
  uint16_t i;
  int len;
  uint8_t *p;
  uint8_t *pe;

  p = string_m;
  pe = encrypted;
  len = string_m_len;
  i = 1;
  while (len >= 16)
  {
    /* form Ai */
    ai[0] = (ZB_CCM_L - 1);
    memcpy(&ai[1], nonce, ZB_CCM_NONCE_LEN);
    ZB_HTOBE16(&ai[ZB_CCM_NONCE_LEN+1], &i);

    /* E(key, Ai) */
    aes128(key, ai, eai);
    /* Ci =  E(key, Ai) ^ Mi */
    xor16(eai, p, pe);

    p += 16;
    pe += 16;
    len -= 16;
    i++;
  }
  if (len)
  {
    /* process the rest with padding */

    /* form Ai */
    ai[0] = (ZB_CCM_L - 1);
    memcpy(&ai[1], nonce, ZB_CCM_NONCE_LEN);
    ZB_HTOBE16(&ai[ZB_CCM_NONCE_LEN+1], &i);

    /* E(key, Ai) */
    aes128(key, ai, eai);
    /* Ci =  E(key, Ai) ^ Mi. Use ai as buffer */
    memset(ai, 0, 16);
    memcpy(ai, p, len);
    xor16(eai, ai, pe);
    pe += len;
  }

  /* Now pe points to the encrypted data end, without padding */

  /* A0 */
  i = 0;
  ai[0] = (ZB_CCM_L - 1);
  memcpy(&ai[1], nonce, ZB_CCM_NONCE_LEN);
  ZB_HTOBE16(&ai[ZB_CCM_NONCE_LEN+1], &i);

  /* Define the 16-octet encryption block S0 by S0 = E(Key, A0) */
  /* store S0 in eai */
  aes128(key, ai, eai);

  /*
    The encrypted authentication tag U is the result of XOR-ing the string
    and the authentication tag T.
    consisting of the leftmost M octets of S0
  */
  memset(ai, 0, 16);
  memcpy(ai, t, ccm_m);
  xor16(eai, ai, pe);
}

/**
   Auth transformation from A.2.2

   Used in both IN and OUT time.

   @param t - (out) T from A.2.2 - procedure result - 16-butes byffer, its actual
            length is ccm_m.
 */
static void auth_trans(
  int ccm_m,
  uint8_t *key,
  uint8_t *nonce,
  uint8_t *string_a,
  uint16_t string_a_len,
  uint8_t *string_m,
  uint16_t string_m_len,
  uint8_t *t)
{
  uint8_t *p;
  int len;
  uint8_t tmp[16];
  uint8_t b[16];

  /* A.2.2Authentication Transformation */

  /*
    1 Form the 1-octet Flags field
     Flags = Reserved || Adata || M || L
    2 Form the 16-octet B0 field
     B0 = Flags || Nonce N || l(m)
  */

  b[0] = (1<<6) | (ZB_CCM_L - 1) | (((ccm_m - 2)/2) << 3);
  memcpy(&b[1], nonce, ZB_CCM_NONCE_LEN);
  ZB_HTOBE16(&b[ZB_CCM_NONCE_LEN+1], &string_m_len);

  /* Execute CBC-MAC (CBC with zero initialization vector) */

  /* iteration 0: E(key, b ^ zero). result (X1) is in t. */
  aes128(key, b, t);

  /* A.2.1Input Transformation */
  /*
    3 Form the padded message AddAuthData by right-concatenating the resulting
    string with the smallest non-negative number of all-zero octets such that the
    octet string AddAuthData has length divisible by 16

    Effectively done by memset(0)
   */
  /*
    1 Form the octet string representation L(a) of the length l(a) of the octet string a:
  */
  ZB_HTOBE16(tmp, &string_a_len);
  /* 2 Right-concatenate the octet string L(a) and the octet string a itself: */
  if (string_a_len <= 16 - 2)
  {
    memset(&tmp[2], 0, 14);
    memcpy(&tmp[2], string_a, string_a_len);
    len = 0;
  }
  else
  {
    memcpy(&tmp[2], string_a, 14);
    len = string_a_len - 16;
  }

  /* iteration 1: L(a) || a in tmp  */
  xor16(t, tmp, b);
  aes128(key, b, t);

  /* next iterations with a */
  p = string_a + 16 - 2;
  /* while no padding necessary */
  while (len >= 16)
  {
    xor16(t, p, b);
    aes128(key, b, t);
    p += 16;
    len -= 16;
  }
  if (len)
  {
    /* rest with padding */
    memset(tmp, 0, 16);
    memcpy(tmp, p, len);
    xor16(t, tmp, b);
    aes128(key, b, t);
  }

  /* done with a, now process m */
  len = string_m_len;
  p = string_m;
  while (len >= 16)
  {
    xor16(t, p, b);
    aes128(key, b, t);
    p += 16;
    len -= 16;
  }
  if (len)
  {
    /* rest with padding */
    memset(tmp, 0, 16);
    memcpy(tmp, p, len);
    xor16(t, tmp, b);
    aes128(key, b, t);
  }

  /* return now. In first ccm_m bytes of t is T */
}

static int
ccm_decrypt_n_auth(
  int ccm_m,
  uint8_t *key,
  uint8_t *nonce,
  uint8_t *string_a, uint16_t string_a_len,
  uint8_t *string_c, uint16_t string_c_len,
  uint8_t *decrypted_text)
{
  uint8_t t[16];
  int decrypted_text_len;

  /* A.3.1Decryption Transformation */

  /*
    2 Form the padded message CiphertextData by right-concatenating the string C
    with the smallest non-negative number of all-zero octets such that the octet
    string CiphertextData has length divisible by 16.

    3 Use the encryption transformation in sub-clause A.2.3, with the data
    CipherTextData and the tag U as inputs.
   */
  encrypt_trans(ccm_m, key, nonce,
                string_c,
                string_c_len - ccm_m,
                /* 1 Parse the message c as C ||U, where the rightmost string U is an M-octet string.  */
                string_c + string_c_len - ccm_m, /* U */
                decrypted_text);
  decrypted_text_len = string_c_len - ccm_m;

  /*
    1 Form the message AuthData using the input transformation in sub-
    clauseA.2.1, with the string a and the octet string m that was established in
    sub-clauseA.3.1 (step4) as inputs.

    2 Use the authentication transformation in sub-clauseA.2.2, with the message
    AuthData as input.
   */
  auth_trans(ccm_m, key, nonce,
             string_a, string_a_len,
             decrypted_text, decrypted_text_len, t);

  if (!memcmp(decrypted_text + decrypted_text_len, t, ccm_m))
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

static int decrypt_nwk_command(nwk_hdr_t *nwk_hdr,
                               int nwk_pkt_size,
                               int nwk_hdr_size_wo_aux,
                               uint8_t *out_buf,
                               uint8_t **frame_payload_p)
{
  int ret;
  uint8_t key[16] = { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0};
  nwk_aux_frame_hdr_t *aux = (nwk_aux_frame_hdr_t *)((uint8_t *)nwk_hdr + nwk_hdr_size_wo_aux);
  unsigned hdr_size = nwk_hdr_size_wo_aux + sizeof(*aux);
  secur_ccm_nonce_t nonce;

#define NWK_STD_SECUR_CONTROL \
  ( 5 /*security level */ | (1 << 3) /* key identifier - NWK key */ | 1 << 5 /* ext nonce */)
#define NWK_STD_SECUR_CONTROL_ZEROED_LEVEL \
  ( 0 /*security level */ | (1 << 3) /* key identifier */ | 1 << 5 /* ext nonce */)


  aux->secur_control = NWK_STD_SECUR_CONTROL;
  memcpy(nonce.source_address, aux->source_address, 8);
  nonce.frame_counter = aux->frame_counter;
  nonce.secur_control = aux->secur_control;

  ret = ccm_decrypt_n_auth(4,
                           key,
                           (uint8_t *)&nonce,
                           (uint8_t *)nwk_hdr, (uint16_t)hdr_size,
                           (uint8_t *)nwk_hdr + hdr_size, (uint16_t)(nwk_pkt_size - hdr_size),
                           out_buf);
  aux->secur_control = NWK_STD_SECUR_CONTROL_ZEROED_LEVEL;
  *frame_payload_p = out_buf;
  return ret;
}

/**
   Translate network address to the node number.

   @return node number of 0xffff if no address in the translation table
 */
uint16_t AddrTranslate(map<int, int> &tt, uint16_t nwk_addr)
{
  if ( tt.find(nwk_addr) !=  tt.end() )
  {
    return tt[nwk_addr];
  }
  else
  {
    return 0xffff;
  }
}

/**
   Add or update nwk address -> node# mapping
 */
void AddrUpdate(map<int,int> &tt, uint16_t n_node, uint16_t nwk_addr)
{
  cout << "AddrUpdate " << (int)n_node << " <--> " << (int)nwk_addr << endl;
  tt[nwk_addr] = n_node;
}


map<int,int> create_map()
{
  map<int,int> m;
  /* node #0 is always a coordinator and has 0 address */
  m[0] = 0;
  return m;
}

int route_packet(uint8_t *raw_packet, uint16_t *nNode)
{
  int ret = 0;
  // packet format: length (1 byte), packet
  mac_frame_format_16bit_addr_t mf;
  static map<int,int> translate_table = create_map();
  uint8_t *p;
  void *vv;
  uint16_t *pp;




#if defined ZB_LITTLE_ENDIAN && defined ZB_BIG_ENDIAN
#error Both little & big endian are defined!!!
#endif

  get_mac_frame_format_16bit_addr(&raw_packet[1], &mf);
  vv = &mf;
  pp = (uint16_t*)vv;

  *nNode = 0xffff; // broadcast by default
  cout << "mf.frame_control.dest_addr_mode " << mf.frame_control.dest_addr_mode << "  all " << *pp << endl;
  cout << "frame " << mf.frame_control.frame_type << " " << mf.frame_control.security_enabled
     << " " << mf.frame_control.frame_pending << " " << mf.frame_control.ack_request 
     << " " << mf.frame_control.pan_id_compression << " " << mf.frame_control.reserved
     << " " << mf.frame_control.dest_addr_mode << " " << mf.frame_control.frame_version
     << " " << mf.frame_control.source_addr_mode << endl;
     
  cout << "addr " << mf.addr.dest_pan_id << " " << mf.addr.dest_addr << " "
    << mf.addr.source_pan_id << " " << mf.addr.source_addr << endl;
		    
  switch ( mf.frame_control.dest_addr_mode )
  {
    case 0:                   // no address: ack or beacon - it is broadcast for us.
      break;

    case 2:                   // 16 bit address
      if ( raw_packet[0] < sizeof(mac_frame_format_16bit_addr_t) )
      {
        cout << "Bad MAC packet, skip it" << endl;
        ret = -1;
        goto done;
      }

      if ( mf.addr.dest_addr != 0xffff )
      {
        // use nwk-node translation table
        *nNode = AddrTranslate(translate_table, mf.addr.dest_addr);

        if ( *nNode == 0xffff )
        {
          // check for Rejoin response
          p = (raw_packet + 1 +                                   // 1 - length byte
                          2                                                 // fc
                          + 1                                               // seq number
                          + (mf.frame_control.dest_addr_mode == 0 ? 0 : 2) // dest pan id
                          + (mf.frame_control.source_addr_mode == 0 ? 0 : (mf.frame_control.source_addr_mode == 2 ? 2 : 8)) // src address
                          + (mf.frame_control.pan_id_compression || mf.frame_control.source_addr_mode == 0 ? 0 : 2)         // src pan id
                          + (mf.frame_control.dest_addr_mode == 0 ? 0 : (mf.frame_control.dest_addr_mode == 2 ? 2 : 8))     // dst address
                          // mac security is impossible
              );

          nwk_hdr_t nwk_hdr;
          get_nwk_hdr(p, &nwk_hdr);
		      
          uint8_t *frame_payload = ( (uint8_t *)p
                                     + 2 // fc
                                     + 2 // destination addr
                                     + 2 // source addr
                                     + 1 // radius
                                     + 1 // sequence number
                                     + (nwk_hdr.frame_control.dest_ieee_addr ? 8 : 0)  // full dst address
                                     + (nwk_hdr.frame_control.src_ieee_addr ? 8 : 0)  // full src address
                                     // + multicast is not supported
                                     // + source route is not supported
            );

          cout << "Got DATA packet command frame: " << nwk_hdr.frame_control.frame_type << " command: " << frame_payload[0] << endl;

          // We need Rejoin Response.
          // For "13.21 TP/PRO/BV-15 Authentication of joining devices" Rejoin
          // Response is encrypted, key is fixed, so we can decrypt it.
          if ( nwk_hdr.frame_control.frame_type == 0x1 ) // command frame
          {
            uint8_t frame_buf[128];
            if ( nwk_hdr.frame_control.security )
            {
              ret = decrypt_nwk_command((nwk_hdr_t*)p,
                                        raw_packet[0] - ((uint8_t *)&nwk_hdr - &raw_packet[1]) - 1/*len*/ - 2/*fcs*/,
                                        frame_payload - (uint8_t*)&nwk_hdr,
                                        frame_buf, &frame_payload);
              cout << "decrypted nwk cmd ret " << ret << endl;
            }

            if ( ret == 0
                 && (frame_payload[0] == 0x07) // rejoin response
                 && (frame_payload[3] == 0x00) // rejoin successful
                 && nwk_hdr.frame_control.dest_ieee_addr ) // full addr
            {
              uint16_t addr;
	      p = &frame_payload[1];
	      addr = FROM_LITTLE_ENDIAN(p);
              cout << "Got rejoin response laddr " << " " << (int)nwk_hdr.dst_ieee_addr[0] << " nwk addr " << (int)addr << cout;
              AddrUpdate(translate_table, nwk_hdr.dst_ieee_addr[0], addr);

              // Use allocated address, not address from MHR: it is invalid now
              *nNode = AddrTranslate(translate_table, addr);
            }
          }
        }

        if ( *nNode == 0xffff )
        {
          cout << "Can't find address " << mf.addr.dest_addr << " in the translation table - skip this frame" << endl;
          ret = -1;
          goto done;
        }
      }
      break;

    case 3:                   // 64-bit address
      // 64-bit address is MAC - nNode in our case (in bytes 0 and 1). Use it as is.
      mac_frame_format_64bit_addr_t laddr;
      get_mac_frame_format_64bit_addr(&raw_packet[1], &laddr);
      *nNode = laddr.addr.dest_addr[0];
      break;
  }


  cout << "nNode " << *nNode << endl;

  // parse special frame types:
  // beacons - to detect ZC. Also, node #0 is ZC by default.
  // MLME-ASSOCIATE.response - to get nNode - nwk address relation.

  // check packet type: unicast/broadcast/beacon
  switch (mf.frame_control.frame_type)
  {
    case 3:                   // command
    {
      // check for MLME-ASSOCIATE.response
      uint8_t *frame_payload =
        (raw_packet + 1 +                                    // 1 - length byte
         (2                                              // fc
          + 1                                          // seq number
          + (mf.frame_control.dest_addr_mode == 0 ? 0 : 2) // dest pan id
          + (mf.frame_control.source_addr_mode == 0 ? 0 : (mf.frame_control.source_addr_mode == 2 ? 2 : 8)) // src address
          + (mf.frame_control.pan_id_compression || mf.frame_control.source_addr_mode == 0 ? 0 : 2)         // src pan id
          + (mf.frame_control.dest_addr_mode == 0 ? 0 : (mf.frame_control.dest_addr_mode == 2 ? 2 : 8)) // dst address
          // do not support security header yet
             )
          );

      if (frame_payload[0] == 2 // associate response
          && frame_payload[3] == 0) // status is ok
      {
        uint8_t *p = frame_payload + 1;
        uint16_t addr = FROM_LITTLE_ENDIAN(p);
        if (mf.frame_control.dest_addr_mode == 3) // dest - 64 bit (probably,
          // it will be so always)
        {
          mac_frame_format_64bit_addr_t laddr;
          get_mac_frame_format_64bit_addr(&raw_packet[1], &laddr);
          cout << "Got associate response laddr " << (int)laddr.addr.dest_addr[0] << " " << (int)laddr.addr.dest_addr[0] << " nwk addr " << (int)addr << " " << endl;
          AddrUpdate(translate_table, laddr.addr.dest_addr[0], addr);
        }
      }
    }
    break;

    case 1:             // Data
    {
      /**
       * Add AddrUpdate for Interpan from dev_annce command
       */
      uint8_t *frame_payload =
        (raw_packet + 1 +                                    // 1 - length byte
         (2                                              // fc
          + 1                                          // seq number
          + (mf.frame_control.dest_addr_mode == 0 ? 0 : 2) // dest pan id
          + (mf.frame_control.source_addr_mode == 0 ? 0 : (mf.frame_control.source_addr_mode == 2 ? 2 : 8)) // src address
          + (mf.frame_control.pan_id_compression || mf.frame_control.source_addr_mode == 0 ? 0 : 2)         // src pan id
          + (mf.frame_control.dest_addr_mode == 0 ? 0 : (mf.frame_control.dest_addr_mode == 2 ? 2 : 8)) // dst address
          // do not support security header yet
             )
          );

      nwk_frame_control_t nwk;
      get_nwk_frame_control(frame_payload, &nwk);
      if (nwk.frame_type==0 &&    // data
          nwk.security==0 && // do not support security header yet
          nwk.multicast==0 &&
          nwk.source_route==0)
      {
        aps_frame_control_t aps;
	uint8_t *p_cluster;
	uint8_t *p_profile;

        frame_payload +=
          (2                                     // fc
            + 2                                  // dest
            + 2                                  // src
            + 1                                  // radius
            + 1                                  // seq number
            + (nwk.dest_ieee_addr == 0 ? 0 : 8) // dest addr
            + (nwk.src_ieee_addr == 0 ? 0 : 8)  // src addr
            // do not support security header yet
          );
	  
	p_cluster = (frame_payload+2);
	p_profile = (frame_payload+4);

	get_aps_frame_control(frame_payload, &aps);

        if (aps.frame_type==0 &&       // data
            aps.delivery_mode==2 &&    // broadcast
            aps.security==0 &&         // do not support security header yet
            aps.extended_header==0 &&  // do not support extended header yet

            frame_payload[1]==0 &&      // endpoint
            FROM_LITTLE_ENDIAN(p_cluster) == 0x0013 &&     // cluster
            FROM_LITTLE_ENDIAN(p_profile) == 0x0000 &&     // profile
            frame_payload[6]==0         // source endpoint
            )
        {
          uint16_t addr;

          frame_payload +=
            (1                   // fc
              + 1                // endpoint
              + 2                // cluster
              + 2                // profile
              + 1                // src endpoint
              + 1                // counter
            );

          p = frame_payload + 1;
          addr = FROM_LITTLE_ENDIAN(p);
          //if (AddrTranslate(translate_table, addr) == 0xffff)	// Atantion! NS test may reuse short addr !!!
          {
            cout << "Got associate response laddr " << (int)frame_payload[3] << " nwk addr " << (int)addr << endl;
            AddrUpdate(translate_table, (int)frame_payload[3], addr);
          }
        }
      }

      if (nwk.frame_type==3 &&    // interpan
          nwk.protocol_version==2 &&
          nwk.security==0 && // do not support security header yet
          nwk.multicast==0 &&
          nwk.source_route==0 &&
          // has been ieee dest addr
          mf.frame_control.dest_addr_mode==3
          )
      {
        frame_payload +=
          (2                                     // fc
            + 7                                  // go to interpan cmd
          );

        if(*frame_payload == 0x12)  //ZB_ZLL_CMD_COMMISSIONING_NETWORK_JOIN_ROUTER_REQ
        {
	  uint8_t *paddr = (uint8_t *)frame_payload + 34;// goto target short address;
          uint16_t addr = FROM_LITTLE_ENDIAN(paddr);

          //if (AddrTranslate(translate_table, addr) == 0xffff)
          {
            uint8_t *p_ieee_addr =
              (raw_packet + 1 +                                    // 1 - length byte
               (2                                              // fc
                + 1                                          // seq number
                + (mf.frame_control.dest_addr_mode == 0 ? 0 : 2) // dest pan id
                // do not support security header yet
               )
             );

            cout << "Got Interpan network join addr " << (int)p_ieee_addr[0] << " nwk addr " << (int)addr << endl;
            AddrUpdate(translate_table, (int)p_ieee_addr[0], addr);
          }
        }
      }

      if (nwk.frame_type==3 &&    // interpan
//          nwk.protocol_version==2 &&
//          nwk.security==1 && //
//          nwk.multicast==0 &&
//          nwk.source_route==0 &&
          // has been ieee dest addr
          mf.frame_control.dest_addr_mode==3
          )
      {
        cout << "our case! ";
        cout << "frame payload " << hex << (int)*frame_payload << "\n";
	cout << "mf.frame_control.dest_addr_mode = " << mf.frame_control.dest_addr_mode << "\n";

        if(*frame_payload == 0x10)  // my payload
        {
          p = (uint8_t *)frame_payload + 33;
/*                             + 1
                             + 1 //skip command id
                             + 4 //transaction id
                             + 8 //ext pan id
                             + 1 //key index
                             + 16 //nwk key
                             + 1 //channel
                             + 2; //pan id
*/
          uint16_t addr = FROM_LITTLE_ENDIAN(p);

          //if (AddrTranslate(translate_table, addr) == 0xffff)
          {
            if (mf.frame_control.dest_addr_mode == 3)
            {
              uint8_t *p_ieee_addr =
                (raw_packet + 1 +                          // 1 - length byte 
                  (2                                       // fc
                   + 1                                     // seq number
                   + (mf.frame_control.dest_addr_mode == 0 ? 0 : 2) // dest pan id
//                   + (mf.frame_control.source_addr_mode == 0 ? 0 : (mf.frame_control.source_addr_mode == 2 ? 2 : 8)) // src address
                   + (mf.frame_control.pan_id_compression || mf.frame_control.source_addr_mode == 0 ? 0 : 2)         // src pan id
//                 + (mf.frame_control.dest_addr_mode == 0 ? 0 : (mf.frame_control.dest_addr_mode == 2 ? 2 : 8)) // dst address
                  // do not support security header yet
                  )
                );

              cout << "Got Interpan network join addr " << (int)p_ieee_addr[0] << " nwk addr " << (int)addr << endl;
              AddrUpdate(translate_table, (int)p_ieee_addr[0], addr);

            }
          }
        }
      }

    }
    break;
  } // switch

  done:
  return ret;
}
