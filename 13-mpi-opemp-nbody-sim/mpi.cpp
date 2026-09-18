#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <mpi.h>

#define DIMS 1000.0
#define LIM_RADIUS 10.0
#define N 100000
#define ITER_COOUNT 3
#define K 1.0
#define DT 0.01

typedef struct Body
{
    double mass{1.0};
    double vx{0.0}, vy{0.0};
    double x, y;
} Body;

void calculate_interactions(Body &b, const std::vector<Body> &arr, const std::vector<std::vector<Body>> &halos)
{
    double dx, dy, distance;
    double f, fx{0}, fy{0};
    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Yerel parçacıklar ile etkileşim
    for (size_t i = 0; i < arr.size(); i++)
    {
        if (&b == &arr[i])
            continue;

        dx = arr[i].x - b.x;
        dy = arr[i].y - b.y;
        distance = std::sqrt(dx * dx + dy * dy);

        if (distance > LIM_RADIUS || distance == 0.0)
            continue;

        f = K * (b.mass * arr[i].mass) / (distance * distance);
        fx += (f * dx / distance);
        fy += (f * dy / distance);
    }

    // Diğer proseslerden gelen halo parçacıkları ile etkileşim
    for (int i = 0; i < size; i++)
    {
        if (rank == i) // Kendi halo verimizi tekrar hesaplamamak için atlıyoruz
            continue;

        for (size_t j = 0; j < halos[i].size(); j++)
        {
            dx = halos[i][j].x - b.x;
            dy = halos[i][j].y - b.y;
            distance = std::sqrt(dx * dx + dy * dy);

            if (distance > LIM_RADIUS || distance == 0.0)
                continue;

            f = K * (b.mass * halos[i][j].mass) / (distance * distance);
            fx += (f * dx / distance);
            fy += (f * dy / distance);
        }
    }

    double ax{fx / b.mass}, ay{fy / b.mass};
    b.vx += ax * DT;
    b.vy += ay * DT;
}

void process_movements(std::vector<Body> &arr)
{
    for (size_t i = 0; i < arr.size(); i++)
    {
        arr[i].x += arr[i].vx * DT;
        arr[i].y += arr[i].vy * DT;

        if (arr[i].x > DIMS)
        {
            arr[i].x = DIMS;
            arr[i].vx *= -1;
        }
        else if (arr[i].x < 0)
        {
            arr[i].x = 0;
            arr[i].vx *= -1;
        }

        if (arr[i].y > DIMS)
        {
            arr[i].y = DIMS;
            arr[i].vy *= -1;
        }
        else if (arr[i].y < 0)
        {
            arr[i].y = 0;
            arr[i].vy *= -1;
        }
    }
}

void initialize_particles(std::vector<Body> &arr)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, DIMS);

    for (size_t i = 0; i < N; i++)
    {
        arr[i].x = dist(gen);
        arr[i].y = dist(gen);
    }
}

void find_halos(const std::vector<Body> &local_grid, std::vector<Body> &local_halos)
{
    local_halos.clear();
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // HATA DUZELTMESI: Tam sayı bölmesi (int truncation) engellenerek double hassasiyet kullanıldı
    int space_dims = std::sqrt(size);
    double grid_size = DIMS / space_dims;

    int col = rank % space_dims;
    int row = rank / space_dims;

    double x_start = col * grid_size;
    double x_end = (col + 1) * grid_size;
    double y_start = row * grid_size;
    double y_end = (row + 1) * grid_size;

    for (const auto &i : local_grid)
    {
        // HATA DUZELTMESI: if-else yapısı yerine X veya Y ekseninde sınıra yakınlık ayrı kontrol edildi.
        // Böylece hem X hem Y sınırındaki (köşe) halo parçacıklar kaçırılmıyor.
        bool near_x = (i.x <= x_start + LIM_RADIUS && i.x >= x_start) || (i.x >= x_end - LIM_RADIUS && i.x <= x_end);
        bool near_y = (i.y <= y_start + LIM_RADIUS && i.y >= y_start) || (i.y >= y_end - LIM_RADIUS && i.y <= y_end);

        if (near_x || near_y)
        {
            local_halos.push_back(i);
        }
    }
}

