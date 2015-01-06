module avst_lfsr_packet_gen
(
	input 			clk,
	input			reset,

	//size
	input wire		asi_size_valid,
	input wire [31:0]	asi_size_data,

	//output packet
	output reg		aso_data_valid,
	output reg		aso_data_sop,
	output reg		aso_data_eop,
	output reg [3:0]	aso_data_empty,
	output wire [127:0]	aso_data_data,
	input wire		aso_data_ready,

        //asi exp data
        input                   asi_seed_valid,
        input           [127:0] asi_seed_data,

	output wire [3:0]	aso_status_data	

);

reg [31:0] count, size;
reg [2:0] current_state, next_state;

//generated data
reg [127:0] prbs_generated;

assign aso_data_data = prbs_generated;
assign aso_status_data = current_state;

localparam STATE_READY = 3'h1;
localparam STATE_START = 3'h2;
localparam STATE_BUSY = 3'h4;

always@(posedge reset, posedge clk)
begin
	if(reset)
	begin
		current_state <= STATE_READY;
		size <= 0;
	end
	else
	begin
		current_state <= next_state;
		if(asi_size_valid)
			size <= asi_size_data;
	end
end

always@(aso_data_ready,current_state)
begin
	case(current_state)
		STATE_READY: begin
			if(asi_size_valid)
				next_state <= STATE_START;
			else
				next_state <= STATE_READY;
		end
		
		STATE_START: begin
			if(size > 0)
				next_state <= STATE_BUSY;
			else
				next_state <= STATE_READY;
		end
	
		STATE_BUSY:begin
			if(aso_data_ready & aso_data_valid & count == 1)
				next_state <= STATE_READY;
			else
				next_state <= STATE_BUSY;
		end

		default: next_state <= STATE_READY;
	endcase
end

