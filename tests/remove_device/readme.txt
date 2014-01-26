Test for REMOVE-DEVICE (4.4.5).
To run this test type 
$ sh run.sh

3 devices: ZC, ZR1, ZR2.
Topology:
ZC-ZR1-ZR2

Test steps:
- ZR1 joins ZC
- ZR2 joins ZR1
- after timeout ZC sends REMOVE-DEVICE ro ZR1
- ZR1 sends LEAVE to ZR2

