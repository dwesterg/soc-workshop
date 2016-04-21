--------------------------------------------------------------------------------
LAB ENVIRONMENT INSTALLATION
--------------------------------------------------------------------------------
GETTING STARTED

To perform the lab work, you first need to configure your local environment on
your development host.  You will need the Linaro ARM linux compiler toolchain,
the linux kernel source tree configured as it is running on your target
development board and the actual lab materials.

To install all of these requirements, the following installation script is
provided on the FAT partition of the SD card that you use to boot your
development board.  With your development board booted, and the USB OTG port
connected to your development host, you should be able to mount the USB mass
storage device on your host.  Once you've accomplished that you should create
a directory on your host where you wish to install the lab materials.

Something like this:
mkdir <path-to>/my_lab_directory

Then change directory into that like this:
cd <path-to>/my_lab_directory

--------------------------------------------------------------------------------
RUNNING THE INSTALLATION SCRIPT

Now you should be able to run the environment installation script like this:
bash <path-to-sdcard>/WS3-DevelopingDrivers/install_lab_environment.sh

When the installation script completes you should see this structure in the lab
directory:

my_lab_directory/
├── download		: toolchain and kernel packages are downloaded here
├── setup_env.src	: setup script created by installation script for you
├── socfpga-4.1-ltsi	: kernel is extracted and configured in this directory
├── toolchain		: toolchain is extracted into this directory
└── ws3_lab_material	: the lab materials that you will work with

--------------------------------------------------------------------------------
SETUP YOUR ENVIRONMENT

At this point, you should source the setup_env.src script like this:
. ./setup_env.src

That should setup these three environment variables like this:
export ARCH=arm
	This is set to 'arm' for our ARM achitecture processor.
	
export CROSS_COMPILE=<path-to>/toolchain/bin/arm-linux-gnueabihf-
	This is set to the path into the cross compiler toolchain and the
	prefix for the tools.
	
export OUT_DIR=<path-to>/socfpga-3.10-ltsi
	This is set to the path into the linux kernel source tree.

--------------------------------------------------------------------------------
BUILDING THE LABS

Now you should explore the ws3_lab_material directory and read through the
README.TXT files.  The directions for how to complete the lab work is documented
in the README.TXT files.

ws3_lab_material/
├── README.TXT		<- this one 1st
├── lab_module
│   └── README.TXT	<- this one 2nd
├── lab_module_test
│   └── README.TXT	<- this one 3rd
├── uio_module
│   └── README.TXT	<- this one 4th
└── uio_module_test
    └── README.TXT	<- this one 5th

