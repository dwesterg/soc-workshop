
ARC_BUILD_INTERMEDIATE_TARGETS := $(foreach r,$(REVISION_LIST),arc_build-$r)

.PHONY: arc_build_all
arc_build_all:
	$(MAKE) http_proxy=$(HTTP_PROXY) https_proxy=$(HTTPS_PROXY) downloads
	$(MAKE) $(ARC_BUILD_INTERMEDIATE_TARGETS)
	$(MAKE) all

	
define arc_build_project

.PHONY: arc_build-$1
arc_build-$1:
	arc submit -i --watch -- arc vnc make -j1 $1.all

endef

$(foreach r,$(REVISION_LIST),$(eval $(call arc_build_project,$r)))
