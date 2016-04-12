
# set build version, toolchain, arch and devicetree source for kernel
LINUX_VARIABLES = PATH=$(TOOLCHAIN_DIR)/bin:$(PATH)
LINUX_VARIABLES += ARCH=$(ARCH)
LINUX_VARIABLES += KBUILD_BUILD_VERSION="$(KBUILD_BUILD_VERSION)" 
LINUX_VARIABLES += CROSS_COMPILE=$(CROSS_COMPILE)
ifneq ("$(DEVICETREE_SRC)","")
	LINUX_VARIABLES += CONFIG_DTB_SOURCE=$(DEVICETREE_SRC)
endif
LINUX_VARIABLES += INSTALL_MOD_PATH=$(CURDIR)/overlay

################################################################################
# linux

# download linux source from rocketboards
.PHONY:	linux.get
linux.get: $(DL)/$(LINUX_BRANCH).tgz
$(DL)/$(LINUX_BRANCH).tgz: 
	$(MKDIR) $(DL)
	wget -O $(DL)/$(LINUX_BRANCH).tgz $(LNX_SOURCE_PACKAGE)

# extract linux source
.PHONY: linux.extract
linux.extract: $(call get_stamp_target,$(LINUX_BRANCH).linux.extract)
$(call get_stamp_target,$(LINUX_BRANCH).linux.extract):$(DL)/$(LINUX_BRANCH).tgz $(LINUX_PATCHES)
	$(RM) $(LINUX_BRANCH)
	$(MKDIR) $(LINUX_BRANCH)
	$(TAR) -xvzf $(DL)/$(LINUX_BRANCH).tgz --strip-components 1 -C $(LINUX_BRANCH)
	$(stamp_target)

.PHONY: linux.patch
linux.patch: $(foreach p,$(LINUX_PATCHES),$(call get_stamp_target,$(LINUX_BRANCH).$(notdir $p)))
define do_patch_lnx
$(call get_stamp_target,$(LINUX_BRANCH).$$(notdir $1)): $(call get_stamp_target,$(LINUX_BRANCH).linux.extract)
	patch -d $(LINUX_BRANCH) -p1 < $1 2>&1
	$$(stamp_target)
endef
$(foreach p,$(LINUX_PATCHES),$(eval $(call do_patch_lnx,$p)))

# apply defconfig
.PHONY: linux.dodefconfig
linux.dodefconfig: $(LINUX_BRANCH)/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)

$(LINUX_BRANCH)/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET): linux.extract linux.patch $(LINUX_DEFCONFIG)
ifneq ("$(LINUX_DEFCONFIG)","")
	$(CP) $(LINUX_DEFCONFIG) $(LINUX_BRANCH)/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
endif
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)

HELP_TARGETS += linux.modules
linux.modules.HELP := Build linux kernel modules
linux.modules: linux.patch linux.dodefconfig linux.build toolchain.extract | logs
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) modules 2>&1 | tee logs/$(notdir $@).log


HELP_TARGETS += linux.modules_install
linux.modules_install.HELP := Build linux kernel modules
linux.modules_install: linux.modules | logs
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) modules_install 2>&1 | tee logs/$(notdir $@).log

# build                                   
$(LINUX_BRANCH)/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET): $(call get_stamp_target,$(LINUX_BRANCH).$(KBUILD_BUILD_VERSION).linux.build)

$(LINUX_MAKE_TARGET).$(LINUX_BRANCH): $(LINUX_BRANCH)/arch/$(ARCH)/boot/$(LINUX_MAKE_TARGET)
	$(CP) $< $@

HELP_TARGETS += linux.build
linux.build.HELP := Build linux kernel
.PHONY: linux.build
linux.build: $(call get_stamp_target,$(LINUX_BRANCH).$(KBUILD_BUILD_VERSION).linux.build) 
$(call get_stamp_target,$(LINUX_BRANCH).$(KBUILD_BUILD_VERSION).linux.build): $(LNX_DEPS)
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) $(LINUX_MAKE_TARGET) 2>&1 | tee logs/$(notdir $@).log
	$(stamp_target)

HELP_TARGETS += linux.clean
linux.clean.HELP := Clean linux kernel
linux.clean: $(LNX_DEPS)
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) clean
	$(RM) zImage*
	$(stamp_target)

HELP_TARGETS += linux.dtbs
linux.dtbs.HELP := Make dtbs in kernel source
linux.dtbs: $(LNX_DEPS)
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) dtbs
	$(RM) zImage*
	$(stamp_target)


# update linux configuration and same defconfig
HELP_TARGETS += linux.menuconfig
linux.menuconfig.HELP := Menuconfig for linux.  Also saves output to SRC directory
linux.menuconfig: $(LINUX_BRANCH)/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
	$(CP) $(LINUX_DEFCONFIG) $(LINUX_BRANCH)/arch/$(ARCH)/configs/$(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) $(LINUX_DEFCONFIG_TARGET)
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) menuconfig
	$(MAKE) -C $(LINUX_BRANCH) $(LINUX_VARIABLES) savedefconfig
	$(CP) $(LINUX_DEFCONFIG) $(LINUX_DEFCONFIG).$(KBUILD_BUILD_VERSION).old
	$(CP)  $(LINUX_BRANCH)/defconfig $(LINUX_DEFCONFIG)
	
################################################################################
