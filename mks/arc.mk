
ARC_BUILD_INTERMEDIATE_TARGETS := $(foreach r,$(REVISION_LIST),arc_build-$r)

ARC_PARALLEL := $(words $(ARC_BUILD_INTERMEDIATE_TARGETS))

.PHONY: arc_build_all
arc_build_all:
	make http_proxy=$(HTTP_PROXY) https_proxy=$(HTTPS_PROXY) downloads
	make -j$(ARC_PARALLEL) create_all_projects
	make arc_build_init
	make -j$(ARC_PARALLEL) $(ARC_BUILD_INTERMEDIATE_TARGETS)
	make -j8 all

	
define arc_build_project

.PHONY: arc_build-$1
arc_build-$1:
	arc submit --watch -- arc vnc make $1.all

endef

$(foreach r,$(REVISION_LIST),$(eval $(call arc_build_project,$r)))
