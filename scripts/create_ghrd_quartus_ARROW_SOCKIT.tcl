set devicefamily CYCLONEV
set device 5CSXFC6D6F31C6

set qipfiles "../ip/altsource_probe/hps_reset.qip,../ip/fft128/fft_test.qip,../ip/demo_driver/meminit.qip,../ip/workshop_validator/meminit.qip"
set hdlfiles "../hdl_src/ghrd_top.v,../ip/edge_detect/altera_edge_detector.v,../ip/debounce/debounce.v"
set topname ghrd_top

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

set_global_assignment -name FAMILY $devicefamily
set_global_assignment -name DEVICE $device

set_global_assignment -name TOP_LEVEL_ENTITY $topname

foreach qipfile $qipfilelist {
  set_global_assignment -name QIP_FILE $qipfile
}

foreach hdlfile $hdlfilelist {
  set_global_assignment -name VERILOG_FILE $hdlfile
}

#set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
set_global_assignment -name EDA_SIMULATION_TOOL "<None>"
set_global_assignment -name EDA_OUTPUT_DATA_FORMAT NONE -section_id eda_simulation
set_global_assignment -name SDC_FILE ../hdl_src/soc_system_timing.sdc

set_parameter -name MEM_A_WIDTH 15
set_parameter -name MEM_BA_WIDTH 3
set_parameter -name MEM_D_WIDTH 32

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

