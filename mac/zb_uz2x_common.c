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
PURPOSE: ubec uz2400 and 2410 common code
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_ubec24xx.h"
#include "zb_types.h"
#include "zb_config.h"
#include "zb_secur.h"
#include "mac_internal.h"


/*! \addtogroup ZB_MAC */
/*! @{ */

#if defined ZB_UZ2400 || defined ZB_UZ2410


void zb_uz24_mask_short_reg(zb_uint8_t reg, zb_uint8_t mask)
{
  ZB_READ_SHORT_REG(reg);
  ZB_MAC_GET_BYTE_VALUE() &= mask;
  ZB_WRITE_SHORT_REG(reg, ZB_MAC_GET_BYTE_VALUE());
}


void zb_uz24_or_mask_short_reg(zb_uint8_t reg, zb_uint8_t mask)
{
  ZB_READ_SHORT_REG(reg);
  ZB_MAC_GET_BYTE_VALUE() |= mask;
  ZB_WRITE_SHORT_REG(reg, ZB_MAC_GET_BYTE_VALUE());
}

#if 0
void zb_uz_write_regs_array(zb_uz_reg_wr_str_t *arr, zb_uint8_t n)
{
  zb_uz_reg_wr_str_t *a = &arr[0];

  while (n--)
  {
    if (a->reg > 0xff)
    {
      ZB_WRITE_LONG_REG(a->reg, a->val);
    }
    else
    {
      ZB_WRITE_SHORT_REG((zb_uint8_t)a->reg, a->val);
    }
    a++;
  }
}
#endif

zb_bool_t zb_check_beacon_mode_on()
{
  zb_bool_t ret = ZB_FALSE;
  zb_uint8_t val = 0;
/*
  Beacon mode on/off SREG0x11[5] (DS-2400 4.5.1. Beacon Mode Setting)
  Coordinator mode on/off SREG0x00[3] (DS-2400 4.5.1. Beacon Mode Setting)
  Beacon interrupt mask on/off SREG0x25[7] (DS-2400 4.5.1. Beacon Mode Setting)

  SREG0x10 ORDER
  Bit 7-4 BO: Beacon order for how often coordinator transmit a beacon frame.
  Bit 3-0 SO: Superframe order, specifies the length of the active portion of the superframe including the beacon frame.
  0<=SO<=BO<=0xF, if BO is 0x0F, no beacon will be transmitted (i.e, no superframe structure). If SO is 0xF,
  there is no active portion in superframe. (DS-2400 C.1 Short Registers)


  Check if beacon mode is enabled:
  - check SREG0x11[5], 1 == on
  - check values of BO and SO, it should not be equal to 0xf (ubec stack
  checks both values); According to ubec 2400 spec - check only B0 value

  SREG0x11 TXMCR
  Bit 5 SLOTTED: Enable slotted mode.
  0010.0000 == 0x20

  SREG0x10 ORDER
  Bit 7-4 BO: Beacon order for how often coordinator transmit a beacon frame.
  1111.0000 == 0xf0
*/

  TRACE_MSG( TRACE_MAC1, ">> zb_check_beacon_mode_on", (FMT__0));

  ZB_READ_SHORT_REG(ZB_SREG_TXMCR);
  val = ZB_MAC_GET_BYTE_VALUE();
  if (val & 0x20)
  {
    /* SLOTTED is on */
    ZB_READ_SHORT_REG(ZB_SREG_ORDER);
    val = ZB_MAC_GET_BYTE_VALUE();
    if ((val & 0xF0) != 0xF0)
    {
      ret = ZB_TRUE;
    }
  }

  TRACE_MSG( TRACE_MAC1, "<< zb_check_beacon_mode_on, ret %i", (FMT__D, ret));
  return ret;
}


