/*
 * Serial Heat Diffusion Simulation
 *
 * Baseline serial implementation of a 2D heat diffusion model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common/grid.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    int rows = 1000;
    int cols = 1000;
    int timesteps = 1000;
    double alpha = 0.1;

    if (argc >= 2) rows = atoi(argv[1]);
    if (argc >= 3) cols = atoi(argv[2]);
    if (argc >= 4) timesteps = atoi(argv[3]);
    if (argc >= 5) alpha = atof(argv[4]);

    printf("Serial Heat Diffusion Simulation\n");
    printf("Grid size: %d x %d\n", rows, cols);
    printf("Timesteps: %d\n", timesteps);
    printf("Alpha: %f\n\n", alpha);

    ensure_output_directories();

    double **grid = allocate_grid(rows, cols);
    double **new_grid = allocate_grid(rows, cols);

    if (!grid || !new_grid) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        free_grid(grid, rows);
        free_grid(new_grid, rows);
        return 1;
    }

    initialize_grid(grid, rows, cols);
    initialize_grid(new_grid, rows, cols);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    simulate_heat_serial(&grid, &new_grid, rows, cols, timesteps, alpha);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Execution time: %.6f seconds\n", elapsed);
    printf("RMSE vs serial baseline: %.10e\n", 0.0);

    save_grid("../data/output_results/serial_output.txt", grid, rows, cols);
    save_timing("../results/timing.csv", "serial", rows, cols, timesteps, 1, elapsed);
    save_rmse("../results/rmse.csv", "serial", rows, cols, timesteps, alpha, 1, 0.0);

    free_grid(grid, rows);
    free_grid(new_grid, rows);

    printf("Simulation completed successfully.\n");
    return 0;
}
