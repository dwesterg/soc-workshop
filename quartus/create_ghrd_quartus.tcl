# to execute this script using quartus_sh for generating Quartus QPF and QSF accordingly
#   quartus_sh --script=create_ghrd_quartus.tcl
#
set devicefamily CYCLONEV
set device 5CSXFC6D6F31C6
set projectname soc_system
# thinking to change project name to ghrd_5csxfc6d6, not yet agreed
set qipfiles "soc_system/synthesis/soc_system.qip,ip/altsource_probe/hps_reset.qip"
set hdlfiles "ip/edge_detect/altera_edge_detector.v,ip/debounce/debounce.v,ghrd_top.v"
set topname ghrd_top

# ... alternatively, above parameters can be passed in as script arguments
#   quartus_sh --script=create_ghrd_quartus.tcl <parameter1 value1 parameter2 value2 ...>
# parameters of this TCL includes
#   devicefamily  : FPGA device family
#   device        : FPGA device number
#   projectname   : Quartus project name
#   qipfiles      : QIP file path(s), multiple paths need seperator of ","
#   hdlfiles      : HDL file path(s), multiple paths need seperator of ","
#   topname       : top module name


proc show_arguments {} {
  global quartus
  global devicefamily
  global device
  global projectname
  global topname
  global qipfiles
  global hdlfiles

  foreach {key value} $quartus(args) {
    puts "-> Accepted parameter: $key,  \tValue: $value"
    if {$key == "devicefamily"} {
      set devicefamily $value
    }
    if {$key == "device"} {
      set device $value
    }
    if {$key == "projectname"} {
      set projectname $value
    }
    if {$key == "topname"} {
      set topname $value
    }
    if {$key == "qipfiles"} {
      set qipfiles $value
    }
    if {$key == "hdlfiles"} {
      set hdlfiles $value
    }

  }
}
show_arguments

#regsub -all {\mfoo\M} $string bar string
#set wordList [regexp -inline -all -- {\S+} $text]
if {[regexp {,} $qipfiles]} {
  set qipfilelist [split $qipfiles ,]
} else {
  set qipfilelist $qipfiles
}

if {[regexp {,} $hdlfiles]} {
  set hdlfilelist [split $hdlfiles ,]
} else {
  set hdlfilelist $hdlfiles
}

project_new -overwrite -family $devicefamily -part $device $projectname

set_global_assignment -name TOP_LEVEL_ENTITY $topname

foreach qipfile $qipfilelist {
  set_global_assignment -name QIP_FILE $qipfile
}

foreach hdlfile $hdlfilelist {
  set_global_assignment -name VERILOG_FILE $hdlfile
}

set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
set_global_assignment -name EDA_SIMULATION_TOOL "<None>"
set_global_assignment -name EDA_OUTPUT_DATA_FORMAT NONE -section_id eda_simulation
set_global_assignment -name SDC_FILE soc_system_timing.sdc

# enabling signaltap 
set_global_assignment -name ENABLE_SIGNALTAP ON
set_global_assignment -name USE_SIGNALTAP_FILE cti_tapping.stp
set_global_assignment -name SIGNALTAP_FILE cti_tapping.stp

# pin location assignments
set_location_assignment PIN_AC18 -to fpga_clk_50
set_location_assignment PIN_AB13 -to fpga_button_pio[1]
set_location_assignment PIN_AA13 -to fpga_button_pio[0]
set_location_assignment PIN_AG11 -to fpga_dipsw_pio[3]
set_location_assignment PIN_AF11 -to fpga_dipsw_pio[2]
set_location_assignment PIN_AH9 -to fpga_dipsw_pio[1]
set_location_assignment PIN_AG10 -to fpga_dipsw_pio[0]
set_location_assignment PIN_AB17 -to fpga_led_pio[3]
set_location_assignment PIN_W15 -to fpga_led_pio[2]
set_location_assignment PIN_Y16 -to fpga_led_pio[1]
set_location_assignment PIN_AK2 -to fpga_led_pio[0]