/*
  Puts MAC command to normal transmit FIFO and starts packet
  send. Command should be formmated and stored in the operation_buf
  @param header_length - mhr length in bytes
  @param fifo_addr - fifo address
  @param buf - buffer to send
  @param need_ack - if 1, retransmit until ACK recv
  @return RET_OK, RET_ERROR
*/
zb_ret_t zb_transceiver_send_fifo_packet(zb_uint8_t header_length, zb_int16_t fifo_addr,
                                         zb_buf_t *buf, zb_uint8_t need_tx) ZB_SDCC_REENTRANT
{
  zb_uint8_t *fc = ZB_BUF_BEGIN(buf);

  TRACE_MSG(TRACE_MAC1, ">> zb_transceiver_send_fifo_packet, %d, addr %x, buf %p, state %hd", (FMT__D_D_P,
                                                                                               (zb_uint16_t)header_length, fifo_addr, buf));

  ZB_ASSERT(fifo_addr == ZB_NORMAL_TX_FIFO);

/* ds-2400 4.3.1. Transmit Packet in Normal FIFO */

  {
    zb_ubec_fifo_header_t *ubec_fifo_header;
    ZB_BUF_ALLOC_LEFT(buf, sizeof(zb_ubec_fifo_header_t), ubec_fifo_header);
    ZB_ASSERT(ubec_fifo_header);
    ubec_fifo_header->header_length = header_length;
    ubec_fifo_header->frame_length = ZB_BUF_LEN(buf) - sizeof(zb_ubec_fifo_header_t);
  }

  ZB_UBEC_CLEAR_NORMAL_FIFO_TX_STATUS();
  ZB_CLEAR_TX_STATUS();

  zb_uz2400_fifo_write(fifo_addr, buf);

  /* TODO: if acknowledgement is required for normal fifo, set ackreq
   * bit (SREG0x1B[2]) */
  if (need_tx)
  {
    /* we need to determine if our frame broadcast or not */

    /* Don't want to parse entire mhr here. All we need is frame control and
     * destination address. Destination address has fixed position in mhr.
     * Fields layout is fc (2b), seq number (1b), dest panid (2b), dest address (2b).
     */
    zb_uint8_t need_ack = (!((ZB_FCF_GET_FRAME_TYPE(fc) == MAC_FRAME_BEACON
                              || (ZB_FCF_GET_DST_ADDRESSING_MODE(fc) == ZB_ADDR_16BIT_DEV_OR_BROADCAST
                                  && fc[5] == 0xff && fc[6] == 0xff)))
                           && ZB_FCF_GET_ACK_REQUEST_BIT(fc));

    /* The same bit is used to start normal and beacon fifio.
       If not joined yet (pac_pan_id is not set), do not request acks.
    */
    TRACE_MSG(TRACE_MAC2, "Need ACK: %hd", (FMT__H, need_ack));
    ZB_START_NORMAL_FIFO_TX(ZB_MAC_PIB_MAX_FRAME_RETRIES, need_ack);
  }
  TRACE_MSG(TRACE_MAC1, "<< zb_transceiver_send_fifo_packet", (FMT__0));
  return RET_OK;
}


