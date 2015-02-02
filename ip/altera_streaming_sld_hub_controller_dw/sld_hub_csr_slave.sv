module sld_hub_csr_slave(
  output reg enable_user_jtag_io,

  input wire csr_write,
  input wire csr_read,
  input wire [0:0] csr_addr,
  input wire [31:0] csr_writedata,
  output wire [31:0] csr_readdata,

  input wire clk,
  input wire reset
);

  assign csr_readdata = { {31 {1'b0}}, enable_user_jtag_io};

  always @(posedge clk or posedge reset) begin
    if (reset) begin
      // 1: user I/O; 0: jsm
      enable_user_jtag_io <= '0;
    end
    else begin
      if (csr_write)
        enable_user_jtag_io <= csr_writedata[0];
    end
  end

endmodule

