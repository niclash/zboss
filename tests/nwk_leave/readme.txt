NWK leave test

2 devices: ZC, ZR.
No security.

- ZR joins ZC
- ZR performs a mgmt_leave: sends mgmt_leave_req to ZC with ext_address == ZR's address, Rejoin false.
- ZC sends LEAVE to ZR, ZR does leave.
- ZR performs NWK Join to ZC
- ZR sends APS data to ZC, ZC sends APS data to ZR
- ZC performs a mgmt_leave, request(DeviceAddress=IEEE of ZR, rejoin=FALSE; remove children=FALSE);
- ZR performs NWK Join to ZC
- ZR sends APS data to ZC, ZC sends APS data to ZR

Result can be verified using Wireshark.
