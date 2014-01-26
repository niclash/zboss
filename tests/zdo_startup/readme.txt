Typical test that checks devices start and dummy packets exchange

zdo_start_zc start as ZC, zdo_start_ze joins to it, then tests send APS
packets to each other infinitely.

To start tests in Linux / ns-3:
- run ./run.sh
- view .pcap file by Wireshark
