module bits_to_bytes
(
	input wire        clk,
	input wire        reset,
	
	input  wire [7:0] snk_data,
	input  wire       snk_valid,
	output wire       snk_ready,
	
	output wire [7:0] src_data,
	output wire       src_valid,
	input  wire       src_ready
);


assign snk_ready = src_ready;
assign src_valid = snk_valid;
assign src_data = snk_data;

endmodule
