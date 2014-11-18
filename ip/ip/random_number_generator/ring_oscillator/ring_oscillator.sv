module ring_oscillator
#(
	parameter BIT_WIDTH = 100
)(
	input  wire clk,
	input  wire reset,
	
	input  wire enable_osc,
	
	output reg  clk_out
);

reg enable;
(*keep*) reg [BIT_WIDTH-1 : 0] ring;

genvar i;
generate

for(i = 0; i < (BIT_WIDTH-1); i=i+1) begin : inverter_ring
	always @ * begin
		ring[i+1] = ~ring[i];
	end
end

if(i & 1) begin
	always @ * begin
		ring[0] = (enable) ? (ring[i]) : (1'b0);
	end
end else begin
	always @ * begin
		ring[0] = (enable) ? (~ring[i]) : (1'b0);
	end
end

always @ * begin
	clk_out = ring[i];
end

endgenerate

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		enable <= 1'b0;
	end else begin
		enable <= enable_osc;
	end
end

endmodule

