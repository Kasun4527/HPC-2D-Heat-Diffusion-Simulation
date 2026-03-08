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
    return 0;
}
