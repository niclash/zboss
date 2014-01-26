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
# PURPOSE: Formation and discovery tests.
#*/

BUILD_HOME=../..
include $(BUILD_HOME)/Options

SRCS1 = nwk_passive_ack_zc.c
OBJS1 = ${SRCS1:.c=.o}

SRCS2 = nwk_passive_ack_zr1.c
OBJS2 = ${SRCS2:.c=.o}

SRCS3 = nwk_passive_ack_zed2.c
OBJS3 = ${SRCS3:.c=.ed.o}

SRCS4 = nwk_passive_ack_zr3.c
OBJS4 = ${SRCS4:.c=.o}

$(BIN)nwk_passive_ack_zc$(EXE) : $(OBJS1)  $(LIBS)
	$(LINK_BINARY)

$(BIN)nwk_passive_ack_zr1$(EXE) : $(OBJS2)  $(LIBS)
	$(LINK_BINARY)

$(BIN)nwk_passive_ack_zed2$(EXE) : $(OBJS3)  $(LIBS_ED)
	$(LINK_BINARY_ED)

$(BIN)nwk_passive_ack_zr3$(EXE) : $(OBJS4)  $(LIBS)
	$(LINK_BINARY)


BINS = $(BIN)nwk_passive_ack_zc$(EXE) $(BIN)nwk_passive_ack_zr1$(EXE) $(BIN)nwk_passive_ack_zed2$(EXE) $(BIN)nwk_passive_ack_zr3$(EXE)

all: $(BINS)

#BINS_TEST='run_test.sh'
