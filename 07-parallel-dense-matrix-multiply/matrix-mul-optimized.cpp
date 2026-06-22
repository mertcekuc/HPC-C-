#include <omp.h>
#include <iostream>
#include <vector>
#include <random>

const int N = 3000;
const int BS = 32;   // block size (cache-friendly)

void fill_vec(std::vector<double>& v) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    for (auto& e : v) {
        e = dis(gen);
    }
}

void transpose(const std::vector<double>& A, std::vector<double>& B) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B[j * N + i] = A[i * N + j];
        }
    }
}

int main() {
    omp_set_num_threads(8);

    std::vector<double> A(N * N);
    std::vector<double> B(N * N);
    std::vector<double> BT(N * N);
    std::vector<double> C(N * N, 0.0);

    fill_vec(A);
    fill_vec(B);
    transpose(B, BT);

    double start = omp_get_wtime();

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {

            for (int kk = 0; kk < N; kk += BS) {

                for (int i = ii; i < ii + BS && i < N; i++) {
                    for (int j = jj; j < jj + BS && j < N; j++) {

                        double sum = C[i * N + j];
                        for (int k = kk; k < kk + BS && k < N; k++) {
                            sum += A[i * N + k] * BT[j * N + k];
                        }

                        C[i * N + j] = sum;
                        
                    }
                }

            }
        }
    }

    double end = omp_get_wtime();

    std::cout << "Time: " << (end - start) << " sec\n";

    return 0;
}