# assignment of HPS SDRAM from back annotation. to avoid Critical Warning
# set_location_assignment PIN_N9 -to hps_memory_mem_a[0]
# set_location_assignment PIN_M9 -to hps_memory_mem_a[1]
# set_location_assignment PIN_N10 -to hps_memory_mem_a[2]
# set_location_assignment PIN_M10 -to hps_memory_mem_a[3]
# set_location_assignment PIN_A8 -to hps_memory_mem_a[4]
# set_location_assignment PIN_B7 -to hps_memory_mem_a[5]
# set_location_assignment PIN_B9 -to hps_memory_mem_a[6]
# set_location_assignment PIN_A9 -to hps_memory_mem_a[7]
# set_location_assignment PIN_D9 -to hps_memory_mem_a[8]
# set_location_assignment PIN_C10 -to hps_memory_mem_a[9]
# set_location_assignment PIN_K7 -to hps_memory_mem_a[10]
# set_location_assignment PIN_J7 -to hps_memory_mem_a[11]
# set_location_assignment PIN_F9 -to hps_memory_mem_a[12]
# set_location_assignment PIN_E9 -to hps_memory_mem_a[13]
# set_location_assignment PIN_D11 -to hps_memory_mem_a[14]
# set_location_assignment PIN_L7 -to hps_memory_mem_ba[0]
# set_location_assignment PIN_C9 -to hps_memory_mem_ba[1]
# set_location_assignment PIN_D8 -to hps_memory_mem_ba[2]
# set_location_assignment PIN_A11 -to hps_memory_mem_ck
# set_location_assignment PIN_B10 -to hps_memory_mem_ck_n
# set_location_assignment PIN_R8 -to hps_memory_mem_cke
# set_location_assignment PIN_H9 -to hps_memory_mem_cs_n
# set_location_assignment PIN_G8 -to hps_memory_mem_ras_n
# set_location_assignment PIN_G9 -to hps_memory_mem_cas_n
# set_location_assignment PIN_J8 -to hps_memory_mem_we_n
# set_location_assignment PIN_E3 -to hps_memory_mem_reset_n
# set_location_assignment PIN_H7 -to hps_memory_mem_odt
# set_location_assignment PIN_C6 -to hps_memory_mem_dm[0]
# set_location_assignment PIN_E4 -to hps_memory_mem_dm[1]
# set_location_assignment PIN_D3 -to hps_memory_mem_dm[2]
# set_location_assignment PIN_D1 -to hps_memory_mem_dm[3]
# set_location_assignment PIN_T7 -to hps_memory_mem_dm[4]
# set_location_assignment PIN_D7 -to hps_memory_mem_dq[0]
# set_location_assignment PIN_C7 -to hps_memory_mem_dq[1]
# set_location_assignment PIN_R10 -to hps_memory_mem_dq[2]
# set_location_assignment PIN_G7 -to hps_memory_mem_dq[3]
# set_location_assignment PIN_A6 -to hps_memory_mem_dq[4]
# set_location_assignment PIN_A7 -to hps_memory_mem_dq[5]
# set_location_assignment PIN_L6 -to hps_memory_mem_dq[6]
# set_location_assignment PIN_D6 -to hps_memory_mem_dq[7]
# set_location_assignment PIN_H6 -to hps_memory_mem_dq[8]
# set_location_assignment PIN_G6 -to hps_memory_mem_dq[9]
# set_location_assignment PIN_N8 -to hps_memory_mem_dq[10]
# set_location_assignment PIN_G5 -to hps_memory_mem_dq[11]
# set_location_assignment PIN_A4 -to hps_memory_mem_dq[12]
# set_location_assignment PIN_A5 -to hps_memory_mem_dq[13]
# set_location_assignment PIN_R9 -to hps_memory_mem_dq[14]
# set_location_assignment PIN_F4 -to hps_memory_mem_dq[15]
# set_location_assignment PIN_J5 -to hps_memory_mem_dq[16]
# set_location_assignment PIN_K5 -to hps_memory_mem_dq[17]
# set_location_assignment PIN_N7 -to hps_memory_mem_dq[18]
# set_location_assignment PIN_F3 -to hps_memory_mem_dq[19]
# set_location_assignment PIN_H3 -to hps_memory_mem_dq[20]
# set_location_assignment PIN_J4 -to hps_memory_mem_dq[21]
# set_location_assignment PIN_M5 -to hps_memory_mem_dq[22]
# set_location_assignment PIN_C3 -to hps_memory_mem_dq[23]
# set_location_assignment PIN_A2 -to hps_memory_mem_dq[24]
# set_location_assignment PIN_A3 -to hps_memory_mem_dq[25]
# set_location_assignment PIN_P7 -to hps_memory_mem_dq[26]
# set_location_assignment PIN_C1 -to hps_memory_mem_dq[27]
# set_location_assignment PIN_G2 -to hps_memory_mem_dq[28]
# set_location_assignment PIN_F2 -to hps_memory_mem_dq[29]
# set_location_assignment PIN_M3 -to hps_memory_mem_dq[30]
# set_location_assignment PIN_E1 -to hps_memory_mem_dq[31]
# set_location_assignment PIN_G1 -to hps_memory_mem_dq[32]
# set_location_assignment PIN_F1 -to hps_memory_mem_dq[33]
# set_location_assignment PIN_P6 -to hps_memory_mem_dq[34]
# set_location_assignment PIN_L1 -to hps_memory_mem_dq[35]
# set_location_assignment PIN_M2 -to hps_memory_mem_dq[36]
# set_location_assignment PIN_M1 -to hps_memory_mem_dq[37]
# set_location_assignment PIN_N1 -to hps_memory_mem_dq[38]
# set_location_assignment PIN_R6 -to hps_memory_mem_dq[39]
# set_location_assignment PIN_F7 -to hps_memory_mem_dqs[0]
# set_location_assignment PIN_D5 -to hps_memory_mem_dqs[1]
# set_location_assignment PIN_G4 -to hps_memory_mem_dqs[2]
# set_location_assignment PIN_C2 -to hps_memory_mem_dqs[3]
# set_location_assignment PIN_J1 -to hps_memory_mem_dqs[4]
# set_location_assignment PIN_E7 -to hps_memory_mem_dqs_n[0]
# set_location_assignment PIN_E6 -to hps_memory_mem_dqs_n[1]
# set_location_assignment PIN_H4 -to hps_memory_mem_dqs_n[2]
# set_location_assignment PIN_D2 -to hps_memory_mem_dqs_n[3]
# set_location_assignment PIN_H1 -to hps_memory_mem_dqs_n[4]
# set_location_assignment PIN_K9 -to hps_memory_oct_rzqin

set_global_assignment -name ENABLE_SIGNALTAP ON
set_global_assignment -name USE_SIGNALTAP_FILE ../hdl_src/fft_msgdma.stp
set_global_assignment -name SIGNALTAP_FILE ../hdl_src/fft_msgdma.stp
set_global_assignment -name SIGNALTAP_FILE ../hdl_src/validator.stp
