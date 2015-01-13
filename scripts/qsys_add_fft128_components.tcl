package require -exact qsys 14.1

#Add Components
add_instance fft128_bridge altera_avalon_mm_bridge 14.1
set_instance_parameter_value fft128_bridge {DATA_WIDTH} {32}
set_instance_parameter_value fft128_bridge {SYMBOL_WIDTH} {8}
set_instance_parameter_value fft128_bridge {ADDRESS_WIDTH} {19}
set_instance_parameter_value fft128_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
set_instance_parameter_value fft128_bridge {ADDRESS_UNITS} {SYMBOLS}
set_instance_parameter_value fft128_bridge {MAX_BURST_SIZE} {1}
set_instance_parameter_value fft128_bridge {MAX_PENDING_RESPONSES} {4}
set_instance_parameter_value fft128_bridge {LINEWRAPBURSTS} {0}
set_instance_parameter_value fft128_bridge {PIPELINE_COMMAND} {1}
set_instance_parameter_value fft128_bridge {PIPELINE_RESPONSE} {1}

add_instance sgdma_to_fft altera_msgdma 14.1
set_instance_parameter_value sgdma_to_fft {MODE} {1}
set_instance_parameter_value sgdma_to_fft {DATA_WIDTH} {32}
set_instance_parameter_value sgdma_to_fft {DATA_FIFO_DEPTH} {32}
set_instance_parameter_value sgdma_to_fft {DESCRIPTOR_FIFO_DEPTH} {128}
set_instance_parameter_value sgdma_to_fft {RESPONSE_PORT} {2}
set_instance_parameter_value sgdma_to_fft {MAX_BYTE} {1024}
set_instance_parameter_value sgdma_to_fft {TRANSFER_TYPE} {Aligned Accesses}
set_instance_parameter_value sgdma_to_fft {BURST_ENABLE} {0}
set_instance_parameter_value sgdma_to_fft {MAX_BURST_COUNT} {2}
set_instance_parameter_value sgdma_to_fft {BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value sgdma_to_fft {ENHANCED_FEATURES} {0}
set_instance_parameter_value sgdma_to_fft {STRIDE_ENABLE} {0}
set_instance_parameter_value sgdma_to_fft {MAX_STRIDE} {1}
set_instance_parameter_value sgdma_to_fft {PROGRAMMABLE_BURST_ENABLE} {0}
set_instance_parameter_value sgdma_to_fft {PACKET_ENABLE} {1}
set_instance_parameter_value sgdma_to_fft {ERROR_ENABLE} {1}
set_instance_parameter_value sgdma_to_fft {ERROR_WIDTH} {2}
set_instance_parameter_value sgdma_to_fft {CHANNEL_ENABLE} {0}
set_instance_parameter_value sgdma_to_fft {CHANNEL_WIDTH} {8}

add_instance fft128_0 fft128 1.0

add_instance sgdma_from_fft altera_msgdma 14.1
set_instance_parameter_value sgdma_from_fft {MODE} {2}
set_instance_parameter_value sgdma_from_fft {DATA_WIDTH} {64}
set_instance_parameter_value sgdma_from_fft {DATA_FIFO_DEPTH} {32}
set_instance_parameter_value sgdma_from_fft {DESCRIPTOR_FIFO_DEPTH} {128}
set_instance_parameter_value sgdma_from_fft {RESPONSE_PORT} {2}
set_instance_parameter_value sgdma_from_fft {MAX_BYTE} {1024}
set_instance_parameter_value sgdma_from_fft {TRANSFER_TYPE} {Aligned Accesses}
set_instance_parameter_value sgdma_from_fft {BURST_ENABLE} {0}
set_instance_parameter_value sgdma_from_fft {MAX_BURST_COUNT} {2}
set_instance_parameter_value sgdma_from_fft {BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value sgdma_from_fft {ENHANCED_FEATURES} {0}
set_instance_parameter_value sgdma_from_fft {STRIDE_ENABLE} {0}
set_instance_parameter_value sgdma_from_fft {MAX_STRIDE} {1}
set_instance_parameter_value sgdma_from_fft {PROGRAMMABLE_BURST_ENABLE} {0}
set_instance_parameter_value sgdma_from_fft {PACKET_ENABLE} {1}
set_instance_parameter_value sgdma_from_fft {ERROR_ENABLE} {0}
set_instance_parameter_value sgdma_from_fft {ERROR_WIDTH} {8}
set_instance_parameter_value sgdma_from_fft {CHANNEL_ENABLE} {0}
set_instance_parameter_value sgdma_from_fft {CHANNEL_WIDTH} {8}

add_instance data altera_avalon_onchip_memory2 14.1
set_instance_parameter_value data {allowInSystemMemoryContentEditor} {0}
set_instance_parameter_value data {blockType} {AUTO}
set_instance_parameter_value data {dataWidth} {32}
set_instance_parameter_value data {dualPort} {0}
set_instance_parameter_value data {initMemContent} {1}
set_instance_parameter_value data {initializationFileName} {onchip_mem.hex}
set_instance_parameter_value data {instanceID} {NONE}
set_instance_parameter_value data {memorySize} {4096.0}
set_instance_parameter_value data {readDuringWriteMode} {DONT_CARE}
set_instance_parameter_value data {simAllowMRAMContentsFile} {0}
set_instance_parameter_value data {simMemInitOnlyFilename} {0}
set_instance_parameter_value data {singleClockOperation} {0}
set_instance_parameter_value data {slave1Latency} {1}
set_instance_parameter_value data {slave2Latency} {1}
set_instance_parameter_value data {useNonDefaultInitFile} {0}
set_instance_parameter_value data {copyInitFile} {0}
set_instance_parameter_value data {useShallowMemBlocks} {0}
set_instance_parameter_value data {writable} {1}
set_instance_parameter_value data {ecc_enabled} {0}
set_instance_parameter_value data {resetrequest_enabled} {1}

add_instance fft_ddr_bridge altera_address_span_extender 14.1
set_instance_parameter_value fft_ddr_bridge {DATA_WIDTH} {64}
set_instance_parameter_value fft_ddr_bridge {MASTER_ADDRESS_WIDTH} {32}
set_instance_parameter_value fft_ddr_bridge {SLAVE_ADDRESS_WIDTH} {28}
set_instance_parameter_value fft_ddr_bridge {BURSTCOUNT_WIDTH} {3}
set_instance_parameter_value fft_ddr_bridge {SUB_WINDOW_COUNT} {1}
set_instance_parameter_value fft_ddr_bridge {MASTER_ADDRESS_DEF} {0}
set_instance_parameter_value fft_ddr_bridge {TERMINATE_SLAVE_PORT} {1}
set_instance_parameter_value fft_ddr_bridge {MAX_PENDING_READS} {8}


# Streaming connections
add_connection fft128_0.out0 sgdma_from_fft.st_sink avalon_streaming
add_connection sgdma_to_fft.st_source fft128_0.in0 avalon_streaming

# fft mm bridge connections
add_connection fft128_bridge.m0 sgdma_to_fft.csr avalon
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.csr arbitrationPriority {1}
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.csr baseAddress {0x1000}
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.csr defaultConnection {0}

add_connection fft128_bridge.m0 sgdma_to_fft.descriptor_slave avalon
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.descriptor_slave arbitrationPriority {1}
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.descriptor_slave baseAddress {0x1020}
set_connection_parameter_value fft128_bridge.m0/sgdma_to_fft.descriptor_slave defaultConnection {0}

add_connection fft128_bridge.m0 sgdma_from_fft.csr avalon
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.csr arbitrationPriority {1}
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.csr baseAddress {0x2000}
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.csr defaultConnection {0}

add_connection fft128_bridge.m0 sgdma_from_fft.descriptor_slave avalon
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.descriptor_slave arbitrationPriority {1}
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.descriptor_slave baseAddress {0x2020}
set_connection_parameter_value fft128_bridge.m0/sgdma_from_fft.descriptor_slave defaultConnection {0}

add_connection sgdma_from_fft.mm_write data.s1 avalon
set_connection_parameter_value sgdma_from_fft.mm_write/data.s1 arbitrationPriority {1}
set_connection_parameter_value sgdma_from_fft.mm_write/data.s1 baseAddress {0x00000000}
set_connection_parameter_value sgdma_from_fft.mm_write/data.s1 defaultConnection {0}

add_connection sgdma_to_fft.mm_read data.s1 avalon
set_connection_parameter_value sgdma_to_fft.mm_read/data.s1 arbitrationPriority {1}
set_connection_parameter_value sgdma_to_fft.mm_read/data.s1 baseAddress {0x00000000}
set_connection_parameter_value sgdma_to_fft.mm_read/data.s1 defaultConnection {0}

add_connection fft128_bridge.m0 data.s1 avalon
set_connection_parameter_value fft128_bridge.m0/data.s1 arbitrationPriority {1}
set_connection_parameter_value fft128_bridge.m0/data.s1 baseAddress {0x00000000}
set_connection_parameter_value fft128_bridge.m0/data.s1 defaultConnection {0}

add_connection sgdma_to_fft.mm_read fft_ddr_bridge.windowed_slave avalon
set_connection_parameter_value sgdma_to_fft.mm_read/fft_ddr_bridge.windowed_slave arbitrationPriority {1}
set_connection_parameter_value sgdma_to_fft.mm_read/fft_ddr_bridge.windowed_slave baseAddress {0x80000000}
set_connection_parameter_value sgdma_to_fft.mm_read/fft_ddr_bridge.windowed_slave defaultConnection {0}

add_connection sgdma_from_fft.mm_write fft_ddr_bridge.windowed_slave avalon
set_connection_parameter_value sgdma_from_fft.mm_write/fft_ddr_bridge.windowed_slave arbitrationPriority {1}
set_connection_parameter_value sgdma_from_fft.mm_write/fft_ddr_bridge.windowed_slave baseAddress {0x80000000}
set_connection_parameter_value sgdma_from_fft.mm_write/fft_ddr_bridge.windowed_slave defaultConnection {0}

# Connectivity to external host
add_connection lw_mm_bridge.m0 fft128_bridge.s0 avalon
set_connection_parameter_value lw_mm_bridge.m0/fft128_bridge.s0 arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/fft128_bridge.s0 baseAddress {0x00060000}
set_connection_parameter_value lw_mm_bridge.m0/fft128_bridge.s0 defaultConnection {0}

add_connection fft_ddr_bridge.expanded_master hps_0.f2h_sdram0_data avalon
set_connection_parameter_value fft_ddr_bridge.expanded_master/hps_0.f2h_sdram0_data arbitrationPriority {1}
set_connection_parameter_value fft_ddr_bridge.expanded_master/hps_0.f2h_sdram0_data baseAddress {0x0000}
set_connection_parameter_value fft_ddr_bridge.expanded_master/hps_0.f2h_sdram0_data defaultConnection {0}

# Interrupts
add_connection hps_0.f2h_irq0 sgdma_from_fft.csr_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/sgdma_from_fft.csr_irq irqNumber {5}

add_connection hps_0.f2h_irq0 sgdma_to_fft.csr_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/sgdma_to_fft.csr_irq irqNumber {6}

# Clocks
add_connection hps_0.h2f_user1_clock fft128_bridge.clk clock
add_connection hps_0.h2f_user1_clock fft_ddr_bridge.clock clock
add_connection hps_0.h2f_user1_clock sgdma_to_fft.clock clock
add_connection hps_0.h2f_user1_clock fft128_0.clock clock
add_connection hps_0.h2f_user1_clock sgdma_from_fft.clock clock
add_connection hps_0.h2f_user1_clock data.clk1 clock

# Clocks
add_connection hps_0.h2f_reset fft128_bridge.reset reset
add_connection hps_0.h2f_reset fft_ddr_bridge.reset reset
add_connection hps_0.h2f_reset sgdma_to_fft.reset_n reset
add_connection hps_0.h2f_reset fft128_0.reset reset
add_connection hps_0.h2f_reset sgdma_from_fft.reset_n reset
add_connection hps_0.h2f_reset data.reset1 reset
               
save_system
