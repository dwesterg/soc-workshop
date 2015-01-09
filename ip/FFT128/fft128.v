// fft128.v



`timescale 1 ps / 1 ps
module fft_adapter (
		// to the fft
		input  wire [31:0] asi_in0_data,   //   in0.data
		output wire        asi_in0_ready,  //      .ready
		input  wire        asi_in0_valid,  //      .valid
		input  wire        asi_in0_startofpacket,  //      .start of packet
		input  wire        asi_in0_endofpacket,  //      .end of packet
		input  wire [1:0]  asi_in0_error,// 	.error
		input wire [1:0]   asi_in0_empty,// empty  // really don't handle but this reduces the warnings. 

		input  wire        clk,            // clock.clk
		input  wire        reset,          // reset.reset
		//from the fft
		output wire [63:0] aso_out0_data,  //  out0.data
		input  wire        aso_out0_ready, //      .ready
		output wire        aso_out0_valid,  //      .valid
		output  wire        aso_out0_startofpacket,  //      .start of packet
		output  wire        aso_out0_endofpacket,  //      .end of packet
		output  wire [1:0]  aso_out0_error,// 	.error
		output wire [2:0]   aso_out0_empty // empty  // really don't handle but this reduces the warnings. 
	
	);


	wire	[23:0]	source_real;
	wire	[23:0]	source_imag;
		// 64 bit output just need to sign extend the upper bits.

	assign  aso_out0_data = {{8{source_real[23]}},source_real,{8{source_imag[23]}},source_imag};  //  out0.data

	assign  aso_out0_empty = 3'b000;

	// megawizard function for a 128 bit fft
      fft_test the_fft_test(
	.clk 		(clk),
	.reset_n	(!reset),
	.fftpts_in	(8'b10000000),  //128 from table 3.2
	.inverse	(1'b0),
	.sink_valid	(asi_in0_valid),
	.sink_sop	(asi_in0_startofpacket),
	.sink_eop	(asi_in0_endofpacket),
	.sink_real	(asi_in0_data[31:16]),
	.sink_imag	(asi_in0_data[15:0]),
	.sink_error	(asi_in0_error),
	.source_ready	(aso_out0_ready),
	.fftpts_out	(),
	.sink_ready	(asi_in0_ready),
	.source_error	(aso_out0_error),
	.source_sop	(aso_out0_startofpacket),
	.source_eop	(aso_out0_endofpacket),
	.source_valid	(aso_out0_valid),
	.source_real	(source_real),
	.source_imag	(source_imag));

endmodule
