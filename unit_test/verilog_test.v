module top (
in1,
in2,
in3,
clk,
out1,
out2,
clkout,
);

// Start PIs
input in1;
input in2;
input in3;
input clk;

// Start POs
output out1;
output out2;

// Start wires
wire w1;
wire w2;

// Start cells
INV_X1 U7 ( .a(n11out2), .o(n7) );
INV_X1 U9 ( .a(n11clkout), .o(n9) );
DFF_X80 U10 ( .d(n7), .ck(n9), .q(out2) );
block U11 ( .in1(in1), .in2(in2), .in3(in3), .clk(clk), .out1(out1), .out2(n11out2), .clkout(n11clkout) );

endmodule
