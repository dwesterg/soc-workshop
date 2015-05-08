SRVR := sj-ice-nx2.altera.com

ARC_BUILD_INTERMEDIATE_TARGETS := $(foreach r,$(REVISION_LIST),arc_build-$r)
#
.PHONY: arc_build_all
arc_build_all:
	$(MAKE) $(ARC_BUILD_INTERMEDIATE_TARGETS)
	
define arc_build_project

.PHONY: arc_build-$1
arc_build-$1:
	arc submit -i --watch acds/15.0,soceds/15.0,git os=linux64 ncpus=1 iwd=$(CURDIR) -- arc vnc make -j1 $1.all

endef

$(foreach r,$(REVISION_LIST),$(eval $(call arc_build_project,$r)))

.PHONY: arc_build_sync
arc_build_sync:
	rsync -e 'ssh -q' -avzP --delete --include=*/preloader/uboot-socfpga/u-boot.img --exclude=*/db --exclude=*/incremental_db --exclude=*/preloader/uboot-socfpga/* $(SRVR):$(CURDIR)/ $(CURDIR)/

