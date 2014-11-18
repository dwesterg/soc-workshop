# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module rng_combiner
# 
set_module_property DESCRIPTION "XOR two Avalon ST symbols together and forward the result."
set_module_property NAME rng_combiner
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME rng_combiner
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL rng_combiner
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file rng_combiner.sv SYSTEM_VERILOG PATH rng_combiner.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL rng_combiner
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file rng_combiner.sv SYSTEM_VERILOG PATH rng_combiner.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL rng_combiner
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file rng_combiner.sv SYSTEM_VERILOG PATH rng_combiner.sv


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
# connection point in0
# 
add_interface in0 avalon_streaming_video end
set_interface_property in0 alphaEnabled false
set_interface_property in0 associatedClock clock
set_interface_property in0 associatedReset reset
set_interface_property in0 chromaFormat 4:4:4
set_interface_property in0 colorSpace RGB
set_interface_property in0 dataBitsPerSymbol 1
set_interface_property in0 errorDescriptor ""
set_interface_property in0 firstSymbolInHighOrderBits true
set_interface_property in0 is422 false
set_interface_property in0 maxChannel 0
set_interface_property in0 pixelsInParallel 1
set_interface_property in0 readyLatency 0
set_interface_property in0 symbolsInParallel 1
set_interface_property in0 symbolsInSequence 1
set_interface_property in0 ENABLED true
set_interface_property in0 EXPORT_OF ""
set_interface_property in0 PORT_NAME_MAP ""
set_interface_property in0 CMSIS_SVD_VARIABLES ""
set_interface_property in0 SVD_ADDRESS_GROUP ""

add_interface_port in0 asi_in0_data data Input 1
add_interface_port in0 asi_in0_valid valid Input 1
add_interface_port in0 asi_in0_ready ready Output 1


# 
# connection point in1
# 
add_interface in1 avalon_streaming_video end
set_interface_property in1 alphaEnabled false
set_interface_property in1 associatedClock clock
set_interface_property in1 associatedReset reset
set_interface_property in1 chromaFormat 4:4:4
set_interface_property in1 colorSpace RGB
set_interface_property in1 dataBitsPerSymbol 1
set_interface_property in1 errorDescriptor ""
set_interface_property in1 firstSymbolInHighOrderBits true
set_interface_property in1 is422 false
set_interface_property in1 maxChannel 0
set_interface_property in1 pixelsInParallel 1
set_interface_property in1 readyLatency 0
set_interface_property in1 symbolsInParallel 1
set_interface_property in1 symbolsInSequence 1
set_interface_property in1 ENABLED true
set_interface_property in1 EXPORT_OF ""
set_interface_property in1 PORT_NAME_MAP ""
set_interface_property in1 CMSIS_SVD_VARIABLES ""
set_interface_property in1 SVD_ADDRESS_GROUP ""

add_interface_port in1 asi_in1_data data Input 1
add_interface_port in1 asi_in1_valid valid Input 1
add_interface_port in1 asi_in1_ready ready Output 1


# 
# connection point out0
# 
add_interface out0 avalon_streaming_video start
set_interface_property out0 alphaEnabled false
set_interface_property out0 associatedClock clock
set_interface_property out0 associatedReset reset
set_interface_property out0 chromaFormat 4:4:4
set_interface_property out0 colorSpace RGB
set_interface_property out0 dataBitsPerSymbol 1
set_interface_property out0 errorDescriptor ""
set_interface_property out0 firstSymbolInHighOrderBits true
set_interface_property out0 is422 false
set_interface_property out0 maxChannel 0
set_interface_property out0 pixelsInParallel 1
set_interface_property out0 readyLatency 0
set_interface_property out0 symbolsInParallel 1
set_interface_property out0 symbolsInSequence 1
set_interface_property out0 ENABLED true
set_interface_property out0 EXPORT_OF ""
set_interface_property out0 PORT_NAME_MAP ""
set_interface_property out0 CMSIS_SVD_VARIABLES ""
set_interface_property out0 SVD_ADDRESS_GROUP ""

add_interface_port out0 aso_out0_data data Output 1
add_interface_port out0 aso_out0_valid valid Output 1

