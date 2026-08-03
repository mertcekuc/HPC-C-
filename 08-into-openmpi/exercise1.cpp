#include <stdio.h>
#include <mpi.h>

int main(){
    MPI_Init(NULL, NULL);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != 4){
        printf("This program requires exactly 4 processes.\n");
        MPI_Finalize();
        return 1;
    }
    int rank;
    int result;
    int number;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        number = 9;
        for (int i = 1; i < size; i++){
            MPI_Send(&number, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        result = number * rank;
        MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0){
        for (int i = 1; i < size; i++){
            MPI_Recv(&result, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Process %d sent result %d\n", i, result);
        }
    }

    MPI_Finalize();
    return 0;
}