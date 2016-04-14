################################################
# Preloader

QSYS_HPS_INST_NAME ?= hps_0

PRELOADER_DISABLE_WATCHDOG ?= 1
ifeq ($(PRELOADER_DISABLE_WATCHDOG),1)
PRELOADER_EXTRA_ARGS += --set spl.boot.WATCHDOG_ENABLE false
endif

PRELOADER_FAT_SUPPORT ?= 1
ifeq ($(PRELOADER_FAT_SUPPORT),1)
PRELOADER_EXTRA_ARGS += --set spl.boot.FAT_SUPPORT true
endif

PRELOADER_EXTRA_ARGS += --set spl.boot.FAT_LOAD_PAYLOAD_NAME $1/u-boot.img

define do_patch_uboot
$(call get_stamp_target_hw,$1.$(notdir $2)): $3
	$(CP) $2 $1/preloader/
	$$(stamp_target)
endef

define build_preloader_revisions

PRELOADER_DIR_$1 := $1/preloader

HELP_TARGETS_$1 += $1.preloader
$1.preloader.HELP := Build Preloader $1 BSP for this design into $(PRELOADER_DIR) directory

.PHONY: $1.preloader
$1.preloader: $$(PRELOADER_STAMP_$1)

# Create and build preloader with watchdog disabled.
# This is useful for board bring up and troubleshooting.
$1/preloader/preloader-mkpimage.bin: $$(PRELOADER_STAMP_$1)

$1/preloader-mkpimage.bin: $1/preloader/preloader-mkpimage.bin
	$(CP) $$< $$@

PRELOADER_EXTRA_ARGS_$1 += $(PRELOADER_EXTRA_ARGS)

$$(PRELOADER_GEN_STAMP_$1): $$(PRELOADER_GEN_DEPS_$1)
	@$(MKDIR) $$(PRELOADER_DIR_$1)
	$(SBT.CREATE_SETTINGS) \
		--type spl \
		--bsp-dir $$(PRELOADER_DIR_$1) \
		--preloader-settings-dir "$1/hps_isw_handoff/$(QSYS_BASE_NAME)_$(QSYS_HPS_INST_NAME)" \
		--settings $$(PRELOADER_DIR_$1)/settings.bsp \
		$$(PRELOADER_EXTRA_ARGS_$1) 
	$$(stamp_target)

$(foreach p,$(UBOOT_PATCHES),$(eval $(call do_patch_uboot,$1,$p,$(PRELOADER_GEN_STAMP_$1))))

.PHONY: $1.uboot_patch
$1.uboot_patch: $$(PRELOADER_FIXUP_STAMP_$1)

$$(PRELOADER_FIXUP_STAMP_$1): $(foreach p,$(UBOOT_PATCHES),$(call get_stamp_target_hw,$1.$(notdir $p)))
	@$(ECHO) "#define CONFIG_SOC_BOARD_NAME $1" >> $1/preloader/generated/build.h
	@$(ECHO) "#define CONFIG_SOCFPGA_GPIO 1" >> $1/preloader/generated/build.h
	@$(ECHO) "#define CONFIG_CMD_GPIO 1" >> $1/preloader/generated/build.h
	@$(ECHO) "#define CONFIG_CMD_CHIPID2MAC 1" >> $1/preloader/generated/build.h
	$$(stamp_target)
	
$$(PRELOADER_STAMP_$1): $$(PRELOADER_DEPS_$1)
	$(MAKE) -C $$(PRELOADER_DIR_$1) 2>&1 | tee logs/$$(notdir $$@).log
	$$(stamp_target)	

$1/preloader/uboot-socfpga/u-boot.img: $$(UBOOT_STAMP_$1)

$1/u-boot.img: $1/preloader/uboot-socfpga/u-boot.img
	$(CP) $$< $$@

$$(UBOOT_STAMP_$1): $$(PRELOADER_STAMP_$1)
	$(MAKE) -C $$(PRELOADER_DIR_$1) uboot 2>&1 | tee logs/$$(notdir $$@).log
	$$(stamp_target)
	
ifeq ($(IS_WINDOWS_HOST),1)
EXE_EXT := .exe
endif
UBOOT_MKIMAGE_$1 := $$(PRELOADER_DIR_$1)/uboot-socfpga/tools/mkimage$(EXE_EXT)

$$(UBOOT_MKIMAGE_$1): $$(PRELOADER_STAMP_$1)

HELP_TARGETS_$1 += $1.uboot
$1.uboot.HELP := Build U-Boot $1 into $$(PRELOADER_DIR_$1) directory

.PHONY: $1.uboot
$1.uboot: $$(UBOOT_STAMP_$1)

endef # build_preloader_revisions

$(foreach r, $(REVISION_LIST), $(eval $(call build_preloader_revisions,$r)))
	