# instance assignments
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_led_pio[0]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_led_pio[1]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_led_pio[2]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_led_pio[3]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_dipsw_pio[0]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_dipsw_pio[1]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_dipsw_pio[2]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_dipsw_pio[3]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_button_pio[0]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_button_pio[1]
set_instance_assignment -name IO_STANDARD "1.5 V" -to fpga_clk_50
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_can0_RX
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_can0_TX
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_MDC
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_MDIO
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RXD0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RXD1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RXD2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RXD3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RX_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_RX_CTL
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TXD0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TXD3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TXD1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TXD2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TX_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_emac1_TX_CTL
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO09
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO35
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO41
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO42
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO43
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_gpio_GPIO44
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_i2c0_SCL
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_i2c0_SDA
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_IO0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_IO1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_IO2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_IO3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_qspi_SS0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_CMD
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_spim0_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_spim0_MISO
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_spim0_MOSI
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_spim0_SS0
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D0
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D1
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D2
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D3
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D4
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D5
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D6
set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to hps_trace_D7
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_uart0_RX
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_uart0_TX
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D4
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D5
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D6
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_D7
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_DIR
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_NXT
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_usb1_STP
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_MDC
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_MDIO
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RXD0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RXD1
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RXD2
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RXD3
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RX_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_RX_CTL
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TXD0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TXD1
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TXD2
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TXD3
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TX_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_emac1_TX_CTL
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D1
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D2
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D3
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D4
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D5
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D6
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_D7
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_DIR
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_NXT
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_usb1_STP
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_i2c0_SCL
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_i2c0_SDA
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_spim0_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_spim0_MISO
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_spim0_MOSI
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_spim0_SS0
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_can0_RX
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_can0_TX
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_gpio_GPIO35
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D0
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D1
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D2
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D3
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D4
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D5
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D6
set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to hps_trace_D7
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_gpio_GPIO41
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_gpio_GPIO42
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_gpio_GPIO43
set_instance_assignment -name CURRENT_STRENGTH_NEW 8MA -to hps_gpio_GPIO44
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_gpio_GPIO09
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_IO0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_IO1
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_IO2
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_IO3
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_qspi_SS0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_CLK
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_CMD
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_D0
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_D1
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_D2
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_sdio_D3
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_uart0_RX
set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to hps_uart0_TX
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_CLK
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D0
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D1
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D2
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D3
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D4
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D5
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D6
set_instance_assignment -name SLEW_RATE 1 -to hps_trace_D7

