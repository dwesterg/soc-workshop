package require -exact qsys 14.1

#Connect HPS clock 1 to clk0 input
add_connection hps_clk_out.clk clk_0.clk_in
add_connection hps_clk_out.clk_reset clk_0.clk_in_reset

add_connection clk_0.clk axi_bridge_for_acp_128_0.clock clock
add_connection clk_0.clk fft_ddr_bridge.clock clock
add_connection clk_0.clk fft_sub.clk clock

save_system
