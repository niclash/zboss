This test is near same as zdo_startup, but security is turned on.

zdo_start_zc start as ZC, zdo_start_ze joins to it, then tests sends APS
packets to each other infinitely.

To start tests in Linux / ns-3:
- run ./run.sh
- view .pcap file by Wireshark



This test can be used to check the APS retransmission.

To start tests in Linux:
- in file include/zb_config.h need on define APS_RETRANSMIT_TEST
- rebuild stack
- run ./run.sh
- view .pcap file by Wireshark

End device (zdo_start_ze) send packets to the coordinator (zdo_start_zá).
Coordinator, in response sends ACK packages for confirmation.
Through time ACK packets contain incorrect data and the end device must in this case 
repeat the package with wrong ACK.
This can be verified with Wireshark.
