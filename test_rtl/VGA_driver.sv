`timescale 1ns / 1ns

module test_rtl(
    input logic clk,
    input logic resetn
);

logic [3:0] counter, counter_next;
assign counter_next = counter + 1;

always_ff @(posedge clk, negedge resetn) begin
    if (!resetn)
        counter = 'b0;
    else
        counter <= counter_next;
end

`ifdef COCOTB_SIM
initial begin
    $dumpfile ("test_rtl.vcd");
    $dumpvars (0, test_rtl);
    #1;
end
`endif

endmodule
