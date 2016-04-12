
# set ARCH, toolchain path, busybox config file, download directory, and overlay directory for BUILDROOT
BUILDROOT_VARIABLES := ARCH=$(ARCH) 
BUILDROOT_VARIABLES += BR2_TOOLCHAIN_EXTERNAL_PATH=$(TOOLCHAIN_DIR) 
BUILDROOT_VARIABLES += BR2_DL_DIR=$(CURDIR)/downloads 
BUILDROOT_VARIABLES += BR2_ROOTFS_OVERLAY=$(CURDIR)/overlay
BUILDROOT_VARIABLES += DEFCONFIG=$(CURDIR)/buildroot.defconfig
ifneq ("$(BUSYBOX_CONFIG_FILE)","")
	BUILDROOT_VARIABLES += BUSYBOX_CONFIG_FILE=$(BUSYBOX_CONFIG_FILE) 
endif

################################################################################
# buildroot
BR_DEPS = buildroot.dodefconfig overlay.make_install toolchain.extract

#Download buildroot source
.PHONY:	buildroot.get
buildroot.get: $(DL)/buildroot.tgz
$(DL)/buildroot.tgz: 
	$(MKDIR) $(DL)
	wget -O $(DL)/buildroot.tgz $(BR_SOURCE_PACKAGE)
	$(stamp_target)

#extract buildroot source
.PHONY: buildroot.extract
buildroot.extract: $(call get_stamp_target,buildroot.extract)
$(call get_stamp_target,buildroot.extract): $(DL)/buildroot.tgz 
	$(RM) buildroot
	$(MKDIR) buildroot
	$(TAR) -xvzf $(DL)/buildroot.tgz --strip-components 1 -C buildroot
	$(stamp_target)

# apply custom buildroot configuration
.PHONY: buildroot.dodefconfig
buildroot.dodefconfig: buildroot.extract $(BUILDROOT_DEFCONFIG) 
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) defconfig

# build buildroot
HELP_TARGETS += buildroot.build
buildroot.build.HELP := Build buildroot
.PHONY: buildroot.build
buildroot.build: $(call get_stamp_target,buildroot.build)
$(call get_stamp_target,buildroot.build): $(BR_DEPS)
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) all
	$(stamp_target)

# dowload all buildroot sources required for the build
#HELP_TARGETS += buildroot.downloads
#buildroot.downloads.HELP := Download buildroot sources required for build
.PHONY: buildroot.downloads
buildroot.downloads: buildroot.dodefconfig
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) source

#channge buildroot config and update stored config file
HELP_TARGETS += buildroot.menuconfig
buildroot.menuconfig.HELP := Menuconfig for buildroot.  Also saves output to SRC directory
.PHONY: buildroot.menuconfig
buildroot.menuconfig: buildroot.dodefconfig
	$(CP) $(BUILDROOT_DEFCONFIG) $(BUILDROOT_DEFCONFIG).$(KBUILD_BUILD_VERSION).old
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) menuconfig
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) savedefconfig

# change busybox config and save it.  Note the busybox path with change when
# buidroot is updated
HELP_TARGETS += busybox.menuconfig
busybox.menuconfig.HELP := Menuconfig for busybox.  Also saves output to SRC directory
busybox.menuconfig: buildroot.dodefconfig
	$(MAKE) -C buildroot $(BUILDROOT_VARIABLES) busybox-menuconfig
	$(CP) $(BUSYBOX_CONFIG_FILE) $(BUSYBOX_CONFIG_FILE).$(KBUILD_BUILD_VERSION).old
	$(CP) buildroot/output/build/busybox-$(BUSYBOX_VERSION)/.config $(BUSYBOX_CONFIG_FILE)
	
buildroot/output/images/rootfs.cpio.uboot: $(call get_stamp_target,buildroot.build)
rootfs.img: buildroot/output/images/rootfs.cpio.uboot
	$(CP) $< $@

buildroot/output/images/rootfs.cpio.gz: $(call get_stamp_target,buildroot.build)
rootfs.cpio.gz: buildroot/output/images/rootfs.cpio.gz
	$(CP) $< $@

################################################################################
