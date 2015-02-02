# (C) 2001-2015 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License Subscription 
# Agreement, Altera MegaCore Function License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


# $Id: //acds/rel/14.1/ip/sld/st/altera_streaming_sld_hub_controller/altera_streaming_sld_hub_controller_core_hw.tcl#1 $
# $Revision: #1 $
# $Date: 2014/10/06 $
# $Author: swbuild $

# +-----------------------------------
# | 
# | altera_streaming_sld_hub_controller_core "altera_streaming_sld_hub_controller_core" v1.0
# | Altera 2011.11.03.15:36:49
# | 
# | 
# | /data/aferrucc/20111103/sld_hub_controller/ip/altera_streaming_sld_hub_controller/altera_streaming_sld_hub_controller_core.sv
# | 
# |    ./altera_streaming_sld_hub_controller_core.sv syn, sim
# |    ./altera_streaming_sld_hub_controller_sld_node.v syn, sim
# | 
# +-----------------------------------

# +-----------------------------------
# | request TCL package from ACDS 11.0
# | 
package require -exact sopc 11.0
# | 
# +-----------------------------------

# +-----------------------------------
# | module altera_streaming_sld_hub_controller_core
# | 
set_module_property NAME altera_streaming_sld_hub_controller_core_dw
set_module_property AUTHOR "Altera Corporation - dw"
set_module_property GROUP "Basic Functions/Simulation; Debug and Verification/Debug and Performance"
set_module_property DISPLAY_NAME "SLD Hub Controller core DW"
set_module_property VERSION 14.1
set_module_property DISPLAY_NAME altera_streaming_sld_hub_controller_core_dw
set_module_property TOP_LEVEL_HDL_FILE altera_streaming_sld_hub_controller_core_dw.sv
set_module_property TOP_LEVEL_HDL_MODULE altera_streaming_sld_hub_controller_core_dw
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL FALSE
set_module_property STATIC_TOP_LEVEL_MODULE_NAME altera_streaming_sld_hub_controller_core_dw
set_module_property FIX_110_VIP_PATH false
set_module_property ELABORATION_CALLBACK elaborate

set_module_assignment debug.virtualInterface.link {debug.controlledBy sink debug.fabricLink {fabric sld} }

add_parameter DEVICE_FAMILY STRING "Stratix IV"
set_parameter_property DEVICE_FAMILY SYSTEM_INFO DEVICE_FAMILY
set_parameter_property DEVICE_FAMILY VISIBLE false
set_parameter_property DEVICE_FAMILY HDL_PARAMETER true

add_parameter ENABLE_JTAG_IO_SELECTION INTEGER 0
set_parameter_property ENABLE_JTAG_IO_SELECTION HDL_PARAMETER true

add_parameter ST_DATA_W INTEGER 8
set_parameter_property ST_DATA_W VISIBLE false

# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
add_file altera_streaming_sld_hub_controller_core_dw.sv {SYNTHESIS SIMULATION}
add_file altera_streaming_sld_hub_controller_sld_node.sv {SYNTHESIS SIMULATION}
add_file altera_streaming_sld_hub_controller_tck_generation.sv {SYNTHESIS SIMULATION}
add_file altera_streaming_sld_hub_controller_altclkctrl.sv {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | display items
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clk
# | 
add_interface clk clock end
set_interface_property clk clockRate 0

set_interface_property clk ENABLED true

add_interface_port clk clk clk Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point reset
# | 
add_interface reset reset end
set_interface_property reset associatedClock clk
set_interface_property reset synchronousEdges DEASSERT

set_interface_property reset ENABLED true

add_interface_port reset reset reset Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point sink
# | 
add_interface sink avalon_streaming end
set_interface_property sink associatedClock clk
set_interface_property sink associatedReset reset
set_interface_property sink dataBitsPerSymbol 8
set_interface_property sink errorDescriptor ""
set_interface_property sink firstSymbolInHighOrderBits true
set_interface_property sink maxChannel 0
set_interface_property sink readyLatency 0

set_interface_property sink ENABLED true

set_interface_assignment sink debug.typeName altera_streaming_sld_hub_controller.sink
set_interface_assignment sink debug.interfaceGroup {associatedT2h source} 
  
add_interface_port sink cmd_valid valid Input 1
add_interface_port sink cmd_sop startofpacket Input 1
add_interface_port sink cmd_eop endofpacket Input 1
add_interface_port sink cmd_data data Input 8
add_interface_port sink cmd_ready ready Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point source
# | 
add_interface source avalon_streaming start
set_interface_property source associatedClock clk
set_interface_property source associatedReset reset
set_interface_property source dataBitsPerSymbol 8
set_interface_property source errorDescriptor ""
set_interface_property source firstSymbolInHighOrderBits true
set_interface_property source maxChannel 0
set_interface_property source readyLatency 0

set_interface_property source ENABLED true

add_interface_port source resp_valid valid Output 1
add_interface_port source resp_sop startofpacket Output 1
add_interface_port source resp_eop endofpacket Output 1
add_interface_port source resp_data data Output 8
add_interface_port source resp_ready ready Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | 
# | connection point csr
# |
#add_interface csr avalon end
#set_interface_property csr addressUnits WORDS
#set_interface_property csr associatedClock clk
#set_interface_property csr associatedReset reset
#set_interface_property csr bitsPerSymbol 8
#set_interface_property csr burstOnBurstBoundariesOnly false
#set_interface_property csr burstcountUnits WORDS
#set_interface_property csr explicitAddressSpan 0
#set_interface_property csr holdTime 0
#set_interface_property csr linewrapBursts false
#set_interface_property csr maximumPendingReadTransactions 0
#set_interface_property csr readLatency 0
#set_interface_property csr readWaitTime 1
#set_interface_property csr setupTime 0
#set_interface_property csr timingUnits Cycles
#set_interface_property csr writeWaitTime 0
#set_interface_property csr ENABLED false

add_interface jtag_io_enable conduit end
set_interface_property jtag_io_enable associatedClock ""
set_interface_property jtag_io_enable associatedReset ""
set_interface_property jtag_io_enable ENABLED true
set_interface_property jtag_io_enable EXPORT_OF ""
set_interface_property jtag_io_enable PORT_NAME_MAP ""
set_interface_property jtag_io_enable CMSIS_SVD_VARIABLES ""
set_interface_property jtag_io_enable SVD_ADDRESS_GROUP ""
set_interface_property jtag_io_enable ENABLED false

add_interface_port jtag_io_enable enable_user_jtag_io export Input 1  

#add_interface_port csr csr_write write Input 1
#set_port_property csr_write termination true
#add_interface_port csr csr_read read Input 1
#set_port_property csr_read termination true
#add_interface_port csr csr_readdata readdata Output 32
#set_port_property csr_readdata termination true
#add_interface_port csr csr_writedata writedata Input 32
#set_port_property csr_writedata termination true
#add_interface_port csr csr_addr address Input 1
#set_port_property csr_addr termination true
# | 
# +-----------------------------------

proc elaborate {} {
  set jtag_io_select [get_parameter_value ENABLE_JTAG_IO_SELECTION]

  if {$jtag_io_select} {
    set_interface_property jtag_io_enable ENABLED true
  }
}

