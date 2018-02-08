#include "timer.h"


// int main(int argc, char** argv)
// {
//     if(argc<5){
//         cout << "no enough arg.\n";
//         my_exit();
//     }

int main(){

    string testcase, argv[10];
    cout << "enter testcase(c17/simple...): ";
    cin >> testcase;
    argv[1] = "testcase_v1.2/" + testcase + "/" + testcase + ".tau2015";
    argv[2] = "testcase_v1.2/" + testcase + "/" + testcase + ".timing";
    argv[3] = "testcase_v1.2/" + testcase + "/" + testcase + ".ops";
    argv[4] = "testcase_v1.2/" + testcase + "/" + testcase + "myoutput";
    Timer timer;
    timer.run(string(argv[1]), string(argv[2]), string(argv[3]), string(argv[4]));
    cout << "myoutput is under testcase_v1.2/" << testcase << endl;
    cout << "88\n";
}
