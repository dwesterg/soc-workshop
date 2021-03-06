set devicefamily CYCLONEV
set device 5CSXFC6C6U23C7

#set qipfiles "../ip/altsource_probe/hps_reset.qip,../ip/fft128/fft_test.qip,../ip/demo_driver/meminit.qip,../ip/workshop_validator/meminit.qip"
set qipfiles "../ip/altsource_probe/hps_reset.qip,../ip/demo_driver/meminit.qip,../ip/workshop_validator/meminit.qip"
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

set_global_assignment -name BLOCK_RAM_TO_MLAB_CELL_CONVERSION OFF

set_parameter -name MEM_A_WIDTH 15
set_parameter -name MEM_BA_WIDTH 3
set_parameter -name MEM_D_WIDTH 40

# Setup Bank Voltages
set_global_assignment -name IOBANK_VCCIO 1.35V -section_id 3A
set_global_assignment -name IOBANK_VCCIO 2.5V -section_id 3B
set_global_assignment -name IOBANK_VCCIO 2.5V -section_id 4A
set_global_assignment -name IOBANK_VCCIO 1.35V -section_id 5A
set_global_assignment -name IOBANK_VCCIO 1.35V -section_id 5B
set_global_assignment -name IOBANK_VCCIO 1.35V -section_id 6A
set_global_assignment -name IOBANK_VCCIO 1.35V -section_id 6B
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 7A
set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 7B
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 7C
set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 7D
set_global_assignment -name IOBANK_VCCIO 2.5V -section_id 8A


set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_MDC
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_MDIO
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RXD0
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RXD1
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RXD2
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RXD3
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RX_CLK
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_RX_CTL
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TXD0
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TXD3
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TXD1
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TXD2
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TX_CLK
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_emac1_TX_CTL
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_CLK
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_IO0
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_IO1
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_IO2
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_IO3
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_qspi_SS0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_CLK
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_CMD
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D0
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D1
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D2
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_sdio_D3
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_uart0_RX
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to hps_uart0_TX
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_CLK
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D0
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D1
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D2
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D3
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D4
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D5
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D6
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_D7
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_DIR
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_NXT
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_usb1_STP

set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_gpio_GPIO00
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_gpio_GPIO09
set_instance_assignment -name IO_STANDARD "1.8 V" -to hps_gpio_GPIO28

set_global_assignment -name ENABLE_SIGNALTAP OFF
set_global_assignment -name USE_SIGNALTAP_FILE ../hdl_src/fft_msgdma.stp
set_global_assignment -name SIGNALTAP_FILE ../hdl_src/fft_msgdma.stp
set_global_assignment -name SIGNALTAP_FILE ../hdl_src/validator.stp
