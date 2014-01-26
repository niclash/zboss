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

SRCS1 = zdo_start_zc.c
OBJS1 = ${SRCS1:.c=.o}
SRCS2 = zdo_start_zr.c
OBJS2 = ${SRCS2:.c=.o}
SRCS3 = zdo_start_ze.c
OBJS3 = ${SRCS3:.c=.ed.o}


$(BIN)zdo_start_zc$(EXE) : $(OBJS1)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_start_zr$(EXE) : $(OBJS2)  $(LIBS)
	$(LINK_BINARY)

$(BIN)zdo_start_ze$(EXE) : $(OBJS3)  $(LIBS_ED)
	$(LINK_BINARY_ED)


#$(BIN)zdo_start_zc_apss$(EXE) : $(SRCS1)  $(LIBS)
#	$(CC) $(CFLAGS) -DAPS_SECUR  $(DEFINES) $(INCPATH) -o $@ $< $(LIBS) $(LIBS)


BINS = $(BIN)zdo_start_zc$(EXE) $(BIN)zdo_start_ze$(EXE) $(BIN)zdo_start_zr$(EXE)
all: $(BINS)

#BINS_TEST='run_test.sh'
