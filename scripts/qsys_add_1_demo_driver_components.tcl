package require -exact qsys 14.1

# Add Components
add_instance demo_driver_subsys_0 demo_driver_subsys 1.0

# HPS Connectivity
add_connection lw_mm_bridge.m0 demo_driver_subsys_0.mm_bridge_s0 avalon
set_connection_parameter_value lw_mm_bridge.m0/demo_driver_subsys_0.mm_bridge_s0 arbitrationPriority {1}
set_connection_parameter_value lw_mm_bridge.m0/demo_driver_subsys_0.mm_bridge_s0 baseAddress {0x00030000}
set_connection_parameter_value lw_mm_bridge.m0/demo_driver_subsys_0.mm_bridge_s0 defaultConnection {0}

add_connection hps_0.f2h_irq0 demo_driver_subsys_0.demo_timer_irq interrupt
set_connection_parameter_value hps_0.f2h_irq0/demo_driver_subsys_0.demo_timer_irq irqNumber {8}

# Clocks and Resets
add_connection clk_0.clk demo_driver_subsys_0.clk clock
add_connection clk_0.clk_reset demo_driver_subsys_0.reset reset

save_system