always@(posedge reset, posedge clk)
begin
	if(reset)
	begin
		aso_data_valid <= 1'b0;
		aso_data_sop <= 1'b0;
		aso_data_eop <= 1'b0;
		aso_data_empty <= 0;
		count <= 0;
	end
	else
	begin
		aso_data_empty <= 0;

		case(current_state)
			STATE_START: begin
				count <= {4'h0,size[31:4]};
				aso_data_sop <= 1'b1;
				aso_data_valid <= 1'b0;
				if(count == 1)
					aso_data_eop <= 1'b1;
				else
					aso_data_eop <= 1'b0;
			end

			STATE_BUSY: begin
				if(count == 1 & aso_data_valid & aso_data_ready)
					aso_data_valid <= 1'b0;
				else
					aso_data_valid <= 1'b1;

				if(aso_data_valid & aso_data_ready)
					aso_data_sop <= 1'b0;
				
				if((aso_data_valid & aso_data_ready & count == 2) | count == 1)
					aso_data_eop <= 1'b1;
				else
					aso_data_eop <= 1'b0;
					
				if (aso_data_valid & aso_data_ready)
					count <= count - 1;

			end
			
			default: begin
				count <= {4'h0,size[31:4]};
				aso_data_valid <= 1'b0;
				aso_data_sop <= 1'b0;
				aso_data_eop <= 1'b0;
			end
		endcase
	end
end

always@(posedge reset, posedge clk)
begin
	if(reset)
	begin
		prbs_generated <= 0;
	end
	else
	begin	
		if (asi_seed_valid)
			prbs_generated <= asi_seed_data;
		else if (aso_data_ready & aso_data_valid)
		begin
			prbs_generated[0] <=  prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[1] <=  prbs_generated[0] ; 
			prbs_generated[2] <=  prbs_generated[1] ; 
			prbs_generated[3] <=  prbs_generated[2] ; 
			prbs_generated[4] <=  prbs_generated[3] ; 
			prbs_generated[5] <=  prbs_generated[4] ; 
			prbs_generated[6] <=  prbs_generated[5] ; 
			prbs_generated[7] <=  prbs_generated[6] ; 
			prbs_generated[8] <=  prbs_generated[0] ^ prbs_generated[6] ; 
			prbs_generated[9] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[6] ; 
			prbs_generated[10] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[6] ; 
			prbs_generated[11] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[12] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[13] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6]; 
			prbs_generated[14] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[15] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[16] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[17] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[18] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[19] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[20] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[4] ; 
			prbs_generated[21] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[22] <=  prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[23] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[24] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[4] ; 
			prbs_generated[25] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[5] ; 
			prbs_generated[26] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[27] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[28] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[29] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[5] ; 
			prbs_generated[30] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[31] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[32] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[33] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[34] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[35] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[36] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[37] <=  prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[38] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[39] <=  prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[40] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[41] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ; 
			prbs_generated[42] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ; 
			prbs_generated[43] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[44] <=  prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[45] <=  prbs_generated[0] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[46] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[5] ; 
			prbs_generated[47] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[6] ; 
			prbs_generated[48] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[49] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[50] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[51] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[52] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[53] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[54] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[55] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[56] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[57] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[58] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[59] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[60] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[61] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[62] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[63] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[64] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[65] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[66] <=  prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[67] <=  prbs_generated[2] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[68] <=  prbs_generated[0] ^ prbs_generated[3] ; 
			prbs_generated[69] <=  prbs_generated[1] ^ prbs_generated[4] ; 
			prbs_generated[70] <=  prbs_generated[2] ^ prbs_generated[5] ; 
			prbs_generated[71] <=  prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[72] <=  prbs_generated[0] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[73] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[74] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ; 
			prbs_generated[75] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ; 
			prbs_generated[76] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[77] <=  prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[78] <=  prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[79] <=  prbs_generated[0] ^ prbs_generated[5] ; 
			prbs_generated[80] <=  prbs_generated[1] ^ prbs_generated[6] ; 
			prbs_generated[81] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[6] ; 
			prbs_generated[82] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[83] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[84] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[85] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[86] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[87] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[88] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[89] <=  prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[90] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[5] ; 
			prbs_generated[91] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[92] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[93] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[94] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[4] ; 
			prbs_generated[95] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[96] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[97] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[98] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[99] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[100] <=  prbs_generated[0] ^ prbs_generated[2] ^ prbs_generated[3] ; 
			prbs_generated[101] <=  prbs_generated[1] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[102] <=  prbs_generated[2] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[103] <=  prbs_generated[3] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[104] <=  prbs_generated[0] ^ prbs_generated[4] ; 
			prbs_generated[105] <=  prbs_generated[1] ^ prbs_generated[5] ; 
			prbs_generated[106] <=  prbs_generated[2] ^ prbs_generated[6] ; 
			prbs_generated[107] <=  prbs_generated[0] ^ prbs_generated[3] ^ prbs_generated[6] ; 
			prbs_generated[108] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[109] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[110] <=  prbs_generated[0] ^ prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ; 
			prbs_generated[111] <=  prbs_generated[1] ^ prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[112] <=  prbs_generated[2] ^ prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[113] <=  prbs_generated[3] ^ prbs_generated[4] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[114] <=  prbs_generated[0] ^ prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[115] <=  prbs_generated[1] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[116] <=  prbs_generated[0] ^ prbs_generated[2] ; 
			prbs_generated[117] <=  prbs_generated[1] ^ prbs_generated[3] ; 
			prbs_generated[118] <=  prbs_generated[2] ^ prbs_generated[4] ; 
			prbs_generated[119] <=  prbs_generated[3] ^ prbs_generated[5] ; 
			prbs_generated[120] <=  prbs_generated[4] ^ prbs_generated[6] ; 
			prbs_generated[121] <=  prbs_generated[0] ^ prbs_generated[5] ^ prbs_generated[6] ; 
			prbs_generated[122] <=  prbs_generated[0] ^ prbs_generated[1] ; 
			prbs_generated[123] <=  prbs_generated[1] ^ prbs_generated[2] ; 
			prbs_generated[124] <=  prbs_generated[2] ^ prbs_generated[3] ; 
			prbs_generated[125] <=  prbs_generated[3] ^ prbs_generated[4] ; 
			prbs_generated[126] <=  prbs_generated[4] ^ prbs_generated[5] ; 
			prbs_generated[127] <=  prbs_generated[5] ^ prbs_generated[6] ;
		end
	end
end

endmodule
	
	
