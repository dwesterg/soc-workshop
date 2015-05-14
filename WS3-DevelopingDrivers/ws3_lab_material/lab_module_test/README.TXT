--------------------------------------------------------------------------------
SETUP DEVELOPMENT ENVIRONMENT

Ensure that you have your development environment setup properly.

You should have the linaro cross compiler tools built and the linux kernel
source tree installed on your development host.

You should have the following environment variables defined ARCH, CROSS_COMPILE,
and OUT_DIR.  Here's an example of what they may look like:

export ARCH=arm
	This is set to 'arm' for our ARM achitecture processor.
	
export CROSS_COMPILE=<path-to>/toolchain/bin/arm-linux-gnueabihf-
	This is set to the path into the cross compiler toolchain and the
	prefix for the tools.
	
export OUT_DIR=<path-to>/socfpga-3.10-ltsi
	This is set to the path into the linux kernel source tree.

NOTE: the <path-to> text above refers to the absolute path to the installation
      location of the relevant directory.
      
--------------------------------------------------------------------------------
BUILD THE TEST APPLICATION

Now to build the lab_module_test application you should invoke this command:

${CROSS_COMPILE:?}gcc \
        -march=armv7-a \
        -mfloat-abi=hard \
        -mfpu=vfp3 \
        -mthumb-interwork \
        -mthumb \
        -O2 \
        -g \
        -feliminate-unused-debug-types  \
        -std=gnu99 \
        -W \
        -Wall \
        -Werror \
        -Wc++-compat \
        -Wwrite-strings \
        -Wstrict-prototypes \
        -pedantic \
	-o lab_module_test \
	lab_module_test.c

--------------------------------------------------------------------------------
FINISHED

When this succeeds, you should see the application ELF file lab_module_test in
this directory.

