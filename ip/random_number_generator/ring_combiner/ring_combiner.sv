module ring_combiner
#(
	parameter SYNC_DEPTH = 5
)(
	input  wire clk,
	input  wire reset,
	
	input  wire ring_osc_0,
	input  wire ring_osc_1,
	input  wire ring_osc_2,
	
	output reg  xor_out
);

wire ring_osc_0_sync;
wire ring_osc_1_sync;
wire ring_osc_2_sync;

always @(posedge clk or posedge reset) begin
	if(reset) begin
		xor_out <= 1'b0;
	end else begin
		xor_out <= ring_osc_0_sync ^ (ring_osc_1_sync ^ ring_osc_2_sync);
	end
end

altera_std_synchronizer #(
	.depth (SYNC_DEPTH)
) ring_osc_0_synchronizer (
	.clk (clk),
	.reset_n (~reset),
	.din (ring_osc_0),
	.dout (ring_osc_0_sync)
);

altera_std_synchronizer #(
	.depth (SYNC_DEPTH)
) ring_osc_1_synchronizer (
	.clk (clk),
	.reset_n (~reset),
	.din (ring_osc_1),
	.dout (ring_osc_1_sync)
);

altera_std_synchronizer #(
	.depth (SYNC_DEPTH)
) ring_osc_2_synchronizer (
	.clk (clk),
	.reset_n (~reset),
	.din (ring_osc_2),
	.dout (ring_osc_2_sync)
);

endmodule
