#!/bin/bash

for ops in vga_lcd/testcases/*.ops; do
    echo '------------------------------------------------------'
    echo '         '$ops
    echo '------------------------------------------------------'

	for exe in bin/*; do
		echo '[+] '$exe
		output=vga_lcd/vga_lcd.out_${exe#bin/}
		echo time ./$exe vga_lcd/vga_lcd.tau2015 vga_lcd/vga_lcd.timing $ops $output
		time ./$exe vga_lcd/vga_lcd.tau2015 vga_lcd/vga_lcd.timing $ops $output 2> result >tmp

		cat result | grep -E '(cache).*([0-9]+) ms'
		cat result | grep -E '(search).*([0-9]+) ms'
		cat result | grep -E '(kth).*([0-9]+) ms'
		cat result | grep -E 'total.*' -o | tail -n 1
		cat result | grep -E '[0-9]:.*elapsed' -o
		echo ''

		rm -f result
		rm -f message.log
		rm -f tmp
	done

	gloden=vga_lcd/vga_lcd.out_iTimerP_final_2018_3_8
	for exe in bin/*; do
		output=vga_lcd/vga_lcd.out_${exe#bin/}

		if [ "$output" == "$gloden" ]; then
			continue;
		fi

		echo ">>> "$output $gloden
		python compare_paths_my.py --reference $gloden --test $output
	done

	echo ''
done
