# This is a special makefile format for compiling kernel drivers.
# You may have to update KDIR to point to your configured Linux kernel for
# the ARM processor on the device.


LINUX_VARIABLES = PATH=$(PATH)
LINUX_VARIABLES += ARCH=$(ARCH)
ifneq ("$(KBUILD_BUILD_VERSION)","")
	LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
endif
LINUX_VARIABLES += CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif
LINUX_VARIABLES += INSTALL_MOD_PATH=$(INSTALL_MOD_PATH)


# Configured Linux kernel source.
ifndef OUT_DIR
    $(error OUT_DIR is undefined, bad environment, you point OUT_DIR to the linux kernel build output directory)
endif

KDIR ?= $(OUT_DIR)


# Final module
obj-m := aclsoc_drv.o

# List of object files to compile for the final module.
aclsoc_drv-y := aclsoc.o aclsoc_fileio.o aclsoc_cmd.o

all :
	$(CROSS_COMPILE)gcc -g -DLINUX -Wall -Werror -o user user.c 
	[ ! -d $(KDIR) ] || CFLAGS=-DCONFIG_ALTERA_STAPL $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=`pwd` modules

modules_install:
	[ ! -d $(KDIR) ] || CFLAGS=-DCONFIG_ALTERA_STAPL $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=`pwd` modules_install
	
clean : clean-intermediates clean-module

clean-module :
	rm -f *.ko

clean-intermediates:
	rm -f $(obj-m)
	rm -f *.o
	rm -f *.mod.*
	rm -f *.symvers
	rm -f *.order
	rm -f .acl*cmd
	rm -rf .tmp_versions

.PHONY: all clean clean-intermediates clean-module
