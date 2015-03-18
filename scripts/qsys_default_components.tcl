#Add Components

add_instance lw_mm_bridge altera_avalon_mm_bridge 14.1
set_instance_parameter_value lw_mm_bridge {DATA_WIDTH} {32}
set_instance_parameter_value lw_mm_bridge {SYMBOL_WIDTH} {8}
set_instance_parameter_value lw_mm_bridge {ADDRESS_WIDTH} {10}
set_instance_parameter_value lw_mm_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
set_instance_parameter_value lw_mm_bridge {ADDRESS_UNITS} {SYMBOLS}
set_instance_parameter_value lw_mm_bridge {MAX_BURST_SIZE} {1}
set_instance_parameter_value lw_mm_bridge {MAX_PENDING_RESPONSES} {4}
set_instance_parameter_value lw_mm_bridge {LINEWRAPBURSTS} {0}
set_instance_parameter_value lw_mm_bridge {PIPELINE_COMMAND} {1}
set_instance_parameter_value lw_mm_bridge {PIPELINE_RESPONSE} {1}

add_instance sysid_qsys altera_avalon_sysid_qsys 
set_instance_parameter_value sysid_qsys {id} {-1395321854}

add_instance jtag_uart altera_avalon_jtag_uart 
set_instance_parameter_value jtag_uart {allowMultipleConnections} {1}
set_instance_parameter_value jtag_uart {hubInstanceID} {0}
set_instance_parameter_value jtag_uart {readBufferDepth} {64}
set_instance_parameter_value jtag_uart {readIRQThreshold} {8}
set_instance_parameter_value jtag_uart {simInputCharacterStream} {}
set_instance_parameter_value jtag_uart {simInteractiveOptions} {INTERACTIVE_ASCII_OUTPUT}
set_instance_parameter_value jtag_uart {useRegistersForReadBuffer} {0}
set_instance_parameter_value jtag_uart {useRegistersForWriteBuffer} {0}
set_instance_parameter_value jtag_uart {useRelativePathForSimFile} {0}
set_instance_parameter_value jtag_uart {writeBufferDepth} {64}
set_instance_parameter_value jtag_uart {writeIRQThreshold} {8}

add_instance onchip_memory2_0 altera_avalon_onchip_memory2 
set_instance_parameter_value onchip_memory2_0 {allowInSystemMemoryContentEditor} {0}
set_instance_parameter_value onchip_memory2_0 {blockType} {AUTO}
set_instance_parameter_value onchip_memory2_0 {dataWidth} {32}
set_instance_parameter_value onchip_memory2_0 {dualPort} {0}
set_instance_parameter_value onchip_memory2_0 {initMemContent} {1}
set_instance_parameter_value onchip_memory2_0 {initializationFileName} {onchip_mem.hex}
set_instance_parameter_value onchip_memory2_0 {instanceID} {NONE}
set_instance_parameter_value onchip_memory2_0 {memorySize} {65536.0}
set_instance_parameter_value onchip_memory2_0 {readDuringWriteMode} {DONT_CARE}
set_instance_parameter_value onchip_memory2_0 {simAllowMRAMContentsFile} {0}
set_instance_parameter_value onchip_memory2_0 {simMemInitOnlyFilename} {0}
set_instance_parameter_value onchip_memory2_0 {singleClockOperation} {0}
set_instance_parameter_value onchip_memory2_0 {slave1Latency} {1}
set_instance_parameter_value onchip_memory2_0 {slave2Latency} {1}
set_instance_parameter_value onchip_memory2_0 {useNonDefaultInitFile} {0}
set_instance_parameter_value onchip_memory2_0 {copyInitFile} {0}
set_instance_parameter_value onchip_memory2_0 {useShallowMemBlocks} {0}
set_instance_parameter_value onchip_memory2_0 {writable} {1}
set_instance_parameter_value onchip_memory2_0 {ecc_enabled} {0}
set_instance_parameter_value onchip_memory2_0 {resetrequest_enabled} {1}

add_instance hps_only_master altera_jtag_avalon_master 
set_instance_parameter_value hps_only_master {USE_PLI} {0}
set_instance_parameter_value hps_only_master {PLI_PORT} {50000}
set_instance_parameter_value hps_only_master {FAST_VER} {0}
set_instance_parameter_value hps_only_master {FIFO_DEPTHS} {2}

add_instance fpga_only_master altera_jtag_avalon_master 
set_instance_parameter_value fpga_only_master {USE_PLI} {0}
set_instance_parameter_value fpga_only_master {PLI_PORT} {50000}
set_instance_parameter_value fpga_only_master {FAST_VER} {0}
set_instance_parameter_value fpga_only_master {FIFO_DEPTHS} {2}

add_instance f2sdram_only_master altera_jtag_avalon_master 
set_instance_parameter_value f2sdram_only_master {USE_PLI} {0}
set_instance_parameter_value f2sdram_only_master {PLI_PORT} {50000}
set_instance_parameter_value f2sdram_only_master {FAST_VER} {0}
set_instance_parameter_value f2sdram_only_master {FIFO_DEPTHS} {2}

