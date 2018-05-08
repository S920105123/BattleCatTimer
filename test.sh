[ "${2}" == "" ] || make all
folder="testcase"
testcase_path=$folder/${1}

cmd=./BattleCatTimer" "$testcase_path/${1}.tau2018" "$testcase_path/${1}.timing" "$testcase_path/${1}.ops" "$testcase_path/${1}.myoutput
echo $cmd
$cmd
