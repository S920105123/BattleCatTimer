[ "${2}" == "" ] || make all
echo ./BattleCatTimer ./testcase_v1.2/${1}/${1}.tau2015 ./testcase_v1.2/${1}/${1}.timing ./testcase_v1.2/${1}/${1}.ops ./testcase_v1.2/${1}/${1}.myoutput
./BattleCatTimer ./testcase_v1.2/${1}/${1}.tau2015 ./testcase_v1.2/${1}/${1}.timing ./testcase_v1.2/${1}/${1}.ops ./testcase_v1.2/${1}/${1}.myoutput
