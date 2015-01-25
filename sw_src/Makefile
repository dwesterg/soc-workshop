
SHELL := /bin/bash

ifndef WORK_ROOT
	WORK_ROOT = ./build
endif

ifndef INSTALL_DIR
	INSTALL_DIR = ./install
endif

################################################################################
# Tools

CAT := cat
CD := cd
CHMOD := chmod
CP := cp -rf
ECHO := echo
DATE := date
HEAD := head
MKDIR := mkdir -p
MV := mv
RM := rm -rf
SED := sed
TAR := tar
TOUCH := touch
FIND := find
PATCH := patch
LN := ln

# Helpful Macros
SPACE := $(empty) $(empty)

################################################################################

LINUX_VARIABLES = PATH=$(PATH)
LINUX_VARIABLES += ARCH=$(ARCH)
ifneq ("$(KBUILD_BUILD_VERSION)","")
	LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
endif
LINUX_VARIABLES += CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif
LINUX_VARIABLES += INSTALL_MOD_PATH=$(CURDIR)/../overlay

.PHONY: all
all: install

.PHONY: install
install: mmlink msgdma_memtest validator_devmem validator_module
	$(MKDIR) $(INSTALL_DIR)/usr/bin
	$(CP) mmlink/mmlink $(INSTALL_DIR)/usr/bin
#	$(CP) msgdma_user_space/memtest $(INSTALL_DIR)/examples
#	$(CP) validator_devmem_app/validator_devmem $(INSTALL_DIR)/examples/validator
	make -C validator_module OUT_DIR=../../linux-socfpga $(LINUX_VARIABLES) modules_install

.PHONY: mmlink
mmlink: 
	make -C mmlink KERNEL_SRC_DIR=$(CURDIR)/../linux-socfpga CROSS_COMPILE=$(CROSS_COMPILE)	

.PHONY: msgdma_memtest
msgdma_memtest:
	make -C msgdma_user_space CROSS_COMPILE=$(CROSS_COMPILE)

.PHONY: validator_devmem	
validator_devmem:
	make -C validator_devmem_app CROSS_COMPILE=$(CROSS_COMPILE)

.PHONY: validator_module
validator_module:
	make -C validator_module OUT_DIR=$(CURDIR)/../linux-socfpga $(LINUX_VARIABLES) modules
	
.PHONY: clean
clean:
	make -C mmlink clean
	make -C msgdma_user_space clean
	make -C validator_devmem_app clean
	make -C validator_module OUT_DIR=$(CURDIR)/../linux-socfpga clean