add_instance clk_0 clock_source 
set_instance_parameter_value clk_0 {clockFrequency} {50000000.0}
set_instance_parameter_value clk_0 {clockFrequencyKnown} {1}
set_instance_parameter_value clk_0 {resetSynchronousEdges} {NONE}

# connections and connection parameters
# LW Bridge
add_connection hps_0.h2f_lw_axi_master lw_mm_bridge.s0 avalon
set_connection_parameter_value hps_0.h2f_lw_axi_master/lw_mm_bridge.s0 arbitrationPriority {1}
set_connection_parameter_value hps_0.h2f_lw_axi_master/lw_mm_bridge.s0 baseAddress {0x0000}
set_connection_parameter_value hps_0.h2f_lw_axi_master/lw_mm_bridge.s0 defaultConnection {0}

add_connection fpga_only_master.master lw_mm_bridge.s0 avalon
set_connection_parameter_value fpga_only_master.master/lw_mm_bridge.s0 arbitrationPriority {1}
set_connection_parameter_value fpga_only_master.master/lw_mm_bridge.s0 baseAddress {0x0000}
set_connection_parameter_value fpga_only_master.master/lw_mm_bridge.s0 defaultConnection {0}

add_connection lw_mm_bridge.m0 sysid_qsys.control_slave avalon
set_connection_parameter_value lw_mm_bridge.m0/sysid_qsys.control_slave arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/sysid_qsys.control_slave baseAddress {0x00010000}
set_connection_parameter_value lw_mm_bridge.m0/sysid_qsys.control_slave defaultConnection {0}

add_connection lw_mm_bridge.m0 jtag_uart.avalon_jtag_slave avalon
set_connection_parameter_value lw_mm_bridge.m0/jtag_uart.avalon_jtag_slave arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/jtag_uart.avalon_jtag_slave baseAddress {0x00020000}
set_connection_parameter_value lw_mm_bridge.m0/jtag_uart.avalon_jtag_slave defaultConnection {0}

# AXI Bridge
add_connection hps_0.h2f_axi_master onchip_memory2_0.s1 avalon
set_connection_parameter_value hps_0.h2f_axi_master/onchip_memory2_0.s1 arbitrationPriority {1}
set_connection_parameter_value hps_0.h2f_axi_master/onchip_memory2_0.s1 baseAddress {0x0000}
set_connection_parameter_value hps_0.h2f_axi_master/onchip_memory2_0.s1 defaultConnection {0}

# Jtag Master
add_connection hps_only_master.master hps_0.f2h_axi_slave avalon
set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave arbitrationPriority {1}
set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave baseAddress {0x0000}
set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave defaultConnection {0}

add_connection f2sdram_only_master.master hps_0.f2h_sdram0_data avalon
set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data arbitrationPriority {1}
set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data baseAddress {0x0000}
set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data defaultConnection {0}

# IRQ
add_connection hps_0.f2h_irq0 jtag_uart.irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/jtag_uart.irq irqNumber {2}

# Clocks
add_connection hps_0.h2f_user1_clock clk_0.clk_in clock
add_connection clk_0.clk hps_0.f2h_sdram0_clock clock
add_connection clk_0.clk hps_0.h2f_axi_clock clock
add_connection clk_0.clk hps_0.f2h_axi_clock clock
add_connection clk_0.clk hps_0.h2f_lw_axi_clock clock
add_connection clk_0.clk hps_only_master.clk clock
add_connection clk_0.clk fpga_only_master.clk clock
add_connection clk_0.clk f2sdram_only_master.clk clock
add_connection clk_0.clk jtag_uart.clk clock
add_connection clk_0.clk sysid_qsys.clk clock
add_connection clk_0.clk onchip_memory2_0.clk1 clock
add_connection clk_0.clk lw_mm_bridge.clk clock

# Resets
add_connection hps_0.h2f_reset clk_0.clk_in_reset reset
add_connection clk_0.clk_reset fpga_only_master.clk_reset reset
add_connection clk_0.clk_reset hps_only_master.clk_reset reset
add_connection clk_0.clk_reset f2sdram_only_master.clk_reset reset
add_connection clk_0.clk_reset jtag_uart.reset reset
add_connection clk_0.clk_reset sysid_qsys.reset reset
add_connection clk_0.clk_reset onchip_memory2_0.reset1 reset
add_connection clk_0.clk_reset lw_mm_bridge.reset reset

# exported interfaces
add_interface memory conduit end
set_interface_property memory EXPORT_OF hps_0.memory
add_interface hps_0_hps_io conduit end
set_interface_property hps_0_hps_io EXPORT_OF hps_0.hps_io
add_interface hps_0_h2f_reset reset source
set_interface_property hps_0_h2f_reset EXPORT_OF clk_0.clk_reset
add_interface hps_0_h2f_clk clock source
set_interface_property hps_0_h2f_clk EXPORT_OF clk_0.clk

# interconnect requirements
set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {0}
set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
set_interconnect_requirement {hps_only_master.master} {qsys_mm.security} {SECURE}

