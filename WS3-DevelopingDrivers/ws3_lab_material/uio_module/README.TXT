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
OBTAIN THE UIO TEMPLATE

Copy the file '${OUT_DIR}/drivers/uio/uio_pdrv_genirq.c' from the kernel source
tree into this directory and rename it 'my_uio_pdrv_genirq.c'.

You could run this sequence of commands to do this in a bash shell:
UIO_FILE="${OUT_DIR}/drivers/uio/uio_pdrv_genirq.c"
[ -f "${UIO_FILE}" ] || {
	echo ""
	echo "ERROR: could not find file '${${UIO_FILE}}'"
	echo ""
	exit 1
}

MY_UIO_FILE="my_$(basename ${UIO_FILE})"
cp "${UIO_FILE}" "${MY_UIO_FILE}"

--------------------------------------------------------------------------------
EDIT THE UIO TEMPLATE

Edit the 'my_uio_pdrv_genirq.c' file, change the 'DRIVER_NAME' macro from
'uio_pdrv_genirq' to 'my_uio_pdrv_genirq', and add the compatible string entry
'{.compatible = "demo,driver-1.0"},' to the 'uio_of_genirq_match[]' array
declaration.

You could run this sequence of commands to do this in a bash shell:
sed -i.bak -e '
/\#define DRIVER_NAME/ s/\"\(.*\)\"/\"my_\1\"/
/uio_of_genirq_match\[\] = / a\
	{.compatible = "demo,driver-1.0"},
' ${MY_UIO_FILE}

--------------------------------------------------------------------------------
CREATE KBUILD FILE

Create a file named 'Kbuild' with this one line in it:
obj-m  := my_uio_pdrv_genirq.o

You could run this sequence of commands to create this in a bash shell:
cat <<EOF > Kbuild
obj-m  := $(echo -n ${MY_UIO_FILE} | sed -e "s/\.c/\.o/")
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

Now run the command 'make', this should build the uio_module kernel module.

--------------------------------------------------------------------------------
FINISHED

When this succeeds, you should see the module ELF file my_uio_pdrv_genirq.ko in
this directory.

