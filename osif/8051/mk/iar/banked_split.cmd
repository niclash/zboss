csplit -z zigbee_iar.hex "/:0200000/" {*}
sed -n '1{d;b};${G;s/$/:00000001FF/;p;b};p;' xx00 >c_b0.hex
sed -n '1{d;b};${G;s/$/:00000001FF/;p;b};p;' xx01 >c_b1.hex
sed -n '1{d;b};${G;s/$/:00000001FF/;p;b};p;' xx02 >c_b2.hex
sed -n '1{d;b};2{x;b};${p;b};{x;p;};' xx03 >c_b3.hex
rm xx*
