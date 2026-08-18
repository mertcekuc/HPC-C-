#include <mpi.h>
#include <iostream>
#include <random>
#include <vector>
#include <array>

#define NUM_ELEMENTS 64 * 1024 * 2 * 2
void fill_array(std::vector<int> &arr);
void merge_sort(std::vector<int> &arr);
std::vector<int> merge_arrs(const std::vector<int> &left, const std::vector<int> &right);

void fill_array(std::vector<int> &arr)
{
    srand(time(0));
    for (size_t i = 0; i < arr.size(); i++)
    {
        arr[i] = rand() % NUM_ELEMENTS;
    }
}

void merge_sort(std::vector<int> &arr)
{
    size_t size{arr.size()};
    if (size <= 1)
    {
        return;
    }

    size_t mid{size / 2};
    std::vector<int> arr_l(arr.begin(), arr.begin() + mid);
    std::vector<int> arr_r(arr.begin() + mid, arr.end());

    size_t size_l{arr_l.size()};
    size_t size_r{arr_r.size()};

    merge_sort(arr_l);
    merge_sort(arr_r);

    arr = merge_arrs(arr_l, arr_r);
}

std::vector<int> merge_arrs(const std::vector<int> &left, const std::vector<int> &right)
{
    size_t size_l{left.size()};
    size_t size_r{right.size()};
    size_t l{0}, r{0};
    std::vector<int> result;
    result.reserve(size_l + size_r);

    for (size_t i = 0; i < size_l + size_r; i++)
    {
        if (l >= size_l)
        {
            result.push_back(right[r]);
            r++;
            continue;
        }
        else if (r >= size_r)
        {
            result.push_back(left[l]);
            l++;
            continue;
        }
        else
        {
            if (left[l] < right[r])
            {
                result.push_back(left[l]);
                l++;
                continue;
            }
            else
            {
                result.push_back(right[r]);
                r++;
                continue;
            }
        }
    }

    return result;
}

int main()
{
    MPI_Init(NULL, NULL);
    std::vector<int> arr(NUM_ELEMENTS);
    int rank, size;
    double start_time;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int num_per_proccess{NUM_ELEMENTS / size};
    std::vector<int> sendcounts(size, num_per_proccess);
    int remaining{NUM_ELEMENTS % size};
    for (int i = 0; i < remaining; i++)
    {
        sendcounts[i]++;
    }
    std::vector<int> displs(size, 0);
    for (int i = 1; i < size; i++)
    {
        displs[i] = displs[i - 1] + sendcounts[i - 1];
    }
    std::vector<int> local_arr(sendcounts[rank]);

    if (rank == 0)
    {
        fill_array(arr);
        start_time = MPI_Wtime();
    }

    MPI_Scatterv(arr.data(),
                 sendcounts.data(),
                 displs.data(),
                 MPI_INT,
                 local_arr.data(),
                 local_arr.size(),
                 MPI_INT,
                 0,
                 MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "Sorting..." << std::endl;
    }

    merge_sort(local_arr);

    int step = 1;
    
    while (step <size)
    {
            if (rank%(step * 2) == step )
            {
                MPI_Ssend(local_arr.data(),
                          local_arr.size(),
                          MPI_INT, rank-step,
                          0, MPI_COMM_WORLD);
            }
            else if (rank%(step * 2) == 0)
            {
                std::vector<int> recv_arr(local_arr.size());
                MPI_Recv(recv_arr.data(),
                         recv_arr.size(),
                         MPI_INT, rank+step,
                         0, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                local_arr = merge_arrs(local_arr, recv_arr);
            }
        
        step *= 2;
    }
    if (rank == 0)
    {
        double end_time = MPI_Wtime();
        std::cout << std::endl;
        std::cout << "Time taken: " << end_time - start_time << " seconds" << std::endl;
    }

    MPI_Finalize();

    return 0;
}
