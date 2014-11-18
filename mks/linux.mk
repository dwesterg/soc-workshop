

WORK_ROOT = $(CURDIR)/build
DEST_ROOT = $(CURDIR)

################################################################################
################################################################################
################################################################################
# Build variables for BR + Kernel

ARCH = arm
LINUX_DEFCONFIG_TARGET = socfpga_custom_defconfig
BUILDROOT_DEFCONFIG_TARGET = br_custom_defconfig
CROSS_COMPILE := arm-linux-gnueabihf-
#LINUX_MAKE_TARGET := vmImage
LINUX_MAKE_TARGET := zImage

# Timestamp for build version and config files
KBUILD_BUILD_VERSION=$(shell /bin/date "+%Y-%m-%d---%H-%M-%S")

# Configuration files buildroot, busybox & linux kernel
LINUX_DEFCONFIG := $(wildcard linux.defconfig)
BUILDROOT_DEFCONFIG := $(wildcard buildroot.defconfig)
BUSYBOX_CONFIG_FILE := $(wildcard $(CURDIR)/buildroot_busybox.config)
#DEVICETREE_SRC := $(firstword $(wildcard $(CURDIR)/quartus/*.dts))
#DEVICETREE_SRC := $(CURDIR)/quartus/soc_system.dts

ifeq ("$(LINUX_DEFCONFIG)","")
	ifeq ("$(ARCH)","arm")
		LINUX_DEFCONFIG_TARGET = socfpga_defconfig
	else
		LINUX_DEFCONFIG_TARGET = 3c120_defconfig
	endif
endif

TOOLCHAIN_EXTERNAL_PATH := $(WORK_ROOT)/toolchain

# set ARCH, toolchain path, busybox config file, download directory, and overlay directory for BUILDROOT
BUILDROOT_VARIABLES := ARCH=$(ARCH) 
BUILDROOT_VARIABLES += BR2_TOOLCHAIN_EXTERNAL_PATH=$(TOOLCHAIN_EXTERNAL_PATH) 
BUILDROOT_VARIABLES += BR2_DL_DIR=$(CURDIR)/downloads 
BUILDROOT_VARIABLES += BR2_ROOTFS_OVERLAY=$(WORK_ROOT)/overlay
ifneq ("$(BUSYBOX_CONFIG_FILE)","")
	BUILDROOT_VARIABLES += BUSYBOX_CONFIG_FILE=$(BUSYBOX_CONFIG_FILE) 
endif

# set build version, toolchain, arch and devicetree source for kernel
LINUX_VARIABLES = ARCH=$(ARCH)
LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
LINUX_VARIABLES += CROSS_COMPILE=$(TOOLCHAIN_EXTERNAL_PATH)/bin/$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif

SRC_VARIABLES = ARCH=$(ARCH) CROSS_COMPILE=$(TOOLCHAIN_EXTERNAL_PATH)/bin/$(CROSS_COMPILE)

DL=downloads

# links for BR, Kernel, and toolchain files
# NB: IF Buildroot is updated, the busybox-menuconfig stuff needs to be changed with an updated path.
BR_SOURCE_PACKAGE := "http://buildroot.uclibc.org/downloads/buildroot-2014.08.tar.gz"
#LNX_SOURCE_PACKAGE := "http://rocketboards.org/gitweb/?p=linux-socfpga.git;a=snapshot;h=refs/heads/socfpga-3.10-ltsi;sf=tgz"
LNX_SOURCE_PACKAGE := "http://rocketboards.org/gitweb/?p=linux-socfpga.git;a=snapshot;h=refs/heads/socfpga-3.17;sf=tgz"
TOOLCHAIN_SOURCE := gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar.xz
TOOLCHAIN_SOURCE_PACKAGE := "http://releases.linaro.org/14.09/components/toolchain/binaries/$(TOOLCHAIN_SOURCE)"


################################################################################

