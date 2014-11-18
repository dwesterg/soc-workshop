# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module two_bit_collector
# 
set_module_property DESCRIPTION "Collect two 1-bit symbols over the input ST interface and pass them out the 2-bit output ST interface.  There's a bug in the current 14.0 Avalon ST data format adapter for this corner case."
set_module_property NAME two_bit_collector
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME two_bit_collector
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL two_bit_collector
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file two_bit_collector.sv SYSTEM_VERILOG PATH two_bit_collector.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL two_bit_collector
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file two_bit_collector.sv SYSTEM_VERILOG PATH two_bit_collector.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL two_bit_collector
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file two_bit_collector.sv SYSTEM_VERILOG PATH two_bit_collector.sv


# 
# parameters
# 


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
# connection point sink
# 
add_interface sink avalon_streaming end
set_interface_property sink associatedClock clock
set_interface_property sink associatedReset reset
set_interface_property sink dataBitsPerSymbol 1
set_interface_property sink errorDescriptor ""
set_interface_property sink firstSymbolInHighOrderBits true
set_interface_property sink maxChannel 0
set_interface_property sink readyLatency 0
set_interface_property sink ENABLED true
set_interface_property sink EXPORT_OF ""
set_interface_property sink PORT_NAME_MAP ""
set_interface_property sink CMSIS_SVD_VARIABLES ""
set_interface_property sink SVD_ADDRESS_GROUP ""

add_interface_port sink snk_data data Input 1
add_interface_port sink snk_valid valid Input 1


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

add_interface_port source src_data data Output 2
add_interface_port source src_valid valid Output 1

