1. Test without authentacation run.sh
ZC creates network
ZR(not accept childs) connect to ZC
ZR sends permit joining request to ZC and disallow joining to ZC
ZE tries to find suitable device to join but failes

2. Auth Test run_auth.sh
ZC creates network
ZR(accept childs) connect to ZC
ZR sends permit joining request to ZC with tc_significance disallowing authentication
ZE joins to ZR
ZR sends Update device
ZC auth is disallowed, so it sends remove device
ZR should send Leave request to ZE, but it can't send leave to non-authenticated child (see 3.6.1.10.2), here is the bug in spec.
ZE freeze

