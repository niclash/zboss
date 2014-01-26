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
PURPOSE: Serial transport for 8051: init
*/

#include "zb_common.h"

#ifndef ZB_SNIFFER
#include "zb_mac_transport.h"
#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_g_context.h"
#endif

/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#include "zb_bank_common.h"

#if defined ZB_TRANSPORT_8051_UART || defined ZB_SNIFFER_SERIAL_TRACE

#ifdef C8051F120

static void set_serial_baud_rate();

/**
  Function initializes serial port, i/o context for it
  @param io_ctx - i/o context to init
  @param baud_rate - baud_rate value
*/
void zb_init_8051_serial()
{
  ZB_DISABLE_SERIAL_INTER();

  set_serial_baud_rate();
  SFRPAGE = LEGACY_PAGE;
  PS = 1;      /* set UART0 interrupt low priority                 */
  SCON = SCON_8N1 | SCON_REN; /* mode 1, 8-bit uart, enable receiver */

  ZB_ENABLE_SERIAL_INTER();
}


/**
  Function initializes serial port, sets up baud rate
 */
static void set_serial_baud_rate()
{
  ZB_ASSERT(ZB_SERIAL_BAUD_RATE == BAUD_RATE_115200);
    // See if we *can* set the desired baud rate.
  PCON  |= PCON_SMOD;
  PCON  &= ~PCON_SMOD;
  {
    char sfrpage_save;                              /* Saving current SFR page,                                    */
    sfrpage_save = SFRPAGE;                 /* common shifting routine                                             */
    SFRPAGE = CONFIG_PAGE;                      /* can be dropped, coz of only 2 pages used                       */
    OSCICN = 0x83;                                      /* Enable the internal Oscillator, F15.3                          */
    while(!(OSCICN & 0x40));                /* Wait until clock is ready                                              */
    XBR0 = 0xf7;                                    /* Enable the SPI and UART, and ALL, F19.6,7,8,9                  */
    XBR2 = 0x40;                                        /* Enable P3 out, just for leds, F19.6,7,8,9              */
    P0MDOUT |= 0x01;                            /* Set TX0 pin to push-pull, F19.11                               */
    P3MDOUT = 0x00;                             /* Led testing, p215, 19.PORT INPUT/OUTPUT                        */
    SFRPAGE = sfrpage_save;             /* switch back SFR page                                                           */
    SSTA0 = 0x10;                                       /* Timer 1 generates UART0 baud rate and                  */
                                                /* UART0 baud rate divide by two disabled, F22.9      */
#ifdef C8051F120
    CKCON |= 0x10;                          /* Timer 1 uses the system clock, F24.6                   */
#endif
    ET1 = 0; /* disable timer 1 interrupts*/

    /* Config. Timer 1 for baud rate generation. */
    TR1 = 0;  /* stop timer 1 */
    TMOD = T1_8BIT_AUTO_RELOAD;   /* timer 1, mode 2, 8-bit reload */
    TH1 = (zb_uint8_t) 0xF3; /* value for specified baud rate - 115200 */
    TR1 = 1;  /* start timer 1*/
  }
}
#endif /*C8051F120 */

#ifdef ZB_UZ2410
void zb_init_8051_serial()
{
  ZB_ASSERT(ZB_SERIAL_BAUD_RATE == BAUD_RATE_115200);

  EA = 0;
  CHIPCFG |= 0x88;

  /* Disable uart0 */
  IE &= ~0x10; /*disable uart0 interrupt	*/
  SYSCFG &= ~0x80; /*disable uart0  */
  SCON = 0; /* disable uart0 reception, clear mode specifier, clear trx interrupt flags	*/

  /* set uart mode */
  SCON |= 0x40; /* select mode 1, SM0 = 0, SM1 = 1*/
  /* set baud rate  */
  TCON &= ~0x40; /* stop timer1 */
  TMOD &= ~(0x20|0x10); /* clear timer1 mode */
  TMOD |= 0x20; /* timer1 in autoreload 8 bit mode	*/

  PCON = 0x80;
  TH1 = 0xFE;  /* 115200 bps */

  /* enabling UART0 */
  TCON |= 0x40; /* start timer */
  SCON |= 0x10; /* enable uart0 reception */
  IE |= 0x10; /* enable uart0 interrupt	 */
  SYSCFG |= 0x80; //enable uart0
  EA = 1;

  /* TODO: setting serial interrupt priority via IPH1, IP1 */
}


#endif /*ZB_UZ2410 */

#ifdef ZB_CC25XX
void zb_init_8051_serial()
{
  ZB_ASSERT(ZB_SERIAL_BAUD_RATE == BAUD_RATE_115200);
  /* Set P2 priority - USART0 over USART1 if both are defined. */
  P2DIR &= ~0xC; /* 00 means priority USART0, USART1, TIMER1 */ 

  PERCFG &= ~0x01;    /* Set UART0 I/O location to P0.*/

  P0SEL  |= 0x0C;                 /* Enable Tx and Rx peripheral functions on pins. */
  ADCCFG &= ~0x0C;                /* Make sure ADC doesnt use this. */
  U0CSR = 0x80;                  /* Mode is UART Mode. */
  U0UCR = 0x80;                 /* Flush it. */
  
  U0BAUD = 216; /* 115200 */
  U0GCR = 11;   /* for 115200 mode, taken from sample code */
  
  U0UCR = 0x02; /* stop UART */
  U0CSR |= 0x40; /* receiver enabled, not sure it's really needed for now */
#ifdef ZB_SNIFFER
  URX0IE = 1; /* Enable serial rx for setting up transceiver channel */
#endif

  /* U0DBUF = 0;  */  /*prime UART pump. BTW we don't really need it, because
  uart can be triggered by setting transmitt flag, no matter, that TI don't
  know it*/
}
#endif /*ZB_CC25XX */
#endif /* ZB_TRANSPORT_8051_UART */


/*! @} */
