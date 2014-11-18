# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module count_entropy_master
# 
set_module_property DESCRIPTION "Entropy counters are accumulated in 1KB onchip RAM external to this component."
set_module_property NAME count_entropy_master
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME count_entropy_master
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL count_entropy_master
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy_master.sv SYSTEM_VERILOG PATH count_entropy_master.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL count_entropy_master
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy_master.sv SYSTEM_VERILOG PATH count_entropy_master.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL count_entropy_master
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy_master.sv SYSTEM_VERILOG PATH count_entropy_master.sv


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

add_interface_port sink snk_data data Input 8
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

add_interface_port source src_data data Output 8
add_interface_port source src_valid valid Output 1


# 
# connection point enable_conduit
# 
add_interface enable_conduit conduit end
set_interface_property enable_conduit associatedClock ""
set_interface_property enable_conduit associatedReset ""
set_interface_property enable_conduit ENABLED true
set_interface_property enable_conduit EXPORT_OF ""
set_interface_property enable_conduit PORT_NAME_MAP ""
set_interface_property enable_conduit CMSIS_SVD_VARIABLES ""
set_interface_property enable_conduit SVD_ADDRESS_GROUP ""

add_interface_port enable_conduit enable export Input 1


# 
# connection point clear_conduit
# 
add_interface clear_conduit conduit end
set_interface_property clear_conduit associatedClock ""
set_interface_property clear_conduit associatedReset ""
set_interface_property clear_conduit ENABLED true
set_interface_property clear_conduit EXPORT_OF ""
set_interface_property clear_conduit PORT_NAME_MAP ""
set_interface_property clear_conduit CMSIS_SVD_VARIABLES ""
set_interface_property clear_conduit SVD_ADDRESS_GROUP ""

add_interface_port clear_conduit clear export Input 1


# 
# connection point tranaction_count
# 
add_interface tranaction_count avalon end
set_interface_property tranaction_count addressUnits WORDS
set_interface_property tranaction_count associatedClock clock
set_interface_property tranaction_count associatedReset reset
set_interface_property tranaction_count bitsPerSymbol 8
set_interface_property tranaction_count burstOnBurstBoundariesOnly false
set_interface_property tranaction_count burstcountUnits WORDS
set_interface_property tranaction_count explicitAddressSpan 0
set_interface_property tranaction_count holdTime 0
set_interface_property tranaction_count linewrapBursts false
set_interface_property tranaction_count maximumPendingReadTransactions 0
set_interface_property tranaction_count maximumPendingWriteTransactions 0
set_interface_property tranaction_count readLatency 0
set_interface_property tranaction_count readWaitTime 1
set_interface_property tranaction_count setupTime 0
set_interface_property tranaction_count timingUnits Cycles
set_interface_property tranaction_count writeWaitTime 0
set_interface_property tranaction_count ENABLED true
set_interface_property tranaction_count EXPORT_OF ""
set_interface_property tranaction_count PORT_NAME_MAP ""
set_interface_property tranaction_count CMSIS_SVD_VARIABLES ""
set_interface_property tranaction_count SVD_ADDRESS_GROUP ""

add_interface_port tranaction_count transaction_readdata readdata Output 32
set_interface_assignment tranaction_count embeddedsw.configuration.isFlash 0
set_interface_assignment tranaction_count embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment tranaction_count embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment tranaction_count embeddedsw.configuration.isPrintableDevice 0


# 
# connection point master
# 
add_interface master avalon start
set_interface_property master addressUnits SYMBOLS
set_interface_property master associatedClock clock
set_interface_property master associatedReset reset
set_interface_property master bitsPerSymbol 8
set_interface_property master burstOnBurstBoundariesOnly false
set_interface_property master burstcountUnits WORDS
set_interface_property master doStreamReads false
set_interface_property master doStreamWrites false
set_interface_property master holdTime 0
set_interface_property master linewrapBursts false
set_interface_property master maximumPendingReadTransactions 0
set_interface_property master maximumPendingWriteTransactions 0
set_interface_property master readLatency 0
set_interface_property master readWaitTime 1
set_interface_property master setupTime 0
set_interface_property master timingUnits Cycles
set_interface_property master writeWaitTime 0
set_interface_property master ENABLED true
set_interface_property master EXPORT_OF ""
set_interface_property master PORT_NAME_MAP ""
set_interface_property master CMSIS_SVD_VARIABLES ""
set_interface_property master SVD_ADDRESS_GROUP ""

add_interface_port master avm_address address Output 10
add_interface_port master avm_byteenable byteenable Output 4
add_interface_port master avm_writedata writedata Output 32
add_interface_port master avm_readdata readdata Input 32
add_interface_port master avm_write write Output 1
add_interface_port master avm_read read Output 1
add_interface_port master avm_waitrequest waitrequest Input 1
add_interface_port master avm_readdatavalid readdatavalid Input 1

