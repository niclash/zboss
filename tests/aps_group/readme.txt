APS group addressing.

This test is for features ALF201, ALF301 - Group addressint in APS.
Also, it uses Optional ALF100 Add Group.
No test for APS Group addressing without NWK multicast in PRO TC,
so need to use self-made test.

2 devices: ZC, ZR
No security.

- ZR joins ZC
- every device adds 2 entries to its binding table:
  a) group address 10, endpoint 66
  b) group address 15, endpoints 77, 88 and 99
- ZC sends packet to group address 10
- ZR sends packet to group address 15

Both devices must get packets to endpoints 11, 77, 88, 99.
