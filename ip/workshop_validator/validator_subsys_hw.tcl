package require qsys 

# module properties
set_module_property NAME validator_subsys
set_module_property DISPLAY_NAME validator_subsys

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {Workshop Validator}
set_module_property DESCRIPTION {Workshop validator top level subsystem.}
set_module_property AUTHOR {RSF}

set_module_property COMPOSITION_CALLBACK compose
set_module_property opaque_address_map true

# Device tree parameters
set_module_assignment embeddedsw.dts.vendor "demo"
set_module_assignment embeddedsw.dts.group "validator"
set_module_assignment embeddedsw.dts.name "validator"
set_module_assignment embeddedsw.dts.compatible "demo,validator-1.0"

proc compose { } {
    # Instances and instance parameters
    # (disabled instances are intentionally culled)
    add_instance val_clk clock_source 
    set_instance_parameter_value val_clk {clockFrequency} {1.0}
    set_instance_parameter_value val_clk {clockFrequencyKnown} {0}
    set_instance_parameter_value val_clk {resetSynchronousEdges} {BOTH}

    add_instance val_rng_subsys rng_top_subsys 1.0

    add_instance val_rng_pio altera_avalon_pio 
    set_instance_parameter_value val_rng_pio {bitClearingEdgeCapReg} {0}
    set_instance_parameter_value val_rng_pio {bitModifyingOutReg} {1}
    set_instance_parameter_value val_rng_pio {captureEdge} {0}
    set_instance_parameter_value val_rng_pio {direction} {InOut}
    set_instance_parameter_value val_rng_pio {edgeType} {RISING}
    set_instance_parameter_value val_rng_pio {generateIRQ} {0}
    set_instance_parameter_value val_rng_pio {irqType} {LEVEL}
    set_instance_parameter_value val_rng_pio {resetValue} {0.0}
    set_instance_parameter_value val_rng_pio {simDoTestBenchWiring} {0}
    set_instance_parameter_value val_rng_pio {simDrivenValue} {0.0}
    set_instance_parameter_value val_rng_pio {width} {3}

    add_instance val_conduit_blender conduit_blender 1.0

    add_instance val_nios2_gen2 altera_nios2_gen2 
    set_instance_parameter_value val_nios2_gen2 {setting_showUnpublishedSettings} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_showInternalSettings} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_preciseIllegalMemAccessException} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_exportPCB} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_clearXBitsLDNonBypass} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_bigEndian} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_export_large_RAMs} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_asic_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_asic_synopsys_translate_on_off} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_asic_third_party_synthesis} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_asic_add_scan_mode_input} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_oci_export_jtag_signals} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_avalonDebugPortPresent} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_alwaysEncrypt} {1}
    set_instance_parameter_value val_nios2_gen2 {io_regionbase} {0}
    set_instance_parameter_value val_nios2_gen2 {io_regionsize} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_support31bitdcachebypass} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_activateTrace} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_allow_break_inst} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_activateTestEndChecker} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_ecc_sim_test_ports} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_activateMonitors} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_HDLSimCachesCleared} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_HBreakTest} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_breakslaveoveride} {0}
    set_instance_parameter_value val_nios2_gen2 {mpu_useLimit} {0}
    set_instance_parameter_value val_nios2_gen2 {mpu_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {mmu_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {mmu_autoAssignTlbPtrSz} {1}
    set_instance_parameter_value val_nios2_gen2 {cpuReset} {0}
    set_instance_parameter_value val_nios2_gen2 {resetrequest_enabled} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_removeRAMinit} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_shadowRegisterSets} {0}
    set_instance_parameter_value val_nios2_gen2 {mpu_numOfInstRegion} {8}
    set_instance_parameter_value val_nios2_gen2 {mpu_numOfDataRegion} {8}
    set_instance_parameter_value val_nios2_gen2 {mmu_TLBMissExcOffset} {0}
    set_instance_parameter_value val_nios2_gen2 {resetOffset} {0}
    set_instance_parameter_value val_nios2_gen2 {exceptionOffset} {32}
    set_instance_parameter_value val_nios2_gen2 {cpuID} {0}
    set_instance_parameter_value val_nios2_gen2 {breakOffset} {32}
    set_instance_parameter_value val_nios2_gen2 {userDefinedSettings} {}
    set_instance_parameter_value val_nios2_gen2 {resetSlave} {val_nios2_tc_ram.s1}
    set_instance_parameter_value val_nios2_gen2 {mmu_TLBMissExcSlave} {None}
    set_instance_parameter_value val_nios2_gen2 {exceptionSlave} {val_nios2_tc_ram.s1}
    set_instance_parameter_value val_nios2_gen2 {breakSlave} {val_nios2_tc_ram.s1}
    set_instance_parameter_value val_nios2_gen2 {setting_interruptControllerType} {Internal}
    set_instance_parameter_value val_nios2_gen2 {setting_branchPredictionType} {Dynamic}
    set_instance_parameter_value val_nios2_gen2 {setting_bhtPtrSz} {8}
    set_instance_parameter_value val_nios2_gen2 {cpuArchRev} {1}
