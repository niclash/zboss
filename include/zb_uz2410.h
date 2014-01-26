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
PURPOSE: ubec uz2410 specific code
*/

#ifndef ZB_UZ2410_H
#define ZB_UZ2410_H


#ifndef ZB_UZ2410_256
#define ZB_NORMAL_FIFO_ADDR 0xFC00
#else
#define ZB_NORMAL_FIFO_ADDR 0x7C00
#endif

#ifndef ZB_IAR

/* Test values, need more info */
sfr CKCON         = 0x8E; /* CLOCK CONTROL */ 
/* */

//80
sfr P0    		= 0x80;
sfr SP    		= 0x81;
sfr DPL   		= 0x82;
sfr DPH   		= 0x83;
sfr DPL1  		= 0x84;
sfr DPH1  		= 0x85;
sfr IER2  		= 0x86;
sfr PCON  		= 0x87;

//88
sfr TCON  		= 0x88;
sfr TMOD  		= 0x89;
sfr TL0   		= 0x8A;
sfr TL1   		= 0x8B;
sfr TH0   		= 0x8C;
sfr TH1   		= 0x8D;
sfr IP2	  		= 0x8E;
sfr IP2H  		= 0x8F;

//90
sfr P1    		= 0x90;
sfr	P1D   		= 0x91;
sfr COBK		= 0x93;
sfr MEX1  		= 0x94;
sfr MEX2  		= 0x95;
sfr MEX3  		= 0x96;
sfr MEXSP 		= 0x97;

//98
sfr SCON  		= 0x98;
sfr SBUF  		= 0x99;
sfr SYSCFG  	= 0x9A;//Config UART, I2C, SPI interface
sfr SPICKR  	= 0x9B;
sfr SPITXBUF 	= 0x9C;
sfr SPICN    	= 0x9D;
sfr SPIRXBUF 	= 0x9E;
sfr CHIPCFG   	= 0x9F;//Config test mode, JTAG, MMI interface

//A0
sfr P2    		= 0xA0;
sfr	P2D   		= 0xA1;
sfr EO	  		= 0xA2;
sfr ADCIF 		= 0xA4;
sfr ADCDH 		= 0xA6;
sfr ADCDL 		= 0xA7;

//A8
sfr IE    		= 0xA8;
sfr DMAEN    	= 0xA9;
sfr I2STXDMAH   = 0xAA; //I2S TX DMA address high byte 
sfr I2STXDMAL   = 0xAB; //I2S TX DMA address low byte 
sfr I2STXDMALEN = 0xAC; //I2S TX DMA length 
sfr I2SRXDMAH   = 0xAD; //I2S RX DMA address high byte 
sfr I2SRXDMAL   = 0xAE; //I2S RX DMA address low byte 
sfr I2SRXDMALEN = 0xAF; //I2S RX DMA length 

//B0
sfr P3    		= 0xB0;
sfr P3D	  		= 0xB1;
sfr DACDMAH 	= 0xB2; //DAC DMA address high byte
sfr DACDMAL 	= 0xB3; //DAC DMA address low byte 
sfr DACDMALEN 	= 0xB4; //DAC DMA length 
sfr ADCDMAH 	= 0xB5;
sfr ADCDMAL 	= 0xB6;
sfr ADCDMALEN 	= 0xB7;

//B8
sfr IP    		= 0xB8;
sfr IPH   		= 0xB9;
sfr KBP1MSK 	= 0xBA; //Keypad: P1 interrupt mask
sfr KBP2MSK 	= 0xBB; //Keypad: P2 interrupt mask
sfr I2STXLH 	= 0xBC;	//I2S TX left data high byte
sfr I2STXLL 	= 0xBD;	//I2S TX left data low byte
sfr I2STXRH		= 0xBE; //I2S TX right data high byte
sfr I2STXRL		= 0xBF; //I2S TX right data low byte 

//C0
sfr T3CON 		= 0xC0; 
sfr DMASR 		= 0xC1; //DMA status register 
sfr DMAINTTHR 	= 0xC2;
sfr KBINTCTRL 	= 0xC3; //Keypad: interrupt control
sfr I2SRXLH		= 0xC4; //I2S RX left data high byte   
sfr I2SRXLL		= 0xC5; //I2S RX left data low byte 
sfr I2SRXRH		= 0xC6; //I2S RX right data high byte
sfr I2SRXRL		= 0xC7; //I2S RX right data low byte 

//C8
sfr	T2CON 		= 0xC8;
sfr T2CF  		= 0xC9;
sfr RCP2L 		= 0xCA;
sfr RCP2H 		= 0xCB;
sfr TL2  		= 0xCC;
sfr TH2  		= 0xCD;

//D0
sfr PSW   		= 0xD0;
sfr I2SCTR1		= 0xD1; //I2S control register 1  
sfr	I2SCTR2		= 0xD2; //I2S control register 2
sfr	I2SCTR3		= 0xD3; //I2S control register 3

//D8
sfr	SCON2 		= 0xD8;
sfr I2CPRERH 	= 0xD9; //I2C clock prescale high byte
sfr I2CPRERL 	= 0xDA; //I2C clock prescale low byte  
sfr I2CRXR 		= 0xDB; //I2C receive data register 
sfr I2CCTR		= 0xDC; //I2C control register  
sfr I2CSR 		= 0xDD; //I2C status register  
sfr I2CTXR 		= 0xDE; //I2C transmit data register
sfr I2CCR 		= 0xDF; //I2C command register

//E0
sfr ACC 		= 0xE0;
sfr FWDATA      = 0xE1;
sfr T3MOD 		= 0xE2;
sfr TL3 		= 0xE3;
sfr TH3 		= 0xE4;
sfr TL4 		= 0xE5;
sfr TH4 		= 0xE6;
sfr FADDR2      = 0xE7;

