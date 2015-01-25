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
overlay.make_all: overlay.extract toolchain.extract linux.build linux.modules
	$(MAKE) -C sw_src $(LINUX_VARIABLES)  INSTALL_DIR=$(CURDIR)/overlay all 2>&1 | tee logs/$$(notdir $$@).log

HELP_TARGETS += overlay.make_install
overlay.make_install.HELP := Install custom apps to overlay directory
.PHONY: overlay.make_install 
overlay.make_install: overlay.make_all linux.modules_install
	$(MAKE) -C sw_src $(LINUX_VARIABLES) INSTALL_DIR=$(CURDIR)/overlay install

HELP_TARGETS += overlay.make_clean
overlay.make_clean.HELP := Clean custom apps to overlay directory
.PHONY: overlay.make_clean
overlay.make_clean:
	$(MAKE) -C sw_src $(LINUX_VARIABLES) INSTALL_DIR=$(CURDIR)/overlay clean


################################################################################
