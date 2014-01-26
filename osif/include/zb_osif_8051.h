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
PURPOSE: 8051-specific platform depenednt stuff
*/

#ifndef ZB_OSIF_8051_H
#define ZB_OSIF_8051_H 1

/*! \addtogroup ZB_OSIF_8051 */
/*! @{ */

#include <stdio.h>
#include <limits.h>
#include <string.h>

#define ZB_VOLATILE volatile

#ifndef ZB_IAR
#define ZB_INTERRUPT void
#else
#define ZB_INTERRUPT __interrupt void
#endif

#ifdef C8051F120                /* this is 2400 with f124 board */

#ifdef SDCC

#include <time.h>
#include "mcs51/C8051F120.h"

#define LED1 P3_0
#define LED2 P3_1

#elif defined KEIL
//#include "REG51.H"
sbit ES   = 0xAC;
sbit RI   = 0x98;
sbit TI   = 0x99;
sfr  SCON = 0x98;
sfr  SBUF = 0x99;
#include "C8051F120.h"

sbit LED1 = P3^0;
sbit LED2 = P3^1;

#elif defined ZB_IAR

#include "ioC8051F120.h"
#define ES IE_bit.ES0
#define TI SCON0_bit.TI0
#define RI SCON0_bit.RI0
#define SBUF SBUF0
#define PS IP_bit.PS0
#define SCON SCON0
#define NSSMD0 SPI0CN_bit.NSSMD0
#define NSSMD1 SPI0CN_bit.NSSMD1
#define SPIEN SPI0CN_bit.SPIEN
#define TR0 TCON_bit.TR0
#define EA IE_bit.EA
#define IT1 TCON_bit.IT1
#define PX1 IP_bit.PX1
#define EX1 IE_bit.EX1
#define ET0 IE_bit.ET0
#define ET1 IE_bit.ET1
#define TR1 TCON_bit.TR1
#define SPIF SPI0CN_bit.SPIF
#define WCOL SPI0CN_bit.WCOL
#define RXOVRN SPI0CN_bit.RXOVRN

#define LED1 P3_bit.P30
#define LED2 P3_bit.P31

#define CONFIG_PAGE       0x0F    /* SYSTEM AND PORT CONFIGURATION PAGE */
#define LEGACY_PAGE       0x00    /* LEGACY SFR PAGE */

#else
#error Port me!!!
#endif  /* compilers switch */

#elif defined ZB_UZ2410 // C8051F120

#include "zb_uz2410.h"

#elif defined ZB_CC25XX
#include <ioCC2530.h>
#else
#error port me!!!

#endif /* MCUs switch */

#define ZB_ABORT() { zb_char_t *p = 0; *p = 0; }

/* 8051-specific trace */

#if defined(SDCC)
/**
   Special reentrant definition for sdcc: declare function as reentrant to
   prevent sloc using by sdcc causing out of DSEG.
 */
#define ZB_SDCC_REENTRANT __reentrant
#define ZB_SDCC_XDATA __xdata
#define ZB_KEIL_REENTRANT

#ifdef ZB_BANKED_BUILD
#define ZB_SDCC_BANKED __banked
#else
#define ZB_SDCC_BANKED
#endif

/**
   In SDCC callback must be reentrant, and all callbacks are banked
 */
#define ZB_CALLBACK __reentrant ZB_SDCC_BANKED
#define ZB_KEIL_REENTRANT

#elif defined(KEIL)
/* Not need 'reentrant' for Keil. Its absence decreases code size but slightly
 * increases used XDATA size. */
#define ZB_SDCC_REENTRANT
#define ZB_SDCC_BANKED
#define ZB_SDCC_XDATA
#define ZB_CALLBACK
/**
  In Keil reentrant prevents compiler from sharing data segment with other
  routines.
  Currently only main scheduler loop declared as reentrant in Keil: must not
  share its variable with functions it calls via pointers.
 */
#define ZB_KEIL_REENTRANT reentrant

#elif defined(ZB_IAR)

#define ZB_SDCC_REENTRANT
#define ZB_SDCC_BANKED
#define ZB_SDCC_XDATA
#define ZB_CALLBACK
#define ZB_KEIL_REENTRANT

#endif

#define ZB_TRACE_LOCK()
#define ZB_TRACE_UNLOCK()

#define ZB_CONFIG_PAGE 0
#define ZB_VOLATILE_PAGE 128
#define ZB_SCRATCHPAD_PAGE_SIZE 128



/**
   Put 8051 into idle state.

   Wnen not do it, at Keil simulator works much faster! It could be useful to
   disable idle when debug (bit not sure...)
   Use ZB_DBG_NO_IDLE define to disable idle.
 */