//E8
sfr IER1 		= 0xE8;
sfr T5CON 		= 0xE9;
sfr TL5 		= 0xEA;
sfr TH5 		= 0xEB;
sfr RCP5L 		= 0xEC;
sfr RCP5H 		= 0xED;
sfr FADDR1      = 0xEE;
sfr FADDR0      = 0xEF;

//F0
sfr B     		= 0xF0;
sfr SBUF2 		= 0xF2;
sfr UART2 		= 0xF3;
sfr FLPROG 		= 0xF4;
sfr RFISRSTS 	= 0xF5;
sfr RFTXSR 		= 0xF6;
sfr RFRXSR 		= 0xF7;

//F8
sfr IP1   		= 0xF8;
sfr IPH1  		= 0xF9;
sfr FDDO  		= 0xFA;
sfr	FDTCK 		= 0xFB;
sfr	FDCR  		= 0xFC;
sfr	FDDI  		= 0xFD;
sfr WDTCMD 		= 0xFE;
sfr WDTSEL 		= 0xFF;

/*  BIT Registers  */
/*  PSW  */
sbit CY    		= 0xD7;
sbit AC    		= 0xD6;
sbit F0    		= 0xD5;
sbit RS1   		= 0xD4;
sbit RS0   		= 0xD3;
sbit OV    		= 0xD2;
sbit F1    		= 0xD1;
sbit P     		= 0xD0;

/*  TCON  */
sbit TF1   		= 0x8F;
sbit TR1   		= 0x8E;
sbit TF0   		= 0x8D;
sbit TR0   		= 0x8C;
sbit IE1   		= 0x8B;
sbit IT1   		= 0x8A;
sbit IE0   		= 0x89;
sbit IT0   		= 0x88;

/*  T3CON  */
sbit TF4   		= 0xC7;
sbit TR4   		= 0xC6;
sbit TF3   		= 0xC5;
sbit TR3   		= 0xC4;
sbit IE4   		= 0xC3;
sbit IT4   		= 0xC2;
sbit IE3   		= 0xC1;
sbit IT3   		= 0xC0;

/*  T2CON  */
sbit TF2  		= 0xCF;
sbit EXF2  		= 0xCE;
sbit RCLK  		= 0xCD;
sbit TCLK  		= 0xCC;
sbit EXEN2  	= 0xCB;
sbit TR2  		= 0xCA;
sbit NT2  		= 0xC9;
sbit NRL2  		= 0xC8;

/*  IE  */
sbit EA    		= 0xAF;
sbit ESPI0              = 0xAE;
sbit ET2   		= 0xAD;
sbit ES    		= 0xAC;
sbit ET1   		= 0xAB;
sbit EX1   		= 0xAA;
sbit ET0   		= 0xA9;
sbit EX0   		= 0xA8;

/*	IE1	*/
sbit EI13  		= 0xEF;
sbit EI12  		= 0xEE;
sbit EI11  		= 0xED;
sbit EI10  		= 0xEC;
sbit EI9   		= 0xEB;
sbit EI8   		= 0xEA;
sbit EI7  		= 0xE9;
sbit EI6   		= 0xE8;
	

/*  IP  */
sbit PS    		= 0xBC;
sbit PT1   		= 0xBB;
sbit PX1   		= 0xBA;
sbit PT0   		= 0xB9;
sbit PX0   		= 0xB8;

/*  P3  */
//sbit RD    	= 0xB7;
//sbit WR    	= 0xB6;
//sbit T1    	= 0xB5;
//sbit T0    	= 0xB4;
//sbit INT1  	= 0xB3;
//sbit INT0  	= 0xB2;
//sbit TXD   	= 0xB1;
//sbit RXD   	= 0xB0;

/*  SCON  */
sbit SM0   		= 0x9F;
sbit SM1   		= 0x9E;
sbit SM2   		= 0x9D;
sbit REN   		= 0x9C;
sbit TB8   		= 0x9B;
sbit RB8   		= 0x9A;
sbit TI    		= 0x99;
sbit RI    		= 0x98;

/*  SCON2  */
sbit SM0_2   	= 0xDF;
sbit SM1_2   	= 0xDE;
sbit SM2_2   	= 0xDD;
sbit REN_2   	= 0xDC;
sbit TB8_2   	= 0xDB;
sbit RB8_2   	= 0xDA;
sbit TI_2    	= 0xD9;
sbit RI_2    	= 0xD8;

/*	P0	*/
sbit P07 		= 0x87;
sbit P06 		= 0x86;
sbit P05 		= 0x85; 
sbit P04 		= 0x84;
sbit P03 		= 0x83;
sbit P02 		= 0x82;
sbit P01 		= 0x81;
sbit P00 		= 0x80;

/*	P1	*/
sbit P17 		= 0x97;
sbit P16 		= 0x96;
sbit P15 		= 0x95; 
sbit P14 		= 0x94;
sbit P13 		= 0x93;
sbit P12 		= 0x92;
sbit P11 		= 0x91;
sbit P10 		= 0x90;

/*	P2	*/
sbit P27 		= 0xA7;
sbit P26 		= 0xA6;
sbit P25 		= 0xA5; 
sbit P24 		= 0xA4;
sbit P23 		= 0xA3;
sbit P22 		= 0xA2;
sbit P21 		= 0xA1;
sbit P20 		= 0xA0;

/*	P3	*/
sbit P37 		= 0xB7;
sbit P36 		= 0xB6;
sbit P35 		= 0xB5; 
sbit P34 		= 0xB4;
sbit P33 		= 0xB3;
sbit P32		= 0xB2;
sbit P31 		= 0xB1;
sbit P30 		= 0xB0;

#else

/* Test values, need more info */
__sfr __no_init volatile unsigned char CKCON         @ 0x8E; /* CLOCK CONTROL */ 
/* */

