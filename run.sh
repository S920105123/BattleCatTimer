[ "${2}" == "" ] || make all
folder="testcase"
testcase_path=$folder/${1}


cmd=./BattleCatTimer" "$testcase_path/${1}.tau2015" "$testcase_path/${1}.timing" "$testcase_path/${1}.ops" "$testcase_path/${1}.myoutput
echo $cmd
$cmd
#$cmd 2> result

compare="python "$testcase_path/compare_paths.py" --reference "$testcase_path/${1}.myoutput" --test "$testcase_path/${1}.myans
echo $compare
$compare
