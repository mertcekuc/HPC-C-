#include <omp.h>
#include <iostream>
#include <vector>
#include <random>
#define N 10000

    template<typename T>
    void print_vec(const std::vector<T>& v){
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
                std::cout << v[i*N+j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void fill_vec(std::vector<double>& v){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        for(auto& e : v){
            e = dis(gen);
        }
    }
    
    void transpose(const std::vector<double>& A, std::vector<double>& B){
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
                B[j*N+i] = A[i*N+j];
            }
        }
    }

    int main(){
        omp_set_num_threads(8);
        std::vector<double> A(N*N), B(N*N), C(N*N), BT(N*N);
        fill_vec(A);
        fill_vec(B);
        transpose(B, BT);
        double result=0;
        double start = omp_get_wtime();
        #pragma omp parallel for collapse(2) shared(A,BT,C)
            for(int i=0;i<N;i++){
                for(int j=0;j<N;j++){
                    double result = 0;
                    for(int k=0;k<N;k++){
                        result += A[i*N+k] * BT[j*N+k];
                    }
                    C[i*N+j] = result;
                }
            }
        
        double end = omp_get_wtime();
        std::cout << "Time: " << end - start << std::endl;
        return 0;
    }
