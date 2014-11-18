
#   qsys-script --script=create_ghrd_qsys.tcl --cmd="set devkitname ALTERA_CV_SOC"

package require -exact qsys 14.0

source ./scripts/devkit_hps_configurations.tcl

if { ![ info exists devkitname ] } {
	set devkitname "ALTERA_CV_SOC"
	puts "-- Setting to default devkit \$devkitname = $devkitname"
} else {
	puts "-- Setting devkit to \$devkitname = $devkitname"
}

switch -exact $devkitname {
	"ALTERA_CV_SOC" {
		set devicefamily "CYCLONEV"
		set device "5CSXFC6D6F31C6"
		set qsys_name "soc_system"
#		set qsys_name $devkitname
#		append qsys_name "_" "soc_system"
	}
	"ALTERA_AV_SOC" {
		set devicefamily "ARRIAV"
		set device "5ASTFD5K3F40I3"
		set qsys_name "soc_system"
#		set qsys_name $devkitname
#		append qsys_name "_" "soc_system"
	}
	"ARROW_SOCKIT" {
		set devicefamily "CYCLONEV"
		set device "5CSXFC6D6F31C6"
		set qsys_name "soc_system"
#		set qsys_name $devkitname
#		append qsys_name "_" "soc_system"
	}
	"MACNICA_HELIO_14" {
		set devicefamily "CYCLONEV"
		set device "5CSXFC5C6U23C7"
		set qsys_name "soc_system"
	}
	"MACNICA_HELIO_12" {
		set devicefamily "CYCLONEV"
		set device "5CSXFC6C6U23C8ES"
		set qsys_name "soc_system"
	}
	"CRITICALLINK_MITYSOM_DEVKIT" {
		set devicefamily "CYCLONEV"
		set device "5CSXFC6C6U23C7"
		set qsys_name "soc_system"
	}
	"NOVTECH_NOVSOMCV_LITE" {
		set devicefamily "CYCLONEV"
		set device "5CSEBA2U19C8"
		set qsys_name "soc_system"
	}
	default {
		puts "-- Using default for DEVKITNAME --"
		set devkitname "ALTERA_CV_SOC"
		set devicefamily "CYCLONEV"
		set device "5CSXFC6D6F31C6"
		set qsys_name "soc_system"
#		set qsys_name $devkitname
#		append qsys_name "_" "soc_system"
	}
}

puts "-- Accepted parameter \$devicefamily = $devicefamily"
puts "-- Accepted parameter \$device = $device"
puts "-- Accepted parameter \$qsys_name = $qsys_name"


proc add_hps { devkitname } {
	switch -exact $devkitname {
		"ALTERA_CV_SOC" {
			add_hps_altera_cv_soc
		}
		"ALTERA_AV_SOC" {
			add_hps_altera_av_soc
		}
		"ARROW_SOCKIT" {
			add_hps_arrow_sockit
		}
		"MACNICA_HELIO_14" {
			add_hps_macnica_helio_rev1_4
		}
		"MACNICA_HELIO_12" {
			add_hps_macnica_helio_rev1_2
		}
		"CRITICALLINK_MITYSOM_DEVKIT" {
			add_hps_criticallink_mitysom_devkit
		}
		"NOVTECH_NOVSOMCV_LITE" {
			add_hps_novtech_novsomcv_lite	
		}
		default {
			add_hps_altera_cv_soc
		}
	}
}

