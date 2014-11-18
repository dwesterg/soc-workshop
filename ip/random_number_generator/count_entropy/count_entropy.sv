module count_entropy
#(
	parameter BIT_WIDTH = 1
)(
	input  wire                 clk,
	input  wire                 reset,
	
	input  wire [BIT_WIDTH-1:0] snk_data,
	input  wire                 snk_valid,
	
	output reg  [BIT_WIDTH-1:0] src_data,
	output reg                  src_valid,
	
	input  wire                 enable,
	input  wire                 clear,
	
	input  wire [BIT_WIDTH-1:0] entropy_address,
	output reg  [31:0]          entropy_readdata,

	output reg  [31:0]          transaction_readdata
);

localparam COUNTER_WIDTH = 24 - log2ceil(BIT_WIDTH);
localparam NUM_COUNTERS = 2**BIT_WIDTH;

reg [COUNTER_WIDTH-1:0] transaction_counter;
reg [COUNTER_WIDTH-1:0] entropy_counter[0:NUM_COUNTERS-1];
reg max_count_reached;

always @ * begin
	src_data <= snk_data;
	src_valid <= snk_valid;
	
	max_count_reached <= (transaction_counter == {COUNTER_WIDTH{1'b1}}) ? {1'b1} : {1'b0};
end

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		transaction_counter <= 0;
		entropy_counter[0:NUM_COUNTERS-1] <= '{NUM_COUNTERS{0}};
	end else begin
		if(enable) begin
			if(snk_valid & ~max_count_reached) begin
				transaction_counter <= transaction_counter + 1;
				entropy_counter[snk_data] <= entropy_counter[snk_data] + 1;
			end
		end else begin
			if(clear) begin
				transaction_counter <= 0;
				entropy_counter[0:NUM_COUNTERS-1] <= '{NUM_COUNTERS{0}};
			end
		end
	end
end

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		entropy_readdata <= 0;
		transaction_readdata <= 0;
	end else begin
		entropy_readdata <= {32'h00000000} | entropy_counter[entropy_address];
		transaction_readdata <= {32'h00000000} | transaction_counter;
	end
end

// --------------------------------------------------
// Ceil(log2()) function log2ceil of 4 = 2
// --------------------------------------------------
function integer log2ceil;
	input reg[63:0] val;
	reg [63:0] i;

	begin
		i = 1;
		log2ceil = 0;

		while (i < val) begin
			log2ceil = log2ceil + 1;
			i = i << 1;
		end
	end
endfunction     

endmodule

