module avst_compare
(
	input			clk,
	input			reset,

	//avst data
	input wire		asi_indata_valid,
	output reg		asi_indata_ready,
	input wire [127:0]	asi_indata_data,
	input wire		asi_indata_sop,
	input wire		asi_indata_eop,

	//avst compdata
	input wire		asi_compdata_valid,
	output reg		asi_compdata_ready,
	input wire [127:0]	asi_compdata_data,
	input wire		asi_compdata_sop,
	input wire		asi_compdata_eop,

	//output err data
	output reg		aso_err_valid,
	output wire [127:0]	aso_err_data,

	output wire [3:0]	aso_status_data
);


reg [2:0] current_state, next_state;
reg [127:0] bitfailure;

assign aso_err_data = bitfailure;
assign aso_status_data = current_state;

wire valid_data;
assign valid_data = asi_indata_valid & asi_indata_ready & asi_compdata_valid & asi_compdata_ready;

localparam STATE_INIT = 3'h1;
localparam STATE_COMP = 3'h2;
localparam STATE_DONE = 3'h4;

always@(posedge reset, posedge clk)
begin
	if(reset)
		current_state <= STATE_INIT;
	else
		current_state <= next_state;
end

always@(*)
begin
	case(current_state)
		STATE_INIT: begin
			if(asi_indata_valid & asi_compdata_valid)
				next_state <= STATE_COMP;
			else
				next_state <= STATE_INIT;
		end

		STATE_COMP: begin
			if(asi_indata_eop & asi_indata_valid & asi_indata_ready)
				next_state <= STATE_DONE;
			else
				next_state <= STATE_COMP;
		end

		STATE_DONE: begin
			next_state <= STATE_INIT;
		end
	endcase
end

always@(posedge reset, posedge clk)
begin
	if(reset)
	begin
		asi_compdata_ready <= 1'b0;
		asi_indata_ready <= 1'b0;
		aso_err_valid <= 1'b0;
	end
	else
	begin
		case(current_state)
			STATE_INIT: begin
				asi_indata_ready	<= 1'b0;
				asi_compdata_ready	<= 1'b0;
				aso_err_valid 		<= 1'b0;
			end

			STATE_COMP: begin
				asi_indata_ready 	<= asi_indata_valid && asi_compdata_valid;
				asi_compdata_ready	<= asi_indata_valid && asi_compdata_valid;
				aso_err_valid <= 1'b0;
			end

			STATE_DONE: begin
				asi_indata_ready	<= 1'b0;
				asi_compdata_ready	<= 1'b0;
				aso_err_valid 		<= 1'b1;
			end
		endcase
	end
end

genvar i;
generate
for ( i = 0; i < 128; i = i + 1)
begin: check_data

        always@(posedge reset, posedge clk)
        if(reset)
        	bitfailure[i] 		<= 0;
        else
	begin
		case(current_state)
			STATE_INIT: begin
				bitfailure[i]   <= 0;
			end
			STATE_COMP: begin
				if(valid_data)
					 bitfailure[i] <= (asi_indata_data[i] ^ asi_compdata_data[i]) | bitfailure[i];
			end

			default: begin
				bitfailure[i] <= bitfailure[i];
			end
		endcase
	end
end
endgenerate

endmodule
