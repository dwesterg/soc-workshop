package require -exact qsys 14.1

#Add Components
add_instance sld_hub_controller_system_0 altera_sld_hub_controller_system

# HPS Connectivity
add_connection lw_mm_bridge.m0 sld_hub_controller_system_0.s0 avalon
set_connection_parameter_value lw_mm_bridge.m0/sld_hub_controller_system_0.s0 arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/sld_hub_controller_system_0.s0 baseAddress {0x00030000}
set_connection_parameter_value lw_mm_bridge.m0/sld_hub_controller_system_0.s0 defaultConnection {0}

# Clocks and Resets
add_connection hps_0.h2f_user1_clock sld_hub_controller_system_0.clk clock
add_connection hps_0.h2f_reset sld_hub_controller_system_0.reset reset

save_system
