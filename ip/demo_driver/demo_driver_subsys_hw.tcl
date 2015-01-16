package require -exact qsys 14.0

# module properties
set_module_property NAME demo_driver_subsys
set_module_property DISPLAY_NAME demo_driver_subsys

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {Demo Driver}
set_module_property DESCRIPTION {Demo driver top level subsystem.}
set_module_property AUTHOR {RSF}

set_module_property COMPOSITION_CALLBACK compose
set_module_property opaque_address_map true

# Device tree parameters
set_module_assignment embeddedsw.dts.vendor "demo"
set_module_assignment embeddedsw.dts.group "driver"
set_module_assignment embeddedsw.dts.name "driver"
set_module_assignment embeddedsw.dts.compatible "demo,driver-1.0"

proc compose { } {
    # Instances and instance parameters
    # (disabled instances are intentionally culled)
    add_instance clk clock_source 14.1
    set_instance_parameter_value clk {clockFrequency} {1.0}
    set_instance_parameter_value clk {clockFrequencyKnown} {0}
    set_instance_parameter_value clk {resetSynchronousEdges} {BOTH}

    add_instance demo_ram altera_avalon_onchip_memory2 14.1
    set_instance_parameter_value demo_ram {allowInSystemMemoryContentEditor} {0}
    set_instance_parameter_value demo_ram {blockType} {AUTO}
    set_instance_parameter_value demo_ram {dataWidth} {32}
    set_instance_parameter_value demo_ram {dualPort} {0}
    set_instance_parameter_value demo_ram {initMemContent} {1}
    set_instance_parameter_value demo_ram {initializationFileName} {demo_ram_random.hex}
    set_instance_parameter_value demo_ram {instanceID} {NONE}
    set_instance_parameter_value demo_ram {memorySize} {1024.0}
    set_instance_parameter_value demo_ram {readDuringWriteMode} {DONT_CARE}
    set_instance_parameter_value demo_ram {simAllowMRAMContentsFile} {0}
    set_instance_parameter_value demo_ram {simMemInitOnlyFilename} {0}
    set_instance_parameter_value demo_ram {singleClockOperation} {0}
    set_instance_parameter_value demo_ram {slave1Latency} {1}
    set_instance_parameter_value demo_ram {slave2Latency} {1}
    set_instance_parameter_value demo_ram {useNonDefaultInitFile} {1}
    set_instance_parameter_value demo_ram {copyInitFile} {0}
    set_instance_parameter_value demo_ram {useShallowMemBlocks} {0}
    set_instance_parameter_value demo_ram {writable} {1}
    set_instance_parameter_value demo_ram {ecc_enabled} {0}
    set_instance_parameter_value demo_ram {resetrequest_enabled} {1}

    add_instance demo_rom altera_avalon_onchip_memory2 14.1
    set_instance_parameter_value demo_rom {allowInSystemMemoryContentEditor} {0}
    set_instance_parameter_value demo_rom {blockType} {AUTO}
    set_instance_parameter_value demo_rom {dataWidth} {32}
    set_instance_parameter_value demo_rom {dualPort} {0}
    set_instance_parameter_value demo_rom {initMemContent} {1}
    set_instance_parameter_value demo_rom {initializationFileName} {demo_rom_random.hex}
    set_instance_parameter_value demo_rom {instanceID} {NONE}
    set_instance_parameter_value demo_rom {memorySize} {1024.0}
    set_instance_parameter_value demo_rom {readDuringWriteMode} {DONT_CARE}
    set_instance_parameter_value demo_rom {simAllowMRAMContentsFile} {0}
    set_instance_parameter_value demo_rom {simMemInitOnlyFilename} {0}
    set_instance_parameter_value demo_rom {singleClockOperation} {0}
    set_instance_parameter_value demo_rom {slave1Latency} {1}
    set_instance_parameter_value demo_rom {slave2Latency} {1}
    set_instance_parameter_value demo_rom {useNonDefaultInitFile} {1}
    set_instance_parameter_value demo_rom {copyInitFile} {0}
    set_instance_parameter_value demo_rom {useShallowMemBlocks} {0}
    set_instance_parameter_value demo_rom {writable} {0}
    set_instance_parameter_value demo_rom {ecc_enabled} {0}
    set_instance_parameter_value demo_rom {resetrequest_enabled} {1}

    add_instance demo_timer altera_avalon_timer 14.1
    set_instance_parameter_value demo_timer {alwaysRun} {0}
    set_instance_parameter_value demo_timer {counterSize} {32}
    set_instance_parameter_value demo_timer {fixedPeriod} {0}
    set_instance_parameter_value demo_timer {period} {10}
    set_instance_parameter_value demo_timer {periodUnits} {MSEC}
    set_instance_parameter_value demo_timer {resetOutput} {0}
    set_instance_parameter_value demo_timer {snapshot} {1}
    set_instance_parameter_value demo_timer {timeoutPulseOutput} {0}

    add_instance mm_bridge altera_avalon_mm_bridge 14.1
    set_instance_parameter_value mm_bridge {DATA_WIDTH} {32}
    set_instance_parameter_value mm_bridge {SYMBOL_WIDTH} {8}
    set_instance_parameter_value mm_bridge {ADDRESS_WIDTH} {10}
    set_instance_parameter_value mm_bridge {USE_AUTO_ADDRESS_WIDTH} {1}
    set_instance_parameter_value mm_bridge {ADDRESS_UNITS} {SYMBOLS}
    set_instance_parameter_value mm_bridge {MAX_BURST_SIZE} {1}
    set_instance_parameter_value mm_bridge {MAX_PENDING_RESPONSES} {4}
    set_instance_parameter_value mm_bridge {LINEWRAPBURSTS} {0}
    set_instance_parameter_value mm_bridge {PIPELINE_COMMAND} {0}
    set_instance_parameter_value mm_bridge {PIPELINE_RESPONSE} {0}

    # connections and connection parameters
    add_connection mm_bridge.m0 demo_rom.s1 avalon
    set_connection_parameter_value mm_bridge.m0/demo_rom.s1 arbitrationPriority {1}
    set_connection_parameter_value mm_bridge.m0/demo_rom.s1 baseAddress {0x0000}
    set_connection_parameter_value mm_bridge.m0/demo_rom.s1 defaultConnection {0}

    add_connection mm_bridge.m0 demo_ram.s1 avalon
    set_connection_parameter_value mm_bridge.m0/demo_ram.s1 arbitrationPriority {1}
    set_connection_parameter_value mm_bridge.m0/demo_ram.s1 baseAddress {0x0400}
    set_connection_parameter_value mm_bridge.m0/demo_ram.s1 defaultConnection {0}

    add_connection mm_bridge.m0 demo_timer.s1 avalon
    set_connection_parameter_value mm_bridge.m0/demo_timer.s1 arbitrationPriority {1}
    set_connection_parameter_value mm_bridge.m0/demo_timer.s1 baseAddress {0x0800}
    set_connection_parameter_value mm_bridge.m0/demo_timer.s1 defaultConnection {0}

    add_connection clk.clk demo_timer.clk clock

    add_connection clk.clk mm_bridge.clk clock

    add_connection clk.clk demo_ram.clk1 clock

    add_connection clk.clk demo_rom.clk1 clock

    add_connection clk.clk_reset demo_timer.reset reset

    add_connection clk.clk_reset mm_bridge.reset reset

    add_connection clk.clk_reset demo_rom.reset1 reset

    add_connection clk.clk_reset demo_ram.reset1 reset

    # exported interfaces
    add_interface clk clock sink
    set_interface_property clk EXPORT_OF clk.clk_in
    add_interface demo_timer_irq interrupt sender
    set_interface_property demo_timer_irq EXPORT_OF demo_timer.irq
    add_interface mm_bridge_s0 avalon slave
    set_interface_property mm_bridge_s0 EXPORT_OF mm_bridge.s0
    add_interface reset reset sink
    set_interface_property reset EXPORT_OF clk.clk_in_reset

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {1}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
}
