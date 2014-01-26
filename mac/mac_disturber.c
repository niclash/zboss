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
PURPOSE: Disturber device implementation
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "mac_internal.h"
#include "zb_mac_transport.h"
#include "zb_secur.h"
#include "zb_zdo.h"

#include "zb_bank_1.h"

#ifdef MAC_CERT_TEST_HACKS

/*! \addtogroup ZB_MAC */
/*! @{ */

void zb_mac_disturber_loop(zb_uint8_t logical_channel)
{
  zb_uint8_t *ptr;
  zb_buf_t *buf;
  zb_mac_mhr_t mhr;
  zb_ushort_t mhr_len = zb_mac_calculate_mhr_length(ZB_ADDR_16BIT_DEV_OR_BROADCAST, ZB_ADDR_16BIT_DEV_OR_BROADCAST, 1);

  ZB_TRANSCEIVER_SET_CHANNEL(logical_channel);

  ZB_BZERO2(mhr.frame_control);
  ZB_FCF_SET_FRAME_TYPE(mhr.frame_control, MAC_FRAME_DATA);
  /* security enable is 0 */
  /* frame pending is 0 */
  /* ack req 0 */
  ZB_FCF_SET_PANID_COMPRESSION_BIT(mhr.frame_control, 1);
  ZB_FCF_SET_DST_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  /* 7.2.3 Frame compatibility: All unsecured frames specified in this
     standard are compatible with unsecured frames compliant with IEEE Std 802.15.4-2003 */
  ZB_FCF_SET_FRAME_VERSION(mhr.frame_control, MAC_FRAME_IEEE_802_15_4_2003);
  ZB_FCF_SET_SRC_ADDRESSING_MODE(mhr.frame_control, ZB_ADDR_16BIT_DEV_OR_BROADCAST);
  /* mac spec 7.5.6.1 Transmission */
  mhr.seq_number = 8;
  /* put our pan id as src and dst pan id */
  mhr.dst_pan_id = mhr.src_pan_id = ZB_DISTURBER_PANID;
  mhr.dst_addr.addr_short = mhr.src_addr.addr_short = -1;

  buf = zb_get_out_buf();

  while (1)
  {
    ZB_BUF_REUSE(buf);
    ZB_BUF_INITIAL_ALLOC(buf, 100,  ptr);
    ZB_MEMSET(ptr, 0xf0, 100);
    ZB_BUF_ALLOC_LEFT(buf, mhr_len,  ptr);
    zb_mac_fill_mhr(ptr, &mhr);
    ZB_TRANS_SEND_COMMAND(mhr_len, buf);
    while (zb_check_cmd_tx_status() != RET_OK)
    {
      if (ZB_GET_TRANS_INT())
      {
        TRACE_MSG(TRACE_COMMON2, "uz2400 int!", (FMT__0 ));
        ZB_CLEAR_TRANS_INT();
        ZB_CHECK_INT_STATUS();
      }
    }
  }
}

/*! @} */
#endif  /* MAC_CERT_TEST_HACKS */