void find_grids(const std::vector<Body> &local, std::vector<std::vector<Body>> &grids)
{
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // HATA DUZELTMESI: Bölmeler double tipine çekilerek hassasiyet kaybı önlendi
    int space_dimm = std::sqrt(size);
    double grid_dimm = DIMS / space_dimm;

    for (int i = 0; i < size; i++)
        grids[i].clear();

    int row, col, rank;
    for (const auto &i : local)
    {
        col = static_cast<int>(i.x / grid_dimm);
        row = static_cast<int>(i.y / grid_dimm);
        col = std::min(col, space_dimm - 1);
        row = std::min(row, space_dimm - 1);
        rank = row * space_dimm + col;
        grids[rank].push_back(i);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    std::vector<Body> particules;
    std::vector<Body> local_grid;
    std::vector<Body> local_halo;
    std::vector<std::vector<Body>> grids(size);
    std::vector<std::vector<Body>> halos(size);

    // HATA DUZELTMESI: grid alan hesaplaması double yapıldı
    int space_dims = std::sqrt(size);
    double grid_dims = DIMS / space_dims;

    if (rank == 0)
    {
        particules.resize(N);
        initialize_particles(particules);
        int row, col, grid;
        for (const auto &i : particules)
        {
            col = static_cast<int>(i.x / grid_dims);
            row = static_cast<int>(i.y / grid_dims);
            col = std::min(col, space_dims - 1);
            row = std::min(row, space_dims - 1);
            grid = row * space_dims + col;
            grids[grid].push_back(i);
        }
        for (int i = 1; i < size; i++)
        {
            int elem_count = grids[i].size();
            MPI_Send(grids[i].data(), elem_count * sizeof(Body), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        local_grid.assign(grids[0].begin(), grids[0].end());
        particules.clear();
    }
    else
    {
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        
        // HATA DUZELTMESI: 'int size' tanımlanarak dıştaki MPI 'size' değişkeninin gölgelenmesi (shadowing) engellendi
        int recv_bytes = 0;
        MPI_Get_count(&status, MPI_BYTE, &recv_bytes);
        local_grid.resize(recv_bytes / sizeof(Body));
        MPI_Recv(local_grid.data(), recv_bytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (rank == 0)
    {
        std::cout << "Starting simulation with " << N << " particules" << std::endl;
    }

    for (int iter = 1; iter <= ITER_COOUNT; iter++)
    {
        find_halos(local_grid, local_halo);

        // HATA DUZELTMESI: Deadlock ve tanımsız değişken (uninitialized variable) hataları giderildi
        for (int j = 0; j < size; j++)
        {
            int size_msg = 0;
            if (j == rank)
            {
                size_msg = local_halo.size();
            }

            // 1. Tüm prosesler kök node'dan (j) mesaj boyutunu alır
            MPI_Bcast(&size_msg, 1, MPI_INT, j, MPI_COMM_WORLD);

            if (j != rank)
            {
                halos[j].resize(size_msg);
            }

            // 2. Mesaj boyutu 0'dan büyükse veri iletimi güvenle yapılır
            if (size_msg > 0)
            {
                MPI_Bcast(
                    (j == rank) ? local_halo.data() : halos[j].data(),
                    size_msg * sizeof(Body),
                    MPI_BYTE,
                    j,
                    MPI_COMM_WORLD
                );
            }
        }

        for (auto &b : local_grid)
        {
            calculate_interactions(b, local_grid, halos);
        }

        process_movements(local_grid);
        find_grids(local_grid, grids);

        // Grid paylaşımı (Mesh redistribüsyonu)
        {
            std::vector<int> send_counts(size);
            std::vector<int> recv_counts(size);
            std::vector<int> recv_displs(size);
            std::vector<int> send_displs(size);

            for (int k = 0; k < size; k++)
            {
                send_counts[k] = grids[k].size() * sizeof(Body);
            }

            MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

            send_displs[0] = 0;
            recv_displs[0] = 0;
            for (int k = 1; k < size; k++)
            {
                recv_displs[k] = recv_displs[k - 1] + recv_counts[k - 1];
                send_displs[k] = send_displs[k - 1] + send_counts[k - 1];
            }

            std::vector<Body> contiguous_grids;
            contiguous_grids.reserve(local_grid.size());

            for (int k = 0; k < size; k++)
            {
                contiguous_grids.insert(
                    contiguous_grids.end(),
                    grids[k].begin(),
                    grids[k].end());
            }

            int recieve_count = 0;
            for (int k = 0; k < size; k++)
                recieve_count += recv_counts[k];

            local_grid.resize(recieve_count / sizeof(Body));

            MPI_Alltoallv(
                contiguous_grids.data(), send_counts.data(), send_displs.data(), MPI_BYTE,
                local_grid.data(), recv_counts.data(), recv_displs.data(), MPI_BYTE,
                MPI_COMM_WORLD
            );

        }
    }

    if (rank == 0)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    MPI_Finalize();
    return 0;
}