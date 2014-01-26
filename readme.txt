Building ZBOSS applications for 8051 using IAR Embedded Workbench.

1. Development in IAR
zboss_open_source.eww workspace is provided to build and debug ZBOSS tests and applications using IAR.
To start, open the specified workspace in the IAR (7.60 at least is required). The workspace contains
zboss_open_source project, which consists of the stack kernel and test application. Stack kernel acts
like a library linked to the applications.
There are two configurations available in this project: Debug and Release. When building Debug 
configuration ".d51" file with all the Debug information is created. It can be used for debugging on
device using "Download & Debug" IAR button. Debug mode was configured and tested on TI SmartRf05Eb.
When building Release configuration ".hex" flash image is created and could be used to program the
device.
Several preprocessor definitions can be used to manage ZBOSS stack features. Note that one of them
ZB_ED_ROLE is used to compile the stack for end devices (coordinator and router roles don't differ
in ZBOSS). For example, to build hex image for end device in IAR project explorer go to the 
tests/zdo_startup exclude coordinator and router sources from build and include end device source,
select Release configuration and enable ZB_ED_ROLE in preprocessor definitions. We use "x" with the
definition to disable feaure but not delete it from the set.

2. Programming the device.
For TI devices it is convinient to use SmartRf Flash Programmer. It is free and can be downloaded from
TI official website. For example to program TI CC253X, start SmartRf Flash Programmer,
select "Program CCxxxx SoC" in the drop-down box, check "Erase, program and verify" radio button,
choose the hex image and click "Perform actions". Notice, that CC2531 USB Dongle can be programmed
only when connected to evaluation board. When programming USB Dongle evaluation module should be
removed from the board and USB Dongle should be connected with the board via debug
connector. If everything is correct SmartRf Flash programmer will detect SmartRf05Eb with
CC2531 system on chip.
Also IAR can be used to download applications to the device. CC2531 USB Dongle si programmed the same 
way (connected to the board).
