// (C) 2001-2015 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// $Id: //acds/rel/14.1/ip/sld/st/altera_streaming_sld_hub_controller/altera_streaming_sld_hub_controller_tck_generation.sv#1 $
// $Revision: #1 $
// $Date: 2014/10/06 $
// $Author: swbranch $

`default_nettype none
module altera_streaming_sld_hub_controller_tck_generation #(
    parameter DEVICE_FAMILY = "Stratix IV",
    parameter USE_GLOBAL_CLOCK = 1,
    parameter USE_GLOBAL_PRIMITIVE = 0
)
(
  input wire clk,
  input wire tck_enable,
  output wire tck
);

  generate begin: tck_generation
    if (USE_GLOBAL_CLOCK) begin : use_global_clock
      if (USE_GLOBAL_PRIMITIVE) begin : use_global_primitive
        default_tck_generation #(
          .USE_GLOBAL_PRIMITIVE (1)
        ) 
          the_altclkctrl(
            .ena (tck_enable),
            .inclk (clk),
            .outclk (tck)
        );
      end
      else begin : use_altclkctrl
        case (DEVICE_FAMILY)
          "Arria II GZ" : begin : arriaiigz
            altera_streaming_sld_hub_controller_arriaiigz_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Arria V" : begin : arriav
            altera_streaming_sld_hub_controller_arriav_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Cyclone III": begin : cycloneiii
            altera_streaming_sld_hub_controller_cycloneiii_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Cyclone IV E" : begin : cycloneive
            altera_streaming_sld_hub_controller_cycloneive_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Cyclone V" : begin : cyclonev
            altera_streaming_sld_hub_controller_cyclonev_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Arria II GX" : begin : arriaiigx
            altera_streaming_sld_hub_controller_arriaiigx_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Cyclone IV GX" : begin : cycloneivgx
            altera_streaming_sld_hub_controller_cycloneivgx_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Stratix V" : begin : stratixv
            altera_streaming_sld_hub_controller_stratixv_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Cyclone III LS" : begin : cycloneiiils
            altera_streaming_sld_hub_controller_cycloneiiils_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Stratix IV": begin : stratixiv
            altera_streaming_sld_hub_controller_stratixiv_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
          "Stratix III" : begin : stratixiii
            altera_streaming_sld_hub_controller_stratixiii_altclkctrl the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end

          default: begin : default_tck
            default_tck_generation the_altclkctrl(
              .ena (tck_enable),
              .inclk (clk),
              .outclk (tck)
            );
          end
        endcase
      end
    end
    else begin : no_global_clock
      default_tck_generation the_altclkctrl(
        .ena (tck_enable),
        .inclk (clk),
        .outclk (tck)
      );
    end
  end endgenerate

endmodule

module default_tck_generation #(
  parameter USE_GLOBAL_PRIMITIVE = 0
)
(
  input wire ena,
  input wire inclk,
  output wire outclk
);
  wire enabled_tck;
  assign enabled_tck = ena & ~inclk;
  generate 
    if (USE_GLOBAL_PRIMITIVE) begin : use_global_primitive
      // Nb. This primitive doesn't seem to force outclk into a global clock
      // resource; it also breaks the simulation.  For now, don't enable this.
      GLOBAL the_global (.in(enabled_tck), .out(outclk));
    end
    else begin : no_global_primitive
      assign outclk = enabled_tck;
    end
  endgenerate
endmodule


`default_nettype wire

