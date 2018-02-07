#include "timer.h"


int main(int argc, char** argv)
{
    if(argc<5){
        cout << "no enough arg.\n";
        my_exit();
    }

    Timer timer;
    timer.run(string(argv[1]), string(argv[2]), string(argv[3]), string(argv[4]));
}
