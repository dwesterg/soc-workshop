package require -exact qsys 14.1

#Connect HPS clock 1 to clk0 input
add_connection hps_clk_out.clk clk_0.clk_in
add_connection hps_clk_out.clk_reset clk_0.clk_in_reset

save_system