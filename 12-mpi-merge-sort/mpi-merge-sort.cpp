#include <mpi.h>
#include <iostream>
#include <random>
#include <vector>


#define NUM_ELEMENTS 64
void fill_array(std::vector<int> &arr);
void merge_sort(std::vector<int> &arr);
std::vector<int> merge_arrs(std::vector<int> left, std::vector<int> right);



void fill_array(std::vector<int> &arr){
    srand(time(0));
    for(size_t i=0;i< arr.size(); i++){
        arr[i]=rand()%NUM_ELEMENTS;
    }
}

void merge_sort(std::vector<int> &arr){
    size_t size{arr.size()};
    if(size<=1){
        return;
    }
    else{
        size_t mid{size/2};
        std::vector<int>arr_l(arr.begin(),arr.begin()+mid);
        std::vector<int>arr_r(arr.begin()+mid,arr.end());

        size_t size_l{arr_l.size()};
        size_t size_r{arr_r.size()};
        if(size_l>1){
            merge_sort(arr_l);
        }
        if(size_r>1){
            merge_sort(arr_r);
        }

        if(size_l == 1 && size_r == 1){
            arr = merge_arrs(arr_l,arr_r);
            return;
        }
        
    }
    
}
std::vector<int> merge_arrs(std::vector<int> left, std::vector<int> right){
    size_t size_l{left.size()};
    size_t size_r{right.size()};

    std::vector<int> result;
    result.reserve(size_l+size_r);

    for(size_t i = 0; i< result.size();i++){
        if(left.size()==0){
            result[i]=right.front();
            right.erase(right.begin());
            continue;
        }
        else if(right.size()==0){
            result[i]=left.front();
            left.erase(left.begin());
            continue;
        }
        else{
            if(left.front()<right.front()){
                result[i]=left.front();
                left.erase(left.begin());
                continue;
            }
            else{
                result[i]=right.front();
                right.erase(right.begin());
                continue;
            }
        }
    }

    return result;


}


int main(){
    MPI_Init(NULL,NULL);
    std::vector<int> arr(NUM_ELEMENTS);
    int rank,size;
    
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int num_per_proccess {NUM_ELEMENTS/size};
    std::vector<int> local_arr(num_per_proccess);

    if (rank == 0){
        std::cout << "Filling array..." << std::endl;
        fill_array(arr);
        std::cout << "Array filled." << std::endl;

        std::cout << "Unsorted array: ";
        for(auto i: arr){
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    MPI_Scatter(arr.data(),num_per_proccess,MPI_INT,local_arr.data(),num_per_proccess,MPI_INT,0,MPI_COMM_WORLD);
    for(int i=0; i<size; i++){
        if(i==rank){
            std::cout << "Rank: " << rank << " received array: ";
            for(auto i: local_arr){
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if(rank==0){
        std::cout << "Sorting..." << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    merge_sort(local_arr);
    for(int i=0; i<size; i++){
        if(i==rank){
            std::cout << "Rank: " << rank << " sorted array: ";
            for(auto i: local_arr){
                std::cout << i << " ";
            }
            std::cout << std::endl;
            std::flush(std::cout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    int step=1;
    int current;
    while(step<=size/2){
        current=1;
        while(current<size){
            if(current==rank){
                MPI_Ssend(local_arr.data(),
                local_arr.size(),
                MPI_INT,rank-step,
                0,MPI_COMM_WORLD);
            }
            else{
                std::vector<int> msg(local_arr.size());
                MPI_Recv(msg.data(),local_arr.size(),MPI_INT,current,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                local_arr = merge_arrs(local_arr,msg);
            }
            current += step*2;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        step *= 2;


    }

    MPI_Finalize();
    std::cout << "Rank: " << rank << " sorted array: ";
    for(auto i: local_arr){
        std::cout << i << " ";
    }
    std::cout << std::endl;
    return 0;
}
