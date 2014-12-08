TAR_SOURCE := tar-1.28.tar.gz
TAR_SOURCE_PACKAGE := "http://ftp.gnu.org/gnu/tar/$(TAR_SOURCE)"

.PHONY: tar.download
tar.download: $(DL)/$(TAR_SOURCE)
$(DL)/$(TAR_SOURCE):
	$(MKDIR) $(DL)
	wget -O $@ $(TAR_SOURCE_PACKAGE)

$(call get_stamp_target,tar.extract): $(DL)/$(TAR_SOURCE)
	$(MKDIR) tar
	$(TAR) -xzvf $< --strip-components 1 -C tar
	$(stamp_target)

.PHONY: tar.compile
tar.compile: $(call get_stamp_target,tar.compile)
$(call get_stamp_target,tar.compile): $(call get_stamp_target,tar.extract)
	$(shell cd tar;./configure > /dev/null)
	$(MAKE) -C tar all
	$(stamp_target)

ARC_BUILD_INTERMEDIATE_TARGETS := $(foreach r,$(REVISION_LIST),arc_build-$r)

.PHONY: arc_build_all
arc_build_all: $(call get_stamp_target,tar.compile)
	$(MAKE) TAR=$(CURDIR)/tar/src/tar http_proxy=$(HTTP_PROXY) https_proxy=$(HTTPS_PROXY) downloads
	$(MAKE) TAR=$(CURDIR)/tar/src/tar $(ARC_BUILD_INTERMEDIATE_TARGETS)
	$(MAKE) TAR=$(CURDIR)/tar/src/tar all

	
define arc_build_project

.PHONY: arc_build-$1
arc_build-$1:
	arc submit -i --watch -- arc vnc make -j1 $1.all

endef

$(foreach r,$(REVISION_LIST),$(eval $(call arc_build_project,$r)))
