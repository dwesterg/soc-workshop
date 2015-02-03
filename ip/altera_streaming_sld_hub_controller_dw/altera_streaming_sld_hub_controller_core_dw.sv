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


// $Id: //acds/rel/14.1/ip/sld/st/altera_streaming_sld_hub_controller/altera_streaming_sld_hub_controller_core.sv#1 $
// $Revision: #1 $
// $Date: 2014/10/06 $
// $Author: swbuild $

`default_nettype none
`timescale 1 ns / 1 ns
module altera_streaming_sld_hub_controller_core_dw #(
    parameter DEVICE_FAMILY = "Stratix IV",
              ENABLE_JTAG_IO_SELECTION = 0,
              ST_DATA_W = 8 // hidden parameter
  )
  (
  // Avalon-ST sink (cmd)
  input  wire       cmd_valid,
  input  wire       cmd_sop,
  input  wire       cmd_eop,
  input  wire [ST_DATA_W - 1 : 0] cmd_data,
  output reg        cmd_ready,

  // Avalon-ST source (resp)
  output reg       resp_valid,
  output reg       resp_sop,
  output reg       resp_eop,
  output reg [ST_DATA_W - 1 : 0] resp_data,
  input  wire      resp_ready,

  // jtag io selection conduit
  // Avalon-MM slave (csr)
//  input wire csr_write,
//  input wire csr_read,
//  input wire [0:0] csr_addr,
//  input wire [31:0] csr_writedata,
//  output wire [31:0] csr_readdata,
  input wire enable_user_jtag_io,
  
  // Clock sink
  input wire clk,
  // Reset sink
  input wire reset
);

  // Registered version of cmd_data, captured upon (cmd_valid & cmd_ready).
  logic [ST_DATA_W - 1 : 0] cmd_data_reg;

  typedef enum logic [2:0] {
    CMD_CONFIG                    = 3'd0,
    CMD_WRITE_TDO_ENABLE_FIFO     = 3'd2,
    CMD_SHORT_CUSTOM_TMS_TDI      = 3'd4,
    CMD_LONG_FIXED_TMS_CUSTOM_TDI = 3'd5,
    CMD_LONG_FIXED_TMS_TDI        = 3'd6
  } t_cmd;
  t_cmd cmd;
  assign cmd = t_cmd'(cmd_data[5 +: 3]);

  // Remember what command code is being processed; captured on exit from
  // ST_IDLE.  This value persists throughout the processing of a single
  // command, allowing the FSM to do slightly different actions for slightly
  // different commands.
  t_cmd cmd_reg;

