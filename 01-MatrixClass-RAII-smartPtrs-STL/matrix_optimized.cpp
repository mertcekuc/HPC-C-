#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <optional>
#include <chrono>

//g++ matrix.cpp -O3 -march=native -ffast-math


class Matrix{
    private:
     u_int rows;
     u_int cols;
     std::vector<float> index;
    inline static std::mt19937 generator{std::random_device{}()};
    public:
    Matrix() : rows(0), cols(0) {}

    Matrix(const u_int rows,const u_int cols){
        this->rows =rows;
        this->cols=cols;
        this->index = 
            std::vector<float>(rows*cols);
        //this->print();
    }

    ~Matrix(){
        std::cout<<"Matrix Destructed"<<std::endl;
    }

    void fillMatrix(){
        //std::default_random_engine generator(std::random_device{}());
        
        std::uniform_real_distribution<float> dist(0,10);
        for(float& i:this->index){
            i = std::round(dist(generator) * 100.0f) / 100.0f;
        }
        std::cout << "Matrix filled\n";
    }

    void print(){
        std::cout << std::endl;

        for(u_int i{0};i<rows;i++){
            for(u_int j{0}; j<cols; j++){
                std::cout << this->index[i*cols + j] << " ";
            }
            std::cout << std::endl;

        }
        std::cout<<"\n";
    }

    Matrix const transpose(){
        Matrix t_matrix = Matrix(cols,rows);

        for (std::size_t i{0}; i<this->rows; i++){
            for (std::size_t j{0}; j<this->cols;j++){
                t_matrix.index[j*t_matrix.cols+i] = this->index[i*cols + j];
            }
        }
        return t_matrix;
    }

    void const add(const Matrix& m2, Matrix& out){

        if(this->cols != m2.cols || this->rows != m2.rows){
            std::cout << "ERROR: Matrixes should be at equal sizes for adding\n";
            return;
        }

        out = Matrix(cols,rows);

        for(std::size_t i{0}; i<this->rows;i++){
            for(std::size_t j{0}; j<cols;j++){
                out.index[i*cols + j] = this->index[i*cols + j] + m2.index[i*cols + j];
            }
        }

    }

    void const dotProduct(Matrix& m2, Matrix& out){
        if(this->cols != m2.rows){
            std::cout << "ERROR: Matrixes are not at avaible sizes for product\n";
            return;
        }
        
        out = Matrix(rows,m2.cols);
        
        auto m2_t = m2.transpose();
        for(std::size_t i{0}; i<out.rows;i++){
            for(std::size_t j{0}; j<m2.cols;j++){ //!!!!!!
                float sum{0};
                for(std::size_t k{0}; k<this->cols;k++){
                    sum += this->index[i*cols + k] * m2_t.index[j*m2_t.cols + k];
                }
                out.index[i*cols + j] = sum;
            }
        }
    
    }

};

int main(){

    auto m1 = std::make_unique<Matrix>(1000,1000);
    m1->fillMatrix();
    auto m2 = std::make_unique<Matrix>(1000,1000);
    m2->fillMatrix();

    auto start_product =
        std::chrono::high_resolution_clock::now();

    Matrix product{};
    m1->dotProduct(*m2, product);

    auto end_product =
        std::chrono::high_resolution_clock::now();

    auto product_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>
        (end_product - start_product);

    auto start_add =
        std::chrono::high_resolution_clock::now();

    Matrix added{};
    m1->add(*m2,added);

    auto end_add =
        std::chrono::high_resolution_clock::now();

    auto add_duration =
        std::chrono::duration_cast<std::chrono::microseconds>
        (end_add - start_add);

    auto start_transpose =
        std::chrono::high_resolution_clock::now();

    auto t_m1 = m1->transpose();
    
    auto end_transpose =
        std::chrono::high_resolution_clock::now();

    auto transpose_duration =
        std::chrono::duration_cast<std::chrono::microseconds>
        (end_transpose - start_transpose);

    std::cout << "\n===== Benchmark =====\n";

    std::cout << "Addition Time: "
              << add_duration.count()
              << " us\n";

    std::cout << "Transpose Time: "
              << transpose_duration.count()
              << " us\n";

    std::cout << "Dot Product Time: "
              << product_duration.count()
              << " ms\n";
}