//80
__sfr __no_init volatile unsigned char P0    		@ 0x80;
__sfr __no_init volatile unsigned char SP    		@ 0x81;
__sfr __no_init volatile unsigned char DPL   		@ 0x82;
__sfr __no_init volatile unsigned char DPH   		@ 0x83;
__sfr __no_init volatile unsigned char DPL1  		@ 0x84;
__sfr __no_init volatile unsigned char DPH1  		@ 0x85;
__sfr __no_init volatile unsigned char IER2  		@ 0x86;
__sfr __no_init volatile unsigned char PCON  		@ 0x87;

//88
__sfr __no_init volatile unsigned char TCON  		@ 0x88;
__sfr __no_init volatile unsigned char TMOD  		@ 0x89;
__sfr __no_init volatile unsigned char TL0   		@ 0x8A;
__sfr __no_init volatile unsigned char TL1   		@ 0x8B;
__sfr __no_init volatile unsigned char TH0   		@ 0x8C;
__sfr __no_init volatile unsigned char TH1   		@ 0x8D;
__sfr __no_init volatile unsigned char IP2	  		@ 0x8E;
__sfr __no_init volatile unsigned char IP2H  		@ 0x8F;

//90
__sfr __no_init volatile unsigned char P1    		@ 0x90;
__sfr __no_init volatile unsigned char	P1D   		@ 0x91;
__sfr __no_init volatile unsigned char MEX1  		@ 0x94;
__sfr __no_init volatile unsigned char MEX2  		@ 0x95;
__sfr __no_init volatile unsigned char MEX3  		@ 0x96;
__sfr __no_init volatile unsigned char MEXSP 		@ 0x97;
__sfr __no_init volatile unsigned char COBK		@ 0x93;

//98
__sfr __no_init volatile unsigned char SCON  		@ 0x98;
__sfr __no_init volatile unsigned char SBUF  		@ 0x99;
__sfr __no_init volatile unsigned char SYSCFG  	@ 0x9A;//Config UART, I2C, SPI interface
__sfr __no_init volatile unsigned char SPICKR  	@ 0x9B;
__sfr __no_init volatile unsigned char SPITXBUF 	@ 0x9C;
__sfr __no_init volatile unsigned char SPICN    	@ 0x9D;
__sfr __no_init volatile unsigned char SPIRXBUF 	@ 0x9E;
__sfr __no_init volatile unsigned char CHIPCFG   	@ 0x9F;//Config test mode, JTAG, MMI interface

//A0
__sfr __no_init volatile unsigned char P2    		@ 0xA0;
__sfr __no_init volatile unsigned char	P2D   		@ 0xA1;
__sfr __no_init volatile unsigned char EO	  		@ 0xA2;
__sfr __no_init volatile unsigned char ADCIF 		@ 0xA4;
__sfr __no_init volatile unsigned char ADCDH 		@ 0xA6;
__sfr __no_init volatile unsigned char ADCDL 		@ 0xA7;

//A8
__sfr __no_init volatile unsigned char IE    		@ 0xA8;
__sfr __no_init volatile unsigned char DMAEN    	@ 0xA9;
__sfr __no_init volatile unsigned char I2STXDMAH   @ 0xAA; //I2S TX DMA address high byte 
__sfr __no_init volatile unsigned char I2STXDMAL   @ 0xAB; //I2S TX DMA address low byte 
__sfr __no_init volatile unsigned char I2STXDMALEN @ 0xAC; //I2S TX DMA length 
__sfr __no_init volatile unsigned char I2SRXDMAH   @ 0xAD; //I2S RX DMA address high byte 
__sfr __no_init volatile unsigned char I2SRXDMAL   @ 0xAE; //I2S RX DMA address low byte 
__sfr __no_init volatile unsigned char I2SRXDMALEN @ 0xAF; //I2S RX DMA length 

//B0
__sfr __no_init volatile unsigned char P3    		@ 0xB0;
__sfr __no_init volatile unsigned char P3D	  		@ 0xB1;
__sfr __no_init volatile unsigned char DACDMAH 	@ 0xB2; //DAC DMA address high byte
__sfr __no_init volatile unsigned char DACDMAL 	@ 0xB3; //DAC DMA address low byte 
__sfr __no_init volatile unsigned char DACDMALEN 	@ 0xB4; //DAC DMA length 
__sfr __no_init volatile unsigned char ADCDMAH 	@ 0xB5;
__sfr __no_init volatile unsigned char ADCDMAL 	@ 0xB6;
__sfr __no_init volatile unsigned char ADCDMALEN 	@ 0xB7;

//B8
__sfr __no_init volatile unsigned char IP    		@ 0xB8;
__sfr __no_init volatile unsigned char IPH   		@ 0xB9;
__sfr __no_init volatile unsigned char KBP1MSK 	@ 0xBA; //Keypad: P1 interrupt mask
__sfr __no_init volatile unsigned char KBP2MSK 	@ 0xBB; //Keypad: P2 interrupt mask
__sfr __no_init volatile unsigned char I2STXLH 	@ 0xBC;	//I2S TX left data high byte
__sfr __no_init volatile unsigned char I2STXLL 	@ 0xBD;	//I2S TX left data low byte
__sfr __no_init volatile unsigned char I2STXRH		@ 0xBE; //I2S TX right data high byte
__sfr __no_init volatile unsigned char I2STXRL		@ 0xBF; //I2S TX right data low byte 

//C0
__sfr __no_init volatile unsigned char T3CON 		@ 0xC0; 
__sfr __no_init volatile unsigned char DMASR 		@ 0xC1; //DMA status register 
__sfr __no_init volatile unsigned char DMAINTTHR 	@ 0xC2;
__sfr __no_init volatile unsigned char KBINTCTRL 	@ 0xC3; //Keypad: interrupt control
__sfr __no_init volatile unsigned char I2SRXLH		@ 0xC4; //I2S RX left data high byte   
__sfr __no_init volatile unsigned char I2SRXLL		@ 0xC5; //I2S RX left data low byte 
__sfr __no_init volatile unsigned char I2SRXRH		@ 0xC6; //I2S RX right data high byte
__sfr __no_init volatile unsigned char I2SRXRL		@ 0xC7; //I2S RX right data low byte 

