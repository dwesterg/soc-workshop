
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

.PHONY: all
all: mmlink

.PHONY: install
install: mmlink
	$(MKDIR) $(INSTALL_DIR)/usr/bin
	$(CP) mmlink/mmlink $(INSTALL_DIR)/usr/bin
	$(CP) msgdma_user_space/memtest $(INSTALL_DIR)/examples

.PHONY: mmlink
mmlink: 
	make -C mmlink KERNEL_SRC_DIR=../../linux-socfpga CROSS_COMPILE=$(CROSS_COMPILE)	
	make -C msgdma_user_space CROSS_COMPILE=$(CROSS_COMPILE)	