/*
  Set new active channel in transceiver
  @param channel_number - new channel number
  @return RET_OK on success, error code on fail
*/
void zb_transceiver_set_channel(zb_uint8_t channel_number)
{
  zb_uint8_t value = 0;
  TRACE_MSG(TRACE_MAC2, ">> zb_transceiver_set_channel, chan num %hd", (FMT__D, channel_number));

  if (channel_number != MAC_CTX().current_channel)
  {
/*
  LREG0x200 RFCTRL0
  Bit 7-4 CHANNEL: RF channel number. IEEE 802.15.4 2.4GHz band channels (11~26) are mapped
  Bit 3-0 RFOPT: Optimize RF control
*/
    ZB_READ_LONG_REG(ZB_LREG_RFCTRL0);
    ZB_MAC_GET_BYTE_VALUE() &= 0x0F; /* clear channel value */
    /* set new channel value */
    ZB_MAC_GET_BYTE_VALUE() |= (channel_number - ZB_TRANSCEIVER_START_CHANNEL_NUMBER) << 4;
    ZB_WRITE_LONG_REG(ZB_LREG_RFCTRL0, ZB_MAC_GET_BYTE_VALUE());

/* TODO: ckeck, UBEC spec uses logic differnt from described in
 * ds-2400: if pan coord role is on, quit, else enter sleep mode and
 * wake up chip after it */

/*
  ds-2400 spec 4.2.4. sialization
  Step 4.
  After the operation channel is set, RF state machine should be reset by setting RFCTL(SREG0x36) to 0x04
  and then setting RFCTL(SREG0x36) to 0x00. After reset, 192us should be waiting for VCO calibration to
  calibrate PLL block to the correct frequency.

  SREG0x36 RFCTL
  Bit 4 -3 WAKECNTEXT: 20MHz clock recovery time extension bits
  Bit 2 RFRST: RF state reset.
  Reset RF state. RF state mus t be reset in order to change the RF channels.
  Write 1 and then write 0 to accomplish the reset operation.
  Bit 1 RFTXMODE: RF is forced into TX mode.
  Bit 0 RFRXMODE: RF is forced into RX mode
*/
    ZB_READ_SHORT_REG(ZB_SREG_RFCTL);
    value = ZB_MAC_GET_BYTE_VALUE();
    ZB_MAC_GET_BYTE_VALUE() |= 0x04; /* set bit RFRST */
    ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, ZB_MAC_GET_BYTE_VALUE());
    value &= 0xFB; /* clear bit RFRST */
    ZB_WRITE_SHORT_REG(ZB_SREG_RFCTL, value);
    /* TODO: check, is it enough for "192us waiting" */
    ZB_8051_DELAY();
    MAC_CTX().current_channel = channel_number;
  }

  TRACE_MSG(TRACE_MAC2, "<< zb_transceiver_set_channel", (FMT__0));
}


void zb_transceiver_get_rssi(zb_uint8_t *rssi_value)
{
  TRACE_MSG(TRACE_MAC2, ">> zb_transceiver_get_rssi", (FMT__0));
  ZB_ASSERT(rssi_value);

/*
  SREG0x3E BBREG6
  Bit 7 RSSIMODE1: RSSI mode 1 enable
  1: calculate RSSI for firmware request, will be clear to 0 when RSSI calculation is finished.
  Bit 6 RSSIMODE2: RSSI mode 2 enable
  1: calculate RSSI for each received packet, the RSSI value will be stored in RXFIFO.
  0: no RSSI calculation for received packet.
  Bit 0 RSSIRDY: RSSI ready signal for RSSIMODE1 use
  If RSSIMODE1 is set, this bit will be cleared to 0 until RSSI calculation is done. When RSSI
  calculation is finished and the RSSI value is ready, this bit will be set to 1 automatically
*/

/* 1000.0000 == 0x80 set bit RSSIMODE1 */
//  zb_uz24_mask_short_reg(ZB_SREG_BBREG6, 0x80);
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG6, 0x80);

  while (1)
  {
    ZB_READ_SHORT_REG(ZB_SREG_BBREG6);
    if (ZB_MAC_GET_BYTE_VALUE() & 0x01) /* check bit RSSIRDY */
    {
      /* RSSI value is ready to read */
      break;
    }
  }
  /* RSSI register is not described in DS-2400, but it is used in
   * ubec stack */
  ZB_READ_LONG_REG(ZB_LREG_RSSI);
  *rssi_value = ZB_MAC_GET_BYTE_VALUE();
  ZB_WRITE_SHORT_REG(ZB_SREG_BBREG6, 0x40);

  TRACE_MSG(TRACE_MAC2, "<< zb_transceiver_get_rssi rssi_value %hd", (FMT__H, *rssi_value));
}


void zb_uz_short_reg_write_2b(zb_uint8_t reg, zb_uint16_t v)
{
  ZB_WRITE_SHORT_REG(reg, ZB_GET_LOW_BYTE(v));
  ZB_WRITE_SHORT_REG(reg+1, ZB_GET_HI_BYTE(v));
}


