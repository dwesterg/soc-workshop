# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module ring_oscillator
# 
set_module_property DESCRIPTION "Creates an inverter ring oscillator."
set_module_property NAME ring_oscillator
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME ring_oscillator
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL ring_oscillator
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_oscillator.sv SYSTEM_VERILOG PATH ring_oscillator.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL ring_oscillator
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_oscillator.sv SYSTEM_VERILOG PATH ring_oscillator.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL ring_oscillator
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file ring_oscillator.sv SYSTEM_VERILOG PATH ring_oscillator.sv


# 
# parameters
# 
add_parameter BIT_WIDTH INTEGER 100 ""
set_parameter_property BIT_WIDTH DEFAULT_VALUE 100
set_parameter_property BIT_WIDTH DISPLAY_NAME BIT_WIDTH
set_parameter_property BIT_WIDTH WIDTH ""
set_parameter_property BIT_WIDTH TYPE INTEGER
set_parameter_property BIT_WIDTH UNITS None
set_parameter_property BIT_WIDTH ALLOWED_RANGES 5:500
set_parameter_property BIT_WIDTH DESCRIPTION "Sets the width of the inverter ring, wider should equal slower oscillation."
set_parameter_property BIT_WIDTH HDL_PARAMETER true


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
# connection point osc_enable
# 
add_interface osc_enable conduit end
set_interface_property osc_enable associatedClock ""
set_interface_property osc_enable associatedReset ""
set_interface_property osc_enable ENABLED true
set_interface_property osc_enable EXPORT_OF ""
set_interface_property osc_enable PORT_NAME_MAP ""
set_interface_property osc_enable CMSIS_SVD_VARIABLES ""
set_interface_property osc_enable SVD_ADDRESS_GROUP ""

add_interface_port osc_enable enable_osc export Input 1


# 
# connection point clock_source
# 
add_interface clock_source clock start
set_interface_property clock_source associatedDirectClock ""
set_interface_property clock_source clockRate 0
set_interface_property clock_source clockRateKnown false
set_interface_property clock_source ENABLED true
set_interface_property clock_source EXPORT_OF ""
set_interface_property clock_source PORT_NAME_MAP ""
set_interface_property clock_source CMSIS_SVD_VARIABLES ""
set_interface_property clock_source SVD_ADDRESS_GROUP ""

add_interface_port clock_source clk_out clk Output 1

