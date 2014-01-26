#!/bin/sh
#
# Generate lines like ?PR?_?ZB_SCHED_LOOP_ITERATION?ZB_SCHEDULER ! ?PR?_ZB_MLME_ACK_ACCEPT?MAC
# for overlay section.
#

gfind . -name \*.c | while read n
do
    sed -n -e '/ZB_SCHEDULE_CALLBACK/s/.*ZB_SCHEDULE_CALLBACK[( ][( ]*\([^,][^,]*\).*$/\1/p' $n
    sed -n -e '/ZB_SCHEDULE_ALARM/s/.*ZB_SCHEDULE_ALARM[( ][( ]*\([^,][^,]*\).*$/\1/p' $n
done | sed -e '/func/d' -e '/TRACE/d' | sort | uniq | gawk '{ print toupper($0) }' | \
while read r
do
    gawk "/^[ ]*BANK.*PR.*${r}/{print \"?PR?_?ZB_SCHED_LOOP_ITERATION?ZB_SCHEDULER ! \" \$NF \",\" ; exit }" build/zig_test.m51
done


#
# Note: zb_mac_ack_timeout must be added manually!!!
#