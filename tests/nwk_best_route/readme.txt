Purpose: choose better route path according to lqi values

This test has the following topology:

                 C(0)
               /   |  \
              /    |   \
           R(1)  R(17)  R(33)
             \     |    /
              \    |   /
                 R(18)

R18 sends packet to C0. It doesn't know Coordinator address, so it initiates route discovery.
It finds the first route(it could be any of 3 routes) and send packet. Lqi values configured
in the way that path R18<->R33 will be the best. So after route discovery ends packets from
R18 to C0 should go through R33.

After timeout R18 sends new packet and it MUST go through R33. To check it analyze *.pcap 

