package require qsys 

# module properties
set_module_property NAME rng_top_subsys
set_module_property DISPLAY_NAME rng_top_subsys

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {Workshop Validator}
set_module_property DESCRIPTION {Random number generator subsystem for validator_subsys subsystem.}
set_module_property AUTHOR {RSF}

set_module_property COMPOSITION_CALLBACK compose
set_module_property opaque_address_map false

proc compose { } {
    # Instances and instance parameters
    # (disabled instances are intentionally culled)
    add_instance clk_0 clock_source 
    set_instance_parameter_value clk_0 {clockFrequency} {1.0}
    set_instance_parameter_value clk_0 {clockFrequencyKnown} {0}
    set_instance_parameter_value clk_0 {resetSynchronousEdges} {BOTH}

    add_instance single_rng_subsys_0 single_rng_subsys 1.0

    add_instance single_rng_subsys_1 single_rng_subsys 1.0

    add_instance rng_combiner_0 rng_combiner 1.0

    add_instance count_entropy_master_0 count_entropy_master 1.0

    add_instance two_bit_collector_0 two_bit_collector 1.0

    add_instance entropy_ram_1k altera_avalon_onchip_memory2 
    set_instance_parameter_value entropy_ram_1k {allowInSystemMemoryContentEditor} {0}
    set_instance_parameter_value entropy_ram_1k {blockType} {AUTO}
    set_instance_parameter_value entropy_ram_1k {dataWidth} {32}
    set_instance_parameter_value entropy_ram_1k {dualPort} {0}
    set_instance_parameter_value entropy_ram_1k {initMemContent} {1}
    set_instance_parameter_value entropy_ram_1k {initializationFileName} {onchip_mem.hex}
    set_instance_parameter_value entropy_ram_1k {instanceID} {NONE}
    set_instance_parameter_value entropy_ram_1k {memorySize} {1024.0}
    set_instance_parameter_value entropy_ram_1k {readDuringWriteMode} {DONT_CARE}
    set_instance_parameter_value entropy_ram_1k {simAllowMRAMContentsFile} {0}
    set_instance_parameter_value entropy_ram_1k {simMemInitOnlyFilename} {0}
    set_instance_parameter_value entropy_ram_1k {singleClockOperation} {0}
    set_instance_parameter_value entropy_ram_1k {slave1Latency} {1}
    set_instance_parameter_value entropy_ram_1k {slave2Latency} {1}
    set_instance_parameter_value entropy_ram_1k {useNonDefaultInitFile} {0}
    set_instance_parameter_value entropy_ram_1k {useShallowMemBlocks} {0}
    set_instance_parameter_value entropy_ram_1k {writable} {1}
    set_instance_parameter_value entropy_ram_1k {ecc_enabled} {0}
    set_instance_parameter_value entropy_ram_1k {resetrequest_enabled} {1}

    add_instance ring_osc_enable_splitter conduit_splitter 
    set_instance_parameter_value ring_osc_enable_splitter {OUTPUT_NUM} {2}

    add_instance rng_mm_bridge altera_avalon_mm_bridge 
    set_instance_parameter_value rng_mm_bridge {DATA_WIDTH} {32}
    set_instance_parameter_value rng_mm_bridge {SYMBOL_WIDTH} {8}
    set_instance_parameter_value rng_mm_bridge {ADDRESS_WIDTH} {10}
    set_instance_parameter_value rng_mm_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
    set_instance_parameter_value rng_mm_bridge {ADDRESS_UNITS} {SYMBOLS}
    set_instance_parameter_value rng_mm_bridge {MAX_BURST_SIZE} {1}
    set_instance_parameter_value rng_mm_bridge {MAX_PENDING_RESPONSES} {4}
    set_instance_parameter_value rng_mm_bridge {LINEWRAPBURSTS} {0}
    set_instance_parameter_value rng_mm_bridge {PIPELINE_COMMAND} {0}
    set_instance_parameter_value rng_mm_bridge {PIPELINE_RESPONSE} {0}

    add_instance random_data_fifo altera_avalon_fifo 
    set_instance_parameter_value random_data_fifo {avalonMMAvalonMMDataWidth} {32}
    set_instance_parameter_value random_data_fifo {avalonMMAvalonSTDataWidth} {32}
    set_instance_parameter_value random_data_fifo {bitsPerSymbol} {1}
    set_instance_parameter_value random_data_fifo {channelWidth} {0}
    set_instance_parameter_value random_data_fifo {errorWidth} {0}
    set_instance_parameter_value random_data_fifo {fifoDepth} {256}
    set_instance_parameter_value random_data_fifo {fifoInputInterfaceOptions} {AVALONST_SINK}
    set_instance_parameter_value random_data_fifo {fifoOutputInterfaceOptions} {AVALONMM_READ}
    set_instance_parameter_value random_data_fifo {showHiddenFeatures} {0}
    set_instance_parameter_value random_data_fifo {singleClockMode} {1}
    set_instance_parameter_value random_data_fifo {singleResetMode} {0}
    set_instance_parameter_value random_data_fifo {symbolsPerBeat} {32}
    set_instance_parameter_value random_data_fifo {useBackpressure} {0}
    set_instance_parameter_value random_data_fifo {useIRQ} {0}
    set_instance_parameter_value random_data_fifo {usePacket} {0}
    set_instance_parameter_value random_data_fifo {useReadControl} {0}
    set_instance_parameter_value random_data_fifo {useRegister} {0}
    set_instance_parameter_value random_data_fifo {useWriteControl} {1}

    # connections and connection parameters
    add_connection clk_0.clk single_rng_subsys_0.clk clock

    add_connection clk_0.clk_reset single_rng_subsys_0.reset reset

    add_connection clk_0.clk single_rng_subsys_1.clk clock

    add_connection clk_0.clk_reset single_rng_subsys_1.reset reset

    add_connection clk_0.clk rng_combiner_0.clock clock

    add_connection clk_0.clk_reset rng_combiner_0.reset reset

    add_connection single_rng_subsys_0.rng_fifo_out rng_combiner_0.in0 avalon_streaming

    add_connection single_rng_subsys_1.rng_fifo_out rng_combiner_0.in1 avalon_streaming

    add_connection clk_0.clk two_bit_collector_0.clock clock

    add_connection clk_0.clk count_entropy_master_0.clock clock

    add_connection clk_0.clk_reset count_entropy_master_0.reset reset

    add_connection clk_0.clk_reset two_bit_collector_0.reset reset

    add_connection rng_combiner_0.out0 two_bit_collector_0.sink avalon_streaming

    add_connection two_bit_collector_0.source count_entropy_master_0.sink avalon_streaming

    add_connection clk_0.clk entropy_ram_1k.clk1 clock

    add_connection clk_0.clk_reset entropy_ram_1k.reset1 reset

    add_connection count_entropy_master_0.master entropy_ram_1k.s1 avalon
    set_connection_parameter_value count_entropy_master_0.master/entropy_ram_1k.s1 arbitrationPriority {1}
    set_connection_parameter_value count_entropy_master_0.master/entropy_ram_1k.s1 baseAddress {0x0000}
    set_connection_parameter_value count_entropy_master_0.master/entropy_ram_1k.s1 defaultConnection {0}

    add_connection ring_osc_enable_splitter.conduit_output_0 single_rng_subsys_0.enable_splitter_conduit_input conduit
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_0/single_rng_subsys_0.enable_splitter_conduit_input endPort {}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_0/single_rng_subsys_0.enable_splitter_conduit_input endPortLSB {0}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_0/single_rng_subsys_0.enable_splitter_conduit_input startPort {}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_0/single_rng_subsys_0.enable_splitter_conduit_input startPortLSB {0}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_0/single_rng_subsys_0.enable_splitter_conduit_input width {0}

    add_connection ring_osc_enable_splitter.conduit_output_1 single_rng_subsys_1.enable_splitter_conduit_input conduit
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_1/single_rng_subsys_1.enable_splitter_conduit_input endPort {}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_1/single_rng_subsys_1.enable_splitter_conduit_input endPortLSB {0}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_1/single_rng_subsys_1.enable_splitter_conduit_input startPort {}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_1/single_rng_subsys_1.enable_splitter_conduit_input startPortLSB {0}
    set_connection_parameter_value ring_osc_enable_splitter.conduit_output_1/single_rng_subsys_1.enable_splitter_conduit_input width {0}

    add_connection clk_0.clk rng_mm_bridge.clk clock

    add_connection clk_0.clk_reset rng_mm_bridge.reset reset

    add_connection rng_mm_bridge.m0 entropy_ram_1k.s1 avalon
    set_connection_parameter_value rng_mm_bridge.m0/entropy_ram_1k.s1 arbitrationPriority {1}
    set_connection_parameter_value rng_mm_bridge.m0/entropy_ram_1k.s1 baseAddress {0x0000}
    set_connection_parameter_value rng_mm_bridge.m0/entropy_ram_1k.s1 defaultConnection {0}

    add_connection rng_mm_bridge.m0 count_entropy_master_0.tranaction_count avalon
    set_connection_parameter_value rng_mm_bridge.m0/count_entropy_master_0.tranaction_count arbitrationPriority {1}
    set_connection_parameter_value rng_mm_bridge.m0/count_entropy_master_0.tranaction_count baseAddress {0x0400}
    set_connection_parameter_value rng_mm_bridge.m0/count_entropy_master_0.tranaction_count defaultConnection {0}

    add_connection count_entropy_master_0.source random_data_fifo.in avalon_streaming

    add_connection rng_mm_bridge.m0 random_data_fifo.out avalon
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.out arbitrationPriority {1}
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.out baseAddress {0x0410}
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.out defaultConnection {0}

    add_connection rng_mm_bridge.m0 random_data_fifo.in_csr avalon
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.in_csr arbitrationPriority {1}
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.in_csr baseAddress {0x0420}
    set_connection_parameter_value rng_mm_bridge.m0/random_data_fifo.in_csr defaultConnection {0}

    add_connection clk_0.clk random_data_fifo.clk_in clock

    add_connection clk_0.clk_reset random_data_fifo.reset_in reset

    # exported interfaces
    add_interface clk clock sink
    set_interface_property clk EXPORT_OF clk_0.clk_in
    add_interface reset reset sink
    set_interface_property reset EXPORT_OF clk_0.clk_in_reset
    add_interface count_entropy_master_0_enable_conduit conduit end
    set_interface_property count_entropy_master_0_enable_conduit EXPORT_OF count_entropy_master_0.enable_conduit
    add_interface count_entropy_master_0_clear_conduit conduit end
    set_interface_property count_entropy_master_0_clear_conduit EXPORT_OF count_entropy_master_0.clear_conduit
    add_interface ring_osc_enable_splitter_conduit_input conduit end
    set_interface_property ring_osc_enable_splitter_conduit_input EXPORT_OF ring_osc_enable_splitter.conduit_input
    add_interface rng_mm_bridge_s0 avalon slave
    set_interface_property rng_mm_bridge_s0 EXPORT_OF rng_mm_bridge.s0

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {1}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
}
