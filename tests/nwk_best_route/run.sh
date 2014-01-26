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
        if [ -f core ]
        then
            echo 'Somebody has crashed'
            killch
            exit 1
        fi
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
    kill $c_pid $r_pid1 $r_pid2 $r_pid3 $r_pid4 $ns_pid
}

killch_ex() {
    killch
    echo Interrupted by user!
    exit 1
}

trap killch_ex TERM INT

rm -f *.log *.pcap *.dump core

echo "run ns"
../../devtools/network_simulator/network_simulator --nNode=5 --pipeName=/tmp/zt --xgml=graph.xgml >ns.txt 2>&1 &
ns_pid=$!
sleep 1

echo "run coordinator"
./nwk_best_route_zc /tmp/zt0.write /tmp/zt0.read &
c_pid=$!
wait_for_start zc
echo ZC1 STARTED OK

echo "run zr1"
./nwk_best_route_zr /tmp/zt1.write /tmp/zt1.read 1 &
r_pid1=$!
wait_for_start zr1
echo ZR1 STARTED OK

echo "run zr2"
./nwk_best_route_zr /tmp/zt2.write /tmp/zt2.read 2 &
r_pid2=$!
wait_for_start zr2
echo ZR2 STARTED OK

echo "run zr3"
./nwk_best_route_zr /tmp/zt3.write /tmp/zt3.read 3 &
r_pid3=$!
wait_for_start zr3
echo ZR3 STARTED OK

sleep 2

echo "run zr4"
./nwk_best_route_zr4 /tmp/zt4.write /tmp/zt4.read &
r_pid4=$!
wait_for_start zr4
echo ZR4 STARTED OK

sleep 30

echo 'shutdown...'
killch


set - `ls *dump`
../../devtools/dump_converter/dump_converter -ns $1 c.pcap
../../devtools/dump_converter/dump_converter -ns $2 r1.pcap
../../devtools/dump_converter/dump_converter -ns $3 r2.pcap
../../devtools/dump_converter/dump_converter -ns $4 r3.pcap
../../devtools/dump_converter/dump_converter -ns $5 r4.pcap

echo 'Now verify traffic dump, please!'

