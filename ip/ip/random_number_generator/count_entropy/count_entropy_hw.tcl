# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module count_entropy
# 
set_module_property DESCRIPTION "Entropy counters."
set_module_property NAME count_entropy
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME count_entropy
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL count_entropy
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy.sv SYSTEM_VERILOG PATH count_entropy.sv TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL count_entropy
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy.sv SYSTEM_VERILOG PATH count_entropy.sv

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL count_entropy
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file count_entropy.sv SYSTEM_VERILOG PATH count_entropy.sv


# 
# parameters
# 
add_parameter BIT_WIDTH INTEGER 1
set_parameter_property BIT_WIDTH DEFAULT_VALUE 1
set_parameter_property BIT_WIDTH DISPLAY_NAME BIT_WIDTH
set_parameter_property BIT_WIDTH DESCRIPTION "Privides 2**BIT_WIDTH counters to count entropy of incoming symbols.  This parameter specifies the Avalon ST input bit width."
set_parameter_property BIT_WIDTH TYPE INTEGER
set_parameter_property BIT_WIDTH UNITS None
set_parameter_property BIT_WIDTH ALLOWED_RANGES 1,2,4,8
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

add_interface_port sink snk_data data Input BIT_WIDTH
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

add_interface_port source src_data data Output BIT_WIDTH
add_interface_port source src_valid valid Output 1


# 
# connection point transaction_counter
# 
add_interface transaction_counter avalon end
set_interface_property transaction_counter addressUnits WORDS
set_interface_property transaction_counter associatedClock clock
set_interface_property transaction_counter associatedReset reset
set_interface_property transaction_counter bitsPerSymbol 8
set_interface_property transaction_counter burstOnBurstBoundariesOnly false
set_interface_property transaction_counter burstcountUnits WORDS
set_interface_property transaction_counter explicitAddressSpan 0
set_interface_property transaction_counter holdTime 0
set_interface_property transaction_counter linewrapBursts false
set_interface_property transaction_counter maximumPendingReadTransactions 0
set_interface_property transaction_counter maximumPendingWriteTransactions 0
set_interface_property transaction_counter readLatency 0
set_interface_property transaction_counter readWaitTime 1
set_interface_property transaction_counter setupTime 0
set_interface_property transaction_counter timingUnits Cycles
set_interface_property transaction_counter writeWaitTime 0
set_interface_property transaction_counter ENABLED true
set_interface_property transaction_counter EXPORT_OF ""
set_interface_property transaction_counter PORT_NAME_MAP ""
set_interface_property transaction_counter CMSIS_SVD_VARIABLES ""
set_interface_property transaction_counter SVD_ADDRESS_GROUP ""

add_interface_port transaction_counter transaction_readdata readdata Output 32
set_interface_assignment transaction_counter embeddedsw.configuration.isFlash 0
set_interface_assignment transaction_counter embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment transaction_counter embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment transaction_counter embeddedsw.configuration.isPrintableDevice 0


# 
# connection point entropy_counters
# 
add_interface entropy_counters avalon end
set_interface_property entropy_counters addressUnits WORDS
set_interface_property entropy_counters associatedClock clock
set_interface_property entropy_counters associatedReset reset
set_interface_property entropy_counters bitsPerSymbol 8
set_interface_property entropy_counters burstOnBurstBoundariesOnly false
set_interface_property entropy_counters burstcountUnits WORDS
set_interface_property entropy_counters explicitAddressSpan 0
set_interface_property entropy_counters holdTime 0
set_interface_property entropy_counters linewrapBursts false
set_interface_property entropy_counters maximumPendingReadTransactions 0
set_interface_property entropy_counters maximumPendingWriteTransactions 0
set_interface_property entropy_counters readLatency 0
set_interface_property entropy_counters readWaitTime 1
set_interface_property entropy_counters setupTime 0
set_interface_property entropy_counters timingUnits Cycles
set_interface_property entropy_counters writeWaitTime 0
set_interface_property entropy_counters ENABLED true
set_interface_property entropy_counters EXPORT_OF ""
set_interface_property entropy_counters PORT_NAME_MAP ""
set_interface_property entropy_counters CMSIS_SVD_VARIABLES ""
set_interface_property entropy_counters SVD_ADDRESS_GROUP ""

add_interface_port entropy_counters entropy_address address Input BIT_WIDTH
add_interface_port entropy_counters entropy_readdata readdata Output 32
set_interface_assignment entropy_counters embeddedsw.configuration.isFlash 0
set_interface_assignment entropy_counters embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment entropy_counters embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment entropy_counters embeddedsw.configuration.isPrintableDevice 0


# 
# connection point enable_conduit
# 
add_interface enable_conduit conduit end
set_interface_property enable_conduit associatedClock none
set_interface_property enable_conduit associatedReset none
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
set_interface_property clear_conduit associatedClock none
set_interface_property clear_conduit associatedReset none
set_interface_property clear_conduit ENABLED true
set_interface_property clear_conduit EXPORT_OF ""
set_interface_property clear_conduit PORT_NAME_MAP ""
set_interface_property clear_conduit CMSIS_SVD_VARIABLES ""
set_interface_property clear_conduit SVD_ADDRESS_GROUP ""

add_interface_port clear_conduit clear export Input 1

