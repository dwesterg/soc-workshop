module count_entropy_master
(
	input  wire        clk,
	input  wire        reset,
	
	input  wire [7:0]  snk_data,
	input  wire        snk_valid,
	
	output reg  [7:0]  src_data,
	output reg         src_valid,
	
	input  wire        enable,
	input  wire        clear,

	output reg  [31:0] transaction_readdata,
	
	output reg  [9:0]  avm_address,
	output reg  [3:0]  avm_byteenable,
	output reg  [31:0] avm_writedata,
	input  wire [31:0] avm_readdata,
	output reg         avm_write,
	output reg         avm_read,
	input  wire        avm_waitrequest,
	input  wire        avm_readdatavalid
);

localparam COUNTER_WIDTH = 32;

reg [COUNTER_WIDTH-1:0] transaction_counter;
reg max_count_reached;

always @ * begin
	src_data <= snk_data;
	src_valid <= snk_valid;
	
	max_count_reached <= (transaction_counter == {COUNTER_WIDTH{1'b1}}) ? {1'b1} : {1'b0};
end

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		transaction_counter <= 0;
	end else begin
		if(enable) begin
			if(snk_valid & ~max_count_reached) begin
				transaction_counter <= transaction_counter + 1;
			end
		end else begin
			if(clear) begin
				transaction_counter <= 0;
			end
		end
	end
end

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		transaction_readdata <= 0;
	end else begin
		transaction_readdata <= {32'h00000000} | transaction_counter;
	end
end

always @ (posedge clk or posedge reset) begin
	if(reset) begin
		avm_address <= 0;
		avm_byteenable <= 0;
		avm_writedata <= 0;
		avm_write <= 0;
		avm_read <= 0;
	end else begin
		if(enable) begin
			if(snk_valid & ~max_count_reached) begin
				avm_address <= {snk_data, 2'b00};
				avm_byteenable <= 4'hF;
				avm_read <= 1'b1;
			end
		end
		if(avm_read) begin
			avm_read <= avm_waitrequest;
		end
		if(avm_readdatavalid) begin
			avm_writedata <= avm_readdata + 32'h00000001;
			avm_write <= 1'b1;
		end
		if(avm_write) begin
			avm_write <= avm_waitrequest;
		end
	end
end

endmodule