AR_REGEX += \
	overlay_template \
	sw_src \
	downloads \
	*.defconfig \
	*.config \
	zImage
	

################################################################################

.PHONY: default
default: help

HELP_TARGETS += all
all.HELP := Build everything
.PHONY: all
all: output


HELP_TARGETS += downloads_clean	
downloads_clean.HELP := Clean downloads directory

.PHONY: downloads_clean
downloads_clean: 
	$(RM) $(DL)/*
	
################################################################################
# Download everything

HELP_TARGETS += downloads
downloads.HELP := Download toolchain, kernel, and buildroot + required buildroot packages

.PHONY: downloads
downloads: toolchain.get buildroot.get linux.get br_downloads

################################################################################
# download and install toolchain

.PHONY: toolchain.get
toolchain.get: $(DL)/$(TOOLCHAIN_SOURCE)
$(DL)/$(TOOLCHAIN_SOURCE):
	$(MKDIR) -p $(DL)
	wget -O $(DL)/$(TOOLCHAIN_SOURCE) $(TOOLCHAIN_SOURCE_PACKAGE)

.PHONY: toolchain.extract
toolchain.extract: $(call get_stamp_target,toolchain.extract)
$(call get_stamp_target,toolchain.extract): $(DL)/$(TOOLCHAIN_SOURCE)
	$(RM) $(TOOLCHAIN_EXTERNAL_PATH)
	$(MKDIR) $(TOOLCHAIN_EXTERNAL_PATH)
	$(TAR) -xvf $(DL)/$(TOOLCHAIN_SOURCE) --strip-components 1 -C $(TOOLCHAIN_EXTERNAL_PATH)
	$(stamp_target)
	
################################################################################
# Patches
# doesnt do anything
#
.PHONY: buildroot.patches
buildroot.patches: $(call get_stamp_target,buildroot.patches)
$(call get_stamp_target,buildroot.patches): $(call get_stamp_target,buildroot.extract) $(EBII_BUILDROOT_PATCHES)
#	ifneq ("$EBII_BUILDROOT_PATCHES","")
#		$(PATCH) -d buildroot -p1 < $(EBII_BUILDROOT_PATCHES)
#	endif
	$(stamp_target)

.PHONY: linux.patches
linux.patches: $(call get_stamp_target,linux.patches)
$(call get_stamp_target,linux.patches): $(call get_stamp_target,linux.extract) $(EBII_LINUX_PATCHES)
#	ifneq ("$EBII_LINUX_PATCHES","")
#		$(PATCH) -d linux-socfpga -p1 < $(EBII_LINUX_PATCHES)
#	endif
	$(stamp_target)

################################################################################
# overlay
# build overlay directory with custom applications and configuration files

# copy template to build directory	
.PHONY: overlay.extract
overlay.extract: $(call get_stamp_target,overlay.extract)
$(call get_stamp_target,overlay.extract):
	$(RM) $(WORK_ROOT)/overlay
	$(MKDIR) -p $(WORK_ROOT)
	$(CP) -a overlay_template $(WORK_ROOT)/overlay
	$(stamp_target)

HELP_TARGETS += overlay.make_all
overlay.make_all.HELP := Install custom apps to overlay directory
.PHONY: overlay.make_all
overlay.make_all: overlay.extract toolchain.extract
	$(MAKE) -C sw_src CROSS_COMPILE=$(TOOLCHAIN_EXTERNAL_PATH)/bin/$(CROSS_COMPILE) WORK_ROOT=$(WORK_ROOT) INSTALL_DIR=$(WORK_ROOT)/overlay all

HELP_TARGETS += overlay.make_install
overlay.make_install.HELP := Install custom apps to overlay directory
.PHONY: overlay.make_install 
overlay.make_install: overlay.make_all linux.modules_install
	$(MAKE) -C sw_src CROSS_COMPILE=$(TOOLCHAIN_EXTERNAL_PATH)/bin/$(CROSS_COMPILE) WORK_ROOT=$(WORK_ROOT) INSTALL_DIR=$(WORK_ROOT)/overlay install

################################################################################
# buildroot
BR_DEPS = buildroot.dodefconfig overlay.make_install toolchain.extract

#Download buildroot source
.PHONY:	buildroot.get
buildroot.get: $(DL)/buildroot.tgz
$(DL)/buildroot.tgz: 
	$(MKDIR) -p $(DL)
	wget -O $(DL)/buildroot.tgz $(BR_SOURCE_PACKAGE)
	$(stamp_target)

#extract buildroot source
.PHONY: buildroot.extract
buildroot.extract: $(call get_stamp_target,buildroot.extract)
$(call get_stamp_target,buildroot.extract): $(DL)/buildroot.tgz $(EBII_BUILDROOT_PATCHES)
	$(RM) $(WORK_ROOT)/buildroot
	$(MKDIR) $(WORK_ROOT)/buildroot
	$(TAR) -xvzf $(DL)/buildroot.tgz --strip-components 1 -C $(WORK_ROOT)/buildroot
	$(stamp_target)

# apply ebii custom buildroot configuration
.PHONY: buildroot.dodefconfig
buildroot.dodefconfig: $(WORK_ROOT)/buildroot/configs/$(BUILDROOT_DEFCONFIG_TARGET)
$(WORK_ROOT)/buildroot/configs/$(BUILDROOT_DEFCONFIG_TARGET): buildroot.extract $(BUILDROOT_DEFCONFIG)
	$(CP) $(BUILDROOT_DEFCONFIG) $(WORK_ROOT)/buildroot/configs/$(BUILDROOT_DEFCONFIG_TARGET)

# build buildroot
HELP_TARGETS += buildroot.build
buildroot.build.HELP := Build buildroot
.PHONY: buildroot.build
buildroot.build: $(call get_stamp_target,buildroot.build)
$(call get_stamp_target,buildroot.build): $(BR_DEPS)
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) $(BUILDROOT_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) all
	$(stamp_target)

# dowload all buildroot sources required for the build
HELP_TARGETS += br_downloads
br_downloads.HELP := Download buildroot sources required for build
.PHONY: br_downloads
br_downloads: buildroot.dodefconfig
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) $(BUILDROOT_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) source

#channge buildroot config and update stored config file
HELP_TARGETS += buildroot.menuconfig
buildroot.menuconfig.HELP := Menuconfig for buildroot.  Also saves output to SRC directory
.PHONY: buildroot.menuconfig
buildroot.menuconfig: buildroot.dodefconfig
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) $(BUILDROOT_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) menuconfig
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) savedefconfig
	$(CP) $(BUILDROOT_DEFCONFIG) $(BUILDROOT_DEFCONFIG).$(KBUILD_BUILD_VERSION).old
	$(CP) $(WORK_ROOT)/buildroot/defconfig $(BUILDROOT_DEFCONFIG)

# change busybox config and save it.  Note the busybox path with change when
# buidroot is updated
HELP_TARGETS += busybox.menuconfig
busybox.menuconfig.HELP := Menuconfig for busybox.  Also saves output to SRC directory
busybox.menuconfig: buildroot.dodefconfig
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) $(BUILDROOT_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/buildroot $(BUILDROOT_VARIABLES) busybox-menuconfig
	$(CP) $(BUSYBOX_CONFIG_FILE) $(BUSYBOX_CONFIG_FILE).$(KBUILD_BUILD_VERSION).old
	$(CP) $(WORK_ROOT)/buildroot/output/build/busybox-1.22.1/.config $(BUSYBOX_CONFIG_FILE)
	

################################################################################
# linux
LNX_DEPS = linux.patches linux.dodefconfig toolchain.extract  buildroot.build $(DEVICETREE_SRC)

# download linux source from rocketboards
.PHONY:	linux.get
linux.get: $(DL)/linux-socfpga.tgz
$(DL)/linux-socfpga.tgz: 
	$(MKDIR) -p $(DL)
	wget -O $(DL)/linux-socfpga.tgz $(LNX_SOURCE_PACKAGE)
	$(stamp_target)

# extract linux source
.PHONY: linux.extract
linux.extract: $(call get_stamp_target,linux.extract)
$(call get_stamp_target,linux.extract):$(DL)/linux-socfpga.tgz $(EBII_LINUX_PATCHES)
	$(RM) $(WORK_ROOT)/linux-socfpga
	$(MKDIR) $(WORK_ROOT)/linux-socfpga
	$(TAR) -xvzf $(DL)/linux-socfpga.tgz --strip-components 1 -C $(WORK_ROOT)/linux-socfpga
	$(stamp_target)

# apply ebii defconfig
.PHONY: linux.dodefconfig
linux.dodefconfig: $(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
$(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET): linux.extract $(LINUX_DEFCONFIG)
ifneq ("$(LINUX_DEFCONFIG)","")
	$(CP) $(LINUX_DEFCONFIG) $(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
endif
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)

HELP_TARGETS += linux.modules
linux.modules.HELP := Build linux kernel modules
linux.modules: linux.patches linux.dodefconfig toolchain.extract
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) INSTALL_MOD_PATH=$(WORK_ROOT)/overlay modules


HELP_TARGETS += linux.modules_install
linux.modules_install.HELP := Build linux kernel modules
linux.modules_install: linux.modules
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) INSTALL_MOD_PATH=$(WORK_ROOT)/overlay modules_install

# builda
$(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET): $(call get_stamp_target,linux.build)

HELP_TARGETS += linux.build
linux.build.HELP := Build linux kernel
.PHONY: linux.build
linux.build: $(call get_stamp_target,linux.build) 
$(call get_stamp_target,linux.build): $(LNX_DEPS)
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) $(LINUX_MAKE_TARGET)
	$(stamp_target)

# update linux configuration and same defconfig
HELP_TARGETS += linux.menuconfig
linux.menuconfig.HELP := Menuconfig for linux.  Also saves output to SRC directory
linux.menuconfig: $(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) menuconfig
	$(MAKE) -C $(WORK_ROOT)/linux-socfpga $(LINUX_VARIABLES) savedefconfig
	$(CP) $(LINUX_DEFCONFIG) $(LINUX_DEFCONFIG).$(KBUILD_BUILD_VERSION).old
	$(CP)  $(WORK_ROOT)/linux-socfpga/defconfig $(LINUX_DEFCONFIG)
	

################################################################################
# force rebuild

HELP_TARGETS += update_rootfs
update_rootfs.HELP := Force rebuild of linux, jtag, cgi, buildroot
.PHONY: update_rootfs
update_rootfs:
	$(RM) $(call get_stamp_target,buildroot.build)
	$(RM) $(call get_stamp_target,linux.build)
	$(MAKE) linux.build


################################################################################
# Generate output files

# vmlinux -> elf of kernel + rootfs just used for debug download
$(WORK_ROOT)/linux-socfpga/vmlinux: $(call get_stamp_target,linux.build)
	$(stamp_target)

# vmlinux.img -> uboot image of kernel + rootfs
$(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET): $(call get_stamp_target,linux.build)
	$(stamp_target)

# copy vmlinux.img -> uboot image of kernel + rootfs
$(DEST_ROOT)/$(LINUX_MAKE_TARGET): $(WORK_ROOT)/linux-socfpga/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET)
	$(MKDIR) -p $(DEST_ROOT)
	$(CP) $< $@
	
# copy vmlinux -> elf of kernel + rootfs just used for debug download
$(DEST_ROOT)/vmlinux: $(WORK_ROOT)/linux-socfpga/vmlinux
	$(MKDIR) -p $(DEST_ROOT)
	$(CP) $< $@


################################################