// To do: when it comes time to implement FIFO reset, config_subcmd_code will
// be needed. Comment it out for now, since it's causing a qis warnings
// failure.
  typedef enum logic [1:0] {
    CMD_CONFIG_RETRIEVE_INFO       = 2'b00,
    CMD_RESET_TDO_ENABLE_FIFO      = 2'b01
  } t_config_subcmd;
  t_config_subcmd config_subcmd_code;
  assign config_subcmd_code = t_config_subcmd'(cmd_data_reg[1:0]);

  logic cmd_data_reg_clken;
  logic cmd_clken;

  typedef enum int unsigned {
    ST_FAIL,
    ST_GOTO_RTI,
    ST_DO_VARIABLE_TDI_SHIFT,
    ST_DO_VARIABLE_TMS_TDI_SHIFT,
    ST_GET_TDI_BYTE,
    ST_GET_TMS_TDI_BYTE,
    ST_CMD2,
    ST_CMD2_FIFO_WRITE,
    ST_CMD3_FIFO_WRITE,
    ST_CMD1,
    ST_IDLE
  } t_state;
  t_state state, p1_state;

  logic [5:0] dr_length;
  assign dr_length = cmd_data_reg[5:0];

  logic p1_cmd_ready;

  logic [3:0] shift_counter, p1_shift_counter;
  logic [12:0] bit_counter, p1_bit_counter;
  logic [13:0] tms_shifter, p1_tms_shifter;
  logic [13:0] tdi_shifter, p1_tdi_shifter;
  // can_capture_tdo is decoupled from tck_enable; at present there's no
  // latency between tck_enable and can_capture_tdo, but if sld hub clocking
  // changed, there could be.
  logic tck_enable, p1_tck_enable, can_capture_tdo;
  assign can_capture_tdo = tck_enable;

  localparam FIFO_WIDTH = 15;
  logic [FIFO_WIDTH-3:0] p1_tdo_enable_fifo_duration, tdo_enable_fifo_duration;

  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      shift_counter <= '0;
      bit_counter <= '0;
      tms_shifter <= '0;
      tdi_shifter <= '0;
      tck_enable <= '0;
      tdo_enable_fifo_duration <= '0;
    end 
    else begin
      shift_counter <= p1_shift_counter;
      bit_counter <= p1_bit_counter;
      tms_shifter <= p1_tms_shifter;
      tdi_shifter <= p1_tdi_shifter;
      tck_enable <= p1_tck_enable;
      tdo_enable_fifo_duration <= p1_tdo_enable_fifo_duration;
    end
  end

  wire tms, tdo, tdi, tck;

  altera_streaming_sld_hub_controller_tck_generation #(
    .USE_GLOBAL_CLOCK (1),
    .USE_GLOBAL_PRIMITIVE (0),
    .DEVICE_FAMILY (DEVICE_FAMILY)
  ) 
  tck_gen(
    .tck_enable (tck_enable),
    .clk (clk),
    .tck (tck)
  );

  logic tdo_enable_fifo_write;
  logic tdo_enable_fifo_reset;
  logic enable_tdo_capture;
  logic fifo_enable_tdo_capture;
  logic set_resp_eop_from_fifo;
  assign enable_tdo_capture = fifo_enable_tdo_capture & can_capture_tdo;
  altera_streaming_sld_hub_controller_fifo #(
    .WIDTH (FIFO_WIDTH)
  ) tdo_enable_fifo(
    .tdo_enable (fifo_enable_tdo_capture),
    .set_eop (set_resp_eop_from_fifo),
    .write (tdo_enable_fifo_write),
    .dec (can_capture_tdo),
    // The lower 13 bits of duration have to be saved from earlier data bytes, 
    // but the top bits can feed directly from cmd_data_reg.
    .duration_in ({cmd_data_reg[1:0], tdo_enable_fifo_duration}),
    .eop_gen_in (cmd_data_reg[6]),
    .tdo_enable_in (cmd_data_reg[7]),

    .clk (clk),
    .soft_reset (tdo_enable_fifo_reset),
    .reset (reset)
  );

