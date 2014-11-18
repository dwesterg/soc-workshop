module two_bit_collector
(
	input  wire       clk,
	input  wire       reset,
	
	input  wire       snk_data,
	input  wire       snk_valid,
	
	output reg  [1:0] src_data,
	output reg        src_valid
);

reg [1:0] state;

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		state <= 2'b00;
		src_data <= 2'b00;
		src_valid <= 1'b0;
	end else begin
		src_valid <= 1'b0;
		if(snk_valid) begin
			if(^state) begin
				src_valid <= 1'b1;
			end
			src_data[0] <= snk_data;
			src_data[1] <= src_data[0];
			
			state[0] <= ~state[1];
			state[1] <= state[0];
		end
	end
end

endmodule
