set_multi_cpu_usage -localCpu 8

read_lib -max systemcdes_Late.lib
read_lib -min systemcdes_Early.lib
read_verilog systemcdes.v
set_top_module systemcdes -ignore_undefined_cell

read_spef systemcdes.spef

create_clock -name CC1 -period 2 -waveform {0 1} x3333
set_propagated_clock CC1

report_timing
source systemcdes_cmd.tcl
