#include <iostream>
#include <memory>
#include <vector>
#include <random>

class Matrix{
    private:
     u_int rows;
     u_int cols;
     std::vector<std::vector<float>> index;

    public:
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

};

int main(){
    std::unique_ptr<Matrix> p1(new Matrix(3,3));
    

}

