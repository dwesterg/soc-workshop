package require -exact qsys 14.1

#Add Components
add_instance fifo_0 altera_avalon_fifo 14.1
set_instance_parameter_value fifo_0 {avalonMMAvalonMMDataWidth} {32}
set_instance_parameter_value fifo_0 {avalonMMAvalonSTDataWidth} {32}
set_instance_parameter_value fifo_0 {bitsPerSymbol} {16}
set_instance_parameter_value fifo_0 {channelWidth} {8}
set_instance_parameter_value fifo_0 {errorWidth} {8}
set_instance_parameter_value fifo_0 {fifoDepth} {256}
set_instance_parameter_value fifo_0 {fifoInputInterfaceOptions} {AVALONMM_WRITE}
set_instance_parameter_value fifo_0 {fifoOutputInterfaceOptions} {AVALONMM_READ}
set_instance_parameter_value fifo_0 {showHiddenFeatures} {0}
set_instance_parameter_value fifo_0 {singleClockMode} {1}
set_instance_parameter_value fifo_0 {singleResetMode} {0}
set_instance_parameter_value fifo_0 {symbolsPerBeat} {2}
set_instance_parameter_value fifo_0 {useBackpressure} {1}
set_instance_parameter_value fifo_0 {useIRQ} {0}
set_instance_parameter_value fifo_0 {usePacket} {1}
set_instance_parameter_value fifo_0 {useReadControl} {0}
set_instance_parameter_value fifo_0 {useRegister} {0}
set_instance_parameter_value fifo_0 {useWriteControl} {1}

add_instance fifo_1 altera_avalon_fifo 14.1
set_instance_parameter_value fifo_1 {avalonMMAvalonMMDataWidth} {32}
set_instance_parameter_value fifo_1 {avalonMMAvalonSTDataWidth} {32}
set_instance_parameter_value fifo_1 {bitsPerSymbol} {16}
set_instance_parameter_value fifo_1 {channelWidth} {8}
set_instance_parameter_value fifo_1 {errorWidth} {8}
set_instance_parameter_value fifo_1 {fifoDepth} {256}
set_instance_parameter_value fifo_1 {fifoInputInterfaceOptions} {AVALONMM_WRITE}
set_instance_parameter_value fifo_1 {fifoOutputInterfaceOptions} {AVALONMM_READ}
set_instance_parameter_value fifo_1 {showHiddenFeatures} {0}
set_instance_parameter_value fifo_1 {singleClockMode} {1}
set_instance_parameter_value fifo_1 {singleResetMode} {0}
set_instance_parameter_value fifo_1 {symbolsPerBeat} {2}
set_instance_parameter_value fifo_1 {useBackpressure} {1}
set_instance_parameter_value fifo_1 {useIRQ} {0}
set_instance_parameter_value fifo_1 {usePacket} {1}
set_instance_parameter_value fifo_1 {useReadControl} {0}
set_instance_parameter_value fifo_1 {useRegister} {0}
set_instance_parameter_value fifo_1 {useWriteControl} {1}

add_instance fifo_2 altera_avalon_fifo 14.1
set_instance_parameter_value fifo_2 {avalonMMAvalonMMDataWidth} {32}
set_instance_parameter_value fifo_2 {avalonMMAvalonSTDataWidth} {32}
set_instance_parameter_value fifo_2 {bitsPerSymbol} {16}
set_instance_parameter_value fifo_2 {channelWidth} {8}
set_instance_parameter_value fifo_2 {errorWidth} {8}
set_instance_parameter_value fifo_2 {fifoDepth} {256}
set_instance_parameter_value fifo_2 {fifoInputInterfaceOptions} {AVALONMM_WRITE}
set_instance_parameter_value fifo_2 {fifoOutputInterfaceOptions} {AVALONMM_READ}
set_instance_parameter_value fifo_2 {showHiddenFeatures} {0}
set_instance_parameter_value fifo_2 {singleClockMode} {1}
set_instance_parameter_value fifo_2 {singleResetMode} {0}
set_instance_parameter_value fifo_2 {symbolsPerBeat} {2}
set_instance_parameter_value fifo_2 {useBackpressure} {1}
set_instance_parameter_value fifo_2 {useIRQ} {0}
set_instance_parameter_value fifo_2 {usePacket} {1}
set_instance_parameter_value fifo_2 {useReadControl} {0}
set_instance_parameter_value fifo_2 {useRegister} {0}
set_instance_parameter_value fifo_2 {useWriteControl} {1}


# HPS Connectivity
add_connection lw_mm_bridge.m0 fifo_0.in_csr avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in_csr arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in_csr baseAddress {0x90000}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in_csr defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_0.in avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in baseAddress {0x90020}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.in defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_0.out avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.out arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.out baseAddress {0x90030}
set_connection_parameter_value lw_mm_bridge.m0/fifo_0.out defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_1.in_csr avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in_csr arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in_csr baseAddress {0x94000}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in_csr defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_1.in avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in baseAddress {0x94020}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.in defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_1.out avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.out arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.out baseAddress {0x94030}
set_connection_parameter_value lw_mm_bridge.m0/fifo_1.out defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_2.in_csr avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in_csr arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in_csr baseAddress {0x98000}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in_csr defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_2.in avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in baseAddress {0x98020}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.in defaultConnection {0}

add_connection lw_mm_bridge.m0 fifo_2.out avalon
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.out arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.out baseAddress {0x98030}
set_connection_parameter_value lw_mm_bridge.m0/fifo_2.out defaultConnection {0}

# Clocks and Resets
add_connection clk_0.clk fifo_0.clk_in clock
add_connection clk_0.clk_reset fifo_0.reset_in reset

add_connection clk_0.clk fifo_1.clk_in clock
add_connection clk_0.clk_reset fifo_1.reset_in reset

add_connection clk_0.clk fifo_2.clk_in clock
add_connection clk_0.clk_reset fifo_2.reset_in reset


save_system