This utility used to collect trace and dump from the device.
Connect device to the serial port (COM4 in this example) and run:
win_com_dump.exe \\\\.\\COM4 trace.log traf.dump

Then reset the device - trace and traffic diles must grow.

Put traf.dump to the Linux box which has Wireshark with our plugin installed.
Convert traf.dump to wireshark format using stack/devtools/dump_converter/dump_converter 
utility:
./dump_converter traf.dump traf.pcap

View .pcap by wireshark:
wireshark traf.pcap