create_system $qsys_name

    set_project_property DEVICE_FAMILY $devicefamily
    set_project_property DEVICE $device
    
    add_instance sysid_qsys altera_avalon_sysid_qsys 14.0
    set_instance_parameter_value sysid_qsys {id} {-1395321854}

    add_hps $devkitname

   add_instance hps_only_master altera_jtag_avalon_master 14.0
    set_instance_parameter_value hps_only_master {USE_PLI} {0}
    set_instance_parameter_value hps_only_master {PLI_PORT} {50000}
    set_instance_parameter_value hps_only_master {FAST_VER} {0}
    set_instance_parameter_value hps_only_master {FIFO_DEPTHS} {2}

    add_instance fpga_only_master altera_jtag_avalon_master 14.0
    set_instance_parameter_value fpga_only_master {USE_PLI} {0}
    set_instance_parameter_value fpga_only_master {PLI_PORT} {50000}
    set_instance_parameter_value fpga_only_master {FAST_VER} {0}
    set_instance_parameter_value fpga_only_master {FIFO_DEPTHS} {2}

    add_instance f2sdram_only_master altera_jtag_avalon_master 14.0
    set_instance_parameter_value f2sdram_only_master {USE_PLI} {0}
    set_instance_parameter_value f2sdram_only_master {PLI_PORT} {50000}
    set_instance_parameter_value f2sdram_only_master {FAST_VER} {0}
    set_instance_parameter_value f2sdram_only_master {FIFO_DEPTHS} {2}

    add_instance jtag_uart altera_avalon_jtag_uart 14.0
    set_instance_parameter_value jtag_uart {allowMultipleConnections} {1}
    set_instance_parameter_value jtag_uart {hubInstanceID} {0}
    set_instance_parameter_value jtag_uart {readBufferDepth} {64}
    set_instance_parameter_value jtag_uart {readIRQThreshold} {8}
    set_instance_parameter_value jtag_uart {simInputCharacterStream} {}
    set_instance_parameter_value jtag_uart {simInteractiveOptions} {INTERACTIVE_ASCII_OUTPUT}
    set_instance_parameter_value jtag_uart {useRegistersForReadBuffer} {0}
    set_instance_parameter_value jtag_uart {useRegistersForWriteBuffer} {0}
    set_instance_parameter_value jtag_uart {useRelativePathForSimFile} {0}
    set_instance_parameter_value jtag_uart {writeBufferDepth} {64}
    set_instance_parameter_value jtag_uart {writeIRQThreshold} {8}

    add_instance clk_0 clock_source 14.0
    set_instance_parameter_value clk_0 {clockFrequency} {50000000.0}
    set_instance_parameter_value clk_0 {clockFrequencyKnown} {1}
    set_instance_parameter_value clk_0 {resetSynchronousEdges} {NONE}

    add_instance validator_subsys_0 validator_subsys 1.0

    # connections and connection parameters
    add_connection hps_0.h2f_lw_axi_master sysid_qsys.control_slave avalon
    set_connection_parameter_value hps_0.h2f_lw_axi_master/sysid_qsys.control_slave arbitrationPriority {1}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/sysid_qsys.control_slave baseAddress {0x00010000}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/sysid_qsys.control_slave defaultConnection {0}

    add_connection hps_0.f2h_irq0 jtag_uart.irq interrupt
    set_connection_parameter_value hps_0.f2h_irq0/jtag_uart.irq irqNumber {2}

    add_connection hps_0.h2f_lw_axi_master jtag_uart.avalon_jtag_slave avalon
    set_connection_parameter_value hps_0.h2f_lw_axi_master/jtag_uart.avalon_jtag_slave arbitrationPriority {1}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/jtag_uart.avalon_jtag_slave baseAddress {0x00020000}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/jtag_uart.avalon_jtag_slave defaultConnection {0}

    add_connection fpga_only_master.master jtag_uart.avalon_jtag_slave avalon
    set_connection_parameter_value fpga_only_master.master/jtag_uart.avalon_jtag_slave arbitrationPriority {1}
    set_connection_parameter_value fpga_only_master.master/jtag_uart.avalon_jtag_slave baseAddress {0x00020000}
    set_connection_parameter_value fpga_only_master.master/jtag_uart.avalon_jtag_slave defaultConnection {0}

    add_connection fpga_only_master.master sysid_qsys.control_slave avalon
    set_connection_parameter_value fpga_only_master.master/sysid_qsys.control_slave arbitrationPriority {1}
    set_connection_parameter_value fpga_only_master.master/sysid_qsys.control_slave baseAddress {0x00010000}
    set_connection_parameter_value fpga_only_master.master/sysid_qsys.control_slave defaultConnection {0}

    add_connection hps_only_master.master hps_0.f2h_axi_slave avalon
    set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave arbitrationPriority {1}
    set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave baseAddress {0x0000}
    set_connection_parameter_value hps_only_master.master/hps_0.f2h_axi_slave defaultConnection {0}

    add_connection f2sdram_only_master.master hps_0.f2h_sdram0_data avalon
    set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data arbitrationPriority {1}
    set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data baseAddress {0x0000}
    set_connection_parameter_value f2sdram_only_master.master/hps_0.f2h_sdram0_data defaultConnection {0}

    add_connection hps_0.h2f_user1_clock clk_0.clk_in clock

    add_connection hps_0.h2f_reset clk_0.clk_in_reset reset

    add_connection hps_0.h2f_user1_clock hps_0.f2h_sdram0_clock clock

    add_connection hps_0.h2f_user1_clock hps_0.h2f_axi_clock clock

    add_connection hps_0.h2f_user1_clock hps_0.f2h_axi_clock clock

    add_connection hps_0.h2f_user1_clock hps_0.h2f_lw_axi_clock clock

    add_connection hps_0.h2f_user1_clock hps_only_master.clk clock

    add_connection hps_0.h2f_user1_clock fpga_only_master.clk clock

    add_connection hps_0.h2f_user1_clock f2sdram_only_master.clk clock

    add_connection hps_0.h2f_user1_clock jtag_uart.clk clock

    add_connection hps_0.h2f_reset fpga_only_master.clk_reset reset

    add_connection hps_0.h2f_reset hps_only_master.clk_reset reset

    add_connection hps_0.h2f_reset f2sdram_only_master.clk_reset reset

    add_connection hps_0.h2f_reset jtag_uart.reset reset

    add_connection hps_0.h2f_user1_clock sysid_qsys.clk clock

    add_connection hps_0.h2f_reset sysid_qsys.reset reset

    add_connection hps_0.h2f_user1_clock validator_subsys_0.clk clock

    add_connection hps_0.h2f_reset validator_subsys_0.reset reset

    add_connection hps_0.h2f_lw_axi_master validator_subsys_0.val_mm_bridge_s0 avalon
    set_connection_parameter_value hps_0.h2f_lw_axi_master/validator_subsys_0.val_mm_bridge_s0 arbitrationPriority {1}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/validator_subsys_0.val_mm_bridge_s0 baseAddress {0x00040000}
    set_connection_parameter_value hps_0.h2f_lw_axi_master/validator_subsys_0.val_mm_bridge_s0 defaultConnection {0}

    add_connection fpga_only_master.master validator_subsys_0.val_mm_bridge_s0 avalon
    set_connection_parameter_value fpga_only_master.master/validator_subsys_0.val_mm_bridge_s0 arbitrationPriority {1}
    set_connection_parameter_value fpga_only_master.master/validator_subsys_0.val_mm_bridge_s0 baseAddress {0x00040000}
    set_connection_parameter_value fpga_only_master.master/validator_subsys_0.val_mm_bridge_s0 defaultConnection {0}

    # exported interfaces
    add_interface memory conduit end
    set_interface_property memory EXPORT_OF hps_0.memory
    add_interface hps_0_hps_io conduit end
    set_interface_property hps_0_hps_io EXPORT_OF hps_0.hps_io
    add_interface hps_0_h2f_reset reset source
    set_interface_property hps_0_h2f_reset EXPORT_OF clk_0.clk_reset
    add_interface hps_0_h2f_clk clock source
    set_interface_property hps_0_h2f_clk EXPORT_OF clk_0.clk

    # interconnect requirements
    set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
    set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {0}
    set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
    set_interconnect_requirement {hps_only_master.master} {qsys_mm.security} {SECURE}
	
  save_system $devkitname/${qsys_name}.qsys
