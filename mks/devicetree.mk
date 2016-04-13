
# set build version, toolchain, arch and devicetree source for kernel
LINUX_VARIABLES = PATH=$(TOOLCHAIN_DIR)/bin:$(PATH)
LINUX_VARIABLES += ARCH=$(ARCH)
LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
LINUX_VARIABLES += CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif
LINUX_VARIABLES += INSTALL_MOD_PATH=$(CURDIR)/overlay


################################################
# Device Tree

define build_dts_revisions

$(LINUX_BRANCH)/arch/arm/boot/dts/$1.dts: devicetrees/$1.dts
	$(CP) $< $@

$(LINUX_BRANCH)/arch/arm/boot/dts/$1.dtb: $(LINUX_BRANCH)/arch/arm/boot/dts/$1.dts
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) $1.dtb
	
$1/soc_system.dtb: $(LINUX_BRANCH)/arch/arm/boot/dts/$1.dtb
	$(CP) $< $@
	
$1/soc_system.dts: $1/soc_system.dtb
	$(DTC) -I dtb -O dts -o $@ $<

endef # build_dts_revisions

$(LINUX_BRANCH)/arch/arm/boot/dts/socfpga_cyclone5_socwks_fpga_overlay.dts: devicetrees/socfpga_cyclone5_socwks_fpga_overlay.dts
	$(CP) $< $@

$(LINUX_BRANCH)/arch/arm/boot/dts/socfpga_cyclone5_socwks_fpga_overlay.dtb: $(LINUX_BRANCH)/arch/arm/boot/dts/socfpga_cyclone5_socwks_fpga_overlay.dts
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) socfpga_cyclone5_socwks_fpga_overlay.dtb

overlay/lib/firmware/socfpga_cyclone5_socwks_fpga_overlay.dtb: $(LINUX_BRANCH)/arch/arm/boot/dts/socfpga_cyclone5_socwks_fpga_overlay.dtb $(call get_stamp_target,overlay.extract)
	$(MKDIR) overlay/lib/firmware
	$(CP) $(LINUX_BRANCH)/arch/arm/boot/dts/socfpga_cyclone5_socwks_fpga_overlay.dtb $@


$(foreach r, $(REVISION_LIST), $(eval $(call build_dts_revisions,$r)))


