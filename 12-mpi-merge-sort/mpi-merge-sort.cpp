#include <mpi.h>
#include <iostream>
#include <random>
#include <vector>
#include <array>

#define NUM_ELEMENTS 64*64*1024
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

void merge_sort_impl(
    std::vector<int>& arr,
    std::vector<int>& temp,
    size_t left,
    size_t right)
{
    if (right - left <= 1)
        return;

    size_t mid = left + (right - left) / 2;

    merge_sort_impl(arr, temp, left, mid);
    merge_sort_impl(arr, temp, mid, right);

    size_t l = left;
    size_t r = mid;
    size_t i = left;

    while (l < mid && r < right)
    {
        if (arr[l] <= arr[r])
        {
            temp[i++] = arr[l++];
        }
        else
        {
            temp[i++] = arr[r++];
        }
    }

    while (l < mid)
    {
        temp[i++] = arr[l++];
    }

    while (r < right)
    {
        temp[i++] = arr[r++];
    }

    for (size_t j = left; j < right; j++)
    {
        arr[j] = temp[j];
    }
}

void merge_sort(std::vector<int>& arr)
{
    if (arr.size() <= 1)
        return;

    std::vector<int> temp(arr.size());

    merge_sort_impl(arr, temp, 0, arr.size());
}

std::vector<int> merge_arrs(
    const std::vector<int>& left,
    const std::vector<int>& right)
{
    const size_t size_l = left.size();
    const size_t size_r = right.size();

    std::vector<int> result(size_l + size_r);

    size_t l = 0;
    size_t r = 0;
    size_t i = 0;

    while (l < size_l && r < size_r)
    {
        if (left[l] <= right[r])
        {
            result[i++] = left[l++];
        }
        else
        {
            result[i++] = right[r++];
        }
    }

    while (l < size_l)
    {
        result[i++] = left[l++];
    }

    while (r < size_r)
    {
        result[i++] = right[r++];
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
        start_time = MPI_Wtime();
    }

    merge_sort(local_arr);

    int step = 1;
    MPI_Status status;
    while (step < size)
    {

        if (rank % (step * 2) == step)
        {

            MPI_Send(local_arr.data(),
                      local_arr.size(),
                      MPI_INT, rank - step,
                      0, MPI_COMM_WORLD);
        }
        else if (rank % (step * 2) == 0)
        {
            MPI_Probe(
                rank + step,
                0,
                MPI_COMM_WORLD,
                &status);

            int recv_count;
            MPI_Get_count(&status, MPI_INT, &recv_count);
            std::vector<int> recv_arr(recv_count);

            MPI_Recv(recv_arr.data(),
                     recv_arr.size(),
                     MPI_INT, rank + step,
                     0, MPI_COMM_WORLD,
                     &status);
            local_arr = merge_arrs(local_arr, recv_arr);
        }

        step *= 2;
    }

    if (rank == 0)
    {
        double end_time = MPI_Wtime();
        std::cout << std::endl;
        std::cout << "Time taken: " << end_time - start_time << " seconds" << std::endl;
        // sorted array
  
    }

    MPI_Finalize();

    return 0;
}
