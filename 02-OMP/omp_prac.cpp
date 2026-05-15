#include <iostream>
#include <omp.h>
#include <chrono>

int main() {

    const int num_threads = 4;
    omp_set_num_threads(num_threads);

    const long long num_steps = 100000000;
    const double step = 1.0 / static_cast<double>(num_steps);

    double total = 0.0;

    auto start = std::chrono::high_resolution_clock::now();

    #pragma omp parallel for reduction(+:total)
    for (long long i = 0; i < num_steps; ++i) {

        double x = i * step;

        total += 4.0 / (1.0 + x * x);
    }

    double pi = total * step;

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    std::cout << "PI: " << pi << '\n';
    std::cout << "Time taken: " << elapsed.count() << " seconds\n";
}