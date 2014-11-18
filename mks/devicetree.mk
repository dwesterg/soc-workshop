################################################
# Device Tree


#DTS.CLOCKINFO ?= hps_clock_info.xml
DTS.SOPC2DTS_ARGS += $(if $(DTS_COMMON),--board $(DTS_COMMON))
DTS.SOPC2DTS_ARGS += --bridge-removal all
DTS.SOPC2DTS_ARGS += --clocks

define build_dts_revisions

DTS.SOPC2DTS_ARGS_$1 += $(DTS.SOPC2DTS_ARGS)
DTS.SOPC2DTS_ARGS_$1 += $(if $$(DTS_BOARDINFO_$1),--board $$(DTS_BOARDINFO_$1))

#$$(DEVICE_TREE_SOURCE_$1): $$(DTS_STAMP_$1)
#
#$$(DTS_STAMP_$1): $$(DTS_DEPS_$1)
#	$(DTS.SOPC2DTS) --input $$(QSYS_SOPCINFO_$1) --output $$(DEVICE_TREE_SOURCE_$1) $$(DTS.SOPC2DTS_ARGS_$1)
#	$$(stamp_target)
$$(DEVICE_TREE_SOURCE_$1):
	$(DTS.SOPC2DTS) --input $$(QSYS_SOPCINFO_$1) --output $$(DEVICE_TREE_SOURCE_$1) $$(DTS.SOPC2DTS_ARGS_$1)
	$$(stamp_target)

HELP_TARGETS_$1 += dts-$1
dts-$1.HELP := Generate a device tree for $1

.PHONY: dts-$1
dts-$1: $$(DTS_STAMP_$1)

#$$(DEVICE_TREE_BLOB_$1): $$(DTB_STAMP_$1)
#
#$$(DTB_STAMP_$1): $$(DTB_DEPS_$1)
#	$(DTS.SOPC2DTS) --type dtb --input $$(QSYS_SOPCINFO_$1) --output $$(DEVICE_TREE_BLOB_$1) $$(DTS.SOPC2DTS_ARGS_$1)
#	$$(stamp_target)
$$(DEVICE_TREE_BLOB_$1):
	$(DTS.SOPC2DTS) --type dtb --input $$(QSYS_SOPCINFO_$1) --output $$(DEVICE_TREE_BLOB_$1) $$(DTS.SOPC2DTS_ARGS_$1)
	$$(stamp_target)



HELP_TARGETS_$1 += dtb-$1
dtb-$1.HELP := Generate a device tree blob for $1

.PHONY: dtb-$1
dtb-$1: $$(DTB_STAMP_$1)

endef # build_dts_revisions

$(foreach r, $(REVISION_LIST), $(eval $(call build_dts_revisions,$r)))


