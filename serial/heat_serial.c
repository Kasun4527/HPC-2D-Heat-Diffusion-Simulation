/*
 * Serial Heat Diffusion Simulation
 *
 * Baseline serial implementation of a 2D heat diffusion model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../common/grid.h"
#include "../common/utils.h"


int main(int argc, char *argv[]) {
    // Default parameters
    int rows = 1000;
    int cols = 1000;
    int timesteps = 1000;
    double alpha = 0.1;
    
    // Parse command line arguments
    if (argc >= 2) rows = atoi(argv[1]);
    if (argc >= 3) cols = atoi(argv[2]);
    if (argc >= 4) timesteps = atoi(argv[3]);
    if (argc >= 5) alpha = atof(argv[4]);
    
    printf("Serial Heat Diffusion Simulation\n");
    printf("Grid size: %d x %d\n", rows, cols);
    printf("Timesteps: %d\n", timesteps);
    printf("Alpha: %f\n\n", alpha);

    // Allocate and initialize grids
    double **grid = allocate_grid(rows, cols);
    double **new_grid = allocate_grid(rows, cols);
    
    if (!grid || !new_grid) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return 1;
    }
    
    initialize_grid(grid, rows, cols);

    // Start timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Main simulation loop
    for (int t = 0; t < timesteps; t++) {
        // Update interior cells
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                new_grid[i][j] = grid[i][j] + alpha * (
                    grid[i-1][j] + grid[i+1][j] +
                    grid[i][j-1] + grid[i][j+1] -
                    4.0 * grid[i][j]
                );
            }
        }
        
        // Swap grids
        double **temp = grid;
        grid = new_grid;
        new_grid = temp;
    }
    
    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Execution time: %.6f seconds\n", elapsed);
    
    return 0;
}
