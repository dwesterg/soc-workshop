
# set build version, toolchain, arch and devicetree source for kernel
LINUX_VARIABLES = PATH=$(TOOLCHAIN_DIR)/bin:$(PATH)
LINUX_VARIABLES += ARCH=$(ARCH)
LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
LINUX_VARIABLES += CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif


################################################################################
# linux

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
	$(RM) linux-socfpga
	$(MKDIR) linux-socfpga
	$(TAR) -xvzf $(DL)/linux-socfpga.tgz --strip-components 1 -C linux-socfpga
	$(stamp_target)

# apply defconfig
.PHONY: linux.dodefconfig
linux.dodefconfig: linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)

linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET): linux.extract $(LINUX_DEFCONFIG)
ifneq ("$(LINUX_DEFCONFIG)","")
	$(CP) $(LINUX_DEFCONFIG) linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
endif
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)

HELP_TARGETS += linux.modules
linux.modules.HELP := Build linux kernel modules
linux.modules: linux.patches linux.dodefconfig toolchain.extract
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) INSTALL_MOD_PATH=overlay modules


HELP_TARGETS += linux.modules_install
linux.modules_install.HELP := Build linux kernel modules
linux.modules_install: linux.modules
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) INSTALL_MOD_PATH=overlay modules_install

# build                                   
linux-socfpga/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET): $(call get_stamp_target,linux.build)

$(LINUX_MAKE_TARGET): linux-socfpga/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET)
	$(CP) $< $@

HELP_TARGETS += linux.build
linux.build.HELP := Build linux kernel
.PHONY: linux.build
linux.build: $(call get_stamp_target,linux.build) 
$(call get_stamp_target,linux.build): $(LNX_DEPS)
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) $(LINUX_MAKE_TARGET)
	$(stamp_target)

# update linux configuration and same defconfig
HELP_TARGETS += linux.menuconfig
linux.menuconfig.HELP := Menuconfig for linux.  Also saves output to SRC directory
linux.menuconfig: linux-socfpga/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) menuconfig
	$(MAKE) -C linux-socfpga $(LINUX_VARIABLES) savedefconfig
	$(CP) $(LINUX_DEFCONFIG) $(LINUX_DEFCONFIG).$(KBUILD_BUILD_VERSION).old
	$(CP)  linux-socfpga/defconfig $(LINUX_DEFCONFIG)     
	
.PHONY: linux.patches
linux.patches: $(call get_stamp_target,linux.patches)
$(call get_stamp_target,linux.patches): $(call get_stamp_target,linux.extract) $(EBII_LINUX_PATCHES)
#	ifneq ("$EBII_LINUX_PATCHES","")
#		$(PATCH) -d linux-socfpga -p1 < $(EBII_LINUX_PATCHES)
#	endif
	$(stamp_target)

	
################################################################################
