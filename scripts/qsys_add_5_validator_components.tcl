package require -exact qsys 14.1

#Add Components
add_instance validator_subsys_0 validator_subsys 1.0

# HPS Connectivity
add_connection lw_mm_bridge.m0 validator_subsys_0.val_mm_bridge_s0 avalon
set_connection_parameter_value lw_mm_bridge.m0/validator_subsys_0.val_mm_bridge_s0 arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/validator_subsys_0.val_mm_bridge_s0 baseAddress {0x00010000}
set_connection_parameter_value lw_mm_bridge.m0/validator_subsys_0.val_mm_bridge_s0 defaultConnection {0}

# Clocks and Resets
add_connection clk_0.clk validator_subsys_0.clk clock
add_connection clk_0.clk_reset validator_subsys_0.reset reset

save_system
