
for ops in vga_lcd/*.ops; do
    echo '------------------------------------------------------'
    echo '         '$ops
    echo '------------------------------------------------------'

    for exe in *.exe; do
        echo '[+] '$exe
        ./$exe vga_lcd/vga_lcd.tau2015 vga_lcd/vga_lcd.timing $ops vga_lcd/vga_lcd.myoutput 2> result

        cat result | grep -E '(search).*([0-9]+) ms'
        cat result | grep -E '(kth).*([0-9]+) ms'
        cat result | grep -E 'total.*' -o | tail -n 1
        echo ''

        rm -f result
        rm -f message.log

    done

done
