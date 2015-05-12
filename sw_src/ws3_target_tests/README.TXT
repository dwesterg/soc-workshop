This directory contains demonstration applications that are used by the
WS3-DevelopingDrivers workshop.

demo_devmem
	This application uses /dev/mem to interact with the demo_driver hardware
	in the FPGA.  This application should only be run with none of the demo
	modules are loaded.
	
demo_map_test
	This application is used to demonstrate demo_module_07.

ioctl_test
	This application is used to demonstrate demo_module_07.

demo_uio_test
	This application is used to demonstrate demo_module_08.

ws3_lab_validator
	This application is used to create the validation material submission 
	after you've completed the take home lab work.
	
ws3_lab_validation_check.sh
	This script is used to validate the lab validator output.
	
target_test_devmem.sh
	This script runs demo_devmem and verifies various outputs to determine
	if the hardware environment is operating properly.

