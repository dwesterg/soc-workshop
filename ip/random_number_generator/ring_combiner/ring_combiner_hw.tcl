# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module ring_combiner
# 
set_module_property DESCRIPTION "XOR three input clocks to create an entropy source output."
set_module_property NAME ring_combiner
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME ring_combiner
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false
set_module_property GROUP "Random Number Generator"


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL ring_combiner
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_combiner.sv SYSTEM_VERILOG PATH ring_combiner.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL ring_combiner
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_combiner.sv SYSTEM_VERILOG PATH ring_combiner.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL ring_combiner
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_combiner.sv SYSTEM_VERILOG PATH ring_combiner.sv


# 
# parameters
# 
add_parameter SYNC_DEPTH INTEGER 5
set_parameter_property SYNC_DEPTH DEFAULT_VALUE 5
set_parameter_property SYNC_DEPTH DISPLAY_NAME SYNC_DEPTH
set_parameter_property SYNC_DEPTH DESCRIPTION "Specify the number of clock domain sync registers to place on each clock path."
set_parameter_property SYNC_DEPTH TYPE INTEGER
set_parameter_property SYNC_DEPTH UNITS None
set_parameter_property SYNC_DEPTH ALLOWED_RANGES 3:50
set_parameter_property SYNC_DEPTH HDL_PARAMETER true


# 
# display items
# 


# 
# connection point clock
# 
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clk clk Input 1


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1


# 
# connection point clock_sink_0
# 
add_interface clock_sink_0 clock end
set_interface_property clock_sink_0 clockRate 0
set_interface_property clock_sink_0 ENABLED true
set_interface_property clock_sink_0 EXPORT_OF ""
set_interface_property clock_sink_0 PORT_NAME_MAP ""
set_interface_property clock_sink_0 CMSIS_SVD_VARIABLES ""
set_interface_property clock_sink_0 SVD_ADDRESS_GROUP ""

add_interface_port clock_sink_0 ring_osc_0 clk Input 1


# 
# connection point clock_sink_1
# 
add_interface clock_sink_1 clock end
set_interface_property clock_sink_1 clockRate 0
set_interface_property clock_sink_1 ENABLED true
set_interface_property clock_sink_1 EXPORT_OF ""
set_interface_property clock_sink_1 PORT_NAME_MAP ""
set_interface_property clock_sink_1 CMSIS_SVD_VARIABLES ""
set_interface_property clock_sink_1 SVD_ADDRESS_GROUP ""

add_interface_port clock_sink_1 ring_osc_1 clk Input 1


# 
# connection point clock_sink_2
# 
add_interface clock_sink_2 clock end
set_interface_property clock_sink_2 clockRate 0
set_interface_property clock_sink_2 ENABLED true
set_interface_property clock_sink_2 EXPORT_OF ""
set_interface_property clock_sink_2 PORT_NAME_MAP ""
set_interface_property clock_sink_2 CMSIS_SVD_VARIABLES ""
set_interface_property clock_sink_2 SVD_ADDRESS_GROUP ""

add_interface_port clock_sink_2 ring_osc_2 clk Input 1


# 
# connection point source
# 
add_interface source avalon_streaming start
set_interface_property source associatedClock clock
set_interface_property source associatedReset reset
set_interface_property source dataBitsPerSymbol 1
set_interface_property source errorDescriptor ""
set_interface_property source firstSymbolInHighOrderBits true
set_interface_property source maxChannel 0
set_interface_property source readyLatency 0
set_interface_property source ENABLED true
set_interface_property source EXPORT_OF ""
set_interface_property source PORT_NAME_MAP ""
set_interface_property source CMSIS_SVD_VARIABLES ""
set_interface_property source SVD_ADDRESS_GROUP ""

add_interface_port source xor_out data Output 1

