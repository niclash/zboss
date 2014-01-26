
13.36TP/PRO/BV-35 Frequency Agility  Network Channel
Manager - ZC
Verify that the DUT acting as a ZC can operate as default Network Channel
Manager.

- Router broadcasts a System_Server_Discovery_req command with the
ServerMask set to Network Manager (bit 6 = 1)
- DUT coordinator shall unicast a System_Server_Discovery_rsp command back to
gZR1 with Status set to SUCCESS and ServerMask set to Network Manager.

to execute test start ru.sh
To check results, analyse log files, on success zdo_zr.log contains
"system_server_discovery received, status: OK"
on fail "ERROR receiving system_server_discovery"
