# 
# request TCL package from ACDS 13.0
# 
package require -exact qsys 13.0


# 
# module conduit_blender
# 
set_module_property DESCRIPTION "Blend the PIO conduit into those required by the RNG subsystem."
set_module_property NAME conduit_blender
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR RSF
set_module_property DISPLAY_NAME conduit_blender
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL conduit_blender
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file conduit_blender.v VERILOG PATH conduit_blender.v TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL conduit_blender
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VERILOG ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file conduit_blender.v VERILOG PATH conduit_blender.v

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL conduit_blender
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file conduit_blender.v VERILOG PATH conduit_blender.v


# 
# parameters
# 


# 
# display items
# 


# 
# connection point external_connection
# 
add_interface external_connection conduit end
set_interface_property external_connection associatedClock ""
set_interface_property external_connection associatedReset ""
set_interface_property external_connection ENABLED true
set_interface_property external_connection EXPORT_OF ""
set_interface_property external_connection PORT_NAME_MAP ""
set_interface_property external_connection CMSIS_SVD_VARIABLES ""
set_interface_property external_connection SVD_ADDRESS_GROUP ""

add_interface_port external_connection in_port export Output 3
add_interface_port external_connection out_port export Input 3


# 
# connection point ring_osc_enable
# 
add_interface ring_osc_enable conduit end
set_interface_property ring_osc_enable associatedClock ""
set_interface_property ring_osc_enable associatedReset ""
set_interface_property ring_osc_enable ENABLED true
set_interface_property ring_osc_enable EXPORT_OF ""
set_interface_property ring_osc_enable PORT_NAME_MAP ""
set_interface_property ring_osc_enable CMSIS_SVD_VARIABLES ""
set_interface_property ring_osc_enable SVD_ADDRESS_GROUP ""

add_interface_port ring_osc_enable ring_osc_enable export Output 1


# 
# connection point entropy_counter_enable
# 
add_interface entropy_counter_enable conduit end
set_interface_property entropy_counter_enable associatedClock ""
set_interface_property entropy_counter_enable associatedReset ""
set_interface_property entropy_counter_enable ENABLED true
set_interface_property entropy_counter_enable EXPORT_OF ""
set_interface_property entropy_counter_enable PORT_NAME_MAP ""
set_interface_property entropy_counter_enable CMSIS_SVD_VARIABLES ""
set_interface_property entropy_counter_enable SVD_ADDRESS_GROUP ""

add_interface_port entropy_counter_enable entropy_counter_enable export Output 1


# 
# connection point entropy_counter_clear
# 
add_interface entropy_counter_clear conduit end
set_interface_property entropy_counter_clear associatedClock ""
set_interface_property entropy_counter_clear associatedReset ""
set_interface_property entropy_counter_clear ENABLED true
set_interface_property entropy_counter_clear EXPORT_OF ""
set_interface_property entropy_counter_clear PORT_NAME_MAP ""
set_interface_property entropy_counter_clear CMSIS_SVD_VARIABLES ""
set_interface_property entropy_counter_clear SVD_ADDRESS_GROUP ""

add_interface_port entropy_counter_clear entropy_counter_clear export Output 1

