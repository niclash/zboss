
13.37TP/PRO/BV-36 Frequency Agility - Receipt of a Mgmt_NWK_Update_req - ZR
Verify that the DUT acting as a ZR correctly responds to a Mgmt_NWK_Update_req
from the Network Channel Manager

1) gZC shall unicast a Mgmt_NWK_Update_req command with error
2) DUT ZR1 shall unicast a Mgmt_NWK_Update_notify command with
Status = INVALID REQUEST
3) gZC shall unicast a Mgmt_NWK_Update_req command correctly
4) DUT ZR1 shall unicast a Mgmt_NWK_Update_notify command with
Status = SUCCESS


To execute test, start run.sh
Analyse log files to check results, run.sh script writes "DONE. TEST PASSED!!!"
on success and "ERROR. TEST FAILED!!!" on error.