#if !defined ZB_DBG_NO_IDLE && defined ZB_ED_ROLE
/*PCON |= !ZG->sched.mac_loop_flag, to prevent interrupt loss */
#define ZB_8051_IDLE() (PCON |= 0x01, PCON = PCON)
#define ZB_FORCE_IDLE() ZB_8051_IDLE()
#else
#define ZB_8051_IDLE() (PCON = PCON)
#define ZB_FORCE_IDLE() (PCON |= 0x01, PCON = PCON)
#endif


/**
   Put 8051 into idle state, interrupt will wakeup it.
 */
#define ZB_GO_IDLE() ZB_8051_IDLE()
#define CHECK_INT_N_TIMER()
/**
   Complilers stuff
 */
#if defined(SDCC)
  #define INTERRUPT_DEFINITION(x, y) __interrupt (x) __using (y)
#elif defined(KEIL)
  #define INTERRUPT_DEFINITION(x, y)
#elif defined(ZB_IAR)
  #define INTERRUPT_DEFINITION(x, y)
#else
  #error Port me!!!
#endif

#if defined(SDCC)
  #define INTERRUPT_DECLARATION(x, y) __interrupt (x) __using (y)
  #define INTERRUPT_DECLARATION_NOBANK(x) __interrupt (x)
#elif defined(KEIL)
 #define INTERRUPT_DECLARATION(x, y) interrupt x using y
 #define INTERRUPT_DECLARATION_NOBANK(x) interrupt x
#elif defined(ZB_IAR)
 #define INTERRUPT_DECLARATION(x, y)
 #define INTERRUPT_DECLARATION_NOBANK(x)
#else
  #error Port me!!!
#endif

#if defined(SDCC)
  #define DECLARE_REGISTER_AT(type, var, val) type at (val) var
#elif defined(KEIL)
  #define DECLARE_REGISTER_AT(type, var, val) type var = val
#elif defined(ZB_IAR)
  #define DECLARE_REGISTER_AT(type, var, val) __sfr __no_init volatile type var @ val
#else
  #error Port me!!!
#endif

//#ifdef SDCC
//#define ZB_OSIF_GLOBAL_LOCK()   __critical {
//#define ZB_OSIF_GLOBAL_UNLOCK() }
//#else
#define ZB_OSIF_GLOBAL_LOCK()   ZB_DISABLE_ALL_INTER()
#define ZB_OSIF_GLOBAL_UNLOCK() ZB_ENABLE_ALL_INTER()
//#endif

#define ZB_TEST_BIT(B) ((B) ? (B = 0, 1) : 0)

#define REGISTER_BANK_1     1
#define REGISTER_BANK_2     2
#define REGISTER_BANK_3     3

#ifndef ZB_IAR
#define SERIAL_INTER_NUMBER 4
#define TIMER0_INTER_NUMBER 1
#ifdef C8051F120
#define SPI_INTER_NUMBER    6
#define UBEC_2400_INTER_NUMBER 2

#define ZB_DISABLE_UBEC_2400() EX1 = 0, EX1 = 0
#define ZB_ENABLE_UBEC_2400() EX1 = 1

#endif
#ifdef ZB_UZ2410
#define UBEC_2400_INTER_NUMBER 6

#define ZB_DISABLE_UBEC_2400() ESPI0 = 0
#define ZB_ENABLE_UBEC_2400() ESPI0 = 1

#endif

#else  /* IAR */

/* see 8051\inc\ioXXX.h in the IAR distributive */
#ifdef ZB_CC25XX
#define SERIAL_INTER_NUMBER UTX0_VECTOR
#ifdef ZB_SNIFFER
#define SERIAL_RX_INTER_NUMBER URX0_VECTOR
#endif
#define TIMER0_INTER_NUMBER T1_VECTOR
#define UBEC_2400_INTER_NUMBER RF_VECTOR /* should be possibly changed */
#else

#define SERIAL_INTER_NUMBER RI0_int
#define TIMER0_INTER_NUMBER TF0_int
#endif
#ifdef C8051F120
#define SPI_INTER_NUMBER    SPIF_int
/*
  See table 11.4 and ioC8051F124.h to match interrupt vector with number.

  For 2400 with F124 int number 2 == 0x13 == IE1_int
*/
#define UBEC_2400_INTER_NUMBER IE1_int
#define ZB_DISABLE_UBEC_2400() EX1 = 0, EX1 = 0
#define ZB_ENABLE_UBEC_2400() EX1 = 1


#elif defined ZB_UZ2410
/*
For 2410 int 6 - 0x33 == SPIF_int
*/
#define UBEC_2400_INTER_NUMBER 0x33
#define ZB_DISABLE_UBEC_2400() ESPI0 = 0, ESPI0 = 0
#define ZB_ENABLE_UBEC_2400() ESPI0 = 1

#endif  /* platforms switch */

#endif  /* IAR */

