#include <iostream>
#include <mpi.h>
#include <cmath>

int main() {
    int intervals, rank, n_procs;
    double mypi, pi, x, h, start, end;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        std::cout << "Enter interval count: " << std::endl;
        std::cin >> intervals;
    }

    MPI_Bcast(&intervals, 1, MPI_INT, 0, MPI_COMM_WORLD);

    h = 2.0 / intervals;

    start = -1.0 + rank * (2.0 / n_procs);
    end   = -1.0 + (rank + 1) * (2.0 / n_procs);

    mypi = 0.0;

    for (x = start; x < end; x += h) {
        mypi += h * std::sqrt(1.0 - x * x);
    }

    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi *= 2.0;

        std::cout << "Calculated PI : " << pi << std::endl;
        std::cout << "Actual PI     : " << M_PI << std::endl;
        std::cout << "Error         : " << std::abs(M_PI - pi) << std::endl;
    }

    MPI_Finalize();
    return 0;
}