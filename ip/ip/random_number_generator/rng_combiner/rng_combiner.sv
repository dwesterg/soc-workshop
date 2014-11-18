module rng_combiner
(
	input  wire clk,
	input  wire reset,
	
	input  wire asi_in0_data,
	input  wire asi_in0_valid,
	output reg  asi_in0_ready,
	
	input  wire asi_in1_data,
	input  wire asi_in1_valid,
	output reg  asi_in1_ready,
	
	output reg  aso_out0_data,
	output reg  aso_out0_valid
);

reg both_valid;

always @ * begin
	both_valid <= asi_in0_valid & asi_in1_valid;
	asi_in0_ready <= both_valid;
	asi_in1_ready <= both_valid;
end

always @(posedge clk or posedge reset) begin
	if(reset) begin		
		aso_out0_data <= 1'b0;
		aso_out0_valid <= 1'b0;
	end else begin
		aso_out0_data <= asi_in0_data ^ asi_in1_data;
		aso_out0_valid <= 1'b0;
		if(both_valid) begin
			aso_out0_valid <= 1'b1;
		end
	end
end

endmodule

