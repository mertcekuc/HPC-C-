#include <iostream>
#include <mpi.h>
#include <cmath>
#include <algorithm>

#define COLUMNS 1000
#define ROWS    1000
#define MAX_TEMP_ERROR 0.01

double Temperature[ROWS+2][COLUMNS+2] = {{0.0}};      // temperature grid
double Temperature_last[ROWS+2][COLUMNS+2] = {{0.0}}; // temperature grid from last iteration

// helper routines
void initialize();
void track_progress(int iter);

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int max_iterations;
    double start_time = 0.0, stop_time = 0.0;

    if (rank == 0) {
        printf("Maximum iterations (100-4000)?\n");
        fflush(stdout);
        if (scanf("%d", &max_iterations) != 1) {
            max_iterations = 1000;
        }
        start_time = MPI_Wtime(); // Zamanlayıcıyı başlat
    }

    // Iteration sayısını tüm süreçlere bildir
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

    initialize();

    int rows_per_process = ROWS / size;
    int start_row = rank * rows_per_process + 1;
    // Bölünmeyen satır kalırsa son süreç üstlenir
    int end_row = (rank == size - 1) ? ROWS : (rank + 1) * rows_per_process;

    // Komşu rank tanımlamaları (Sınır dışındakiler için MPI_PROC_NULL kullanılır)
    int up = (rank == 0) ? MPI_PROC_NULL : rank - 1;
    int down = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

    double max_dt = 100.0;
    int iteration = 1;

    while (max_dt > MAX_TEMP_ERROR && iteration <= max_iterations) {

        // 1. GHOST CELL EXCHANGE (Halo takası hesaplamadan ÖNCE yapılmalı)
        // Aşağıya gönder, yukarıdan al
        MPI_Sendrecv(&Temperature_last[end_row][0], COLUMNS + 2, MPI_DOUBLE, down, 0,
                     &Temperature_last[start_row - 1][0], COLUMNS + 2, MPI_DOUBLE, up, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Yukarıya gönder, aşağıdan al
        MPI_Sendrecv(&Temperature_last[start_row][0], COLUMNS + 2, MPI_DOUBLE, up, 1,
                     &Temperature_last[end_row + 1][0], COLUMNS + 2, MPI_DOUBLE, down, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. HESAPLAMA (Temperature_last dizisinden oku, Temperature dizisine yaz)
        for (int i = start_row; i <= end_row; i++) {
            for (int j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                            Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }

        // 3. FARK HESABI (dt) VE GÜNCELLEME
        double local_dt = 0.0;
        for (int i = start_row; i <= end_row; i++) {
            for (int j = 1; j <= COLUMNS; j++) {
                local_dt = std::fmax(std::fabs(Temperature[i][j] - Temperature_last[i][j]), local_dt);
                Temperature_last[i][j] = Temperature[i][j]; // Gelecek iterasyon için kopyala
            }
        }

        // 4. GLOBAL MAX ERROR (Tüm süreçlerdeki en büyük dt değerini bul)
        MPI_Allreduce(&local_dt, &max_dt, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        if (rank == 0 && (iteration % 100) == 0) {
            track_progress(iteration);
        }

        iteration++;
    }

    if (rank == 0) {
        stop_time = MPI_Wtime();
        double elapsed_time = stop_time - start_time;
        printf("\nMax error at iteration %d was %f\n", iteration - 1, max_dt);
        printf("Total time was %f seconds.\n", elapsed_time);
    }

    MPI_Finalize();
    return 0;
}

void initialize() {
    for (int i = 0; i <= ROWS + 1; i++) {
        Temperature_last[i][0] = 0.0;
        Temperature_last[i][COLUMNS + 1] = (100.0 / ROWS) * i;
    }

    for (int j = 0; j <= COLUMNS + 1; j++) {
        Temperature_last[0][j] = 0.0;
        Temperature_last[ROWS + 1][j] = (100.0 / COLUMNS) * j;
    }
}

void track_progress(int iteration) {
    printf("---------- Iteration number: %d ------------\n", iteration);
    for (int i = ROWS - 5; i <= ROWS; i++) {
        printf("[%d,%d]: %5.2f ", i, i, Temperature[i][i]);
    }
    printf("\n");
}