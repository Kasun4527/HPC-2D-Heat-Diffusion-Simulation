/*
 * OpenMP Heat Diffusion Simulation
 *
 * Shared-memory implementation of a 2D heat diffusion model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "../common/grid.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    int rows = 1000;
    int cols = 1000;
    int timesteps = 1000;
    double alpha = 0.1;
    int num_threads = 4;

    if (argc >= 2) rows = atoi(argv[1]);
    if (argc >= 3) cols = atoi(argv[2]);
    if (argc >= 4) timesteps = atoi(argv[3]);
    if (argc >= 5) alpha = atof(argv[4]);
    if (argc >= 6) num_threads = atoi(argv[5]);

    omp_set_num_threads(num_threads);

    printf("OpenMP 2D-Heat Diffusion Simulation\n");
    printf("Grid size of simulation: %d x %d\n", rows, cols);
    printf("Timesteps to perform: %d\n", timesteps);
    printf("Alpha: %f\n", alpha);
    printf("Number of threads: %d\n\n", num_threads);

    ensure_output_directories();

    double **grid = allocate_grid(rows, cols);
    double **new_grid = allocate_grid(rows, cols);

    if (!grid || !new_grid) {
        fprintf(stderr, " Error when memory allocating\n");
        free_grid(grid, rows);
        free_grid(new_grid, rows);
        return 1;
    }

    initialize_grid(grid, rows, cols);
    initialize_grid(new_grid, rows, cols);

    double start_time = omp_get_wtime();

    for (int t = 0; t < timesteps; t++) {
        #pragma omp parallel for schedule(static)
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                new_grid[i][j] = grid[i][j] + alpha * (
                    grid[i-1][j] + grid[i+1][j] +
                    grid[i][j-1] + grid[i][j+1] -
                    4.0 * grid[i][j]
                );
            }
        }

        double **temp = grid;
        grid = new_grid;
        new_grid = temp;
    }

    double elapsed = omp_get_wtime() - start_time;

    double **serial_grid = allocate_grid(rows, cols);
    double **serial_new_grid = allocate_grid(rows, cols);
    if (!serial_grid || !serial_new_grid) {
        fprintf(stderr, "Error when allocating serial baseline memory\n");
        free_grid(grid, rows);
        free_grid(new_grid, rows);
        free_grid(serial_grid, rows);
        free_grid(serial_new_grid, rows);
        return 1;
    }

    initialize_grid(serial_grid, rows, cols);
    initialize_grid(serial_new_grid, rows, cols);
    simulate_heat_serial(&serial_grid, &serial_new_grid, rows, cols, timesteps, alpha);
    double rmse = calculate_rmse(grid, serial_grid, rows, cols);

    printf("Time to execution: %.6f seconds\n", elapsed);
    printf("RMSE vs serial baseline: %.10e\n", rmse);

    save_grid("../data/output_results/openmp_output.txt", grid, rows, cols);
    save_timing("../results/timing.csv", "openmp", rows, cols, timesteps, num_threads, elapsed);
    save_rmse("../results/rmse.csv", "openmp", rows, cols, timesteps, alpha, num_threads, rmse);

    free_grid(grid, rows);
    free_grid(new_grid, rows);
    free_grid(serial_grid, rows);
    free_grid(serial_new_grid, rows);

    printf("Simulation successfully completed.\n");
    return 0;
}
