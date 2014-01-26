Two flash images contain simple ZBOSS test:
1. ZigBee coordinator starts and forms a network.
2. ZigBee end device starts and joins the network.
3. Coordinator and end device exchange dummy APS messages.

Images were tested on SmartRf05Eb. To program hardware SmartRf Flash
programmer is convenient to use. Start SmartRf Flash Programmer,
select "Program CCxxxx SoC" in the drop-down box,
check "Erase, program and verify" radio button,
choose the hex image and click "Perform actions".
Notice, that CC2531 USB Dongle can be programmed only when connected
to the evaluation board. When programming USB Dongle evaluation module
should be removed from the board and USB Dongle should be connected
with the board via debug connector. If everything is correct
SmartRf Flash programmer will detect SmartRf05Eb with
CC2531 system on chip.
Message exchange between Coordinator and End device can be sniffed on the 19th
(0x13) channel.