//C8
__sfr __no_init volatile unsigned char	T2CON 		@ 0xC8;
__sfr __no_init volatile unsigned char T2CF  		@ 0xC9;
__sfr __no_init volatile unsigned char RCP2L 		@ 0xCA;
__sfr __no_init volatile unsigned char RCP2H 		@ 0xCB;
__sfr __no_init volatile unsigned char TL2  		@ 0xCC;
__sfr __no_init volatile unsigned char TH2  		@ 0xCD;

//D0
__sfr __no_init volatile unsigned char PSW   		@ 0xD0;
__sfr __no_init volatile unsigned char I2SCTR1		@ 0xD1; //I2S control register 1  
__sfr __no_init volatile unsigned char	I2SCTR2		@ 0xD2; //I2S control register 2
__sfr __no_init volatile unsigned char	I2SCTR3		@ 0xD3; //I2S control register 3

//D8
__sfr __no_init volatile unsigned char	SCON2 		@ 0xD8;
__sfr __no_init volatile unsigned char I2CPRERH 	@ 0xD9; //I2C clock prescale high byte
__sfr __no_init volatile unsigned char I2CPRERL 	@ 0xDA; //I2C clock prescale low byte  
__sfr __no_init volatile unsigned char I2CRXR 		@ 0xDB; //I2C receive data register 
__sfr __no_init volatile unsigned char I2CCTR		@ 0xDC; //I2C control register  
__sfr __no_init volatile unsigned char I2CSR 		@ 0xDD; //I2C status register  
__sfr __no_init volatile unsigned char I2CTXR 		@ 0xDE; //I2C transmit data register
__sfr __no_init volatile unsigned char I2CCR 		@ 0xDF; //I2C command register

//E0
__sfr __no_init volatile unsigned char ACC 		@ 0xE0;
__sfr __no_init volatile unsigned char FWDATA      @ 0xE1;
__sfr __no_init volatile unsigned char T3MOD 		@ 0xE2;
__sfr __no_init volatile unsigned char TL3 		@ 0xE3;
__sfr __no_init volatile unsigned char TH3 		@ 0xE4;
__sfr __no_init volatile unsigned char TL4 		@ 0xE5;
__sfr __no_init volatile unsigned char TH4 		@ 0xE6;
__sfr __no_init volatile unsigned char FADDR2      @ 0xE7;

//E8
__sfr __no_init volatile unsigned char IER1 		@ 0xE8;
__sfr __no_init volatile unsigned char T5CON 		@ 0xE9;
__sfr __no_init volatile unsigned char TL5 		@ 0xEA;
__sfr __no_init volatile unsigned char TH5 		@ 0xEB;
__sfr __no_init volatile unsigned char RCP5L 		@ 0xEC;
__sfr __no_init volatile unsigned char RCP5H 		@ 0xED;
__sfr __no_init volatile unsigned char FADDR1      @ 0xEE;
__sfr __no_init volatile unsigned char FADDR0      @ 0xEF;

//F0
__sfr __no_init volatile unsigned char B     		@ 0xF0;
__sfr __no_init volatile unsigned char SBUF2 		@ 0xF2;
__sfr __no_init volatile unsigned char UART2 		@ 0xF3;
__sfr __no_init volatile unsigned char FLPROG 		@ 0xF4;
__sfr __no_init volatile unsigned char RFISRSTS 	@ 0xF5;
__sfr __no_init volatile unsigned char RFTXSR 		@ 0xF6;
__sfr __no_init volatile unsigned char RFRXSR 		@ 0xF7;

//F8
__sfr __no_init volatile unsigned char IP1   		@ 0xF8;
__sfr __no_init volatile unsigned char IPH1  		@ 0xF9;
__sfr __no_init volatile unsigned char FDDO  		@ 0xFA;
__sfr __no_init volatile unsigned char	FDTCK 		@ 0xFB;
__sfr __no_init volatile unsigned char	FDCR  		@ 0xFC;
__sfr __no_init volatile unsigned char	FDDI  		@ 0xFD;
__sfr __no_init volatile unsigned char WDTCMD 		@ 0xFE;
__sfr __no_init volatile unsigned char WDTSEL 		@ 0xFF;

/*  BIT Registers  */
/*  PSW  */
#include <stdbool.h>

__bit  __no_init volatile bool  CY    		@ 0xD7;
__bit  __no_init volatile bool  AC    		@ 0xD6;
__bit  __no_init volatile bool  F0    		@ 0xD5;
__bit  __no_init volatile bool  RS1   		@ 0xD4;
__bit  __no_init volatile bool  RS0   		@ 0xD3;
__bit  __no_init volatile bool  OV    		@ 0xD2;
__bit  __no_init volatile bool  F1    		@ 0xD1;
__bit  __no_init volatile bool  P     		@ 0xD0;

/*  TCON  */
__bit  __no_init volatile bool  TF1   		@ 0x8F;
__bit  __no_init volatile bool  TR1   		@ 0x8E;
__bit  __no_init volatile bool  TF0   		@ 0x8D;
__bit  __no_init volatile bool  TR0   		@ 0x8C;
__bit  __no_init volatile bool  IE1   		@ 0x8B;
__bit  __no_init volatile bool  IT1   		@ 0x8A;
__bit  __no_init volatile bool  IE0   		@ 0x89;
__bit  __no_init volatile bool  IT0   		@ 0x88;

/*  T3CON  */
__bit  __no_init volatile bool  TF4   		@ 0xC7;
__bit  __no_init volatile bool  TR4   		@ 0xC6;
__bit  __no_init volatile bool  TF3   		@ 0xC5;
__bit  __no_init volatile bool  TR3   		@ 0xC4;
__bit  __no_init volatile bool  IE4   		@ 0xC3;
__bit  __no_init volatile bool  IT4   		@ 0xC2;
__bit  __no_init volatile bool  IE3   		@ 0xC1;
__bit  __no_init volatile bool  IT3   		@ 0xC0;

