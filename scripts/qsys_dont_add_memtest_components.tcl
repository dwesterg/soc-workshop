package require -exact qsys 14.1

#Add Components
add_instance dma_read_master dma_read_master
set_instance_parameter_value dma_read_master {DATA_WIDTH} {128}
set_instance_parameter_value dma_read_master {LENGTH_WIDTH} {32}
set_instance_parameter_value dma_read_master {FIFO_DEPTH} {32}
set_instance_parameter_value dma_read_master {STRIDE_ENABLE} {0}
set_instance_parameter_value dma_read_master {GUI_STRIDE_WIDTH} {1}
set_instance_parameter_value dma_read_master {BURST_ENABLE} {0}
set_instance_parameter_value dma_read_master {GUI_MAX_BURST_COUNT} {2}
set_instance_parameter_value dma_read_master {GUI_PROGRAMMABLE_BURST_ENABLE} {0}
set_instance_parameter_value dma_read_master {GUI_BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value dma_read_master {TRANSFER_TYPE} {Unaligned Accesses}
set_instance_parameter_value dma_read_master {PACKET_ENABLE} {1}
set_instance_parameter_value dma_read_master {ERROR_ENABLE} {0}
set_instance_parameter_value dma_read_master {ERROR_WIDTH} {8}
set_instance_parameter_value dma_read_master {CHANNEL_ENABLE} {0}
set_instance_parameter_value dma_read_master {CHANNEL_WIDTH} {8}
set_instance_parameter_value dma_read_master {FIFO_SPEED_OPTIMIZATION} {1}

add_instance dma_write_master dma_write_master
set_instance_parameter_value dma_write_master {DATA_WIDTH} {128}
set_instance_parameter_value dma_write_master {LENGTH_WIDTH} {32}
set_instance_parameter_value dma_write_master {FIFO_DEPTH} {32}
set_instance_parameter_value dma_write_master {STRIDE_ENABLE} {0}
set_instance_parameter_value dma_write_master {GUI_STRIDE_WIDTH} {1}
set_instance_parameter_value dma_write_master {BURST_ENABLE} {0}
set_instance_parameter_value dma_write_master {GUI_MAX_BURST_COUNT} {2}
set_instance_parameter_value dma_write_master {GUI_PROGRAMMABLE_BURST_ENABLE} {0}
set_instance_parameter_value dma_write_master {GUI_BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value dma_write_master {TRANSFER_TYPE} {Unaligned Accesses}
set_instance_parameter_value dma_write_master {PACKET_ENABLE} {1}
set_instance_parameter_value dma_write_master {ERROR_ENABLE} {0}
set_instance_parameter_value dma_write_master {ERROR_WIDTH} {8}
set_instance_parameter_value dma_write_master {FIFO_SPEED_OPTIMIZATION} {1}

add_instance memtest memtest 1.0

add_instance memtest_sys_bridge altera_avalon_mm_bridge
set_instance_parameter_value memtest_sys_bridge {DATA_WIDTH} {32}
set_instance_parameter_value memtest_sys_bridge {SYMBOL_WIDTH} {8}
set_instance_parameter_value memtest_sys_bridge {ADDRESS_WIDTH} {10}
set_instance_parameter_value memtest_sys_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
set_instance_parameter_value memtest_sys_bridge {ADDRESS_UNITS} {SYMBOLS}
set_instance_parameter_value memtest_sys_bridge {MAX_BURST_SIZE} {1}
set_instance_parameter_value memtest_sys_bridge {MAX_PENDING_RESPONSES} {1}
set_instance_parameter_value memtest_sys_bridge {LINEWRAPBURSTS} {0}
set_instance_parameter_value memtest_sys_bridge {PIPELINE_COMMAND} {1}
set_instance_parameter_value memtest_sys_bridge {PIPELINE_RESPONSE} {1}

add_instance read_msgdma_disp modular_sgdma_dispatcher
set_instance_parameter_value read_msgdma_disp {MODE} {1}
set_instance_parameter_value read_msgdma_disp {GUI_RESPONSE_PORT} {0}
set_instance_parameter_value read_msgdma_disp {DESCRIPTOR_FIFO_DEPTH} {128}
set_instance_parameter_value read_msgdma_disp {ENHANCED_FEATURES} {0}
set_instance_parameter_value read_msgdma_disp {CSR_ADDRESS_WIDTH} {3}
set_instance_parameter_value read_msgdma_disp {DATA_WIDTH} {32}
set_instance_parameter_value read_msgdma_disp {DATA_FIFO_DEPTH} {32}
set_instance_parameter_value read_msgdma_disp {MAX_BYTE} {1024}
set_instance_parameter_value read_msgdma_disp {TRANSFER_TYPE} {Aligned Accesses}
set_instance_parameter_value read_msgdma_disp {BURST_ENABLE} {0}
set_instance_parameter_value read_msgdma_disp {MAX_BURST_COUNT} {2}
set_instance_parameter_value read_msgdma_disp {BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value read_msgdma_disp {STRIDE_ENABLE} {0}
set_instance_parameter_value read_msgdma_disp {MAX_STRIDE} {1}
set_instance_parameter_value read_msgdma_disp {PROGRAMMABLE_BURST_ENABLE} {0}

add_instance write_msgdma_disp modular_sgdma_dispatcher
set_instance_parameter_value write_msgdma_disp {MODE} {2}
set_instance_parameter_value write_msgdma_disp {GUI_RESPONSE_PORT} {2}
set_instance_parameter_value write_msgdma_disp {DESCRIPTOR_FIFO_DEPTH} {128}
set_instance_parameter_value write_msgdma_disp {ENHANCED_FEATURES} {0}
set_instance_parameter_value write_msgdma_disp {CSR_ADDRESS_WIDTH} {3}
set_instance_parameter_value write_msgdma_disp {DATA_WIDTH} {32}
set_instance_parameter_value write_msgdma_disp {DATA_FIFO_DEPTH} {32}
set_instance_parameter_value write_msgdma_disp {MAX_BYTE} {1024}
set_instance_parameter_value write_msgdma_disp {TRANSFER_TYPE} {Aligned Accesses}
set_instance_parameter_value write_msgdma_disp {BURST_ENABLE} {0}
set_instance_parameter_value write_msgdma_disp {MAX_BURST_COUNT} {2}
set_instance_parameter_value write_msgdma_disp {BURST_WRAPPING_SUPPORT} {0}
set_instance_parameter_value write_msgdma_disp {STRIDE_ENABLE} {0}
set_instance_parameter_value write_msgdma_disp {MAX_STRIDE} {1}
set_instance_parameter_value write_msgdma_disp {PROGRAMMABLE_BURST_ENABLE} {0}

# mm bridge connections
add_connection memtest_sys_bridge.m0 write_msgdma_disp.CSR avalon
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.CSR arbitrationPriority {1}
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.CSR baseAddress {0x0020}
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.CSR defaultConnection {0}

add_connection memtest_sys_bridge.m0 read_msgdma_disp.CSR avalon
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.CSR arbitrationPriority {1}
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.CSR baseAddress {0x0000}
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.CSR defaultConnection {0}

add_connection memtest_sys_bridge.m0 write_msgdma_disp.Descriptor_Slave avalon
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.Descriptor_Slave arbitrationPriority {1}
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.Descriptor_Slave baseAddress {0x0050}
set_connection_parameter_value memtest_sys_bridge.m0/write_msgdma_disp.Descriptor_Slave defaultConnection {0}

add_connection memtest_sys_bridge.m0 read_msgdma_disp.Descriptor_Slave avalon
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.Descriptor_Slave arbitrationPriority {1}
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.Descriptor_Slave baseAddress {0x0040}
set_connection_parameter_value memtest_sys_bridge.m0/read_msgdma_disp.Descriptor_Slave defaultConnection {0}

add_connection memtest_sys_bridge.m0 memtest.control avalon
set_connection_parameter_value memtest_sys_bridge.m0/memtest.control arbitrationPriority {1}
set_connection_parameter_value memtest_sys_bridge.m0/memtest.control baseAddress {0x0080}
set_connection_parameter_value memtest_sys_bridge.m0/memtest.control defaultConnection {0}

# HPS Connections
add_connection lw_mm_bridge.m0 memtest_sys_bridge.s0 avalon
set_connection_parameter_value lw_mm_bridge.m0/memtest_sys_bridge.s0 arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/memtest_sys_bridge.s0 baseAddress {0x00050000}
set_connection_parameter_value lw_mm_bridge.m0/memtest_sys_bridge.s0 defaultConnection {0}

# Interrupts
add_connection hps_0.f2h_irq0 read_msgdma_disp.csr_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/read_msgdma_disp.csr_irq irqNumber {3}

add_connection hps_0.f2h_irq0 write_msgdma_disp.csr_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/write_msgdma_disp.csr_irq irqNumber {4}

add_connection dma_read_master.Data_Read_Master onchip_memory2_0.s1 avalon
set_connection_parameter_value dma_read_master.Data_Read_Master/onchip_memory2_0.s1 arbitrationPriority {1}
set_connection_parameter_value dma_read_master.Data_Read_Master/onchip_memory2_0.s1 baseAddress {0x00000000}
set_connection_parameter_value dma_read_master.Data_Read_Master/onchip_memory2_0.s1 defaultConnection {0}

add_connection dma_write_master.Data_Write_Master onchip_memory2_0.s1 avalon
set_connection_parameter_value dma_write_master.Data_Write_Master/onchip_memory2_0.s1 arbitrationPriority {1}
set_connection_parameter_value dma_write_master.Data_Write_Master/onchip_memory2_0.s1 baseAddress {0x00000000}
set_connection_parameter_value dma_write_master.Data_Write_Master/onchip_memory2_0.s1 defaultConnection {0}

# Streaming
add_connection dma_read_master.Data_Source memtest.indata avalon_streaming
add_connection read_msgdma_disp.Read_Command_Source dma_read_master.Command_Sink avalon_streaming
add_connection dma_read_master.Response_Source read_msgdma_disp.Read_Response_Sink avalon_streaming
add_connection dma_write_master.Response_Source write_msgdma_disp.Write_Response_Sink avalon_streaming
add_connection write_msgdma_disp.Write_Command_Source dma_write_master.Command_Sink avalon_streaming
add_connection memtest.gendata dma_write_master.Data_Sink avalon_streaming

# Clocks
add_connection hps_0.h2f_user1_clock dma_write_master.Clock clock
add_connection hps_0.h2f_user1_clock dma_read_master.Clock clock
add_connection hps_0.h2f_user1_clock memtest.clk clock
add_connection hps_0.h2f_user1_clock memtest_sys_bridge.clk clock
add_connection hps_0.h2f_user1_clock read_msgdma_disp.clock clock
add_connection hps_0.h2f_user1_clock write_msgdma_disp.clock clock

# Resets
add_connection hps_0.h2f_reset dma_read_master.Clock_reset reset
add_connection hps_0.h2f_reset dma_write_master.Clock_reset reset
add_connection hps_0.h2f_reset write_msgdma_disp.clock_reset reset
add_connection hps_0.h2f_reset read_msgdma_disp.clock_reset reset
add_connection hps_0.h2f_reset memtest_sys_bridge.reset reset
add_connection hps_0.h2f_reset memtest.reset reset    
    
save_system