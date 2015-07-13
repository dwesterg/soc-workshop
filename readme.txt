SoC Workshop
------------

The SoC Workshop is thoroughly documented on Rocketboards at the following link.

http://rocketboards.org/foswiki/view/Documentation/AlteraSoCWorkshopSeries

This repository manages the build of the Quartus project, Preloader, devicetree, u-boot, kernel, and buildroot based root file system for the supported platforms.  Each platfrom has its own make target, but make all by default builds everything.

make help, make help-revs, and make help-revisions give details of all make targets.

Directories

1) scripts => scripts for creating the various quartus projects and qsys systems
2) board_info => xml files for board specific information required for devicetree generation
3) hdl_src => hdl source for the quartus project
4) sw_src => source for various applications / kernel modules running on the arm
5) ip => FPGA based ip blocks used in QSys.
6) mks => Makefile fragments
7) patches => patches for uboot and the kernel
8) overlay_template => template for files added to rootfs image
9) utils => various utilities for cration of quartus project
10) WS* => Source for SoC Workshop examples / labs



mmlink (turned off by default as JTAG and mmlink based debug are mutually exclusive)
------
The basic design includes the ability for the SoC to access the SLD nodes in the fpga via FPGA hardware.  This allows for ethernet based remote jtag via the SoC.  The mmlink kernel module and user application need to be insmoded and run. On the host side, system-console is used to create a jtag interface using a command like the following

system-console -jtag_server -rc_script=scripts/mmlink_setup.tcl ALTERA_CV_SOC/output_files/ALTERA_CV_SOC.sof 192.165.1.2 3333

mmlink_setup.tcl is in the scripts directory
