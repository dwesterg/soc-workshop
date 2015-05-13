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
CREATE KBUILD FILE

Create a file named 'Kbuild' with this one line in it:
obj-m  := lab_module.o

You could run this sequence of commands to create this in a bash shell:
cat <<EOF > Kbuild
obj-m  := lab_module.o
EOF

--------------------------------------------------------------------------------
CREATE MAKEFILE

Create a file named 'Makefile' with these lines in it:
ifndef OUT_DIR
    $(error OUT_DIR is undefined, bad environment, you point OUT_DIR to the linux kernel build output directory)
endif

KDIR ?= $(OUT_DIR)

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

help:
	$(MAKE) -C $(KDIR) M=$$PWD help

modules:
	$(MAKE) -C $(KDIR) M=$$PWD modules

modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

You could run this sequence of commands to create this in a bash shell:
cat <<EOF > Makefile
ifndef OUT_DIR
    \$(error OUT_DIR is undefined, bad environment, you point OUT_DIR to the linux kernel build output directory)
endif

KDIR ?= \$(OUT_DIR)

default:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD

clean:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD clean

help:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD help

modules:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules

modules_install:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules_install
EOF
--------------------------------------------------------------------------------
EXECUTE MAKE

Now run the command 'make', this should build the lab_module kernel module.

--------------------------------------------------------------------------------
FINISHED

When this succeeds, you should see the module ELF file lab_module.ko in
this directory.