# assignment of HPS SDRAM from back annotation. to avoid Critical Warning
set_location_assignment PIN_F26 -to hps_memory_mem_a[0]
set_location_assignment PIN_G30 -to hps_memory_mem_a[1]
set_location_assignment PIN_F28 -to hps_memory_mem_a[2]
set_location_assignment PIN_F30 -to hps_memory_mem_a[3]
set_location_assignment PIN_J25 -to hps_memory_mem_a[4]
set_location_assignment PIN_J27 -to hps_memory_mem_a[5]
set_location_assignment PIN_F29 -to hps_memory_mem_a[6]
set_location_assignment PIN_E28 -to hps_memory_mem_a[7]
set_location_assignment PIN_H27 -to hps_memory_mem_a[8]
set_location_assignment PIN_G26 -to hps_memory_mem_a[9]
set_location_assignment PIN_D29 -to hps_memory_mem_a[10]
set_location_assignment PIN_C30 -to hps_memory_mem_a[11]
set_location_assignment PIN_B30 -to hps_memory_mem_a[12]
set_location_assignment PIN_C29 -to hps_memory_mem_a[13]
set_location_assignment PIN_H25 -to hps_memory_mem_a[14]
set_location_assignment PIN_E29 -to hps_memory_mem_ba[0]
set_location_assignment PIN_J24 -to hps_memory_mem_ba[1]
set_location_assignment PIN_J23 -to hps_memory_mem_ba[2]
set_location_assignment PIN_M23 -to hps_memory_mem_ck
set_location_assignment PIN_L23 -to hps_memory_mem_ck_n
set_location_assignment PIN_L29 -to hps_memory_mem_cke
set_location_assignment PIN_H24 -to hps_memory_mem_cs_n
set_location_assignment PIN_D30 -to hps_memory_mem_ras_n
set_location_assignment PIN_E27 -to hps_memory_mem_cas_n
set_location_assignment PIN_C28 -to hps_memory_mem_we_n
set_location_assignment PIN_P30 -to hps_memory_mem_reset_n
set_location_assignment PIN_H28 -to hps_memory_mem_odt
set_location_assignment PIN_K28 -to hps_memory_mem_dm[0]
set_location_assignment PIN_M28 -to hps_memory_mem_dm[1]
set_location_assignment PIN_R28 -to hps_memory_mem_dm[2]
set_location_assignment PIN_W30 -to hps_memory_mem_dm[3]
set_location_assignment PIN_W27 -to hps_memory_mem_dm[4]
set_location_assignment PIN_K23 -to hps_memory_mem_dq[0]
set_location_assignment PIN_K22 -to hps_memory_mem_dq[1]
set_location_assignment PIN_H30 -to hps_memory_mem_dq[2]
set_location_assignment PIN_G28 -to hps_memory_mem_dq[3]
set_location_assignment PIN_L25 -to hps_memory_mem_dq[4]
set_location_assignment PIN_L24 -to hps_memory_mem_dq[5]
set_location_assignment PIN_J30 -to hps_memory_mem_dq[6]
set_location_assignment PIN_J29 -to hps_memory_mem_dq[7]
set_location_assignment PIN_K26 -to hps_memory_mem_dq[8]
set_location_assignment PIN_L26 -to hps_memory_mem_dq[9]
set_location_assignment PIN_K29 -to hps_memory_mem_dq[10]
set_location_assignment PIN_K27 -to hps_memory_mem_dq[11]
set_location_assignment PIN_M26 -to hps_memory_mem_dq[12]
set_location_assignment PIN_M27 -to hps_memory_mem_dq[13]
set_location_assignment PIN_L28 -to hps_memory_mem_dq[14]
set_location_assignment PIN_M30 -to hps_memory_mem_dq[15]
set_location_assignment PIN_U26 -to hps_memory_mem_dq[16]
set_location_assignment PIN_T26 -to hps_memory_mem_dq[17]
set_location_assignment PIN_N29 -to hps_memory_mem_dq[18]
set_location_assignment PIN_N28 -to hps_memory_mem_dq[19]
set_location_assignment PIN_P26 -to hps_memory_mem_dq[20]
set_location_assignment PIN_P27 -to hps_memory_mem_dq[21]
set_location_assignment PIN_N27 -to hps_memory_mem_dq[22]
set_location_assignment PIN_R29 -to hps_memory_mem_dq[23]
set_location_assignment PIN_P24 -to hps_memory_mem_dq[24]
set_location_assignment PIN_P25 -to hps_memory_mem_dq[25]
set_location_assignment PIN_T29 -to hps_memory_mem_dq[26]
set_location_assignment PIN_T28 -to hps_memory_mem_dq[27]
set_location_assignment PIN_R27 -to hps_memory_mem_dq[28]
set_location_assignment PIN_R26 -to hps_memory_mem_dq[29]
set_location_assignment PIN_V30 -to hps_memory_mem_dq[30]
set_location_assignment PIN_W29 -to hps_memory_mem_dq[31]
set_location_assignment PIN_W26 -to hps_memory_mem_dq[32]
set_location_assignment PIN_R24 -to hps_memory_mem_dq[33]
set_location_assignment PIN_U27 -to hps_memory_mem_dq[34]
set_location_assignment PIN_V28 -to hps_memory_mem_dq[35]
set_location_assignment PIN_T25 -to hps_memory_mem_dq[36]
set_location_assignment PIN_U25 -to hps_memory_mem_dq[37]
set_location_assignment PIN_V27 -to hps_memory_mem_dq[38]
set_location_assignment PIN_Y29 -to hps_memory_mem_dq[39]
set_location_assignment PIN_N18 -to hps_memory_mem_dqs[0]
set_location_assignment PIN_N25 -to hps_memory_mem_dqs[1]
set_location_assignment PIN_R19 -to hps_memory_mem_dqs[2]
set_location_assignment PIN_R22 -to hps_memory_mem_dqs[3]
set_location_assignment PIN_T24 -to hps_memory_mem_dqs[4]
set_location_assignment PIN_M19 -to hps_memory_mem_dqs_n[0]
set_location_assignment PIN_N24 -to hps_memory_mem_dqs_n[1]
set_location_assignment PIN_R18 -to hps_memory_mem_dqs_n[2]
set_location_assignment PIN_R21 -to hps_memory_mem_dqs_n[3]
set_location_assignment PIN_T23 -to hps_memory_mem_dqs_n[4]
set_location_assignment PIN_D27 -to hps_memory_oct_rzqin


project_close

