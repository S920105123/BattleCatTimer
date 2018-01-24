set_multi_cpu_usage -localCpu 8

read_lib -max vga_lcd_Late.lib
read_lib -min vga_lcd_Early.lib
read_verilog vga_lcd.v
set_top_module vga_lcd -ignore_undefined_cell

read_spef vga_lcd.spef

create_clock -name CC1 -period 2 -waveform {0 1} x2469
set_propagated_clock CC1

report_timing

source vga_lcd_cmd.tcl