/*  T2CON  */
__bit  __no_init volatile bool  TF2  		@ 0xCF;
__bit  __no_init volatile bool  EXF2  		@ 0xCE;
__bit  __no_init volatile bool  RCLK  		@ 0xCD;
__bit  __no_init volatile bool  TCLK  		@ 0xCC;
__bit  __no_init volatile bool  EXEN2  	@ 0xCB;
__bit  __no_init volatile bool  TR2  		@ 0xCA;
__bit  __no_init volatile bool  NT2  		@ 0xC9;
__bit  __no_init volatile bool  NRL2  		@ 0xC8;

/*  IE  */
__bit  __no_init volatile bool  EA    		@ 0xAF;
__bit  __no_init volatile bool  ET2   		@ 0xAD;
__bit  __no_init volatile bool  ES    		@ 0xAC;
__bit  __no_init volatile bool  ET1   		@ 0xAB;
__bit  __no_init volatile bool  EX1   		@ 0xAA;
__bit  __no_init volatile bool  ET0   		@ 0xA9;
__bit  __no_init volatile bool  EX0   		@ 0xA8;

/*	IE1	*/
__bit  __no_init volatile bool  EI13  		@ 0xEF;
__bit  __no_init volatile bool  EI12  		@ 0xEE;
__bit  __no_init volatile bool  EI11  		@ 0xED;
__bit  __no_init volatile bool  EI10  		@ 0xEC;
__bit  __no_init volatile bool  EI9   		@ 0xEB;
__bit  __no_init volatile bool  EI8   		@ 0xEA;
__bit  __no_init volatile bool  EI7  		@ 0xE9;
__bit  __no_init volatile bool  EI6   		@ 0xE8;
	

/*  IP  */
__bit  __no_init volatile bool  PS    		@ 0xBC;
__bit  __no_init volatile bool  PT1   		@ 0xBB;
__bit  __no_init volatile bool  PX1   		@ 0xBA;
__bit  __no_init volatile bool  PT0   		@ 0xB9;
__bit  __no_init volatile bool  PX0   		@ 0xB8;

/*  P3  */
//__bit  __no_init volatile bool  RD    	@ 0xB7;
//__bit  __no_init volatile bool  WR    	@ 0xB6;
//__bit  __no_init volatile bool  T1    	@ 0xB5;
//__bit  __no_init volatile bool  T0    	@ 0xB4;
__bit  __no_init volatile bool  INT1  	@ 0xB3;
//__bit  __no_init volatile bool  INT0  	@ 0xB2;
//__bit  __no_init volatile bool  TXD   	@ 0xB1;
//__bit  __no_init volatile bool  RXD   	@ 0xB0;

/*  SCON  */
__bit  __no_init volatile bool  SM0   		@ 0x9F;
__bit  __no_init volatile bool  SM1   		@ 0x9E;
__bit  __no_init volatile bool  SM2   		@ 0x9D;
__bit  __no_init volatile bool  REN   		@ 0x9C;
__bit  __no_init volatile bool  TB8   		@ 0x9B;
__bit  __no_init volatile bool  RB8   		@ 0x9A;
__bit  __no_init volatile bool  TI    		@ 0x99;
__bit  __no_init volatile bool  RI    		@ 0x98;

/*  SCON2  */
__bit  __no_init volatile bool  SM0_2   	@ 0xDF;
__bit  __no_init volatile bool  SM1_2   	@ 0xDE;
__bit  __no_init volatile bool  SM2_2   	@ 0xDD;
__bit  __no_init volatile bool  REN_2   	@ 0xDC;
__bit  __no_init volatile bool  TB8_2   	@ 0xDB;
__bit  __no_init volatile bool  RB8_2   	@ 0xDA;
__bit  __no_init volatile bool  TI_2    	@ 0xD9;
__bit  __no_init volatile bool  RI_2    	@ 0xD8;

/*	P0	*/
__bit  __no_init volatile bool  P07 		@ 0x87;
__bit  __no_init volatile bool  P06 		@ 0x86;
__bit  __no_init volatile bool  P05 		@ 0x85; 
__bit  __no_init volatile bool  P04 		@ 0x84;
__bit  __no_init volatile bool  P03 		@ 0x83;
__bit  __no_init volatile bool  P02 		@ 0x82;
__bit  __no_init volatile bool  P01 		@ 0x81;
__bit  __no_init volatile bool  P00 		@ 0x80;

/*	P1	*/
__bit  __no_init volatile bool  P17 		@ 0x97;
__bit  __no_init volatile bool  P16 		@ 0x96;
__bit  __no_init volatile bool  P15 		@ 0x95; 
__bit  __no_init volatile bool  P14 		@ 0x94;
__bit  __no_init volatile bool  P13 		@ 0x93;
__bit  __no_init volatile bool  P12 		@ 0x92;
__bit  __no_init volatile bool  P11 		@ 0x91;
__bit  __no_init volatile bool  P10 		@ 0x90;

/*	P2	*/
__bit  __no_init volatile bool  P27 		@ 0xA7;
__bit  __no_init volatile bool  P26 		@ 0xA6;
__bit  __no_init volatile bool  P25 		@ 0xA5; 
__bit  __no_init volatile bool  P24 		@ 0xA4;
__bit  __no_init volatile bool  P23 		@ 0xA3;
__bit  __no_init volatile bool  P22 		@ 0xA2;
__bit  __no_init volatile bool  P21 		@ 0xA1;
__bit  __no_init volatile bool  P20 		@ 0xA0;

__bit  __no_init volatile bool  ESPI0           @ 0xAE;