/*  wire enable_user_jtag_io;
  csr_slave the_csr_slave(
    .enable_user_jtag_io (enable_user_jtag_io),
    .csr_write (csr_write),
    .csr_read (csr_read),
    .csr_addr (csr_addr),
    .csr_writedata (csr_writedata),
    .csr_readdata (csr_readdata),
    .clk (clk),
    .reset (reset)
  );
*/

  assign tms = tms_shifter[0];
  assign tdi = tdi_shifter[0];
  altera_streaming_sld_hub_controller_sld_node #(
      .ENABLE_JTAG_IO_SELECTION (ENABLE_JTAG_IO_SELECTION)
    )
    the_node (
      .tck (tck),
      .tms (tms),
      .tdi (tdi),
      .select_me (enable_user_jtag_io),
      .tdo (tdo)
    );
  logic response_beat;
  logic pending_response_sop_set;
  logic trigger_tdo_response;
  logic trigger_config_response;

  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      resp_sop   <= '0;
      resp_eop   <= '0;
    end 
    else begin
      // Response sop generation.  Simple, if a beat with sop is received, 
      // drive response sop until the response beat completes.
      if (pending_response_sop_set)
        resp_sop <= '1;
      else if (response_beat)
        resp_sop <= '0;

      // Actually drive out a response eop.
      if (trigger_config_response | set_resp_eop_from_fifo)
        resp_eop <= '1;
      else if (response_beat)
        resp_eop <= '0;

      if (cmd_data_reg_clken)
        cmd_data_reg <= cmd_data;

      if (cmd_clken)
        cmd_reg <= cmd;
    end
  end

  // Note: with the current settings, the config-retrieve-info response is
  // '0. This is a nice coincidence, as most other potential values for
  // resp_data are also 0. I take advantage of this below in the assignment to
  // resp_data; when CONFIG_RESPONSE_RESERVED or VERSION changes, that
  // assignment must be revisited.
  localparam CONFIG_RESPONSE_RESERVED = 4'h0;
  localparam VERSION = 4'h0;

  logic clear_tdo_bit_select = '0; // TODO: set on response valid and ready?

  logic [ST_DATA_W - 1 : 0] tdo_bit_select;
  logic [ST_DATA_W - 1 : 0] tdo_shifter;
  logic response_pending;
  logic d1_fifo_enable_tdo_capture;
  assign trigger_tdo_response = 
    // We're capturing the MSB of response data.  Trigger the response
    // state machine to transmit the byte.
    enable_tdo_capture & tdo_bit_select[ST_DATA_W-1] | 
    // some tdo data is captured, and tdo-capture just went false.
    response_pending & ~fifo_enable_tdo_capture & d1_fifo_enable_tdo_capture;

  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      tdo_bit_select <= {{ST_DATA_W - 1 {1'b0}}, 1'b1};
      clear_tdo_bit_select <= '0;
      response_pending <= '0;
      d1_fifo_enable_tdo_capture <= '0;
    end
    else begin
      clear_tdo_bit_select <= trigger_tdo_response | trigger_config_response;
      if (clear_tdo_bit_select) begin
        // reset tdo capture back to the LSB.
        tdo_bit_select <= {{ST_DATA_W - 1 {1'b0}}, 1'b1};
      end
      else if (enable_tdo_capture) begin
        // rotate-left tdo bit select
        tdo_bit_select <=
          {tdo_bit_select[ST_DATA_W-2:0], tdo_bit_select[ST_DATA_W-1]};
      end

      if (enable_tdo_capture)
        response_pending <= '1;
      else if (trigger_tdo_response)
        response_pending <= '0;

      d1_fifo_enable_tdo_capture <= fifo_enable_tdo_capture;
    end
  end

  genvar i;
  generate 
    // This assignment takes advantage of the fact that config-retrieve-info's
    // response is '0.  The only source of 1 bits for resp_data is tdo, when
    // tdo-capture is enabled. Optimizations here must be revisited if VERSION
    // or CONFIG_RESPONSE_RESERVED change.
    for (i = 0; i < ST_DATA_W; ++i) begin : tdo_shifter_assignment
      always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
          resp_data[i] <= '0;
        end
        else begin
          if (
            trigger_config_response |
            (enable_tdo_capture & tdo_bit_select[i]) |
            response_beat
          )
            resp_data[i] <= (enable_tdo_capture & tdo_bit_select[i]) & tdo;
        end
      end
    end
  endgenerate

  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      state <= ST_IDLE;
      cmd_ready  <= '0;
    end
    else begin
      state <= p1_state;
      cmd_ready  <= p1_cmd_ready;
    end
  end

  // Inter-FSM communication
  logic response_is_idle;

  always @* begin : state_transition
    // default assignments
    p1_state = state;

    p1_cmd_ready = '0;
    p1_shift_counter = 'x;
    p1_bit_counter = 'x;
    p1_tck_enable = '0;
    p1_tms_shifter = 'x;
    p1_tdi_shifter = 'x;
    pending_response_sop_set = '0;
    cmd_data_reg_clken = '0;
    cmd_clken = '0;
    trigger_config_response = '0;

    p1_tdo_enable_fifo_duration = tdo_enable_fifo_duration;
    tdo_enable_fifo_write = '0;
    tdo_enable_fifo_reset = '0;

    case (state)
      ST_IDLE: begin
        // For now, stall until response state machine is idle.  Could refine
        // this by allowing commands which have no response to proceed.
        // If a CMD_CONFIG arrives just after a response-generating shift
        // sequence, stall the CMD_CONFIG while the response goes out.
        // If the CMD_CONFIG arrives in the middle of the shift sequence, 
        // (that is, while more shift data is expected, and a partial response 
        // has been built up), the command sink will never assert ready.
        // The driver shouldn't do this, but it's a pretty dire outcome.

        // If a response is being driven, but hasn't been accepted, stall
        // here, to avoid the current response byte being overwritten by
        // the current command.
        p1_cmd_ready = ~(resp_valid & ~resp_ready);
        if (~(response_pending && (cmd == CMD_CONFIG)) && 
          response_is_idle && cmd_valid && cmd_ready) begin
            p1_state = ST_CMD1;
            p1_cmd_ready = '0;

            // Capture cmd_data_reg.  Optimization: don't make it dependent on
            // cmd_valid?  If a bogus value is captured, it'll be overwritten
            // upon next cmd_valid.
            cmd_data_reg_clken = '1;
            cmd_clken = '1;
            pending_response_sop_set = cmd_sop;
        end
      end

      ST_CMD1: begin
        p1_cmd_ready = '1;
        // To do: can optimize by setting cmd_data_reg_clken in all cases?
        case (cmd_reg)
          default:
            p1_state = ST_FAIL;

          CMD_CONFIG: begin
            p1_state = ST_IDLE;
            p1_cmd_ready = response_is_idle;
            if (config_subcmd_code == CMD_RESET_TDO_ENABLE_FIFO) begin
              tdo_enable_fifo_reset = '1;
            end
            if (config_subcmd_code == CMD_CONFIG_RETRIEVE_INFO) begin
              trigger_config_response = '1;
            end
          end

          CMD_SHORT_CUSTOM_TMS_TDI: begin
            p1_bit_counter = {8'b0, dr_length[4:0]};
            p1_state = ST_GET_TMS_TDI_BYTE;
          end

          CMD_LONG_FIXED_TMS_CUSTOM_TDI, CMD_LONG_FIXED_TMS_TDI: begin
            p1_cmd_ready = '0;
            cmd_data_reg_clken = '1;
            p1_bit_counter = {8'b0, dr_length[4:0]};
            p1_state = ST_CMD2;
          end
          
          CMD_WRITE_TDO_ENABLE_FIFO: begin
            p1_cmd_ready = '0;
            cmd_data_reg_clken = '1;
            p1_tdo_enable_fifo_duration[4:0] = dr_length[4:0];
            p1_state = ST_CMD2_FIFO_WRITE;
          end

        endcase
      end

      ST_CMD2: begin
        // Optimization: do not qualify cmd_data_reg_clken, p1_bit_counter
        // assignment with cmd_valid.  This means that bogus data will be 
        // captured in cmd_data_reg and bit_counter in this 
        // state, but it will eventually be overwritten with correct data
        // (when cmd_valid is asserted).
        cmd_data_reg_clken = '1;
        p1_bit_counter = {cmd_data_reg, bit_counter[4:0]};

        if (cmd_valid) begin

          // Stay in this state until the 2nd length byte is accepted.
          p1_cmd_ready = ~cmd_ready;
          if (cmd_ready)
            p1_state = ST_GET_TDI_BYTE;

        end

      end

      ST_CMD2_FIFO_WRITE: begin
        cmd_data_reg_clken = '1;
        p1_tdo_enable_fifo_duration[12:5] = cmd_data_reg;

        if (cmd_valid) begin

          // Stay in this state until the 2nd length byte is accepted.
          p1_cmd_ready = ~cmd_ready;
          if (cmd_ready)
            p1_state = ST_CMD3_FIFO_WRITE;

        end
      end

      ST_CMD3_FIFO_WRITE: begin
        cmd_data_reg_clken = '1;

        if (cmd_valid) begin
          // Stay in this state until the 2nd length byte is accepted.
          p1_cmd_ready = ~cmd_ready;
          if (cmd_ready) begin
            p1_state = ST_IDLE;
            // Must have both valid & ready for FIFO write
            tdo_enable_fifo_write = '1;
          end
        end
      end

      ST_GET_TMS_TDI_BYTE: begin
        p1_shift_counter = (bit_counter > 4'd3) ? 4'd3 : bit_counter[3:0];
        p1_bit_counter = bit_counter;
        p1_cmd_ready = '1;

        if ((!fifo_enable_tdo_capture || response_is_idle) && cmd_valid) begin

          p1_state = ST_DO_VARIABLE_TMS_TDI_SHIFT;
          p1_cmd_ready = '0;
  
          p1_tck_enable = '1;
          cmd_data_reg_clken = '1;

          // Set up tdi, tms from the first nibbles of data.
          p1_tms_shifter = {10'bx, cmd_data[7:4]};
          p1_tdi_shifter = {10'bx, cmd_data[3:0]};

        end
      end

      ST_DO_VARIABLE_TMS_TDI_SHIFT: begin
        p1_tck_enable = '1;
        p1_shift_counter = shift_counter - 1'b1;
        p1_bit_counter = bit_counter - 1'b1;

        p1_tms_shifter = tms_shifter >> 1;
        p1_tdi_shifter = tdi_shifter >> 1;

        if (shift_counter == 0) begin
          // In any case, the next state will be different.  No more tck.
          p1_tck_enable = '0;

          // For custom tms/tdi, there are half as many response bytes as there
          // are command-shift bytes (modulo rounding).  So the logic for
          // triggering tdo response has to take into account 1) whether we've
          // shifted in a full 8 bits, 2) whether this is the last partial byte
          // of a custom-tms-tdi command.
          // 1) send back the full 8 bit response

          // tdo_bit_select[ST_DATA_W-2] is used to determine one cycle before
          // the MSB of tdo_shifter is assigned.

          if (bit_counter == 0) begin
            // All bits are shifted.  We are done here.
            // Send a partial byte response.
            p1_state = ST_IDLE;
            // Prepare to accept data unless a response is pending or stalled- 
            // this avoids premature acceptance of the next command byte.
            p1_cmd_ready <= ~response_pending & !(resp_valid & ~resp_ready);
          end
          else begin
            // A byte just shifted, but there's at least one more byte to go.
            p1_state = ST_GET_TMS_TDI_BYTE;
            // Prepare to accept data on the next cycle.
            p1_cmd_ready <= '1;

          end
        end
      end

      ST_GET_TDI_BYTE: begin
        p1_shift_counter = (bit_counter > 4'd7) ? 4'd7 : bit_counter[3:0];
        p1_bit_counter = bit_counter;

        // Note: p1_tms_shifter is set independent of cmd_valid; this is ok
        // because tck is not enabled until cmd_valid.
        // To do: optimize cmd_reg decoding; only need to look at 2 bits, is
        // QIS smart enough to see that?
        // To do: presently, stall until response is idle, if responses are
        // needed. Does this waste cycles?
        if (!fifo_enable_tdo_capture || response_is_idle) begin
          p1_tms_shifter = {6'bx, 8'b0};

          if (cmd_reg == CMD_LONG_FIXED_TMS_TDI) begin
            // Set up to shift out up to 8 more 0's.
            p1_tdi_shifter = {6'bx, 8'b0};
            p1_state = ST_DO_VARIABLE_TDI_SHIFT;
            p1_tck_enable = '1;
          end
          else begin // CMD_LONG_FIXED_TMS_CUSTOM_TDI
            if (cmd_valid) begin

              p1_cmd_ready = '1;
              p1_state = ST_DO_VARIABLE_TDI_SHIFT;

              p1_tck_enable = '1;
              cmd_data_reg_clken = '1;
              p1_tdi_shifter = {6'bx, cmd_data};
            end
          end
        end

      end

      ST_DO_VARIABLE_TDI_SHIFT: begin
        p1_tck_enable = '1;
        p1_shift_counter = shift_counter - 1'b1;
        p1_bit_counter = bit_counter - 1'b1;

        p1_tms_shifter = tms_shifter >> 1;
        p1_tdi_shifter = tdi_shifter >> 1;

        if (shift_counter == 0) begin
          if (bit_counter == 0) begin
            // All bits are shifted in long-fixed-tms-custom-tdi.  Return to
            // idle.
            p1_state = ST_IDLE;
            p1_tck_enable = '0;
          end
          else begin // bit_counter != 0
            // A byte just shifted, but there's at least one more byte to go.
            p1_state = ST_GET_TDI_BYTE;
            p1_tck_enable = '0;
          end
        end
      end

      // From SIR or SDR, return to RTI.  Note: the FD seems to suggest that
      // it's ok to leave the sld hub jtag state parked at UIR/UDR... I'm not
      // sure that's ok, so for now I return to RTI.  The way to answer this
      // question is to observe the sld hub state machine when hooked to the
      // JSM... what happens there?
      ST_GOTO_RTI: begin
        p1_tck_enable = '1;
        p1_shift_counter = shift_counter - 1'b1;
        p1_tms_shifter = tms_shifter >> 1;
        p1_tdi_shifter = tdi_shifter >> 1;
        if (shift_counter == 0) begin
          p1_tck_enable = '0;
          p1_state = ST_IDLE;
        end
      end

      default: begin
        p1_state = ST_FAIL;
      end
       
      ST_FAIL: begin
        p1_state = ST_FAIL;
      end
    endcase
  end

  resp_fsm the_resp_fsm(
    .clk (clk),
    .reset (reset),
    .trigger_config_response (trigger_config_response),
    .trigger_tdo_response (trigger_tdo_response),

    .response_beat (response_beat),
    .response_is_idle (response_is_idle),

    .resp_valid (resp_valid),
    .resp_ready (resp_ready)
  );

endmodule

module resp_fsm (
  input wire clk,
  input wire reset,
  
  input wire trigger_config_response,
  input wire trigger_tdo_response,
  input wire resp_ready,

  output reg response_beat,
  output wire response_is_idle,

  output reg resp_valid
);

  typedef enum int unsigned {
    ST_IDLE,
    ST_DO_RESP,
    ST_LOAD_RESP,
    ST_DO_RESP_FAIL
  } t_resp_state;
  t_resp_state resp_state, p1_resp_state;

  // Response state machine
  logic p1_resp_valid;

  assign response_is_idle = ~trigger_tdo_response & (p1_resp_state == ST_IDLE);
  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      resp_state <= ST_IDLE;
      resp_valid <= '0;
    end
    else begin
      resp_state <= p1_resp_state;
      resp_valid <= p1_resp_valid;
    end
  end
  always @* begin : resp_state_transition
    // default assignments
    p1_resp_state = resp_state;
    p1_resp_valid = '0;
    response_beat = '0;

    case (resp_state)
      ST_IDLE: begin
        if (trigger_config_response) begin
          p1_resp_state = ST_DO_RESP;
          p1_resp_valid = '1;
        end
        if (trigger_tdo_response) begin
          p1_resp_state = ST_DO_RESP;
          p1_resp_valid = '1;
        end
      end

      ST_LOAD_RESP: begin
        // 1 cycle delay while response data loads.
        p1_resp_state = ST_DO_RESP;
        p1_resp_valid = '1;
      end

      ST_DO_RESP: begin
        p1_resp_valid = '1;

        if (resp_valid & resp_ready) begin
          p1_resp_valid = '0;
          response_beat = '1;

          p1_resp_state = ST_IDLE;
        end
      end

      default: begin
        p1_resp_state = ST_DO_RESP_FAIL;
      end
       
      ST_DO_RESP_FAIL: begin
        p1_resp_state = ST_DO_RESP_FAIL;
      end
    endcase
  end
endmodule

module altera_streaming_sld_hub_controller_fifo #(
  parameter WIDTH = 15
  ) (
    output wire tdo_enable,
    output wire set_eop,

    input wire write,
    input wire dec,
    input wire [WIDTH-1:0] duration_in,
    input wire eop_gen_in,
    input wire tdo_enable_in,
    input wire soft_reset,

    input wire clk,
    input wire reset
);

  logic write_ptr, read_ptr;

  logic p1_underflow0, p1_underflow1;
  logic tdo_enable0, tdo_enable1;
  logic valid0, valid1;
  logic eop_gen0, eop_gen1;

  altera_streaming_sld_hub_controller_fifo_element #(
    .WIDTH (WIDTH)
  ) el0(
    .write ((write_ptr == 1'b0) & write),
    .duration_in (duration_in),
    .tdo_enable_in (tdo_enable_in),
    .eop_gen_in (eop_gen_in),
    .dec ((read_ptr == 1'b0) & dec),

    .p1_underflow (p1_underflow0),
    .tdo_enable (tdo_enable0),
    .valid (valid0),
    .eop_gen (eop_gen0),

    .clk (clk),
    .soft_reset (soft_reset),
    .reset (reset)
  );

  altera_streaming_sld_hub_controller_fifo_element #(
    .WIDTH (WIDTH)
  ) el1(
    .write ((write_ptr == 1'b1) & write),
    .duration_in (duration_in),
    .tdo_enable_in (tdo_enable_in),
    .eop_gen_in (eop_gen_in),
    .dec ((read_ptr == 1'b1) & dec),

    .p1_underflow (p1_underflow1),
    .tdo_enable (tdo_enable1),
    .valid (valid1),
    .eop_gen (eop_gen1),

    .clk (clk),
    .soft_reset (soft_reset),
    .reset (reset)
  );

  logic p1_underflow;
  assign p1_underflow = 
    ((read_ptr == 1'b0) & p1_underflow0) | 
    ((read_ptr == 1'b1) & p1_underflow1);

  assign tdo_enable = 
    ((read_ptr == 1'b0) & tdo_enable0) | 
    ((read_ptr == 1'b1) & tdo_enable1);

  logic read_is_valid;
  assign read_is_valid = 
    ((read_ptr == 1'b0) & valid0) | 
    ((read_ptr == 1'b1) & valid1);

  assign set_eop = 
    ((read_ptr == 1'b0) & eop_gen0 & p1_underflow0 & dec) | 
    ((read_ptr == 1'b1) & eop_gen1 & p1_underflow1 & dec);

  // read, write pointer management.
  always @(posedge clk or posedge reset) begin
    if (reset) begin
      write_ptr <= '0;
      read_ptr <= '0;
    end
    else begin
      if (soft_reset) begin
        write_ptr <= '0;
        read_ptr <= '0;
      end
      else begin
        // Note: write and dec cannot occur simultaneously.
        if (write) begin
          write_ptr <= write_ptr + 1'b1;
          // If we are about to overwrite a valid location, drop it.
          if ((write_ptr == read_ptr) && read_is_valid)
            read_ptr <= read_ptr - 1'b1;
        end
        if (dec) begin
          if (p1_underflow)
            read_ptr <= read_ptr - 1'b1;
        end
      end
    end
  end

endmodule

module altera_streaming_sld_hub_controller_fifo_element #(
  parameter WIDTH = 15
  ) (
    input wire write,
    input wire [WIDTH-1:0] duration_in,
    input wire tdo_enable_in,
    input wire eop_gen_in,
    input wire dec,

    output logic p1_underflow,
    output logic tdo_enable,
    output logic valid,
    output logic eop_gen,

    input wire clk,
    input wire soft_reset,
    input wire reset
);

  localparam ELEMENT_WIDTH = WIDTH + 3;
  localparam VALID = ELEMENT_WIDTH - 1;
  localparam ENABLE = ELEMENT_WIDTH - 2;
  localparam EOP_GEN = ELEMENT_WIDTH - 3;

  // FIFO element
  // +---------------------------------------------+
  // | valid | enable | eop_gen |  duration[WIDTH] |
  // +---------------------------------------------+
  logic [ELEMENT_WIDTH-1:0] fifo_element;

  always @(posedge clk or posedge reset) begin
    if (reset) begin
      fifo_element <= '0;
    end
    else begin
      if (soft_reset) begin
        fifo_element <= '0;
      end
      else begin
        if (write) begin
          fifo_element <= {1'b1, tdo_enable_in, eop_gen_in, duration_in};
        end
        else if (valid & dec) begin
          if (p1_underflow)
            fifo_element <= '0;
          else
            fifo_element <= 
              {1'b1, fifo_element[ENABLE], fifo_element[EOP_GEN], fifo_element[WIDTH-1:0] - 1'b1};
        end
      end
    end
  end

  assign p1_underflow = valid && (fifo_element[WIDTH-1:0] == '0);

  assign tdo_enable = fifo_element[ENABLE] & fifo_element[VALID]; 
  
  assign valid = fifo_element[VALID];

  assign eop_gen = p1_underflow & fifo_element[EOP_GEN];
endmodule

/*
module csr_slave(
  output reg enable_user_jtag_io,

  input wire csr_write,
  input wire csr_read,
  input wire [0:0] csr_addr,
  input wire [31:0] csr_writedata,
  output wire [31:0] csr_readdata,

  input wire clk,
  input wire reset
);

  assign csr_readdata = { {31 {1'b0}}, enable_user_jtag_io};

  always @(posedge clk or posedge reset) begin
    if (reset) begin
      // 1: user I/O; 0: jsm
      enable_user_jtag_io <= '0;
    end
    else begin
      if (csr_write)
        enable_user_jtag_io <= csr_writedata[0];
    end
  end

endmodule
*/

`default_nettype wire

