#include <iostream>
#include <omp.h>
#include <sstream>

int main() {
    omp_set_num_threads(8);
    double sum_pi{0};
    double step{0.000000001};
    double interval {1.0/8.0};
    

    #pragma omp parallel
    {
        double x{omp_get_thread_num() * interval};
        double end {x+interval};
        double sum=0;
        while(x<=end){
            sum += 4/(1+(x*x));
            x+=step;
        }
        #pragma omp atomic
            sum_pi+=sum;
        
    }

    std::cout<<"\nPI:" << sum_pi;
}