#/***************************************************************************
#*                      ZBOSS ZigBee Pro 2007 stack                         *
#*                                                                          *
#*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
#*                       http://www.dsr-wireless.com                        *
#*                                                                          *
#*                            All rights reserved.                          *
#*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
#*                       http://www.claridy.com/                            *
#*                                                                          *
#*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
#*                             Hsinchu, Taiwan.                             *
#*                       http://www.ubec.com.tw/                            *
#*                                                                          *
#*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
#*                       http://www.dsr-wireless.com                        *
#*                                                                          *
#*                            All rights reserved.                          *
#*                                                                          *
#*                                                                          *
#* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
#* under either the terms of the Commercial License or the GNU General      *
#* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
#* may choose which license to receive this code under (except as noted in  *
#* per-module LICENSE files).                                               *
#*                                                                          *
#* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
#* Research LLC.                                                            *
#*                                                                          *
#* GNU General Public License Usage                                         *
#* This file may be used under the terms of the GNU General Public License  *
#* version 2.0 as published by the Free Software Foundation and appearing   *
#* in the file LICENSE.GPL included in the packaging of this file.  Please  *
#* review the following information to ensure the GNU General Public        *
#* License version 2.0 requirements will be met:                            *
#* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
#*                                                                          *
#* Commercial Usage                                                         *
#* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
#* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
#* Agreement provided with the Software or, alternatively, in accordance    *
#* with the terms contained in a written agreement between you and          *
#* ClarIDy/UBEC/DSR.                                                        *
#*                                                                          *
#****************************************************************************
#PURPOSE: Trivial NWK test makefile
#*/

BUILD_HOME=../..
include $(BUILD_HOME)/Options

SRCS1 = zdo_zc_lqi_test.c
OBJS1 = ${SRCS1:.c=.o}
SRCS2 = zdo_zr1_lqi_test.c
OBJS2 = ${SRCS2:.c=.o}

SRCS3 = zdo_ze1_lqi_test.c
OBJS3 = ${SRCS3:.c=.o}

SRCS4 = zdo_ze2_lqi_test.c
OBJS4 = ${SRCS4:.c=.o}

SRCS5 = zdo_ze3_lqi_test.c
OBJS5 = ${SRCS5:.c=.o}

$(BIN)zdo_zc_lqi_test$(EXE) : $(OBJS1)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_zr1_lqi_test$(EXE) : $(OBJS2)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_ze1_lqi_test$(EXE) : $(OBJS3)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_ze2_lqi_test$(EXE) : $(OBJS4)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_ze3_lqi_test$(EXE) : $(OBJS5)  $(LIBS)
	$(LINK_BINARY)

BINS = $(BIN)zdo_zc_lqi_test$(EXE) $(BIN)zdo_zr1_lqi_test$(EXE) $(BIN)zdo_ze1_lqi_test$(EXE) $(BIN)zdo_ze2_lqi_test$(EXE) $(BIN)zdo_ze3_lqi_test$(EXE)

all: $(BINS)
