
ARC_BUILD_INTERMEDIATE_TARGETS := $(foreach r,$(REVISION_LIST),arc_build-$r)

ARC_PARALLEL := $(words $(ARC_BUILD_INTERMEDIATE_TARGETS))

ARC_PARALLEL_ADD := $(shell echo ${ARC_PARALLEL}+1 | bc)

.PHONY: arc_build_all
arc_build_all:
	$(MAKE) -j8 http_proxy=$(HTTP_PROXY) https_proxy=$(HTTPS_PROXY) downloads
	$(MAKE) -j$(ARC_PARALLEL) create_all_projects
	$(MAKE) -j$(ARC_PARALLEL) $(ARC_BUILD_INTERMEDIATE_TARGETS)
	$(MAKE) -j8 all

	
define arc_build_project

.PHONY: arc_build-$1
arc_build-$1:
	arc submit -i --watch -- arc vnc make -j1 $1.all

endef

$(foreach r,$(REVISION_LIST),$(eval $(call arc_build_project,$r)))
