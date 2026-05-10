#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <optional>

class Matrix{
    private:
     u_int rows;
     u_int cols;
     std::vector<std::vector<float>> index;

    public:
    Matrix() : rows(0), cols(0) {}
    Matrix(const u_int rows,const u_int cols){
        this->rows =rows;
        this->cols=cols;
        this->index = 
            std::vector<std::vector<float>> 
            (rows, std::vector<float>(cols));

        this->fillMatrix();
        this->print();
    }

    ~Matrix(){
        std::cout<<"Matrix Destructed"<<std::endl;
    }

    void fillMatrix(){
        std::default_random_engine generator(std::random_device{}());
        std::uniform_real_distribution<float> dist(0,10);
        for(std::vector<float>& i:this->index){
            for(float& j: i){
                j = std::round(dist(generator) * 100.0f) / 100.0f;
            }
        }
        std::cout << "Matrix filled\n";
    }

    void print(){
        std::cout << std::endl;

        for(const std::vector<float> i:this->index){
            for(const float j: i){
                std::cout << j << " ";
            }
            std::cout << std::endl;

        }
        std::cout<<"\n";
    }

    Matrix const transpose(){
        Matrix t_matrix = Matrix();
        t_matrix.index = std::vector<std::vector<float>>(this->cols,std::vector<float>(this->rows));
        for (std::size_t i{0}; i<this->index.size(); i++){
            for (std::size_t j{0}; j<this->index.at(i).size();j++){
                t_matrix.index[j][i] = this->index[i][j];
            }
        }
        return t_matrix;
    }

    std::optional<Matrix> const add(const Matrix m2){
        if(this->cols != m2.cols || this->rows != m2.rows){
            std::cout << "ERROR: Matrixes should be at equal sizes for adding\n";
            return std::nullopt;
        }
        Matrix result = Matrix();
        result.cols=cols;
        result.rows=rows;
        result.index = std::vector<std::vector<float>>(this->rows,std::vector<float>(cols));
        for(std::size_t i{0}; i<this->index.size();i++){
            for(std::size_t j{0}; j<this->index[i].size();j++){
                result.index[i][j] = this->index[i][j] + m2.index[i][j];
            }
        }
        return result;
    }

    std::optional<Matrix> const dotProduct(const Matrix m2){
        if(this->cols != m2.rows){
            std::cout << "ERROR: Matrixes are not at avaible sizes for product\n";
            return std::nullopt;
        }
        Matrix result = Matrix();
        result.rows = this->rows;
        result.cols = m2.cols;
        result.index = std::vector<std::vector<float>>(this->rows,std::vector<float>(m2.cols));
        
        for(std::size_t i{0}; i<result.index.size();i++){
            for(std::size_t j{0}; j<result.index[i].size();j++){
                float sum{0};
                for(std::size_t k{0}; k<this->cols;k++){
                    sum += this->index[i][k] * m2.index[k][j];
                }
                result.index[i][j] = sum;
            }
        }
        return result;
    }

};

int main(){
    std::unique_ptr<Matrix> m1(new Matrix(3,3));
    auto m2 = std::make_unique<Matrix>(3,3);
    auto product = m1->dotProduct(*m2);
    auto added = m1->add(*m2);

    auto t_m1 = std::make_unique<Matrix>();
    *t_m1 = m1->transpose();
    std::cout<<"\nTranspose:\n";
    t_m1->print();
    if(added){
    std::cout<<"Matrix addition:\n";
    added->print();
    }

    if(product){
        std::cout<<"Product:"<<std::endl;
        product->print();
    }
}

