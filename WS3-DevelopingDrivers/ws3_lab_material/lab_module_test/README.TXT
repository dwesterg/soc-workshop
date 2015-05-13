--------------------------------------------------------------------------------
SETUP DEVELOPMENT ENVIRONMENT

Ensure that you have your development environment setup properly.

You should have the linaro cross compiler tools built and the linux kernel
source tree installed on your development host.

You should have the following environment variables defined ARCH, CROSS_COMPILE,
and OUT_DIR.  Here's an example of what they may look like:

#
# ARCH should be arm for the Cortex A9
#
export ARCH=arm

#
# CROSS_COMPILE should point into the target cross compiler tools directory and
# contain the prefix used by all the target tools in that directory
#
export CROSS_COMPILE=<path-to>/soc-workshop/toolchain/bin/arm-linux-gnueabihf-

#
# OUT_DIR should point into the linux kernel build directory, that's where the
# .config file resides for the kernel that we're building against.
#
export OUT_DIR=<path-to>/soc-workshop/socfpga-3.10-ltsi

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

