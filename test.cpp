#include <iostream>
#include <omp.h>

using namespace std;

void func(int x)
{
    #pragma omp parallel sections
    {

        #pragma omp section
        {
            std::cout<<x << " sec1 func total thread: " << omp_get_num_threads()<<std::endl;
            cout << x << " sec1 func tid = " << omp_get_thread_num() << endl;
        }
        #pragma omp section
        {
            std::cout<<x << " sec2  func total thread: " << omp_get_num_threads()<<std::endl;
            cout << x << " sec2 func tid = " << omp_get_thread_num() << endl;
        }
    }

}
int main (int argc, const char * argv[])
{
    int nProcessors = omp_get_max_threads();

    std::cout<<nProcessors<<std::endl;

    omp_set_num_threads(nProcessors);

    omp_set_nested(2);

    #pragma omp parallel sections
    {
            #pragma omp section
            {
                std::cout<<"main total thread: " << omp_get_num_threads()<<std::endl;
                std::cout<<"tid : " << omp_get_thread_num()<<std::endl;
                func(1);
            }
            #pragma omp section
            {
                std::cout<<"main total thread: " << omp_get_num_threads()<<std::endl;
                std::cout<<"tid : " << omp_get_thread_num()<<std::endl;
                func(2);
            }
    }

}
