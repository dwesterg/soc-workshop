module memtest_controller
(
	input			clk,
	input			reset,
	
	//avmm control
        input                   avs_control_read,
        input                   avs_control_write,
        input           [3:0]   avs_control_address,
        input           [31:0]  avs_control_writedata,
        output reg      [31:0]  avs_control_readdata,

	//packet status
	input wire	[3:0]	asi_pktstatus_data,

	//comp status
	input wire	[3:0]	asi_cmpstatus_data,
	input wire	[3:0]	asi_cmppktstatus_data,

	//seed
	output reg		aso_seed_valid,
	output wire [127:0]	aso_seed_data,

	//size
	output reg		aso_size_valid,
	output wire [31:0]	aso_size_data,

	//err
	input wire 		asi_comperr_valid,
	input wire [127:0]	asi_comperr_data
);

/* Control reg */
/* 0 -> Constant output from data gen */

reg     [31:0]  control_reg [15:0];

wire    [31:0] control;
wire    [31:0] size;

reg	[31:0] comb_comperr;

//statys control_reg[0]
//err control_reg[1]
assign size = control_reg[2];
assign control = control_reg[3];
assign aso_seed_data = {control_reg[7],control_reg[6],control_reg[5],control_reg[4]};
assign aso_size_data = size;

always@(asi_comperr_data)
begin
	for(int loop=0; loop < 32; loop = loop +1)
	begin
		comb_comperr[loop] <= asi_comperr_data[loop + 0] || asi_comperr_data[loop + 32] || asi_comperr_data[loop + 64] || asi_comperr_data[loop + 96];
	end
end

always@(posedge reset, posedge clk)
        if(reset)
                control_reg[0] <= 0;
        else
                control_reg[0] <= {16'h0,asi_cmpstatus_data,asi_cmppktstatus_data,4'h0,asi_pktstatus_data};

always@(posedge reset, posedge clk)
        if(reset)
                control_reg[1] <= 0;
        else if(asi_comperr_valid)
                control_reg[1] <= comb_comperr;

genvar i;
generate
for ( i = 2; i < 16; i = i +1)
begin: control_loop
if(i < 8) begin
	always@(posedge reset, posedge clk)
	if(reset)
		control_reg[i] <= 0;
	else if(avs_control_write)
		if(avs_control_address == i)
			control_reg[i] <= avs_control_writedata;
end

if((i > 7) & (i <12)) begin
	always@(posedge reset, posedge clk)
	if(reset)
		control_reg[i] <= 0;
	else
		if(asi_comperr_valid)
			control_reg[i] <= asi_comperr_data[31+32*(i-8):32*(i-8)];
end

if((i > 11) & (i <16)) begin
	always@(posedge reset, posedge clk)
	if(reset)
		control_reg[i] <= 0;
	else
		control_reg[i] <= 0;

end

end
endgenerate


//read
always@(posedge reset, posedge clk)
        if(reset)
                avs_control_readdata <= 0;
        else
                avs_control_readdata <= control_reg[avs_control_address];

always@(posedge reset, posedge clk)
begin
	if(reset)
	begin
		aso_seed_valid <= 1'b0;
		aso_size_valid <= 1'b0;
	end
	else
	begin
		if((avs_control_address == 3'h2) & (avs_control_write))
			aso_size_valid <= 1'b1;
		else
			aso_size_valid <= 1'b0;

		if((((avs_control_address > 4'h3) & (avs_control_address < 4'h9)))  & (avs_control_write))
			aso_seed_valid <= 1'b1;
		else	
			aso_seed_valid <= 1'b0 | control[0];
	end
end
endmodule
