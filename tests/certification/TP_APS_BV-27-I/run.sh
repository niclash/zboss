#!/bin/sh
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
#PURPOSE: 
#*/

wait_for_start() {
    nm=$1
    s=''
    while [ A"$s" = A ]
    do
        sleep 1
        s=`grep Device tp_aps_bv_27_i_${nm}*.log`
    done
    if echo $s | grep OK
    then
        return
    else
        echo $s
        killch
        exit 1
    fi
}

killch() {
    kill $routerPID1 $routerPID2 $coordPID $PipePID
}

killch_ex() {
    killch
    echo Interrupted by user!
    exit 1
}

trap killch_ex TERM INT

export LD_LIBRARY_PATH=`pwd`/../../../../ns/ns-3.7/build/debug
rm -f *.log *.pcap *.dump

echo "run ns-3"
../../../devtools/network_simulator/network_simulator --nNode=3 --pipeName=/tmp/aaa &
PipePID=$!

sleep 5

echo "run coordinator"
./tp_aps_bv_27_i_gZC /tmp/aaa0.write /tmp/aaa0.read &
coordPID=$!
wait_for_start gZC

echo gZC STARTED OK
sleep 1

echo "run router 1"
./tp_aps_bv_27_i_DUTZR1 /tmp/aaa1.write /tmp/aaa1.read &
routerPID1=$!
wait_for_start DUTZR1

echo DUTZR1 STARTED OK
sleep 1

echo "run router 2"
./tp_aps_bv_27_i_gZR2 /tmp/aaa2.write /tmp/aaa2.read &
routerPID2=$!
wait_for_start gZR2

echo gZR2 STARTED OK
sleep 1

sleep 100

echo shutdown...
killch


set - `ls *dump`
../../../devtools/dump_converter/dump_converter -ns $1 c.pcap
../../../devtools/dump_converter/dump_converter -ns $2 r1.pcap
../../../devtools/dump_converter/dump_converter -ns $3 r2.pcap
