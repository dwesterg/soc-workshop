package require qsys 

# module properties
set_module_property NAME single_rng_subsys
set_module_property DISPLAY_NAME single_rng_subsys

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {Workshop Validator}
set_module_property DESCRIPTION {Single random number generator chain subsystem for rng_top_subsys subsystem.}
set_module_property AUTHOR {RSF}

set_module_property COMPOSITION_CALLBACK compose
set_module_property opaque_address_map false

proc compose { } {
    # Instances and instance parameters
    # (disabled instances are intentionally culled)
    add_instance clk_0 clock_source 
    set_instance_parameter_value clk_0 {clockFrequency} {1.0}
    set_instance_parameter_value clk_0 {clockFrequencyKnown} {0}
    set_instance_parameter_value clk_0 {resetSynchronousEdges} {DEASSERT}

    add_instance ring_oscillator_113 ring_oscillator 1.0
    set_instance_parameter_value ring_oscillator_113 {BIT_WIDTH} {113}

    add_instance ring_oscillator_97 ring_oscillator 1.0
    set_instance_parameter_value ring_oscillator_97 {BIT_WIDTH} {97}

    add_instance ring_oscillator_83 ring_oscillator 1.0
    set_instance_parameter_value ring_oscillator_83 {BIT_WIDTH} {83}

    add_instance ring_combiner_0 ring_combiner 1.0
    set_instance_parameter_value ring_combiner_0 {SYNC_DEPTH} {5}

    add_instance rng_deskew_0 rng_deskew 1.0

    add_instance rng_deskew_1 rng_deskew 1.0

    add_instance rng_deskew_2 rng_deskew 1.0

    add_instance two_bit_collector_0 two_bit_collector 1.0

    add_instance two_bit_collector_1 two_bit_collector 1.0

    add_instance two_bit_collector_2 two_bit_collector 1.0

    add_instance rng_fifo altera_avalon_sc_fifo 
    set_instance_parameter_value rng_fifo {SYMBOLS_PER_BEAT} {1}
    set_instance_parameter_value rng_fifo {BITS_PER_SYMBOL} {1}
    set_instance_parameter_value rng_fifo {FIFO_DEPTH} {8192}
    set_instance_parameter_value rng_fifo {CHANNEL_WIDTH} {0}
    set_instance_parameter_value rng_fifo {ERROR_WIDTH} {0}
    set_instance_parameter_value rng_fifo {USE_PACKETS} {0}
    set_instance_parameter_value rng_fifo {USE_FILL_LEVEL} {0}
    set_instance_parameter_value rng_fifo {EMPTY_LATENCY} {3}
    set_instance_parameter_value rng_fifo {USE_MEMORY_BLOCKS} {1}
    set_instance_parameter_value rng_fifo {USE_STORE_FORWARD} {0}
    set_instance_parameter_value rng_fifo {USE_ALMOST_FULL_IF} {0}
    set_instance_parameter_value rng_fifo {USE_ALMOST_EMPTY_IF} {0}
    set_instance_parameter_value rng_fifo {ENABLE_EXPLICIT_MAXCHANNEL} {0}
    set_instance_parameter_value rng_fifo {EXPLICIT_MAXCHANNEL} {0}

    add_instance enable_splitter conduit_splitter 
    set_instance_parameter_value enable_splitter {OUTPUT_NUM} {3}

    # connections and connection parameters
    add_connection clk_0.clk ring_oscillator_113.clock clock

    add_connection clk_0.clk ring_oscillator_97.clock clock

    add_connection clk_0.clk ring_oscillator_83.clock clock

    add_connection clk_0.clk ring_combiner_0.clock clock

    add_connection ring_oscillator_113.clock_source ring_combiner_0.clock_sink_0 clock

    add_connection ring_oscillator_97.clock_source ring_combiner_0.clock_sink_1 clock

    add_connection ring_oscillator_83.clock_source ring_combiner_0.clock_sink_2 clock

    add_connection clk_0.clk rng_deskew_0.clock clock

    add_connection clk_0.clk rng_deskew_1.clock clock

    add_connection clk_0.clk rng_deskew_2.clock clock

    add_connection ring_combiner_0.source two_bit_collector_0.sink avalon_streaming

    add_connection two_bit_collector_0.source rng_deskew_0.sink avalon_streaming

    add_connection rng_deskew_0.source two_bit_collector_1.sink avalon_streaming

    add_connection two_bit_collector_1.source rng_deskew_1.sink avalon_streaming

    add_connection rng_deskew_1.source two_bit_collector_2.sink avalon_streaming

    add_connection two_bit_collector_2.source rng_deskew_2.sink avalon_streaming

    add_connection clk_0.clk two_bit_collector_0.clock clock

    add_connection clk_0.clk two_bit_collector_1.clock clock

    add_connection clk_0.clk two_bit_collector_2.clock clock

    add_connection rng_deskew_2.source rng_fifo.in avalon_streaming

    add_connection clk_0.clk rng_fifo.clk clock

    add_connection clk_0.clk_reset ring_oscillator_113.reset reset

    add_connection clk_0.clk_reset ring_oscillator_97.reset reset

    add_connection clk_0.clk_reset ring_oscillator_83.reset reset

    add_connection clk_0.clk_reset ring_combiner_0.reset reset

    add_connection clk_0.clk_reset rng_deskew_0.reset reset

    add_connection clk_0.clk_reset rng_deskew_1.reset reset

    add_connection clk_0.clk_reset rng_deskew_2.reset reset

    add_connection clk_0.clk_reset two_bit_collector_0.reset reset

    add_connection clk_0.clk_reset two_bit_collector_1.reset reset

    add_connection clk_0.clk_reset two_bit_collector_2.reset reset

    add_connection clk_0.clk_reset rng_fifo.clk_reset reset

    add_connection enable_splitter.conduit_output_0 ring_oscillator_113.osc_enable conduit
    set_connection_parameter_value enable_splitter.conduit_output_0/ring_oscillator_113.osc_enable endPort {}
    set_connection_parameter_value enable_splitter.conduit_output_0/ring_oscillator_113.osc_enable endPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_0/ring_oscillator_113.osc_enable startPort {}
    set_connection_parameter_value enable_splitter.conduit_output_0/ring_oscillator_113.osc_enable startPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_0/ring_oscillator_113.osc_enable width {0}

    add_connection enable_splitter.conduit_output_1 ring_oscillator_97.osc_enable conduit
    set_connection_parameter_value enable_splitter.conduit_output_1/ring_oscillator_97.osc_enable endPort {}
    set_connection_parameter_value enable_splitter.conduit_output_1/ring_oscillator_97.osc_enable endPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_1/ring_oscillator_97.osc_enable startPort {}
    set_connection_parameter_value enable_splitter.conduit_output_1/ring_oscillator_97.osc_enable startPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_1/ring_oscillator_97.osc_enable width {0}

    add_connection enable_splitter.conduit_output_2 ring_oscillator_83.osc_enable conduit
    set_connection_parameter_value enable_splitter.conduit_output_2/ring_oscillator_83.osc_enable endPort {}
    set_connection_parameter_value enable_splitter.conduit_output_2/ring_oscillator_83.osc_enable endPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_2/ring_oscillator_83.osc_enable startPort {}
    set_connection_parameter_value enable_splitter.conduit_output_2/ring_oscillator_83.osc_enable startPortLSB {0}
    set_connection_parameter_value enable_splitter.conduit_output_2/ring_oscillator_83.osc_enable width {0}

    # exported interfaces
    add_interface clk clock sink
    set_interface_property clk EXPORT_OF clk_0.clk_in
    add_interface reset reset sink
    set_interface_property reset EXPORT_OF clk_0.clk_in_reset
    add_interface rng_fifo_out avalon_streaming source
    set_interface_property rng_fifo_out EXPORT_OF rng_fifo.out
    add_interface enable_splitter_conduit_input conduit end
    set_interface_property enable_splitter_conduit_input EXPORT_OF enable_splitter.conduit_input

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {1}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
}