void zb_transceiver_set_coord_ext_addr(zb_ieee_addr_t coord_addr_long)
{
  zb_uint8_t i = 0;
  while (i < sizeof(zb_ieee_addr_t))
  {
    /* write one bye one 8 bytes of the extended address */
    ZB_WRITE_LONG_REG(ZB_LREG_ASSOEADR0 + i, coord_addr_long[i]);
    i++;
  }
}


void zb_transceiver_set_coord_short_addr(zb_uint16_t coord_addr_short)
{
  ZB_WRITE_LONG_REG(ZB_LREG_ASSOSADR0, ZB_GET_LOW_BYTE(coord_addr_short));
  ZB_WRITE_LONG_REG(ZB_LREG_ASSOSADR1, ZB_GET_HI_BYTE(coord_addr_short));
}


void zb_transceiver_update_long_mac()
{
  zb_ushort_t i;
  for (i = 0 ; i < 8 ; i++)
  {
    ZB_WRITE_SHORT_REG(EADR0 + i, ZB_PIB_EXTENDED_ADDRESS()[i]);
  }
}


void zb_uz2400_standby()
{
  TRACE_MSG(TRACE_COMMON2, "transceiver fall asleep", (FMT__0));
  /* So, we fall asleep */
  ZB_WRITE_SHORT_REG(ZB_SREG_SLPACK, 0x80);
}

void zb_uz2400_register_wakeup()
{
  ZB_WRITE_SHORT_REG(ZB_SREG_WAKECTL, 0xC0);
/* interrupt catch loop, 'coz we can't go on, while 2400 still sleeps */
  do
  {
    ZB_READ_SHORT_REG(0x31);
    TRACE_MSG(TRACE_COMMON2,"isr: %hd", (FMT__H));
  } while (!(ZB_MAC_GET_BYTE_VALUE()&0x40)>>6);
  ZB_READ_SHORT_REG(ZB_SREG_WAKECTL);
  ZB_MAC_GET_BYTE_VALUE()&=0x3F;
  ZB_WRITE_SHORT_REG(ZB_SREG_WAKECTL, ZB_MAC_GET_BYTE_VALUE() );
}



/* security functions */
#ifdef ZB_SECURITY

/*! \addtogroup ZB_SECUR */
/*! @{ */

static void zb_set_nonce(zb_uint8_t *nonce)
{
  /* TODO: only frame counter really changes in nonce.
     It can be possible to update only it.
     Can also exclude passing of nonce parameter.
  */
  zb_ushort_t i;
  for (i = 0; i < ZB_NONCE_LENGTH; i++)
  {
    ZB_WRITE_LONG_REG(ZB_LREG_UPNONCE_12 - i, nonce[i]);
  }
}

static void zb_set_key(zb_uint8_t *key)
{
  zb_ushort_t i;
  for (i=0;i<ZB_KEY_LENGTH;i++)
  {
    ZB_WRITE_LONG_REG(ZB_LREG_KEY_0 + i, key[i]);
  }
}


static void zb_uz_secur_init(zb_uint8_t *key,
                             zb_uint8_t *nonce,
                             zb_ushort_t string_a_len,
                             zb_buf_t *buf)
{
/*Set LREG0x24D[5] = 1 to enable security support of IEEE 802.15.4 2006.
  DS-2400-51_v0_6_RN.pdf, 163/180*/
  ZB_READ_LONG_REG(ZB_LREG_SECCTRL);
  ZB_MAC_GET_BYTE_VALUE() |= 0x20;
  ZB_WRITE_LONG_REG(ZB_LREG_SECCTRL, ZB_MAC_GET_BYTE_VALUE());
  /*step 2, fill nonce */
  zb_set_nonce(nonce);
  /*step 3, fill key */
  zb_set_key(key);
  /* step 4, Set cipher mode of the TXNFIFO at SREG0x2C[2:0] */
  //ZB_ASSERT_COMPILE_TIME(ZB_CCM_M == 4);
  if ((MAC_CTX().tx_wait_cb)&&(!MAC_CTX().tx_cnt))
  {
    ZB_WAIT_FOR_TX();
  }
  /*step 1, but moved down to give more time to */
  /* previous TX. Fill txnfifo, DS-2400-51_v0_6_RN.pdf, 76-77/180  */
  ZB_TRANS_FILL_FIFO(string_a_len ,buf);
  zb_uz24_or_mask_short_reg(ZB_SREG_SECCR0, 0x04);
}