#    set_instance_parameter_value val_nios2_gen2 {stratix_dspblock_shift_mul} {0}
#    set_instance_parameter_value val_nios2_gen2 {shifterType} {fast_le_shift}
#    set_instance_parameter_value val_nios2_gen2 {multiplierType} {mul_fast32}
    set_instance_parameter_value val_nios2_gen2 {dividerType} {srt2}
    set_instance_parameter_value val_nios2_gen2 {mpu_minInstRegionSize} {12}
    set_instance_parameter_value val_nios2_gen2 {mpu_minDataRegionSize} {12}
    set_instance_parameter_value val_nios2_gen2 {mmu_uitlbNumEntries} {4}
    set_instance_parameter_value val_nios2_gen2 {mmu_udtlbNumEntries} {6}
    set_instance_parameter_value val_nios2_gen2 {mmu_tlbPtrSz} {7}
    set_instance_parameter_value val_nios2_gen2 {mmu_tlbNumWays} {16}
    set_instance_parameter_value val_nios2_gen2 {mmu_processIDNumBits} {8}
    set_instance_parameter_value val_nios2_gen2 {impl} {Fast}
    set_instance_parameter_value val_nios2_gen2 {icache_size} {0}
    set_instance_parameter_value val_nios2_gen2 {icache_tagramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {icache_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {icache_numTCIM} {1}
    set_instance_parameter_value val_nios2_gen2 {icache_burstType} {None}
    set_instance_parameter_value val_nios2_gen2 {dcache_bursts} {false}
    set_instance_parameter_value val_nios2_gen2 {dcache_victim_buf_impl} {ram}
    set_instance_parameter_value val_nios2_gen2 {dcache_size} {0}
    set_instance_parameter_value val_nios2_gen2 {dcache_tagramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {dcache_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {dcache_numTCDM} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_exportvectors} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_usedesignware} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_ecc_present} {0}
    set_instance_parameter_value val_nios2_gen2 {setting_ic_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_rf_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_mmu_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_dc_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_itcm_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {setting_dtcm_ecc_present} {1}
    set_instance_parameter_value val_nios2_gen2 {regfile_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {ocimem_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {ocimem_ramInit} {0}
    set_instance_parameter_value val_nios2_gen2 {mmu_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {bht_ramBlockType} {Automatic}
    set_instance_parameter_value val_nios2_gen2 {cdx_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {mpx_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_enabled} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_triggerArming} {1}
    set_instance_parameter_value val_nios2_gen2 {debug_debugReqSignals} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_assignJtagInstanceID} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_jtagInstanceID} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_OCIOnchipTrace} {_128}
    set_instance_parameter_value val_nios2_gen2 {debug_hwbreakpoint} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_datatrigger} {0}
    set_instance_parameter_value val_nios2_gen2 {debug_traceType} {none}
    set_instance_parameter_value val_nios2_gen2 {debug_traceStorage} {onchip_trace}

    add_instance val_nios2_tc_ram altera_avalon_onchip_memory2 
    set_instance_parameter_value val_nios2_tc_ram {allowInSystemMemoryContentEditor} {0}
    set_instance_parameter_value val_nios2_tc_ram {blockType} {AUTO}
    set_instance_parameter_value val_nios2_tc_ram {dataWidth} {32}
    set_instance_parameter_value val_nios2_tc_ram {dualPort} {1}
    set_instance_parameter_value val_nios2_tc_ram {initMemContent} {1}
    set_instance_parameter_value val_nios2_tc_ram {initializationFileName} {validator_sw.hex}
    set_instance_parameter_value val_nios2_tc_ram {instanceID} {NONE}
    set_instance_parameter_value val_nios2_tc_ram {memorySize} {16384.0}
    set_instance_parameter_value val_nios2_tc_ram {readDuringWriteMode} {DONT_CARE}
    set_instance_parameter_value val_nios2_tc_ram {simAllowMRAMContentsFile} {0}
    set_instance_parameter_value val_nios2_tc_ram {simMemInitOnlyFilename} {0}
    set_instance_parameter_value val_nios2_tc_ram {singleClockOperation} {0}
    set_instance_parameter_value val_nios2_tc_ram {slave1Latency} {1}
    set_instance_parameter_value val_nios2_tc_ram {slave2Latency} {1}
    set_instance_parameter_value val_nios2_tc_ram {useNonDefaultInitFile} {1}
    set_instance_parameter_value val_nios2_tc_ram {useShallowMemBlocks} {0}
    set_instance_parameter_value val_nios2_tc_ram {writable} {1}
    set_instance_parameter_value val_nios2_tc_ram {ecc_enabled} {0}
    set_instance_parameter_value val_nios2_tc_ram {resetrequest_enabled} {1}

    add_instance val_handshake_ram_1k altera_avalon_onchip_memory2 
    set_instance_parameter_value val_handshake_ram_1k {allowInSystemMemoryContentEditor} {0}
    set_instance_parameter_value val_handshake_ram_1k {blockType} {AUTO}
    set_instance_parameter_value val_handshake_ram_1k {dataWidth} {32}
    set_instance_parameter_value val_handshake_ram_1k {dualPort} {0}
    set_instance_parameter_value val_handshake_ram_1k {initMemContent} {1}
    set_instance_parameter_value val_handshake_ram_1k {initializationFileName} {onchip_mem.hex}
    set_instance_parameter_value val_handshake_ram_1k {instanceID} {NONE}
    set_instance_parameter_value val_handshake_ram_1k {memorySize} {1024.0}
    set_instance_parameter_value val_handshake_ram_1k {readDuringWriteMode} {DONT_CARE}
    set_instance_parameter_value val_handshake_ram_1k {simAllowMRAMContentsFile} {0}
    set_instance_parameter_value val_handshake_ram_1k {simMemInitOnlyFilename} {0}
    set_instance_parameter_value val_handshake_ram_1k {singleClockOperation} {0}
    set_instance_parameter_value val_handshake_ram_1k {slave1Latency} {1}
    set_instance_parameter_value val_handshake_ram_1k {slave2Latency} {1}
    set_instance_parameter_value val_handshake_ram_1k {useNonDefaultInitFile} {0}
    set_instance_parameter_value val_handshake_ram_1k {useShallowMemBlocks} {0}
    set_instance_parameter_value val_handshake_ram_1k {writable} {1}
    set_instance_parameter_value val_handshake_ram_1k {ecc_enabled} {0}
    set_instance_parameter_value val_handshake_ram_1k {resetrequest_enabled} {1}

    add_instance val_des_hash_rom altera_connection_identification_rom_wrapper 
    set_instance_parameter_value val_des_hash_rom {LATENCY} {2}

    add_instance val_altchip_id altchip_id 
    set_instance_parameter_value val_altchip_id {ID_VALUE} {18446744073709551615}

    add_instance val_chip_id_read_mm chip_id_read_mm 1.0

    add_instance val_mm_bridge altera_avalon_mm_bridge 
    set_instance_parameter_value val_mm_bridge {DATA_WIDTH} {32}
    set_instance_parameter_value val_mm_bridge {SYMBOL_WIDTH} {8}
    set_instance_parameter_value val_mm_bridge {ADDRESS_WIDTH} {10}
    set_instance_parameter_value val_mm_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
    set_instance_parameter_value val_mm_bridge {ADDRESS_UNITS} {SYMBOLS}
    set_instance_parameter_value val_mm_bridge {MAX_BURST_SIZE} {1}
    set_instance_parameter_value val_mm_bridge {MAX_PENDING_RESPONSES} {4}
    set_instance_parameter_value val_mm_bridge {LINEWRAPBURSTS} {0}
    set_instance_parameter_value val_mm_bridge {PIPELINE_COMMAND} {0}
    set_instance_parameter_value val_mm_bridge {PIPELINE_RESPONSE} {0}

    # connections and connection parameters
    add_connection val_clk.clk_reset val_rng_subsys.reset reset

    add_connection val_clk.clk val_rng_subsys.clk clock

    add_connection val_clk.clk val_rng_pio.clk clock

    add_connection val_clk.clk_reset val_rng_pio.reset reset

    add_connection val_rng_pio.external_connection val_conduit_blender.external_connection conduit
    set_connection_parameter_value val_rng_pio.external_connection/val_conduit_blender.external_connection endPort {}
    set_connection_parameter_value val_rng_pio.external_connection/val_conduit_blender.external_connection endPortLSB {0}
    set_connection_parameter_value val_rng_pio.external_connection/val_conduit_blender.external_connection startPort {}
    set_connection_parameter_value val_rng_pio.external_connection/val_conduit_blender.external_connection startPortLSB {0}
    set_connection_parameter_value val_rng_pio.external_connection/val_conduit_blender.external_connection width {0}

    add_connection val_conduit_blender.ring_osc_enable val_rng_subsys.ring_osc_enable_splitter_conduit_input conduit
    set_connection_parameter_value val_conduit_blender.ring_osc_enable/val_rng_subsys.ring_osc_enable_splitter_conduit_input endPort {}
    set_connection_parameter_value val_conduit_blender.ring_osc_enable/val_rng_subsys.ring_osc_enable_splitter_conduit_input endPortLSB {0}
    set_connection_parameter_value val_conduit_blender.ring_osc_enable/val_rng_subsys.ring_osc_enable_splitter_conduit_input startPort {}
    set_connection_parameter_value val_conduit_blender.ring_osc_enable/val_rng_subsys.ring_osc_enable_splitter_conduit_input startPortLSB {0}
    set_connection_parameter_value val_conduit_blender.ring_osc_enable/val_rng_subsys.ring_osc_enable_splitter_conduit_input width {0}

    add_connection val_conduit_blender.entropy_counter_enable val_rng_subsys.count_entropy_master_0_enable_conduit conduit
    set_connection_parameter_value val_conduit_blender.entropy_counter_enable/val_rng_subsys.count_entropy_master_0_enable_conduit endPort {}
    set_connection_parameter_value val_conduit_blender.entropy_counter_enable/val_rng_subsys.count_entropy_master_0_enable_conduit endPortLSB {0}
    set_connection_parameter_value val_conduit_blender.entropy_counter_enable/val_rng_subsys.count_entropy_master_0_enable_conduit startPort {}
    set_connection_parameter_value val_conduit_blender.entropy_counter_enable/val_rng_subsys.count_entropy_master_0_enable_conduit startPortLSB {0}
    set_connection_parameter_value val_conduit_blender.entropy_counter_enable/val_rng_subsys.count_entropy_master_0_enable_conduit width {0}

    add_connection val_conduit_blender.entropy_counter_clear val_rng_subsys.count_entropy_master_0_clear_conduit conduit
    set_connection_parameter_value val_conduit_blender.entropy_counter_clear/val_rng_subsys.count_entropy_master_0_clear_conduit endPort {}
    set_connection_parameter_value val_conduit_blender.entropy_counter_clear/val_rng_subsys.count_entropy_master_0_clear_conduit endPortLSB {0}
    set_connection_parameter_value val_conduit_blender.entropy_counter_clear/val_rng_subsys.count_entropy_master_0_clear_conduit startPort {}
    set_connection_parameter_value val_conduit_blender.entropy_counter_clear/val_rng_subsys.count_entropy_master_0_clear_conduit startPortLSB {0}
    set_connection_parameter_value val_conduit_blender.entropy_counter_clear/val_rng_subsys.count_entropy_master_0_clear_conduit width {0}

    add_connection val_clk.clk val_nios2_tc_ram.clk2 clock

    add_connection val_clk.clk val_nios2_tc_ram.clk1 clock

    add_connection val_clk.clk_reset val_nios2_tc_ram.reset1 reset

    add_connection val_clk.clk_reset val_nios2_tc_ram.reset2 reset

    add_connection val_nios2_gen2.tightly_coupled_instruction_master_0 val_nios2_tc_ram.s1 avalon
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_instruction_master_0/val_nios2_tc_ram.s1 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_instruction_master_0/val_nios2_tc_ram.s1 baseAddress {0x0000}
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_instruction_master_0/val_nios2_tc_ram.s1 defaultConnection {0}

    add_connection val_nios2_gen2.tightly_coupled_data_master_0 val_nios2_tc_ram.s2 avalon
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_data_master_0/val_nios2_tc_ram.s2 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_data_master_0/val_nios2_tc_ram.s2 baseAddress {0x0000}
    set_connection_parameter_value val_nios2_gen2.tightly_coupled_data_master_0/val_nios2_tc_ram.s2 defaultConnection {0}

    add_connection val_clk.clk val_nios2_gen2.clk clock

    add_connection val_clk.clk_reset val_nios2_gen2.reset reset

    add_connection val_nios2_gen2.data_master val_rng_subsys.rng_mm_bridge_s0 avalon
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_subsys.rng_mm_bridge_s0 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_subsys.rng_mm_bridge_s0 baseAddress {0x00040000}
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_subsys.rng_mm_bridge_s0 defaultConnection {0}

    add_connection val_clk.clk val_handshake_ram_1k.clk1 clock

    add_connection val_clk.clk_reset val_handshake_ram_1k.reset1 reset

    add_connection val_nios2_gen2.data_master val_handshake_ram_1k.s1 avalon
    set_connection_parameter_value val_nios2_gen2.data_master/val_handshake_ram_1k.s1 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.data_master/val_handshake_ram_1k.s1 baseAddress {0x00010800}
    set_connection_parameter_value val_nios2_gen2.data_master/val_handshake_ram_1k.s1 defaultConnection {0}

    add_connection val_clk.clk_reset val_des_hash_rom.reset reset

    add_connection val_clk.clk val_des_hash_rom.clock clock

    add_connection val_nios2_gen2.data_master val_des_hash_rom.s0 avalon
    set_connection_parameter_value val_nios2_gen2.data_master/val_des_hash_rom.s0 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.data_master/val_des_hash_rom.s0 baseAddress {0x00010c00}
    set_connection_parameter_value val_nios2_gen2.data_master/val_des_hash_rom.s0 defaultConnection {0}

    add_connection val_clk.clk val_altchip_id.clkin clock

    add_connection val_clk.clk_reset val_altchip_id.reset reset

    add_connection val_clk.clk val_chip_id_read_mm.clock clock

    add_connection val_clk.clk_reset val_chip_id_read_mm.reset reset

    add_connection val_altchip_id.output val_chip_id_read_mm.in0 avalon_streaming

    add_connection val_nios2_gen2.data_master val_chip_id_read_mm.s0 avalon
    set_connection_parameter_value val_nios2_gen2.data_master/val_chip_id_read_mm.s0 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.data_master/val_chip_id_read_mm.s0 baseAddress {0x00010c10}
    set_connection_parameter_value val_nios2_gen2.data_master/val_chip_id_read_mm.s0 defaultConnection {0}

    add_connection val_nios2_gen2.data_master val_rng_pio.s1 avalon
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_pio.s1 arbitrationPriority {1}
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_pio.s1 baseAddress {0x00010c20}
    set_connection_parameter_value val_nios2_gen2.data_master/val_rng_pio.s1 defaultConnection {0}

    add_connection val_mm_bridge.m0 val_handshake_ram_1k.s1 avalon
    set_connection_parameter_value val_mm_bridge.m0/val_handshake_ram_1k.s1 arbitrationPriority {1}
    set_connection_parameter_value val_mm_bridge.m0/val_handshake_ram_1k.s1 baseAddress {0x0000}
    set_connection_parameter_value val_mm_bridge.m0/val_handshake_ram_1k.s1 defaultConnection {0}

    add_connection val_clk.clk val_mm_bridge.clk clock

    add_connection val_clk.clk_reset val_mm_bridge.reset reset

    # exported interfaces
    add_interface clk clock sink
    set_interface_property clk EXPORT_OF val_clk.clk_in
    add_interface reset reset sink
    set_interface_property reset EXPORT_OF val_clk.clk_in_reset
    add_interface val_mm_bridge_s0 avalon slave
    set_interface_property val_mm_bridge_s0 EXPORT_OF val_mm_bridge.s0

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {0}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
}
