`ifdef NOVTECH_NOVSOMCV_LITE
  `define NO_QSPI
`elsif DE0_NANO_SOC
  `define NO_QSPI
`endif

module ghrd_top 
#(
        parameter MEM_A_WIDTH,
        parameter MEM_D_WIDTH,
        parameter MEM_BA_WIDTH
)
(
    // HPS memory controller ports
	output wire [MEM_A_WIDTH - 1:0] 	hps_memory_mem_a,                           
	output wire [MEM_BA_WIDTH - 1:0]	hps_memory_mem_ba,                          
	output wire       			hps_memory_mem_ck,                          
	output wire        			hps_memory_mem_ck_n,                        
	output wire        			hps_memory_mem_cke,                         
	output wire        			hps_memory_mem_cs_n,                        
	output wire        			hps_memory_mem_ras_n,                       
	output wire        			hps_memory_mem_cas_n,                       
	output wire        			hps_memory_mem_we_n,                        
	output wire        			hps_memory_mem_reset_n,                     
	inout  wire [MEM_D_WIDTH - 1:0]		hps_memory_mem_dq,                          
	inout  wire [(MEM_D_WIDTH/8) -1:0]	hps_memory_mem_dqs,                         
	inout  wire [(MEM_D_WIDTH/8) -1:0]	hps_memory_mem_dqs_n,                       
	output wire        			hps_memory_mem_odt,                         
	output wire [(MEM_D_WIDTH/8) -1:0] 	hps_memory_mem_dm,                          
	input  wire        			hps_memory_oct_rzqin,                       
    // HPS peripherals
	output wire        hps_emac1_TX_CLK,   
	output wire        hps_emac1_TXD0,     
	output wire        hps_emac1_TXD1,     
	output wire        hps_emac1_TXD2,     
	output wire        hps_emac1_TXD3,     
	input  wire        hps_emac1_RXD0,     
	inout  wire        hps_emac1_MDIO,     
	output wire        hps_emac1_MDC,      
	input  wire        hps_emac1_RX_CTL,   
	output wire        hps_emac1_TX_CTL,   
	input  wire        hps_emac1_RX_CLK,   
	input  wire        hps_emac1_RXD1,     
	input  wire        hps_emac1_RXD2,     
	input  wire        hps_emac1_RXD3, 
`ifdef NO_QSPI
`else
	inout  wire        hps_qspi_IO0,       
	inout  wire        hps_qspi_IO1,       
	inout  wire        hps_qspi_IO2,       
	inout  wire        hps_qspi_IO3,       
	output wire        hps_qspi_SS0,       
	output wire        hps_qspi_CLK,
`endif
`ifdef CRITICALLINK_MITYSOM_DEVKIT
	inout wire        hps_gpio_GPIO00,
	inout wire        hps_gpio_GPIO09,
	inout wire        hps_gpio_GPIO28,
`endif
	inout  wire        hps_sdio_CMD,       
	inout  wire        hps_sdio_D0,        
	inout  wire        hps_sdio_D1,        
	output wire        hps_sdio_CLK,       
	inout  wire        hps_sdio_D2,        
	inout  wire        hps_sdio_D3,        
	inout  wire        hps_usb1_D0,        
	inout  wire        hps_usb1_D1,        
	inout  wire        hps_usb1_D2,        
	inout  wire        hps_usb1_D3,        
	inout  wire        hps_usb1_D4,        
	inout  wire        hps_usb1_D5,        
	inout  wire        hps_usb1_D6,        
	inout  wire        hps_usb1_D7,        
	input  wire        hps_usb1_CLK,       
	output wire        hps_usb1_STP,       
	input  wire        hps_usb1_DIR,       
	input  wire        hps_usb1_NXT,       
	input  wire        hps_uart0_RX,       
	output wire        hps_uart0_TX       
);


// SoC sub-system module

