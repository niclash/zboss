zdo_zc_test_nwk_addr starts as ZC, zdo_zr_test_nwk_addr joins to it, then test sends
descriptors requests

To start tests in Linux / ns-3:
- setup ns-3 as described in doc/ns-3/README, cd ns/ns-3.7/build/debug/examples/udp-pipe,
run ./udp-pipe --nNode=2 --PipeName=/tmp/zzz --Join=1
- at another tty run ./zdo_zc_test_nwk_addr /tmp/zzz0.write /tmp/zzz0.read
- wait for Formation complete: seek for "Device STARTED OK" in the trace
- at another tty run ./zdo_zr_test_nwk_addr /tmp/zzz1.write /tmp/zzz1.read

Router (zdo_zr_test_nwk_addr) sends nwk address requests to coordinator.
Coordinator, in response sends nwk address.
