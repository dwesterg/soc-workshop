package require -exact qsys 14.1

#Add Components
add_instance memcpy_msgdma altera_msgdma 14.1
set_instance_parameter_value memcpy_msgdma {MODE} {0}
set_instance_parameter_value memcpy_msgdma {DATA_WIDTH} {64}
set_instance_parameter_value memcpy_msgdma {DATA_FIFO_DEPTH} {32}
set_instance_parameter_value memcpy_msgdma {DESCRIPTOR_FIFO_DEPTH} {128}
set_instance_parameter_value memcpy_msgdma {RESPONSE_PORT} {2}
set_instance_parameter_value memcpy_msgdma {MAX_BYTE} {1024}
set_instance_parameter_value memcpy_msgdma {TRANSFER_TYPE} {Unaligned Accesses}
set_instance_parameter_value memcpy_msgdma {BURST_ENABLE} {1}
set_instance_parameter_value memcpy_msgdma {MAX_BURST_COUNT} {8}
set_instance_parameter_value memcpy_msgdma {BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value memcpy_msgdma {ENHANCED_FEATURES} {0}
set_instance_parameter_value memcpy_msgdma {STRIDE_ENABLE} {0}
set_instance_parameter_value memcpy_msgdma {MAX_STRIDE} {1}
set_instance_parameter_value memcpy_msgdma {PROGRAMMABLE_BURST_ENABLE} {0}
set_instance_parameter_value memcpy_msgdma {PACKET_ENABLE} {0}
set_instance_parameter_value memcpy_msgdma {ERROR_ENABLE} {0}
set_instance_parameter_value memcpy_msgdma {ERROR_WIDTH} {8}
set_instance_parameter_value memcpy_msgdma {CHANNEL_ENABLE} {0}
set_instance_parameter_value memcpy_msgdma {CHANNEL_WIDTH} {8}


add_connection lw_mm_bridge.m0 memcpy_msgdma.csr avalon
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.csr arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.csr baseAddress {0x00020000}
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.csr defaultConnection {0}

add_connection lw_mm_bridge.m0 memcpy_msgdma.descriptor_slave avalon
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.descriptor_slave arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.descriptor_slave baseAddress {0x00020020}
set_connection_parameter_value lw_mm_bridge.m0/memcpy_msgdma.descriptor_slave defaultConnection {0}

add_connection hps_0.f2h_irq0 memcpy_msgdma.csr_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/memcpy_msgdma.csr_irq irqNumber {7}

add_connection memcpy_msgdma.mm_read axi_bridge_for_acp_128_0.s0 avalon
set_connection_parameter_value memcpy_msgdma.mm_read/axi_bridge_for_acp_128_0.s0 arbitrationPriority {1}
set_connection_parameter_value memcpy_msgdma.mm_read/axi_bridge_for_acp_128_0.s0 baseAddress {0x0000}
set_connection_parameter_value memcpy_msgdma.mm_read/axi_bridge_for_acp_128_0.s0 defaultConnection {0}

add_connection memcpy_msgdma.mm_write axi_bridge_for_acp_128_0.s0 avalon
set_connection_parameter_value memcpy_msgdma.mm_write/axi_bridge_for_acp_128_0.s0 arbitrationPriority {1}
set_connection_parameter_value memcpy_msgdma.mm_write/axi_bridge_for_acp_128_0.s0 baseAddress {0x0000}
set_connection_parameter_value memcpy_msgdma.mm_write/axi_bridge_for_acp_128_0.s0 defaultConnection {0}

# Clocks and Resets
add_connection clk_0.clk memcpy_msgdma.clock clock
add_connection clk_0.clk_reset memcpy_msgdma.reset_n reset
                                  
save_system
