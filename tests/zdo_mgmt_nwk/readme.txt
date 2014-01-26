To start test run run_test.sh. It starts ns-3, coordinator and router sides.
Router performs zdo management nwk calls to coordinator and checks responses:
1) call zb_zdo_mgmt_nwk_update_req to change active channel
2) call zb_zdo_mgmt_nwk_update_req to change channel mask
3) call zb_zdo_mgmt_nwk_update_req to scan channels
4) call zb_zdo_mgmt_nwk_update_req with incorrect parameters to check
parameters handling