soc_system soc_inst (
  .memory_mem_a                          (hps_memory_mem_a),                               
  .memory_mem_ba                         (hps_memory_mem_ba),                         
  .memory_mem_ck                         (hps_memory_mem_ck),                         
  .memory_mem_ck_n                       (hps_memory_mem_ck_n),                       
  .memory_mem_cke                        (hps_memory_mem_cke),                        
  .memory_mem_cs_n                       (hps_memory_mem_cs_n),                       
  .memory_mem_ras_n                      (hps_memory_mem_ras_n),                      
  .memory_mem_cas_n                      (hps_memory_mem_cas_n),                      
  .memory_mem_we_n                       (hps_memory_mem_we_n),                       
  .memory_mem_reset_n                    (hps_memory_mem_reset_n),                    
  .memory_mem_dq                         (hps_memory_mem_dq),                         
  .memory_mem_dqs                        (hps_memory_mem_dqs),                        
  .memory_mem_dqs_n                      (hps_memory_mem_dqs_n),                      
  .memory_mem_odt                        (hps_memory_mem_odt),                            
  .memory_mem_dm                         (hps_memory_mem_dm),                         
  .memory_oct_rzqin                      (hps_memory_oct_rzqin),                      
  .hps_0_hps_io_hps_io_emac1_inst_TX_CLK (hps_emac1_TX_CLK), 
  .hps_0_hps_io_hps_io_emac1_inst_TXD0   (hps_emac1_TXD0),   
  .hps_0_hps_io_hps_io_emac1_inst_TXD1   (hps_emac1_TXD1),   
  .hps_0_hps_io_hps_io_emac1_inst_TXD2   (hps_emac1_TXD2),   
  .hps_0_hps_io_hps_io_emac1_inst_TXD3   (hps_emac1_TXD3),   
  .hps_0_hps_io_hps_io_emac1_inst_RXD0   (hps_emac1_RXD0),   
  .hps_0_hps_io_hps_io_emac1_inst_MDIO   (hps_emac1_MDIO),   
  .hps_0_hps_io_hps_io_emac1_inst_MDC    (hps_emac1_MDC),    
  .hps_0_hps_io_hps_io_emac1_inst_RX_CTL (hps_emac1_RX_CTL), 
  .hps_0_hps_io_hps_io_emac1_inst_TX_CTL (hps_emac1_TX_CTL), 
  .hps_0_hps_io_hps_io_emac1_inst_RX_CLK (hps_emac1_RX_CLK), 
  .hps_0_hps_io_hps_io_emac1_inst_RXD1   (hps_emac1_RXD1),   
  .hps_0_hps_io_hps_io_emac1_inst_RXD2   (hps_emac1_RXD2),   
  .hps_0_hps_io_hps_io_emac1_inst_RXD3   (hps_emac1_RXD3),
`ifdef NO_QSPI
`else  
  .hps_0_hps_io_hps_io_qspi_inst_IO0     (hps_qspi_IO0),     
  .hps_0_hps_io_hps_io_qspi_inst_IO1     (hps_qspi_IO1),     
  .hps_0_hps_io_hps_io_qspi_inst_IO2     (hps_qspi_IO2),     
  .hps_0_hps_io_hps_io_qspi_inst_IO3     (hps_qspi_IO3),     
  .hps_0_hps_io_hps_io_qspi_inst_SS0     (hps_qspi_SS0),     
  .hps_0_hps_io_hps_io_qspi_inst_CLK     (hps_qspi_CLK),
`endif
  .hps_0_hps_io_hps_io_sdio_inst_CMD     (hps_sdio_CMD),     
  .hps_0_hps_io_hps_io_sdio_inst_D0      (hps_sdio_D0),      
  .hps_0_hps_io_hps_io_sdio_inst_D1      (hps_sdio_D1),      
  .hps_0_hps_io_hps_io_sdio_inst_CLK     (hps_sdio_CLK),     
  .hps_0_hps_io_hps_io_sdio_inst_D2      (hps_sdio_D2),      
  .hps_0_hps_io_hps_io_sdio_inst_D3      (hps_sdio_D3),      
  .hps_0_hps_io_hps_io_usb1_inst_D0      (hps_usb1_D0),      
  .hps_0_hps_io_hps_io_usb1_inst_D1      (hps_usb1_D1),      
  .hps_0_hps_io_hps_io_usb1_inst_D2      (hps_usb1_D2),      
  .hps_0_hps_io_hps_io_usb1_inst_D3      (hps_usb1_D3),      
  .hps_0_hps_io_hps_io_usb1_inst_D4      (hps_usb1_D4),      
  .hps_0_hps_io_hps_io_usb1_inst_D5      (hps_usb1_D5),      
  .hps_0_hps_io_hps_io_usb1_inst_D6      (hps_usb1_D6),      
  .hps_0_hps_io_hps_io_usb1_inst_D7      (hps_usb1_D7),      
  .hps_0_hps_io_hps_io_usb1_inst_CLK     (hps_usb1_CLK),     
  .hps_0_hps_io_hps_io_usb1_inst_STP     (hps_usb1_STP),     
  .hps_0_hps_io_hps_io_usb1_inst_DIR     (hps_usb1_DIR),     
  .hps_0_hps_io_hps_io_usb1_inst_NXT     (hps_usb1_NXT),     
  .hps_0_hps_io_hps_io_uart0_inst_RX     (hps_uart0_RX),     
  .hps_0_hps_io_hps_io_uart0_inst_TX     (hps_uart0_TX),     
`ifdef CRITICALLINK_MITYSOM_DEVKIT
  .hps_0_hps_io_hps_io_gpio_inst_GPIO00  (hps_gpio_GPIO00),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO09  (hps_gpio_GPIO09),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO28  (hps_gpio_GPIO28),
`endif 
  .hps_0_h2f_reset_reset_n               (),
  .hps_0_h2f_clk_clk                     ()
);  

endmodule

/*
#(
        parameter MEM_A_WIDTH = 15,
        parameter MEM_D_WIDTH = 40,
        parameter MEM_BA_WIDTH = 3
)
*/