static zb_ret_t zb_uz_secur_process()
{
  /*step 6, Trigger encryption by setting SREG0x1B[1:0]=11.
   */
#ifndef ZB_USE_RX_QUEUE
  ZB_READ_SHORT_REG(ZB_SREG_TXNTRIG);
  /* Unfortunately, we need to process TXNTRIG state, to prevent pending bit loss */
  if (ZB_MAC_GET_BYTE_VALUE() & 0x10)
  {
    ZB_MAC_SET_PENDING_DATA();
    TRACE_MSG(TRACE_MAC2, "Pending data set", (FMT__0));
  }
  ZB_WRITE_SHORT_REG(ZB_SREG_TXNTRIG, ZB_MAC_GET_BYTE_VALUE() | 0x03);
  while(1)
  {
    if (!(ZB_MAC_GET_SECURITY_PROCESS())) break;
    TRACE_MSG(TRACE_SECUR1, "Secured frame processing!", (FMT__0));
  }
  ZB_READ_SHORT_REG(ZB_SREG_TXSR);
#else
  /*step 7 (interrupt) - step 8, Check SREG0x24[0]. If SREG0x24[0]=0, it means the encryption process is done. */
  ZB_MAC_SET_SECURITY_PROCESS();
  ZB_WRITE_SHORT_REG(ZB_SREG_TXNTRIG, 0x03);
  while(1)
  {
    if (!(ZB_MAC_GET_SECURITY_PROCESS())) break;
    TRACE_MSG(TRACE_SECUR1, "Secured frame processing!", (FMT__0));
  }
  ZB_MAC_GET_BYTE_VALUE() = TRANS_CTX().tx_status;
#endif
  if ((ZB_MAC_GET_BYTE_VALUE()&0x01)==0x00)
  {
    TRACE_MSG(TRACE_SECUR1, "En/Decryption successfull", (FMT__0));
    return RET_OK;
  }
  return RET_ERROR;

}


zb_ret_t zb_ccm_encrypt_n_auth(
  zb_uint8_t *key,
  zb_uint8_t *nonce,
  zb_uint8_t *string_a,
  zb_ushort_t string_a_len,
  zb_uint8_t *string_m,
  zb_ushort_t string_m_len,
  zb_buf_t *buf)
{

  zb_ret_t ret = RET_ERROR;
  zb_uint8_t *ptr;

  TRACE_MSG(TRACE_MAC2, ">>zb_ccm_encrypt_n_auth a_len %hd m_len %hd", (FMT__H_H, string_a_len, string_m_len));

  ZB_BUF_INITIAL_ALLOC(buf, string_m_len + string_a_len, ptr);
  ZB_BZERO(ptr, string_m_len + string_a_len);
  ZB_MEMCPY(ptr, string_a, string_a_len);
  ZB_MEMCPY(ptr+string_a_len, string_m, string_m_len);

  zb_uz_secur_init(key, nonce, string_a_len, buf);

  /* step 5, Enable Upper-Layer-Cipher encryption by setting SREG0x37[6]=1
     Bit 6   UPENC: Upper Layer Security Encryption Mode
     1: Perform upper layer encryption using TX Normal FIFO. Bit is automatically cleared to 0 by
     hardware. */
  zb_uz24_or_mask_short_reg(ZB_SREG_SECCR2, 0x40);
  ret = zb_uz_secur_process();

  /* disable upper cipher encryption */
  zb_uz24_mask_short_reg(ZB_SREG_SECCR2, (zb_uint8_t)~0x40);

  (void)zb_buf_smart_alloc_right(buf, ZB_CCM_M + ZB_FIFO_HEADER_SIZE);
  ZB_TRANS_READ_TXNFIFO(buf, string_m_len + string_a_len + ZB_CCM_M + ZB_FIFO_HEADER_SIZE);
  ZB_DUMP_INCOMING_DATA(buf);
  (void)zb_buf_cut_left(buf, ZB_FIFO_HEADER_SIZE);

  TRACE_MSG(TRACE_MAC2, "<<zb_ccm_encrypt_n_auth ret %d len %hd", (FMT__D_H, ret, ZB_BUF_LEN(buf)));
  return ret;
}


