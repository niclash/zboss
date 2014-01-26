#!/bin/sh
#
#/***************************************************************************
#*                                                                          *
#* INSERT COPYRIGHT HERE!                                                   *
#*                                                                          *
#****************************************************************************
#
# Purpose:
#*/

wait_for_start() {
    nm=$1
    s=''
    while [ A"$s" = A ]
    do
        sleep 1
        if [ -f core ]
        then
            echo 'Somebody has crashed (ns?)'
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
    kill $router2PID $router1PID $coordPID $PipePID
}

killch_ex() {
    killch
    echo Interrupted by user!
    exit 1
}

trap killch_ex TERM INT


rm -f *.log *.pcap *.dump core

echo "run ns"

../../devtools/network_simulator/network_simulator --nNode=3 --pipeName=/tmp/zt >ns.txt 2>&1 &
PipePID=$!
sleep 1

echo "run coordinator"
./remove_dev_zc /tmp/zt0.write /tmp/zt0.read &
coordPID=$!
wait_for_start zc

echo ZC STARTED OK

echo "run zr1"
./remove_dev_zr1 /tmp/zt1.write /tmp/zt1.read &
router1PID=$!
wait_for_start zr1

echo ZR1 STARTED OK

echo "run zr2"
./remove_dev_zr2 /tmp/zt2.write /tmp/zt2.read &
router2PID=$!
wait_for_start zr2

echo ZR2 STARTED OK

sleep 40

if [ -f core ]
then
    echo 'Somebody has crashed'
fi

echo 'shutdown...'
killch


set - `ls *dump`
../../devtools/dump_converter/dump_converter -ns $1 c.pcap
../../devtools/dump_converter/dump_converter -ns $2 r1.pcap
../../devtools/dump_converter/dump_converter -ns $3 r2.pcap

echo 'Now verify traffic dump, please!'

