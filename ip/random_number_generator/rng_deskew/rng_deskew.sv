module rng_deskew
(
	input  wire       clk,
	input  wire       reset,
	
	input  wire [1:0] snk_data,
	input  wire       snk_valid,
	
	output reg        src_data,
	output reg        src_valid
);

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		src_data <= 1'b0;
		src_valid <= 1'b0;
	end else begin
		src_valid <= 1'b0;
		if(snk_valid) begin
			if(snk_data == 2'b01) begin
				src_data <= 1'b0;
				src_valid <= 1'b1;
			end
			if(snk_data == 2'b10) begin
				src_data <= 1'b1;
				src_valid <= 1'b1;
			end
		end
	end
end

endmodule