zb_ret_t zb_ccm_decrypt_n_auth(
  zb_uint8_t *key,
  zb_uint8_t *nonce,
  zb_buf_t *buf,
  zb_ushort_t string_a_len,
  zb_ushort_t string_c_len)
{
  zb_ret_t ret = RET_OK;
  zb_uint8_t mic_err_cnt = 0;

  TRACE_MSG(TRACE_MAC2, ">>zb_ccm_decrypt_n_auth", (FMT__0));
  do
  {
    zb_uz_secur_init(key, nonce, string_a_len, buf);

    /* step 5, Enable Upper-Layer-Cipher decryption*/
    zb_uz24_or_mask_short_reg(ZB_SREG_SECCR2, 0x80);
    /* when decrypting, using a TXFIFO, fifo length should be modified */
    ZB_WRITE_LONG_REG(0x01,string_c_len + string_a_len + ZB_FIFO_HEADER_SIZE);
    TRACE_MSG(TRACE_MAC1, ">>secur_process", (FMT__0));
    zb_uz_secur_process();
    TRACE_MSG(TRACE_MAC1, "<<secur_process", (FMT__0));
    ZB_READ_SHORT_REG(ZB_SREG_RXSR);
    if ((ZB_MAC_GET_BYTE_VALUE() & 0x20))
    {
      TRACE_MSG(TRACE_MAC1, "MIC check failed!", (FMT__0));
      ZB_WRITE_SHORT_REG(ZB_SREG_RXSR,0x20); /* write '1' to clear error flag */
      ret = RET_ERROR;
    }
    else
    {
      ret = RET_OK;
    }
    if (ret == RET_ERROR) mic_err_cnt++;
  } while ((ret == RET_ERROR)&&(mic_err_cnt<2));
  /* disable upper cipher */
  zb_uz24_mask_short_reg(ZB_SREG_SECCR2, (zb_uint8_t)~0x80);

  (void)zb_buf_smart_alloc_left(buf, ZB_FIFO_HEADER_SIZE);
  TRACE_MSG(TRACE_MAC1, ">>reading TXFIFO", (FMT__0));
  ZB_TRANS_READ_TXNFIFO(buf, string_c_len + string_a_len + ZB_FIFO_HEADER_SIZE);
  TRACE_MSG(TRACE_MAC1, "<<reading TXFIFO", (FMT__0));
  ZB_BUF_CUT_RIGHT(buf, ZB_CCM_M); /* MIC */
  ZB_BUF_CUT_LEFT2(buf, ZB_FIFO_HEADER_SIZE); /* hdr len, frame len */
  TRACE_MSG(TRACE_MAC2, "<<zb_ccm_decrypt_n_auth", (FMT__0));
  return ret;
}


