#include "timer.h"


// int main(int argc, char** argv)
// {
//     if(argc<5){
//         cout << "no enough arg.\n";
//         my_exit();
//     }
//     Timer timer;
//     timer.run(argv[1], argv[2], argv[3], argv[4]);
// }

int main(int argc, char **argv){

    if(argc>1){
        string testcase = argv[1];
        string timer_argv[10];
        timer_argv[1] = "testcase_v1.2/" + testcase + "/" + testcase + ".tau2015";
        timer_argv[2] = "testcase_v1.2/" + testcase + "/" + testcase + ".ops";
        timer_argv[3] = argv[2];
        Timer timer;
        cout << " --- Timer gen " << testcase << " testcase ---" << endl;
        timer.gen_test(timer_argv[3], timer_argv[1], timer_argv[2]);
    }
    else{
        string testcase, timer_argv[10];
        cout << "enter testcase(c17/simple...): ";
        cin >> testcase;
        timer_argv[1] = "testcase_v1.2/" + testcase + "/" + testcase + ".tau2015";
        timer_argv[2] = "testcase_v1.2/" + testcase + "/" + testcase + ".timing";
        timer_argv[3] = "testcase_v1.2/" + testcase + "/" + testcase + ".ops";
        timer_argv[4] = "testcase_v1.2/" + testcase + "/" + testcase + ".myoutput";
        Timer timer;
        timer.run(string(timer_argv[1]), string(timer_argv[2]), string(timer_argv[3]), string(timer_argv[4]));
    }
    // cout << "myoutput is under testcase_v1.2/" << testcase << endl;
}
