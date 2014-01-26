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

PIPE_NAME=/tmp/aaa

wait_for_start() {
    nm=$1
    s=''
    while [ A"$s" = A ]
    do
        sleep 1
        s=`grep Device zdo_${nm}*.log`
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
    kill $r2PID $r1PID $cPID $PipePID
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
../../../devtools/network_simulator/network_simulator --nNode=3 --pipeName=${PIPE_NAME} --xgml=tp_aps_bv_12_i.xgml 1>sim_log.txt 2>&1 &
PipePID=$!

sleep 1

echo "run coordinator"
./tp_aps_bv_12_i_zc ${PIPE_NAME}0.write ${PIPE_NAME}0.read &
cPID=$!
wait_for_start zc
echo ZC STARTED OK

echo "run zr1"
./tp_aps_bv_12_i_zr1 ${PIPE_NAME}1.write ${PIPE_NAME}1.read &
r1PID=$!
wait_for_start zr1
echo ZR1 STARTED OK

echo "run zr2"
./tp_aps_bv_12_i_zr2 ${PIPE_NAME}2.write ${PIPE_NAME}2.read &
r2PID=$!
wait_for_start zr2
echo ZR2 STARTED OK

sleep 90

echo shutdown...
killch

set - `ls *dump`
../../../devtools/dump_converter/dump_converter -ns $1 zc.pcap
../../../devtools/dump_converter/dump_converter -ns $2 zr1.pcap
../../../devtools/dump_converter/dump_converter -ns $3 zr2.pcap

echo 'Now verify traffic dump, please!'