/*	P3	*/
__bit  __no_init volatile bool  P37 		@ 0xB7;
__bit  __no_init volatile bool  P36 		@ 0xB6;
__bit  __no_init volatile bool  P35 		@ 0xB5; 
__bit  __no_init volatile bool  P34 		@ 0xB4;
__bit  __no_init volatile bool  P33 		@ 0xB3;
__bit  __no_init volatile bool  P32		@ 0xB2;
__bit  __no_init volatile bool  P31 		@ 0xB1;
__bit  __no_init volatile bool  P30 		@ 0xB0;

#endif  /* IAR */

#define         SADRL           0x03
#define         SADRH           0x04
#define         EADR0           0x05
#define         EADR1           0x06
#define         EADR2           0x07
#define         EADR3           0x08
#define         EADR4           0x09
#define         EADR5           0x0a
#define         EADR6           0x0b
#define         EADR7           0x0c

#define ZB_SREG_RXMCR              0x00
#define ZB_SREG_PANIDL             0x01
#define ZB_SREG_PANIDH             0x02
#define ZB_SREG_RXFLUSH            0x0D
#define ZB_SREG_ORDER              0x10
#define ZB_SREG_TXMCR              0x11
#define ZB_SREG_ACKTMOUT           0x12
#define ZB_SREG_FIFOEN             0x18
#define ZB_SREG_TXNTRIG            0x1B
#define ZB_SREG_TXPEND             0x21
#define ZB_SREG_WAKECTL            0x22
#define ZB_SREG_TXSR               0x24
#define ZB_SREG_TXBCNMSK           0x25
#define ZB_SREG_SOFTRST            0x2A
#define ZB_SREG_ISRSTS             0x31
#define ZB_SREG_INTMSK             0x32
#define ZB_SREG_SLPACK             0x35
#define ZB_SREG_RFCTL              0x36
#define ZB_SREG_BBREG2             0x3A
#define ZB_SREG_BBREG6             0x3E
#define ZB_SREG_BBREG7             0x3F

#define ZB_LREG_RFCTRL0            0x200
#define ZB_LREG_RFCTL2             0x202
#define ZB_LREG_RFCTL3             0x203
#define ZB_LREG_RFCTL6             0x206
#define ZB_LREG_RFCTL7             0x207
#define ZB_LREG_RFCTL8             0x208
#define ZB_LREG_RFCTRL77           0x277
#define ZB_LREG_RSSI               0x210
#define ZB_LREG_SCLKDIV            0x220
#define ZB_LREG_TESTMODE           0x22F
#define ZB_LREG_ASSOEADR0          0x230
#define ZB_LREG_ASSOEADR1          0x231
#define ZB_LREG_ASSOEADR2          0x232
#define ZB_LREG_ASSOEADR3          0x233
#define ZB_LREG_ASSOEADR4          0x234
#define ZB_LREG_ASSOEADR5          0x235
#define ZB_LREG_ASSOEADR6          0x236
#define ZB_LREG_ASSOEADR7          0x237
#define ZB_LREG_ASSOSADR0          0x238
#define ZB_LREG_ASSOSADR1          0x239
#define ZB_LREG_RXFRMTYPE          0x23C
#define TXMCR_SLOTTED_MASK         0x20


#if 0
// ----------------------------------------------------
// UZ2410 register address definitions
// ----------------------------------------------------
#define SREG00 *(volatile unsigned char xdata *)(0xF800)
#define SREG01 *(volatile unsigned char xdata *)(0xF801)
#define SREG02 *(volatile unsigned char xdata *)(0xF802)
#define SREG03 *(volatile unsigned char xdata *)(0xF803)
#define SREG04 *(volatile unsigned char xdata *)(0xF804)
#define SREG05 *(volatile unsigned char xdata *)(0xF805)
#define SREG06 *(volatile unsigned char xdata *)(0xF806)
#define SREG07 *(volatile unsigned char xdata *)(0xF807)
#define SREG08 *(volatile unsigned char xdata *)(0xF808)
#define SREG09 *(volatile unsigned char xdata *)(0xF809)
#define SREG0A *(volatile unsigned char xdata *)(0xF80A)
#define SREG0B *(volatile unsigned char xdata *)(0xF80B)
#define SREG0C *(volatile unsigned char xdata *)(0xF80C)
#define SREG0D *(volatile unsigned char xdata *)(0xF80D)
#define SREG0E *(volatile unsigned char xdata *)(0xF80E)
#define SREG0F *(volatile unsigned char xdata *)(0xF80F)
#define SREG10 *(volatile unsigned char xdata *)(0xF810)
#define SREG11 *(volatile unsigned char xdata *)(0xF811)
#define SREG12 *(volatile unsigned char xdata *)(0xF812)
#define SREG13 *(volatile unsigned char xdata *)(0xF813)
#define SREG14 *(volatile unsigned char xdata *)(0xF814)
#define SREG15 *(volatile unsigned char xdata *)(0xF815)
#define SREG16 *(volatile unsigned char xdata *)(0xF816)
#define SREG17 *(volatile unsigned char xdata *)(0xF817)
#define SREG18 *(volatile unsigned char xdata *)(0xF818)
#define SREG19 *(volatile unsigned char xdata *)(0xF819)
#define SREG1A *(volatile unsigned char xdata *)(0xF81A)
#define SREG1B *(volatile unsigned char xdata *)(0xF81B)
#define SREG1C *(volatile unsigned char xdata *)(0xF81C)
#define SREG1D *(volatile unsigned char xdata *)(0xF81D)
#define SREG1E *(volatile unsigned char xdata *)(0xF81E)
#define SREG1F *(volatile unsigned char xdata *)(0xF81F)
#define SREG20 *(volatile unsigned char xdata *)(0xF820)
#define SREG21 *(volatile unsigned char xdata *)(0xF821)
#define SREG22 *(volatile unsigned char xdata *)(0xF822)
#define SREG23 *(volatile unsigned char xdata *)(0xF823)
#define SREG24 *(volatile unsigned char xdata *)(0xF824)
#define SREG25 *(volatile unsigned char xdata *)(0xF825)
#define SREG26 *(volatile unsigned char xdata *)(0xF826)
#define SREG27 *(volatile unsigned char xdata *)(0xF827)
#define SREG28 *(volatile unsigned char xdata *)(0xF828)
#define SREG29 *(volatile unsigned char xdata *)(0xF829)
#define SREG2A *(volatile unsigned char xdata *)(0xF82A)
#define SREG2B *(volatile unsigned char xdata *)(0xF82B)
#define SREG2C *(volatile unsigned char xdata *)(0xF82C)
#define SREG2D *(volatile unsigned char xdata *)(0xF82D)
#define SREG2E *(volatile unsigned char xdata *)(0xF82E)
#define SREG2F *(volatile unsigned char xdata *)(0xF82F)
#define SREG30 *(volatile unsigned char xdata *)(0xF830)
#define SREG31 *(volatile unsigned char xdata *)(0xF831)
#define SREG32 *(volatile unsigned char xdata *)(0xF832)
#define SREG33 *(volatile unsigned char xdata *)(0xF833)
#define SREG34 *(volatile unsigned char xdata *)(0xF834)
#define SREG35 *(volatile unsigned char xdata *)(0xF835)
#define SREG36 *(volatile unsigned char xdata *)(0xF836)
#define SREG37 *(volatile unsigned char xdata *)(0xF837)
#define SREG38 *(volatile unsigned char xdata *)(0xF838)
#define SREG39 *(volatile unsigned char xdata *)(0xF839)
#define SREG3A *(volatile unsigned char xdata *)(0xF83A)
#define SREG3B *(volatile unsigned char xdata *)(0xF83B)
#define SREG3C *(volatile unsigned char xdata *)(0xF83C)
#define SREG3D *(volatile unsigned char xdata *)(0xF83D)
#define SREG3E *(volatile unsigned char xdata *)(0xF83E)
#define SREG3F *(volatile unsigned char xdata *)(0xF83F)

