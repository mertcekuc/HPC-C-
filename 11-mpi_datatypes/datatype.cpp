#include <mpi.h>
#include <iostream>

int main() {

    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nums[] = {1, 2, 3, 4, 5};

    MPI_Datatype particles;

    int displacements[] = {0, 2, 4};

    MPI_Type_create_indexed_block(
        3,                  // number of blocks
        1,                  // 1 MPI_INT per block
        displacements,      // positions: 0, 2, 4
        MPI_INT,            // type of each element
        &particles
    );

    MPI_Type_commit(&particles);

    if (rank == 0) {

        MPI_Send(
            nums,             // buffer
            1,                // one "particles" datatype
            particles,        // derived datatype
            1,                // destination
            0,                // tag
            MPI_COMM_WORLD
        );

    }
    else if (rank == 1) {

        int recv_nums[3];

        MPI_Recv(
            recv_nums,        // receive buffer
            3,                // three MPI_INT
            MPI_INT,          // receive type
            0,                // source
            0,                // tag
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        std::cout << "Received numbers: "
                  << recv_nums[0] << ", "
                  << recv_nums[1] << ", "
                  << recv_nums[2] << std::endl;
    }

    MPI_Type_free(&particles);

    MPI_Finalize();

    return 0;
}