module conduit_blender (
		output wire [2:0] in_port,                //    external_connection.export
		input  wire [2:0] out_port,               //                       .export
		output wire       ring_osc_enable,        //        ring_osc_enable.export
		output wire       entropy_counter_enable, // entropy_counter_enable.export
		output wire       entropy_counter_clear   //  entropy_counter_clear.export
	);

	assign in_port = out_port;

	assign ring_osc_enable = out_port[0];

	assign entropy_counter_enable = out_port[1];

	assign entropy_counter_clear = out_port[2];

endmodule

