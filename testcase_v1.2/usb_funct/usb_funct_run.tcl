set_multi_cpu_usage -localCpu 8

read_lib -max usb_funct_Late.lib
read_lib -min usb_funct_Early.lib
read_verilog usb_funct.v
set_top_module usb_funct -ignore_undefined_cell

read_spef usb_funct.spef

create_clock -name CC1 -period 2 -waveform {0 1} x3675
set_propagated_clock CC1

report_timing

source usb_funct_cmd.tcl