/**
   Unsecure received frame using UZ24x0 MAC security capabilities
*/
void zb_uz_secur_handle_rx()
{
  if (ZB_UBEC_GET_SECUR_STATUS())
  {
    /* Try trivial solution: when got security intr, skip decrypt now and later
     * decrypt packet using same routine which used for NWK security */
    TRACE_MSG(TRACE_MAC2, "Got security RX. Try to ignore security", (FMT__0));
    zb_uz24_or_mask_short_reg(ZB_SREG_SECCR0, 0x80);
  }

#if 0

  zb_ushort_t is_long_addr;
  zb_ushort_t i;
  zb_ret_t ret = RET_OK;

  /*
    Step 1.
    When a packet comes with the security enabled bit set in the frame control field of the packet header, a
    security interrupt, SREG0x31[4], is issued right after the complete packet header is received.
  */
  if (ZB_UBEC_GET_SECUR_STATUS())
  {
    zb_ushort_t hlen;
    zb_mac_mhr_t mhr;

    TRACE_MSG(TRACE_MAC2, "security intr", (FMT__0));
    ZB_MAC_SET_SECURITY_PROCESS();
    /*
      Step 2.
      Check LREG0x21E for received header length. This header length of the received packet only includes frame
      control field, sequence number and address information. User can use the length to find out the RXFIFO start
      address of AXU security header.
    */


    ZB_READ_LONG_REG(ZB_LREG_HLEN);
    hlen = ZB_MAC_GET_BYTE_VALUE() + MAC_SECUR_LEV5_KEYID1_AUX_HDR_SIZE;
    /* For now hard-code security parameters from MAC TC: level 5, key id mode 1 */
    zb_buf_initial_alloc(ZG->mac.mac_ctx.operation_buf, hlen);
    /*
      Step 3.
      Rewrite the LREG0x21E to tell UZ2400 that the real un-encrypted length of the received frame. The length will
      be used during the decryption process. It does not include the length of MIC
      code.
    */

    zb_uz2400_fifo_read(0, ZG->mac.mac_ctx.encryption_buf, hlen + ZB_MAC_PACKET_LENGTH_SIZE);

    hlen = zb_parse_mhr(&mhr, ZB_BUF_BEGIN(ZG->mac.mac_ctx.encryption_buf) + ZB_MAC_PACKET_LENGTH_SIZE);
    ZB_WRITE_LONG_REG(ZB_LREG_HLEN, hlen);
    TRACE_MSG(TRACE_MAC2, "hdr len (with aux) %hd", (FMT__H, hlen));

    /*
      Step 4.
      Check LREG0x212[1:0] for the source address mode of the received packet. If LREG0x212[1:0]='10' (short
      address), user should load the 64-bit long address of source device to LREG0x213 ~ LREG0x21A, LSB first. The
      64-bit long address will be needed when the decryption is processed.
    */
    ZB_READ_LONG_REG(ZB_LREG_SADRCTRL);
    is_long_addr = ((ZB_MAC_GET_BYTE_VALUE() & 3) == ZB_ADDR_64BIT_DEV);

    /* fill nonce, check frame counter */

    /* to fill nonce, needs long address. If device or its address absent in
     * macDeviceTable, can't decrypt. */

    /* find dev in device table: need long address and packets counter. */
    for (i = 0 ; i < MAC_PIB().mac_device_table_entries ; ++i)
    {
      if (MAC_PIB().mac_device_table[i].pan_id == mhr.src_pan_id)
      {
        if (is_long_addr)
        {
          if (MAC_PIB().mac_device_table[i].short_address == mhr.src_addr.addr_short)
          {
            break;
          }
        }
        else
        {
          if (ZB_IEEE_ADDR_CMP(MAC_PIB().mac_device_table[i].long_address, mhr.src_addr.addr_long))
          {
            break;
          }
        }
      }
    }
    if (i == MAC_PIB().mac_device_table_entries)
    {
      /* no such device - can't decrypt */
      TRACE_MSG(TRACE_SECUR1, "device not found - MAC unsecure failed", (FMT__0));
      ret = RET_ERROR;
    }
    else if (MAC_PIB().mac_device_table[i].frame_counter > mhr.frame_counter
             || MAC_PIB().mac_device_table[i].frame_counter == (zb_uint32_t)~0)
    {
      ret = RET_ERROR;
      TRACE_MSG(TRACE_SECUR1, "frame cnt %ld->%ld shift back - MAC unsecure failed",
                (FMT__L_L, MAC_PIB().mac_device_table[i].frame_counter > mhr.frame_counter));
    }

    if (ret != RET_OK)
    {
      /* If there is no suitable key for the received packet, user can ignore the decryption
         process this time by setting the short register SREG0x2C[7]=1. */
      TRACE_MSG(TRACE_MAC2, "will ignore security", (FMT__0));
      zb_uz24_or_mask_short_reg(ZB_SREG_SECCR0, 0x80);
    }
    else
    {

      if (!is_long_addr)
      {
        zb_ushort_t j;
        TRACE_MSG(TRACE_MAC2, "short src - set long addr", (FMT__0));

        for (j = 0 ; j < 8 ; ++j)
        {
          ZB_WRITE_LONG_REG(ZB_LREG_SRCADR_0 + j, MAC_PIB().mac_device_table[i].long_address[j]);
        }
      }

      /*
        Step 5.
        Load the nonce into LREG0x240 - 0x24C. About the nonce structure, please refer to Figure 77 of IEEE 802.15.4
        - 2006. User can get the source MAC address from the received header of the packet, the self database or the
        address mapping table. The security level and frame counter can be fetched from the AUX security header
        field.
      */
      /* nonce - see 7.6.3.2 CCM* Nonce. */
      {
        zb_secur_ccm_nonce_t nonce;

        ZB_IEEE_ADDR_COPY(nonce.source_address, MAC_PIB().mac_device_table[i].long_address);
        ZB_HTOLE32(&nonce.frame_counter, mhr.frame_counter);
        nonce.secur_control = ZB_MAC_SECURITY_LEVEL;
        zb_set_nonce((zb_uint8_t *)&nonce);
      }

      /*
        Step 6.
        According to the content of the received AUX security header, the host should perform the key searching and
        fill in the key FIFO of RX with the suitable security key.
      */
      zb_set_key(MAC_PIB().mac_key);

      /*
        Step 7.
        Besides setting the RXFIFO key, the MCU host should also perform the security mode decision and set the
        corresponding cipher mode in SREG0x2C[5:3].
        After both the key and the security mode are set, the security engine should be configured and enabled by
        setting SREG0x2C[6]=1. If there is no suitable key for the received packet, user can ignore the decryption
        process this time by setting the short register SREG0x2C[7]=1.
      */
      zb_uz24_or_mask_short_reg(ZB_SREG_SECCR0, (4 << 3) | (1 << 6));
    } /* else (ok) */
  }
  else if (ZB_UBEC_GET_RX_DATA_STATUS() && ZB_MAC_GET_SECURITY_PROCESS())
  {
    TRACE_MSG(TRACE_MAC2, "rx intr after security magic", (FMT__0));
    /*
      Step 8.
      After the packet is successfully decrypted or the decryption is ignored, an RX interrupt is issued to notify that
      the whole processes of receiving and decryption has been done. User can check the error status from
      SREG0x30[2]. If SREG0x30[2]='0', the decrypted data is available in RXFIFO. If SREG0x30[2]='1', it means
      decryption failed and user must perform RXFIFO flush by setting SREG0x0D[0].
    */

    ZB_MAC_CLEAR_SECURITY_PROCESS();
    ZB_READ_SHORT_REG(ZB_SREG_RXSR);
    if (ZB_MAC_GET_BYTE_VALUE() & (1<<2))
    {
      /* Decryption failed. Flush. */
      TRACE_MSG(TRACE_MAC2, "Decryption failed. Flush.", (FMT__0));
      ZB_UBEC_CLEAR_RX_DATA_STATUS();
      zb_uz24_or_mask_short_reg(ZB_SREG_RXFLUSH, 1);
    }
    else
    {
      TRACE_MSG(TRACE_MAC2, "Decrypted ok.", (FMT__0));
    }
  }
#endif  /* 0 */

}

#endif  /* ZB_SECURITY */


/* @} */

#endif  /* 2400 or 2410 */
