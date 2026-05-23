/*
 * MPI Heat Diffusion Simulation
 *
 * Distributed-memory implementation using horizontal row decomposition.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "../common/grid.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Giving idis to all process
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = 1000;
    int cols = 1000;
    int timesteps = 1000;
    double alpha = 0.1;

    if (argc >= 2) rows = atoi(argv[1]);
    if (argc >= 3) cols = atoi(argv[2]);
    if (argc >= 4) timesteps = atoi(argv[3]);
    if (argc >= 5) alpha = atof(argv[4]);

    if (rank == 0) {
        printf("MPI Heat Diffusion Simulation\n");
        printf("Grid size: %d x %d\n", rows, cols);
        printf("Timesteps: %d\n", timesteps);
        printf("Alpha: %f\n", alpha);
        printf("MPI Processes: %d\n\n", size);
        ensure_output_directories();
    }

    int local_rows = rows / size;
    int extra_rows = rows % size;
    int start_row = rank * local_rows + (rank < extra_rows ? rank : extra_rows);
    if (rank < extra_rows) local_rows++;

    int total_local_rows = local_rows + 2;

    double **local_grid = allocate_grid(total_local_rows, cols);
    double **local_new_grid = allocate_grid(total_local_rows, cols);

    if (!local_grid || !local_new_grid) {
        fprintf(stderr, "Process %d: Failed to allocate memory\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < total_local_rows; i++) {
        int global_row = start_row + i - 1;
        for (int j = 0; j < cols; j++) {
            if (global_row == 0 || global_row == rows - 1 ||
                j == 0 || j == cols - 1) {
                local_grid[i][j] = 100.0;
            } else {
                local_grid[i][j] = 0.0;
            }
        }
    }
    copy_grid(local_new_grid, local_grid, total_local_rows, cols);

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int t = 0; t < timesteps; t++) {
        if (rank > 0) {
            MPI_Sendrecv(&local_grid[1][0], cols, MPI_DOUBLE, rank - 1, 0,
                         &local_grid[0][0], cols, MPI_DOUBLE, rank - 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (rank < size - 1) {
            MPI_Sendrecv(&local_grid[local_rows][0], cols, MPI_DOUBLE, rank + 1, 0,
                         &local_grid[local_rows + 1][0], cols, MPI_DOUBLE, rank + 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int start_i = (rank == 0) ? 2 : 1;
        int end_i = (rank == size - 1) ? local_rows : local_rows + 1;

        for (int i = start_i; i < end_i; i++) {
            for (int j = 1; j < cols - 1; j++) {
                local_new_grid[i][j] = local_grid[i][j] + alpha * (
                    local_grid[i-1][j] + local_grid[i+1][j] +
                    local_grid[i][j-1] + local_grid[i][j+1] -
                    4.0 * local_grid[i][j]
                );
            }
        }

        double **temp = local_grid;
        local_grid = local_new_grid;
        local_new_grid = temp;
    }

    double elapsed = MPI_Wtime() - start_time;
    double max_elapsed;
    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double **global_grid = NULL;
    if (rank == 0) {
        global_grid = allocate_grid(rows, cols);
        if (!global_grid) {
            fprintf(stderr, "Process 0: Failed to allocate global grid\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    for (int p = 0; p < size; p++) {
        if (rank == p) {
            if (rank == 0) {
                for (int i = 1; i <= local_rows; i++) {
                    memcpy(global_grid[i - 1], local_grid[i], cols * sizeof(double));
                }
            } else {
                for (int i = 1; i <= local_rows; i++) {
                    MPI_Send(local_grid[i], cols, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
                }
            }
        } else if (rank == 0) {
            int p_start = p * (rows / size) + (p < extra_rows ? p : extra_rows);
            int p_rows = (rows / size) + (p < extra_rows ? 1 : 0);
            for (int i = 0; i < p_rows; i++) {
                MPI_Recv(global_grid[p_start + i], cols, MPI_DOUBLE,
                         p, i + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    if (rank == 0) {
        double **serial_grid = allocate_grid(rows, cols);
        double **serial_new_grid = allocate_grid(rows, cols);
        if (!serial_grid || !serial_new_grid) {
            fprintf(stderr, "Process 0: Failed to allocate serial baseline memory\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        initialize_grid(serial_grid, rows, cols);
        initialize_grid(serial_new_grid, rows, cols);
        simulate_heat_serial(&serial_grid, &serial_new_grid, rows, cols, timesteps, alpha);
        double rmse = calculate_rmse(global_grid, serial_grid, rows, cols);

        printf("Execution time: %.6f seconds\n", max_elapsed);
        printf("RMSE vs serial baseline: %.10e\n", rmse);

        save_grid("../data/output_results/mpi_output.txt", global_grid, rows, cols);
        save_timing("../results/timing.csv", "mpi", rows, cols, timesteps, size, max_elapsed);
        save_rmse("../results/rmse.csv", "mpi", rows, cols, timesteps, alpha, size, rmse);

        free_grid(global_grid, rows);
        free_grid(serial_grid, rows);
        free_grid(serial_new_grid, rows);
        printf("Simulation completed successfully.\n");
    }

    free_grid(local_grid, total_local_rows);
    free_grid(local_new_grid, total_local_rows);

    MPI_Finalize();
    return 0;
}
