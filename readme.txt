TODO:

1) make a real readme
2) document how to add boards
3) some boards need different bootscripts and config files.  How to support that? (MACNICA needs jic and no HPS fpga config)
4) add 1.3 Macnica board
5) add DE0 nano
6) add support for compiling sw source (including kernel modules)
7) validate images on all boards


questions

1) how does a user get their app to the board
	a) ssh?
	b) ftp?
2) should i spew out a default sdcard.img so that users could program on winblows and update the preloader using chris's utility?


mmlink
------
The basic design includes the ability for the SoC to access the SLD nodes in the fpga via FPGA hardware.  This allows for ethernet based remote jtag via the SoC.  The mmlink kernel module and user application need to be insmoded and run. On the host side, system-console is used to create a jtag interface using a command like the following

system-console -jtag_server -rc_script=mmlink_setup.tcl ALTERA_CV_SOC/output_files/ALTERA_CV_SOC.sof 192.165.1.2 3333

mmlink_setup.tcl is in the scripts directory