#define LREG00 *(volatile unsigned char xdata *)(0xFE00)
#define LREG01 *(volatile unsigned char xdata *)(0xFE01)
#define LREG02 *(volatile unsigned char xdata *)(0xFE02)
#define LREG03 *(volatile unsigned char xdata *)(0xFE03)
#define LREG04 *(volatile unsigned char xdata *)(0xFE04)
#define LREG05 *(volatile unsigned char xdata *)(0xFE05)
#define LREG06 *(volatile unsigned char xdata *)(0xFE06)
#define LREG07 *(volatile unsigned char xdata *)(0xFE07)
#define LREG08 *(volatile unsigned char xdata *)(0xFE08)
#define LREG09 *(volatile unsigned char xdata *)(0xFE09)
#define LREG0A *(volatile unsigned char xdata *)(0xFE0A)
#define LREG0B *(volatile unsigned char xdata *)(0xFE0B)
#define LREG0C *(volatile unsigned char xdata *)(0xFE0C)
#define LREG0D *(volatile unsigned char xdata *)(0xFE0D)
#define LREG0E *(volatile unsigned char xdata *)(0xFE0E)
#define LREG0F *(volatile unsigned char xdata *)(0xFE0F)
#define LREG10 *(volatile unsigned char xdata *)(0xFE10)
#define LREG11 *(volatile unsigned char xdata *)(0xFE11)
#define LREG12 *(volatile unsigned char xdata *)(0xFE12)
#define LREG13 *(volatile unsigned char xdata *)(0xFE13)
#define LREG14 *(volatile unsigned char xdata *)(0xFE14)
#define LREG15 *(volatile unsigned char xdata *)(0xFE15)
#define LREG16 *(volatile unsigned char xdata *)(0xFE16)
#define LREG17 *(volatile unsigned char xdata *)(0xFE17)
#define LREG18 *(volatile unsigned char xdata *)(0xFE18)
#define LREG19 *(volatile unsigned char xdata *)(0xFE19)
#define LREG1A *(volatile unsigned char xdata *)(0xFE1A)
#define LREG1B *(volatile unsigned char xdata *)(0xFE1B)
#define LREG1C *(volatile unsigned char xdata *)(0xFE1C)
#define LREG1D *(volatile unsigned char xdata *)(0xFE1D)
#define LREG1E *(volatile unsigned char xdata *)(0xFE1E)
#define LREG1F *(volatile unsigned char xdata *)(0xFE1F)
#define LREG20 *(volatile unsigned char xdata *)(0xFE20)
#define LREG21 *(volatile unsigned char xdata *)(0xFE21)
#define LREG22 *(volatile unsigned char xdata *)(0xFE22)
#define LREG23 *(volatile unsigned char xdata *)(0xFE23)
#define LREG24 *(volatile unsigned char xdata *)(0xFE24)
#define LREG25 *(volatile unsigned char xdata *)(0xFE25)
#define LREG26 *(volatile unsigned char xdata *)(0xFE26)
#define LREG27 *(volatile unsigned char xdata *)(0xFE27)
#define LREG28 *(volatile unsigned char xdata *)(0xFE28)
#define LREG29 *(volatile unsigned char xdata *)(0xFE29)
#define LREG2A *(volatile unsigned char xdata *)(0xFE2A)
#define LREG2B *(volatile unsigned char xdata *)(0xFE2B)
#define LREG2C *(volatile unsigned char xdata *)(0xFE2C)
#define LREG2D *(volatile unsigned char xdata *)(0xFE2D)
#define LREG2E *(volatile unsigned char xdata *)(0xFE2E)
#define LREG2F *(volatile unsigned char xdata *)(0xFE2F)
#define LREG30 *(volatile unsigned char xdata *)(0xFE30)
#define LREG31 *(volatile unsigned char xdata *)(0xFE31)
#define LREG32 *(volatile unsigned char xdata *)(0xFE32)
#define LREG33 *(volatile unsigned char xdata *)(0xFE33)
#define LREG34 *(volatile unsigned char xdata *)(0xFE34)
#define LREG35 *(volatile unsigned char xdata *)(0xFE35)
#define LREG36 *(volatile unsigned char xdata *)(0xFE36)
#define LREG37 *(volatile unsigned char xdata *)(0xFE37)
#define LREG38 *(volatile unsigned char xdata *)(0xFE38)
#define LREG39 *(volatile unsigned char xdata *)(0xFE39)
#define LREG3A *(volatile unsigned char xdata *)(0xFE3A)
#define LREG3B *(volatile unsigned char xdata *)(0xFE3B)
#define LREG3C *(volatile unsigned char xdata *)(0xFE3C)
#define LREG3D *(volatile unsigned char xdata *)(0xFE3D)
#define LREG3E *(volatile unsigned char xdata *)(0xFE3E)
#define LREG3F *(volatile unsigned char xdata *)(0xFE3F)
#define LREG40 *(volatile unsigned char xdata *)(0xFE40)
#define LREG41 *(volatile unsigned char xdata *)(0xFE41)
#define LREG42 *(volatile unsigned char xdata *)(0xFE42)
#define LREG43 *(volatile unsigned char xdata *)(0xFE43)
#define LREG44 *(volatile unsigned char xdata *)(0xFE44)
#define LREG45 *(volatile unsigned char xdata *)(0xFE45)
#define LREG46 *(volatile unsigned char xdata *)(0xFE46)
#define LREG47 *(volatile unsigned char xdata *)(0xFE47)
#define LREG48 *(volatile unsigned char xdata *)(0xFE48)
#define LREG49 *(volatile unsigned char xdata *)(0xFE49)
#define LREG4A *(volatile unsigned char xdata *)(0xFE4A)
#define LREG4B *(volatile unsigned char xdata *)(0xFE4B)
#define LREG4C *(volatile unsigned char xdata *)(0xFE4C)
#define LREG4D *(volatile unsigned char xdata *)(0xFE4D)
#define LREG4E *(volatile unsigned char xdata *)(0xFE4E)
#define LREG4F *(volatile unsigned char xdata *)(0xFE4F)
#define LREG50 *(volatile unsigned char xdata *)(0xFE50)
#define LREG51 *(volatile unsigned char xdata *)(0xFE51)
#define LREG52 *(volatile unsigned char xdata *)(0xFE52)
#define LREG53 *(volatile unsigned char xdata *)(0xFE53)
#define LREG54 *(volatile unsigned char xdata *)(0xFE54)
#define LREG55 *(volatile unsigned char xdata *)(0xFE55)
#define LREG56 *(volatile unsigned char xdata *)(0xFE56)
#define LREG57 *(volatile unsigned char xdata *)(0xFE57)
#define LREG58 *(volatile unsigned char xdata *)(0xFE58)
#define LREG59 *(volatile unsigned char xdata *)(0xFE59)
#define LREG5A *(volatile unsigned char xdata *)(0xFE5A)
#define LREG5B *(volatile unsigned char xdata *)(0xFE5B)
#define LREG5C *(volatile unsigned char xdata *)(0xFE5C)
#define LREG5D *(volatile unsigned char xdata *)(0xFE5D)
#define LREG5E *(volatile unsigned char xdata *)(0xFE5E)
#define LREG5F *(volatile unsigned char xdata *)(0xFE5F)
#define LREG60 *(volatile unsigned char xdata *)(0xFE60)
#define LREG61 *(volatile unsigned char xdata *)(0xFE61)
#define LREG62 *(volatile unsigned char xdata *)(0xFE62)
#define LREG63 *(volatile unsigned char xdata *)(0xFE63)
#define LREG64 *(volatile unsigned char xdata *)(0xFE64)
#define LREG65 *(volatile unsigned char xdata *)(0xFE65)
#define LREG66 *(volatile unsigned char xdata *)(0xFE66)
#define LREG67 *(volatile unsigned char xdata *)(0xFE67)
#define LREG68 *(volatile unsigned char xdata *)(0xFE68)
#define LREG69 *(volatile unsigned char xdata *)(0xFE69)
#define LREG6A *(volatile unsigned char xdata *)(0xFE6A)
#define LREG6B *(volatile unsigned char xdata *)(0xFE6B)
#define LREG6C *(volatile unsigned char xdata *)(0xFE6C)
#define LREG6D *(volatile unsigned char xdata *)(0xFE6D)
#define LREG6E *(volatile unsigned char xdata *)(0xFE6E)
#define LREG6F *(volatile unsigned char xdata *)(0xFE6F)
#define LREG70 *(volatile unsigned char xdata *)(0xFE70)
#define LREG71 *(volatile unsigned char xdata *)(0xFE71)
#define LREG72 *(volatile unsigned char xdata *)(0xFE72)
#define LREG73 *(volatile unsigned char xdata *)(0xFE73)
#define LREG74 *(volatile unsigned char xdata *)(0xFE74)
#define LREG75 *(volatile unsigned char xdata *)(0xFE75)
#define LREG76 *(volatile unsigned char xdata *)(0xFE76)
#define LREG77 *(volatile unsigned char xdata *)(0xFE77)
#define LREG78 *(volatile unsigned char xdata *)(0xFE78)
#define LREG79 *(volatile unsigned char xdata *)(0xFE79)
#define LREG7A *(volatile unsigned char xdata *)(0xFE7A)
#define LREG7B *(volatile unsigned char xdata *)(0xFE7B)
#define LREG7C *(volatile unsigned char xdata *)(0xFE7C)
#define LREG7D *(volatile unsigned char xdata *)(0xFE7D)
#define LREG7E *(volatile unsigned char xdata *)(0xFE7E)
#define LREG7F *(volatile unsigned char xdata *)(0xFE7F)
#endif

#define RI0_int 0x23
#define TF0_int 0x0B
#define IE1_int 0x13

#endif  /* ZB_UZ2410_H */
