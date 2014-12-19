################################################################################
# overlay
# build overlay directory with custom applications and configuration files

# copy template to build directory	
.PHONY: overlay.extract
overlay.extract: $(call get_stamp_target,overlay.extract)
$(call get_stamp_target,overlay.extract):
	$(RM) overlay
	$(CP) -a overlay_template overlay
	$(stamp_target)

HELP_TARGETS += overlay.make_all
overlay.make_all.HELP := Install custom apps to overlay directory
.PHONY: overlay.make_all
overlay.make_all: overlay.extract toolchain.extract linux.patch
	$(MAKE) -C sw_src PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) CROSS_COMPILE=$(CROSS_COMPILE)  INSTALL_DIR=$(CURDIR)/overlay all

HELP_TARGETS += overlay.make_install
overlay.make_install.HELP := Install custom apps to overlay directory
.PHONY: overlay.make_install 
overlay.make_install: overlay.make_all linux.modules_install
	$(MAKE) -C sw_src PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_DIR=$(CURDIR)/overlay install

################################################################################
