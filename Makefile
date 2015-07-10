################################################
#
# Makefile to Manage QuartusII/QSys Design
#
# Copyright Altera (c) 2014
# All Rights Reserved
#
################################################

SHELL := /bin/bash

.SUFFIXES: # Delete the default suffixes

include mks/default.mk

################################################
# Configuration

DL := downloads

HTTP_PROXY := sj-proxy:8080
HTTPS_PROXY := sj-proxy:8080

# Source 
TCL_SOURCE = $(wildcard scripts/*.tcl)
QUARTUS_HDL_SOURCE = $(wildcard src/*.v) $(wildcard src/*.vhd) $(wildcard src/*.sv)
QUARTUS_MISC_SOURCE = $(wildcard src/*.stp) $(wildcard src/*.sdc)
PROJECT_ASSIGN_SCRIPTS = $(filter scripts/create_ghrd_quartus_%.tcl,$(TCL_SOURCE))

QSYS_ADD_COMP_TCLS := $(sort $(wildcard scripts/qsys_add_*.tcl))

#UBOOT_PATCHES = patches/soc_workshop_uboot_patch.patch patches/soc_workshop_uboot_patch_2.patch
LINUX_BRANCH ?= socfpga-3.10-ltsi
UBOOT_PATCHES = $(sort $(wildcard patches/u-boot/*.patch))
LINUX_PATCHES = $(sort $(wildcard patches/$(LINUX_BRANCH)/*.patch))
DTS_COMMON = board_info/hps_common_board_info.xml
DTS_BOARD_INFOS = $(wildcard board_info/board_info_*.xml)


# Board revisions
REVISION_LIST = $(patsubst create_ghrd_quartus_%,%,$(basename $(notdir $(PROJECT_ASSIGN_SCRIPTS))))

QUARTUS_DEFAULT_REVISION_FILE = \
        $(if \
        $(filter create_ghrd_quartus_$(PROJECT_NAME).tcl,$(notdir $(PROJECT_ASSIGN_SCRIPTS))), \
        create_ghrd_quartus_$(PROJECT_NAME).tcl, \
        $(firstword $(PROJECT_ASSIGN_SCRIPTS)))

QUARTUS_DEFAULT_REVISION = $(patsubst create_ghrd_quartus_%, \
        %, \
        $(basename $(notdir $(QUARTUS_DEFAULT_REVISION_FILE))))

SCRIPT_DIR = utils

# Project specific settings
PROJECT_NAME = soc_workshop_system
QSYS_BASE_NAME = soc_system
TOP_LEVEL_ENTITY = ghrd_top

QSYS_HPS_INST_NAME = hps_0
PRELOADER_DISABLE_WATCHDOG = 1
PRELOADER_FAT_SUPPORT = 1

# Board specific preloader extra args
PRELOADER_EXTRA_ARGS_ALTERA_CV_SOC = --set spl.boot.SDRAM_SCRUBBING true
PRELOADER_EXTRA_ARGS_ALTERA_AV_SOC = --set spl.boot.SDRAM_SCRUBBING true
PRELOADER_EXTRA_ARGS_CRITICALLINK_MITYSOM_DEVKIT = --set spl.boot.SDRAM_SCRUBBING true

#SW Config
ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-
TOOLCHAIN_DIR := $(CURDIR)/toolchain
TOOLCHAIN_SOURCE := gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar.xz
TOOLCHAIN_SOURCE_TAR := gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar
TOOLCHAIN_SOURCE_PACKAGE := "http://releases.linaro.org/14.09/components/toolchain/binaries/$(TOOLCHAIN_SOURCE)"

# Kernel Config
LNX_SOURCE_PACKAGE := "https://github.com/altera-opensource/linux-socfpga/tarball/$(LINUX_BRANCH)"
#LNX_SOURCE_PACKAGE := "http://rocketboards.org/gitweb/?p=linux-socfpga.git;a=snapshot;h=refs/heads/$(LINUX_BRANCH);sf=tgz"
#LNX_SOURCE_PACKAGE := "http://rocketboards.org/gitweb/?p=linux-socfpga.git;a=snapshot;h=refs/heads/socfpga-3.10-ltsi;sf=tgz"
#LNX_SOURCE_PACKAGE := "http://rocketboards.org/gitweb/?p=linux-socfpga.git;a=snapshot;h=refs/heads/socfpga-3.17;sf=tgz"
LINUX_DEFCONFIG_TARGET = socfpga_custom_defconfig
LINUX_DEFCONFIG := $(wildcard linux.defconfig.$(LINUX_BRANCH))
LINUX_MAKE_TARGET := zImage
KBUILD_BUILD_VERSION=$(shell /bin/date "+%Y-%m-%d---%H-%M-%S")
#LNX_DEPS = linux.patches linux.dodefconfig toolchain.extract  buildroot.build
LNX_DEPS = linux.patch linux.dodefconfig toolchain.extract | logs

# Buildroot Config
BUILDROOT_DEFCONFIG_TARGET = br_custom_defconfig
BUILDROOT_DEFCONFIG := $(wildcard buildroot.defconfig)
BUSYBOX_CONFIG_FILE := $(wildcard $(CURDIR)/buildroot_busybox.config)
# NB: IF Buildroot is updated, the busybox-menuconfig stuff needs to be changed with an updated path.
#BR_SOURCE_PACKAGE := "http://buildroot.uclibc.org/downloads/buildroot-2014.08.tar.gz"
BR_SOURCE_PACKAGE := "http://buildroot.uclibc.org/downloads/buildroot-2014.11.tar.gz"

# AR_FILTER_OUT := downloads

# initial save file list
AR_REGEX += ip readme.txt mks                                                                      
AR_REGEX += scripts                                                                        
AR_REGEX += $(SCRIPT_DIR) 
AR_REGEX += patches
AR_REGEX += board_info
AR_REGEX += hdl_src
AR_REGEX += sw_src
AR_REGEX += utils
AR_REGEX += overlay_template
AR_REGEX += downloads
AR_REGEX += *.defconfig.*
AR_REGEX += *.config
AR_REGEX += build.sh
AR_REGEX += arc_build.sh
AR_REGEX += boot.script
AR_REGEX += buildroot.defconfig
AR_REGEX += WS1-IntroToSoC
AR_REGEX += WS2-IntroToLinux
AR_REGEX += WS3-DevelopingDrivers
AR_REGEX += CREATE_ALL_SD_CARD_COMPRESSED_IMAGES.sh
AR_REGEX += create_pre_only_sd_card_images.sh
AR_REGEX += create_sd_card_images.sh

################################################
.PHONY: default
default: help
################################################

################################################
.PHONY: all
all: sd-fat

################################################
# DEPS
                                                                          
define create_deps
CREATE_PROJECT_STAMP_$1 := $(call get_stamp_target,$1.create_project)

CREATE_PROJECT_DEPS_$1 := scripts/create_ghrd_quartus_$1.tcl | logs

QUARTUS_STAMP_$1 := $(call get_stamp_target,$1.quartus)

PRELOADER_GEN_STAMP_$1 := $(call get_stamp_target,$1.preloader_gen)
PRELOADER_FIXUP_STAMP_$1 := $(call get_stamp_target,$1.preloader_fixup)
PRELOADER_STAMP_$1 := $(call get_stamp_target,$1.preloader)

UBOOT_STAMP_$1 := $(call get_stamp_target,$1.uboot)

DTS_STAMP_$1 := $(call get_stamp_target,$1.dts)
DTB_STAMP_$1 := $(call get_stamp_target,$1.dtb)

QSYS_STAMP_$1 := $(call get_stamp_target,$1.qsys_compile)
QSYS_GEN_STAMP_$1 := $(call get_stamp_target,$1.qsys_gen)
QSYS_ADD_ALL_COMP_STAMP_$1 := $(call get_stamp_target,$1.qsys_add_all_comp)
#QSYS_ADD_COMP_STAMP_$1 := $(foreach t,$(QSYS_ADD_COMP_TCLS),$(call get_stamp_target,$1.$t))
QSYS_RUN_ADD_COMPS_$1 = $(foreach t,$(QSYS_ADD_COMP_TCLS),$(call get_stamp_target,$1.$(notdir $t)))

QSYS_PIN_ASSIGNMENTS_STAMP_$1 := $$(call get_stamp_target,$1.quartus_pin_assignments)

QUARTUS_DEPS_$1 += $$(QUARTUS_PROJECT_STAMP_$1) $(QUARTUS_HDL_SOURCE) $(QUARTUS_MISC_SOURCE)
QUARTUS_DEPS_$1 += $$(CREATE_PROJECT_STAMP_$1)
QUARTUS_DEPS_$1 += $$(QSYS_PIN_ASSIGNMENTS_STAMP_$1) $$(QSYS_STAMP_$1)

PRELOADER_GEN_DEPS_$1 += $$(QUARTUS_STAMP_$1)
PRELOADER_FIXUP_DEPS_$1 += $$(PRELOADER_GEN_STAMP_$1)
PRELOADER_DEPS_$1 += $$(PRELOADER_FIXUP_STAMP_$1)

# QSYS_DEPS_$1 := $$(QSYS_GEN_STAMP_$1)
QSYS_DEPS_$1 += $1/$(QSYS_BASE_NAME).qsys $1/addon_components.ipx
QSYS_GEN_DEPS_$1 += $$(CREATE_PROJECT_STAMP_$1)
# QSYS_GEN_DEPS_$1 += scripts/create_ghrd_qsys.tcl scripts/devkit_hps_configurations.tcl
QSYS_GEN_DEPS_$1 += scripts/create_ghrd_qsys_$1.tcl scripts/qsys_default_components.tcl
QSYS_GEN_DEPS_$1 += $(QSYS_ADD_COMP_TCLS)
QSYS_GEN_DEPS_$1 += $1/addon_components.ipx

#only support one custom board xml
#DTS_BOARDINFO_$1 := $(firstword $(filter $1, $(DTS_BOARD_INFOS)))
DTS_BOARDINFO_$1 := board_info/board_info_$1.xml

DTS_DEPS_$1 += $$(DTS_BOARDINFO_$1) $(DTS_COMMON) $$(QSYS_STAMP_$1)
DTB_DEPS_$1 += $$(DTS_BOARDINFO_$1) $(DTS_COMMON) $$(QSYS_STAMP_$1)

QUARTUS_QPF_$1 := $1/$1.qpf
QUARTUS_QSF_$1 := $1/$1.qsf
QUARTUS_SOF_$1 := $1/output_files/$1.sof
QUARTUS_RBF_$1 := $1/output_files/$1.rbf
QUARTUS_JDI_$1 := $1/output_files/$1.jdi

QSYS_FILE_$1 := $1/$(QSYS_BASE_NAME).qsys
QSYS_SOPCINFO_$1 := $1/$(QSYS_BASE_NAME).sopcinfo

DEVICE_TREE_SOURCE_$1 := $1/$(QSYS_BASE_NAME).dts
DEVICE_TREE_BLOB_$1 := $1/$(QSYS_BASE_NAME).dtb

#QSYS_SOPCINFO_$1 := $(patsubst $1/%.qsys,$1/%.sopcinfo,$$(QSYS_FILE_$1))
#DEVICE_TREE_SOURCE_$1 := $(patsubst $1/%.sopcinfo,$1/%.dts,$$(QSYS_SOPCINFO_$1))
#DEVICE_TREE_BLOB_$1 := $(patsubst $1/%.sopcinfo,$1/%.dtb,$$(QSYS_SOPCINFO_$1))

AR_FILES += $$(QUARTUS_RBF_$1) $$(QUARTUS_SOF_$1)	
AR_FILES += $$(QSYS_SOPCINFO_$1) $$(QSYS_FILE_$1)

AR_REGEX += $1/$(QSYS_BASE_NAME)/*.svd

AR_FILES += $$(DEVICE_TREE_SOURCE_$1)
AR_FILES += $$(DEVICE_TREE_BLOB_$1)

AR_FILES += $1/preloader/uboot-socfpga/u-boot.img
AR_FILES += $1/preloader/preloader-mkpimage.bin

AR_FILES += $1/u-boot.img
AR_FILES += $1/preloader-mkpimage.bin
#AR_FILES += $1/boot.script $1/u-boot.scr

ALL_DEPS_$1 += $$(QUARTUS_RBF_$1) $$(QUARTUS_SOF_$1) $$(QUARTUS_JDI_$1)
ALL_DEPS_$1 += $$(DEVICE_TREE_SOURCE_$1) $$(DEVICE_TREE_BLOB_$1)
ALL_DEPS_$1 += $1/u-boot.img $1/preloader-mkpimage.bin
ALL_DEPS_$1 += $$(QUARTUS_JDI_$1) $$(QSYS_SOPCINFO_$1) $$(QSYS_FILE_$1)
ALL_DEPS_$1 += $1/hps_isw_handoff $1/$1.qpf $1/$1.qsf

SD_FAT_$1 += $$(ALL_DEPS_$1)
SD_FAT_$1 += boot.script u-boot.scr
SD_FAT_$1 += hdl_src
SD_FAT_$1 += board_info
SD_FAT_$1 += ip
SD_FAT_$1 += WS1-IntroToSoC
SD_FAT_$1 += WS2-IntroToLinux
SD_FAT_$1 += WS3-DevelopingDrivers
SD_FAT_$1 += WS3-DevelopingDrivers/ws3_lab_environment.src
SD_FAT_$1 += WS3-DevelopingDrivers/$(LINUX_DEFCONFIG)
SD_FAT_$1 += WS3-DevelopingDrivers/example_drivers.tgz

.PHONY:$1.all
$1.all: $$(ALL_DEPS_$1)
HELP_TARGETS += $1.all
$1.all.HELP := Build Quartus / preloader / uboot / devicetree / boot scripts for $1 board

#tar file target per project just 4 kicks.
HELP_TARGETS += sd_fat_$1.tar.gz
sd_fat_$1.tar.gz.HELP := Tar of SDCard's FAT filesystem contents for $1

sd_fat_$1.tar.gz: $$(SD_FAT_$1) rootfs.img zImage.$(LINUX_BRANCH)
	$(RM) $@
	$(TAR) -czf $$@ $$^ zImage.*

endef
$(foreach r,$(REVISION_LIST),$(eval $(call create_deps,$r)))

AR_FILES += boot.script u-boot.scr

include mks/qsys.mk mks/quartus.mk mks/preloader_uboot.mk mks/devicetree.mk 
include mks/bootscript.mk mks/kernel.mk mks/buildroot.mk mks/overlay.mk
include mks/arc.mk

################################################

################################################
# Toolchain

.PHONY: toolchain.get
toolchain.get: $(DL)/$(TOOLCHAIN_SOURCE)
$(DL)/$(TOOLCHAIN_SOURCE):
	$(MKDIR) $(DL)
	wget -O $@ $(TOOLCHAIN_SOURCE_PACKAGE)

$(DL)/$(TOOLCHAIN_SOURCE_TAR): $(DL)/$(TOOLCHAIN_SOURCE)
	$(CAT) $< | $(XZ) -d > $@

.PHONY: toolchain.extract
toolchain.extract: $(call get_stamp_target,toolchain.extract)
$(call get_stamp_target,toolchain.extract): $(DL)/$(TOOLCHAIN_SOURCE_TAR)
	$(RM) $(TOOLCHAIN_DIR)
	$(MKDIR) $(TOOLCHAIN_DIR)
	$(TAR) -xvf $< --strip-components 1 -C $(TOOLCHAIN_DIR)
	$(stamp_target)

################################################

################################################
# All projects stuff
#define create_project

#.PHONY: create_project-$1
#create_project-$1: $$(QSYS_GEN_STAMP_$1) $$(CREATE_PROJECT_STAMP_$1)

#HELP_TARGETS_$1 += create_project-$1
#create_project-$1.HELP := Create all files for $1 project

#endef
#$(foreach r,$(REVISION_LIST),$(eval $(call create_project,$r)))

.PHONY: create_all_projects
create_all_projects: $(foreach r,$(REVISION_LIST),$r.create_project)
HELP_TARGETS += create_all_projects
create_all_projects.HELP := Create all quartus projects

.PHONY: create_all_qsys
create_all_qsys: $(foreach r,$(REVISION_LIST),qsys_generate_qsys-$r)
HELP_TARGETS += create_all_qsys                                                                              
create_all_qsys.HELP := Create all qsys files

#.PHONY: compile_all_projects
#compile_all_projects: $(foreach r,$(REVISION_LIST),quartus_compile-$r)  
#HELP_TARGETS += compile_all_projects
#compile_all_projects.HELP := Compile all quartus projects

################################################

################################################

SD_FAT_TGZ := sd_fat.$(KBUILD_BUILD_VERSION).tar.gz

SD_FAT_TGZ_DEPS += $(foreach r,$(REVISION_LIST),$(ALL_DEPS_$r))
SD_FAT_TGZ_DEPS += zImage.$(LINUX_BRANCH)
SD_FAT_TGZ_DEPS += rootfs.img
SD_FAT_TGZ_DEPS += boot.script u-boot.scr
SD_FAT_TGZ_DEPS += hdl_src
SD_FAT_TGZ_DEPS += board_info
SD_FAT_TGZ_DEPS += ip
SD_FAT_TGZ_DEPS += patches
SD_FAT_TGZ_DEPS += WS1-IntroToSoC
SD_FAT_TGZ_DEPS += WS2-IntroToLinux
SD_FAT_TGZ_DEPS += WS3-DevelopingDrivers
SD_FAT_TGZ_DEPS += WS3-DevelopingDrivers/ws3_lab_environment.src
SD_FAT_TGZ_DEPS += WS3-DevelopingDrivers/$(LINUX_DEFCONFIG)
SD_FAT_TGZ_DEPS += WS3-DevelopingDrivers/example_drivers.tgz

$(SD_FAT_TGZ): $(SD_FAT_TGZ_DEPS)
	@$(RM) $@
	@$(MKDIR) $(@D)
	$(TAR) -czf $@ $^ zImage.*

QUARTUS_OUT_TGZ := quartus_out.tar.gz 
QUARTUS_OUT_TGZ_DEPS += $(ALL_QUARTUS_RBF) $(ALL_QUARTUS_SOF)

$(QUARTUS_OUT_TGZ): $(QUARTUS_OUT_TGZ_DEPS)
	@$(RM) $@
	@$(MKDIR) $(@D)
	$(TAR) -czf $@ $^	
	
HELP_TARGETS += sd-fat
sd-fat.HELP := Generate tar file with contents for sdcard fat partition	
	
.PHONY: sd-fat
sd-fat: $(SD_FAT_TGZ)

AR_FILES += $(wildcard $(SD_FAT_TGZ))

SCRUB_CLEAN_FILES += $(SD_FAT_TGZ)

################################################


################################################
# Clean-up and Archive

AR_TIMESTAMP := $(if $(SOCEDS_VERSION),$(subst .,_,$(SOCEDS_VERSION))_)$(subst $(SPACE),,$(shell $(DATE) +%m%d%Y_%k%M%S))

AR_DIR := tgz
AR_FILE := $(AR_DIR)/soc_workshop_$(AR_TIMESTAMP).tar.gz

AR_FILES += $(filter-out $(AR_FILTER_OUT),$(wildcard $(AR_REGEX)))

CLEAN_FILES += $(filter-out $(AR_DIR) $(AR_FILES),$(wildcard *))

HELP_TARGETS += tgz
tgz.HELP := Create a tarball with the barebones source files that comprise this design

.PHONY: tarball tgz
tarball tgz: $(AR_FILE)

$(AR_FILE):
	@$(MKDIR) $(@D)
	@$(if $(wildcard $(@D)/*.tar.gz),$(MKDIR) $(@D)/.archive;$(MV) $(@D)/*.tar.gz $(@D)/.archive)
	@$(ECHO) "Generating $@..."
	@$(TAR) -czf $@ $(wildcard $(AR_FILES))

SCRUB_CLEAN_FILES += $(CLEAN_FILES)

HELP_TARGETS += scrub_clean
scrub_clean.HELP := Restore design to its barebones state

.PHONY: scrub scrub_clean
scrub scrub_clean:
	$(MAKE) -C sw_src clean
	$(if $(strip $(wildcard $(SCRUB_CLEAN_FILES))),$(RM) $(wildcard $(SCRUB_CLEAN_FILES)),@$(ECHO) "You're already as clean as it gets!")

.PHONY: test_scrub_clean
test_scrub_clean:
	$(if $(strip $(wildcard $(SCRUB_CLEAN_FILES))),$(ECHO) $(wildcard $(SCRUB_CLEAN_FILES)),@$(ECHO) "You're already as clean as it gets!")

.PHONY: tgz_scrub_clean
tgz_scrub_clean:
	$(FIND) $(SOFTWARE_DIR) \( -name '*.o' -o -name '.depend*' -o -name '*.d' -o -name '*.dep' \) -delete || true
	$(MAKE) tgz AR_FILE=$(AR_FILE)
	$(MAKE) -s scrub_clean
	$(TAR) -xzf $(AR_FILE)

################################################


################################################
# Download everything

HELP_TARGETS += downloads
downloads.HELP := Download toolchain, kernel, and buildroot + required buildroot packages

.PHONY: downloads
downloads: toolchain.get buildroot.get linux.get buildroot.downloads coreutils.get tar.get

HELP_TARGETS += downloads_clean	
downloads_clean.HELP := Clean downloads directory

.PHONY: downloads_clean
downloads_clean: 
	$(RM) $(DL)/*

################################################

logs:
	$(MKDIR) logs

################################################
# Utils

.PHONY: setup_sw_env
setup_sw_env: toolchain.extract linux.dodefconfig buildroot.dodefconfig
HELP_TARGETS += setup_sw_env
setup_sw_env.HELP := Setup SW build environment

.PHONY: enable_signaltap
HELP_TARGETS += enable_signaltap
enable_signaltap.HELP := Enable SignalTap in project create scripts

enable_signaltap:
	sed -i '/ENABLE_SIGNALTAP/c set_global_assignment -name ENABLE_SIGNALTAP ON' scripts/create_ghrd_quartus_*.tcl

.PHONY: disable_signaltap
HELP_TARGETS += disable_signaltap
disable_signaltap.HELP := Disable SignalTap in project create scripts

disable_signaltap:
	sed -i '/ENABLE_SIGNALTAP/c set_global_assignment -name ENABLE_SIGNALTAP OFF' scripts/create_ghrd_quartus_*.tcl


################################################
# WorkShop outputs

WS3_OUTPUT_DEPS += WS3-DevelopingDrivers/ws3_lab_environment.src
WS3_OUTPUT_DEPS += WS3-DevelopingDrivers/$(LINUX_DEFCONFIG)
WS3_OUTPUT_DEPS += WS3-DevelopingDrivers/example_drivers.tgz

.PHONY: ws3_output
ws3_output: $(WS3_OUTPUT_DEPS)

WS3-DevelopingDrivers/ws3_lab_environment.src: linux.build
	$(ECHO) "TOOLCHAIN_SOURCE=$(TOOLCHAIN_SOURCE)" > $@
	$(ECHO) "TOOLCHAIN_SOURCE_TAR=$(TOOLCHAIN_SOURCE_TAR)" >> $@
	$(ECHO) "TOOLCHAIN_SOURCE_PACKAGE=$(TOOLCHAIN_SOURCE_PACKAGE)" >> $@
	$(ECHO) "KBUILD_BUILD_VERSION=\"$(KBUILD_BUILD_VERSION)\"" >> $@
	$(ECHO) "LINUX_BRANCH=$(LINUX_BRANCH)" >> $@
	$(ECHO) "LINUX_DEFCONFIG_TARGET=$(LINUX_DEFCONFIG_TARGET)" >> $@
	$(ECHO) "LINUX_DEFCONFIG=$(LINUX_DEFCONFIG)" >> $@
	$(ECHO) "LNX_SOURCE_PACKAGE=$(LNX_SOURCE_PACKAGE)" >> $@

WS3-DevelopingDrivers/$(LINUX_DEFCONFIG): linux.build
	$(CP) $(LINUX_DEFCONFIG) WS3-DevelopingDrivers/$(LINUX_DEFCONFIG)

WS3_DRIVER_SRC_FILES += demo_devmem_app/demo_devmem.c
WS3_DRIVER_SRC_FILES += demo_devmem_app/demo_devmem.h
WS3_DRIVER_SRC_FILES += demo_devmem_app/Makefile
WS3_DRIVER_SRC_FILES += demo_devmem_app/my_altera_avalon_timer_regs.h
WS3_DRIVER_SRC_FILES += demo_devmem_app/my_altera_msgdma_csr_regs.h
WS3_DRIVER_SRC_FILES += demo_devmem_app/my_altera_msgdma_descriptor_regs.h
WS3_DRIVER_SRC_FILES += demo_ioctl_test_app/ioctl_test.c
WS3_DRIVER_SRC_FILES += demo_ioctl_test_app/ioctl_test.h
WS3_DRIVER_SRC_FILES += demo_ioctl_test_app/Makefile
WS3_DRIVER_SRC_FILES += demo_map_test_app/demo_map_test.c
WS3_DRIVER_SRC_FILES += demo_map_test_app/demo_map_test.h
WS3_DRIVER_SRC_FILES += demo_map_test_app/Makefile
WS3_DRIVER_SRC_FILES += demo_map_test_app/my_altera_avalon_timer_regs.h
WS3_DRIVER_SRC_FILES += demo_uio_test_app/demo_uio_test.c
WS3_DRIVER_SRC_FILES += demo_uio_test_app/demo_uio_test.h
WS3_DRIVER_SRC_FILES += demo_uio_test_app/Makefile
WS3_DRIVER_SRC_FILES += demo_uio_test_app/my_altera_avalon_timer_regs.h
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_01.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_01t.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_02.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_03.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_04.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_05.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_05t.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_06.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_07.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_08.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_09.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_10.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_11.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_11.h
WS3_DRIVER_SRC_FILES += driver_modules/demo_module_11t.c
WS3_DRIVER_SRC_FILES += driver_modules/demo_module.h
WS3_DRIVER_SRC_FILES += driver_modules/Kbuild
WS3_DRIVER_SRC_FILES += driver_modules/Makefile
WS3_DRIVER_SRC_FILES += driver_modules/my_altera_avalon_fifo_regs.h
WS3_DRIVER_SRC_FILES += driver_modules/my_altera_avalon_timer_regs.h
WS3_DRIVER_SRC_FILES += driver_modules/my_altera_msgdma_csr_regs.h
WS3_DRIVER_SRC_FILES += driver_modules/my_altera_msgdma_descriptor_regs.h

WS3-DevelopingDrivers/example_drivers.tgz: linux.build
	$(RM) $@
	$(TAR) -czf $@ -C sw_src $(WS3_DRIVER_SRC_FILES)

################################################
# Help system

.PHONY: help
help: help-init help-targets help-fini

.PHONY: help-revisions
help-revisions: help-revisions-init help-revisions-list help-revisions-fini help-revision-target

.PHONY: help-revs
help-revs: help-revisions-init help-revisions-list help-revisions-fini help-revision-target-short


HELP_TARGETS_X := $(patsubst %,help-%,$(sort $(HELP_TARGETS)))

HELP_TARGET_REVISION_X := $(foreach r,$(REVISION_LIST),$(patsubst %,help_rev-%,$(sort $(HELP_TARGETS_$r))))

HELP_TARGET_REVISION_SHORT_X := $(sort $(patsubst $(firstword $(REVISION_LIST)).%,help_rev-REVISIONNAME.%,$(filter-out $(firstword $(REVISION_LIST)),$(HELP_TARGETS_$(firstword $(REVISION_LIST))))))

$(foreach h,$(filter-out $(firstword $(REVISION_LIST)),$(HELP_TARGETS_$(firstword $(REVISION_LIST)))),$(eval $(patsubst %-$(firstword $(REVISION_LIST)),%-REVISIONNAME,$h).HELP := $(subst $(firstword $(REVISION_LIST)),REVISIONNAME,$($h.HELP)))) 

HELP_REVISION_LIST := $(patsubst %,rev_list-%,$(sort $(REVISION_LIST)))

#cheat, put help at the end
HELP_TARGETS_X += help-help-revisions
help-revisions.HELP := Displays Revision specific Target Help

HELP_TARGETS_X += help-help-revs
help-revs.HELP := Displays Short Revision specific Target Help


HELP_TARGETS_X += help-help
help.HELP := Displays this info (i.e. the available targets)


.PHONY: $(HELP_TARGETS_X)
help-targets: $(HELP_TARGETS_X)
$(HELP_TARGETS_X): help-%:
	@$(ECHO) "*********************"
	@$(ECHO) "* Target: $*"
	@$(ECHO) "*   $($*.HELP)"

.PHONY: help-init
help-init:
	@$(ECHO) "*****************************************"
	@$(ECHO) "*                                       *"
	@$(ECHO) "* Manage QuartusII/QSys design          *"
	@$(ECHO) "*                                       *"
	@$(ECHO) "*     Copyright (c) 2014                *"
	@$(ECHO) "*     All Rights Reserved               *"
	@$(ECHO) "*                                       *"
	@$(ECHO) "*****************************************"
	@$(ECHO) ""

.PHONY: help-revisions-init
help-revisions-init:
	@$(ECHO) ""
	@$(ECHO) "*****************************************"
	@$(ECHO) "*                                       *"
	@$(ECHO) "* Revision specific Targets             *"
	@$(ECHO) "*    target-REVISIONNAME                *"
	@$(ECHO) "*                                       *"
	@$(ECHO) "*    Available Revisions:               *"
	

.PHONY: $(HELP_REVISION_LIST)
help-revisions-list: $(HELP_REVISION_LIST)
$(HELP_REVISION_LIST): rev_list-%: 
	@$(ECHO) "*    -> $*"

.PHONY: help-revisions-fini
help-revisions-fini:
	@$(ECHO) "*                                       *"
	@$(ECHO) "*****************************************"
	@$(ECHO) ""

.PHONY: $(HELP_TARGET_REVISION_X)
.PHONY: $(HELP_TARGET_REVISION_SHORT_X)
help-revision-target: $(HELP_TARGET_REVISION_X)
help-revision-target-short: $(HELP_TARGET_REVISION_SHORT_X)
$(HELP_TARGET_REVISION_X) $(HELP_TARGET_REVISION_SHORT_X): help_rev-%:
	@$(ECHO) "*********************"
	@$(ECHO) "* Target: $*"
	@$(ECHO) "*   $($*.HELP)"
	
.PHONY: help-fini
help-fini:
	@$(ECHO) "*********************"

################################################