#define ZB_ENABLE_ALL_INTER() (EA = 1)
#define ZB_DISABLE_ALL_INTER() (EA = 0, EA = 0)


#define ZB_DISABLE_TRANSIVER_INT() ZB_DISABLE_UBEC_2400()
#define ZB_ENABLE_TRANSIVER_INT() ZB_ENABLE_UBEC_2400()

#define ZB_POWER_SOWN() (PCON |= 2, PCON = PCON)


// Common XTAL frequencies (Hz).
#define ZB_SHORT_XTAL_11_059MHZ  11.059200
#define ZB_SHORT_XTAL_12_000MHZ  12.000000
#define ZB_SHORT_XTAL_12_288MHZ  12.288000
#define ZB_SHORT_XTAL_16_000MHZ  16.000000
#define ZB_SHORT_XTAL_20_000MHZ  20.000000
#define ZB_SHORT_XTAL_22_118MHZ  22.118400
#define ZB_SHORT_XTAL_24_000MHZ  24.000000
#define ZB_SHORT_XTAL_24_500MHZ  24.500000
#define ZB_SHORT_XTAL_32_000MHZ  32.000000


#ifdef ZB_UZ2410 /* there is a 32MHz xtal on PCB, but 2410 will divide it to 8Mhz to system clock */
#define ZB_SHORT_XTAL_FREQ 8
#elif defined ZB_CC25XX
#define ZB_SHORT_XTAL_FREQ ZB_SHORT_XTAL_32_000MHZ
#else
#define ZB_SHORT_XTAL_FREQ ZB_SHORT_XTAL_24_500MHZ /* TODO: define it depending on ZB_XTAL_FREQ value */
#endif

#ifdef C8051F120
#define ZB_STOP_WATCHDOG() \
(                          \
  WDTCN = 0xde,            \
  WDTCN = 0xad             \
)
#else
#define ZB_STOP_WATCHDOG()
#endif  /*TODO seems like wd on uz2410 is disabled by default, but need to check this */

#define ZB_START_DEVICE() ZB_TIMER_INIT(), ZB_ENABLE_ALL_INTER()


/**
   Return random value

   TODO: implement it!
 */
zb_uint16_t zb_random();
#define ZB_RANDOM() zb_random()

/* use macros to be able to redefine */
#define ZB_MEMCPY memcpy
#define ZB_MEMMOVE memmove
#define ZB_MEMSET memset
#define ZB_MEMCMP memcmp

void zb_bzero_short(char *s, zb_uint8_t n);
#define ZB_BZERO(s,l) zb_bzero_short((char*)(s), (l))
#define ZB_BZERO2(s) ZB_BZERO(s, 2)

#define ZB_CODE_MEM ((zb_uint8_t code *)0x00)
#define ZB_XDATA_MEM ((zb_uint8_t xdata *)0x00)


/* for SDCC: handler prototype MUST be defined in the same file with main() function */
#define DECLARE_UBEC_2400_INTER_HANDLER() \
  void ubec_2400_handler(void) INTERRUPT_DEFINITION(UBEC_2400_INTER_NUMBER, REGISTER_BANK_0);

#ifndef KEIL
#define ZVUNUSED(v) (void)v
#else
/* Keil does not understand this kind of warning suppression */
#define ZVUNUSED(v) if (v) {}
#endif
/* nvram functions */
/* dummy
typedef zb_uint8_t zb_node_desc_t[15];
typedef zb_uint8_t zb_power_desc_t[2];
typedef zb_uint8_t zb_simple_desc_t[12];
*/

zb_ret_t zb_write_nvram_config(zb_uint8_t aps_designated_coord, zb_uint8_t aps_use_insecure_join, zb_uint8_t aps_use_extended_pan_id,
    zb_ieee_addr_t mac_extended_address);

zb_ret_t zb_config_from_nvram();

void zb_erase_nvram(zb_uint8_t page);

zb_ret_t zb_write_formdesc_data(zb_uint8_t profile_in_use, zb_ieee_addr_t long_parent_addr, zb_uint32_t aps_channel_mask,
zb_uint16_t short_parent_addr, zb_uint8_t     depth, zb_uint16_t pan_id, zb_ext_pan_id_t ext_pan_id, zb_uint16_t nwk_short_addr);

zb_ret_t zb_read_formdesc_data();


zb_ret_t zb_write_security_key();

zb_ret_t zb_read_security_key();

zb_ret_t zb_write_up_counter();

zb_ret_t zb_read_up_counter();

/* config section (for nvram routines */
#define ZB_CONFIG_SIZE 9

/**
   Configure device to issue UZ2400 interrupt
 */

void zb_ext_int_init(); /* external interrupt init */
void zb_xram_init(); /*off-chip xram initialization */


/*! @} */

#endif /* ZB_OSIF_8051_H */
