package require -exact qsys 14.0

# module properties
set_module_property NAME {memtest}
set_module_property DISPLAY_NAME {memtest}

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {Memtest}
set_module_property DESCRIPTION {default description}
set_module_property AUTHOR {author}

set_module_property COMPOSITION_CALLBACK compose
set_module_property opaque_address_map false

proc compose { } {
    # Instances and instance parameters
    # (disabled instances are intentionally culled)
    add_instance avst_comparator_128bits avst_comparator_128bits 1.0

    add_instance avst_lfsr_packet_gen avst_lfsr_packet_gen 1.0

    add_instance avst_lfsr_packet_gen_compdata avst_lfsr_packet_gen 1.0

    add_instance clk_0 clock_source 14.1
    set_instance_parameter_value clk_0 {clockFrequency} {50000000.0}
    set_instance_parameter_value clk_0 {clockFrequencyKnown} {1}
    set_instance_parameter_value clk_0 {resetSynchronousEdges} {NONE}

    add_instance control_splitter_seed altera_avalon_st_splitter 14.1
    set_instance_parameter_value control_splitter_seed {NUMBER_OF_OUTPUTS} {2}
    set_instance_parameter_value control_splitter_seed {QUALIFY_VALID_OUT} {1}
    set_instance_parameter_value control_splitter_seed {USE_READY} {0}
    set_instance_parameter_value control_splitter_seed {USE_VALID} {1}
    set_instance_parameter_value control_splitter_seed {USE_PACKETS} {0}
    set_instance_parameter_value control_splitter_seed {USE_CHANNEL} {0}
    set_instance_parameter_value control_splitter_seed {USE_ERROR} {0}
    set_instance_parameter_value control_splitter_seed {USE_DATA} {1}
    set_instance_parameter_value control_splitter_seed {DATA_WIDTH} {128}
    set_instance_parameter_value control_splitter_seed {CHANNEL_WIDTH} {1}
    set_instance_parameter_value control_splitter_seed {ERROR_WIDTH} {1}
    set_instance_parameter_value control_splitter_seed {BITS_PER_SYMBOL} {8}
    set_instance_parameter_value control_splitter_seed {MAX_CHANNELS} {1}
    set_instance_parameter_value control_splitter_seed {READY_LATENCY} {0}
    set_instance_parameter_value control_splitter_seed {ERROR_DESCRIPTOR} {}

    add_instance control_splitter_size altera_avalon_st_splitter 14.1
    set_instance_parameter_value control_splitter_size {NUMBER_OF_OUTPUTS} {2}
    set_instance_parameter_value control_splitter_size {QUALIFY_VALID_OUT} {1}
    set_instance_parameter_value control_splitter_size {USE_READY} {0}
    set_instance_parameter_value control_splitter_size {USE_VALID} {1}
    set_instance_parameter_value control_splitter_size {USE_PACKETS} {0}
    set_instance_parameter_value control_splitter_size {USE_CHANNEL} {0}
    set_instance_parameter_value control_splitter_size {USE_ERROR} {0}
    set_instance_parameter_value control_splitter_size {USE_DATA} {1}
    set_instance_parameter_value control_splitter_size {DATA_WIDTH} {32}
    set_instance_parameter_value control_splitter_size {CHANNEL_WIDTH} {1}
    set_instance_parameter_value control_splitter_size {ERROR_WIDTH} {1}
    set_instance_parameter_value control_splitter_size {BITS_PER_SYMBOL} {8}
    set_instance_parameter_value control_splitter_size {MAX_CHANNELS} {1}
    set_instance_parameter_value control_splitter_size {READY_LATENCY} {0}
    set_instance_parameter_value control_splitter_size {ERROR_DESCRIPTOR} {}

    add_instance memtest_controller memtest_controller 1.0

    # connections and connection parameters
    add_connection avst_lfsr_packet_gen_compdata.data avst_comparator_128bits.compdata avalon_streaming

    add_connection avst_comparator_128bits.err memtest_controller.comperr avalon_streaming

    add_connection control_splitter_seed.out0 avst_lfsr_packet_gen.seed avalon_streaming

    add_connection control_splitter_size.out0 avst_lfsr_packet_gen.size avalon_streaming

    add_connection control_splitter_seed.out1 avst_lfsr_packet_gen_compdata.seed avalon_streaming

    add_connection control_splitter_size.out1 avst_lfsr_packet_gen_compdata.size avalon_streaming

    add_connection memtest_controller.seed control_splitter_seed.in avalon_streaming

    add_connection memtest_controller.size control_splitter_size.in avalon_streaming

    add_connection avst_lfsr_packet_gen_compdata.status memtest_controller.cmppktstatus avalon_streaming

    add_connection avst_comparator_128bits.status memtest_controller.cmpstatus avalon_streaming

    add_connection avst_lfsr_packet_gen.status memtest_controller.pktstatus avalon_streaming

    add_connection clk_0.clk control_splitter_seed.clk clock

    add_connection clk_0.clk control_splitter_size.clk clock

    add_connection clk_0.clk avst_comparator_128bits.clock clock

    add_connection clk_0.clk memtest_controller.clock clock

    add_connection clk_0.clk avst_lfsr_packet_gen.clock clock

    add_connection clk_0.clk avst_lfsr_packet_gen_compdata.clock clock

    add_connection clk_0.clk_reset control_splitter_seed.reset reset

    add_connection clk_0.clk_reset avst_comparator_128bits.reset reset

    add_connection clk_0.clk_reset memtest_controller.reset reset

    add_connection clk_0.clk_reset control_splitter_size.reset reset

    add_connection clk_0.clk_reset avst_lfsr_packet_gen.reset reset

    add_connection clk_0.clk_reset avst_lfsr_packet_gen_compdata.reset reset

    # exported interfaces
    add_interface clk clock sink
    set_interface_property clk EXPORT_OF clk_0.clk_in
    add_interface control avalon slave
    set_interface_property control EXPORT_OF memtest_controller.control
    add_interface gendata avalon_streaming source
    set_interface_property gendata EXPORT_OF avst_lfsr_packet_gen.data
    add_interface indata avalon_streaming sink
    set_interface_property indata EXPORT_OF avst_comparator_128bits.indata
    add_interface reset reset sink
    set_interface_property reset EXPORT_OF clk_0.clk_in_reset

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {1}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
}
