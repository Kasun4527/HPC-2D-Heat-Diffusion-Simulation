#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "../common/grid.h"
#include "../common/utils.h"

typedef struct {
    double **grid;
    double **new_grid;
    int start_row;
    int end_row;
    int cols;
    double alpha;
    pthread_barrier_t *barrier;
    int timesteps;
} ThreadData;

//heat computing function
void *compute_heat(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    
    for (int t = 0; t < data->timesteps; t++) {
        // update interior cells for this thread's rows
        for (int i = data->start_row; i < data->end_row; i++) {
            for (int j = 1; j < data->cols - 1; j++) {
                data->new_grid[i][j] = data->grid[i][j] + data->alpha * (
                    data->grid[i-1][j] + data->grid[i+1][j] +
                    data->grid[i][j-1] + data->grid[i][j+1] -
                    4.0 * data->grid[i][j]
                );
            }
        }
        
        // wait for all threads to complete this timestep
        pthread_barrier_wait(data->barrier);
        
        // swap grids locally for each thread
        double **temp = data->grid;
        data->grid = data->new_grid;
        data->new_grid = temp;
        
        // wait for swap to complete
        pthread_barrier_wait(data->barrier);
    }
    
    return NULL;
}


int main(int argc, char *argv[]) {
    //default parameters
    int rows = 1000;
    int cols = 1000;
    int timesteps = 1000;
    double alpha = 0.1;
    int num_threads = 4;

    // parse command line arguments
    if (argc >= 2) rows = atoi(argv[1]);
    if (argc >= 3) cols = atoi(argv[2]);
    if (argc >= 4) timesteps = atoi(argv[3]);
    if (argc >= 5) alpha = atof(argv[4]);
    if (argc >= 6) num_threads = atoi(argv[5]);

    //print the input parameters
    printf("Grid size: %d x %d\n", rows, cols);
    printf("Timesteps: %d\n", timesteps);
    printf("Alpha: %f\n", alpha);
    printf("Threads: %d\n\n", num_threads);

    ensure_output_directories();

    //allocate memory for the grid
    double **grid = allocate_grid(rows, cols);
    double **new_grid = allocate_grid(rows, cols);

    if (!grid || !new_grid)
    {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        free_grid(grid, rows);
        free_grid(new_grid, rows);
        return 1;
    }
    //initialize the grids
    initialize_grid(grid, rows, cols); 
    initialize_grid(new_grid, rows, cols);
    
    //initialize barrier
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    //create threads
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));

    //calculate rows per thread
    int rows_per_thread = (rows - 2) / num_threads;
    
    // start timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    //distribute the work among threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].grid = grid;
        thread_data[i].new_grid = new_grid;
        thread_data[i].start_row = 1 + i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? rows - 1 : 
                                  1 + (i + 1) * rows_per_thread;
        thread_data[i].cols = cols;
        thread_data[i].alpha = alpha;
        thread_data[i].barrier = &barrier;
        thread_data[i].timesteps = timesteps;

        pthread_create(&threads[i], NULL, compute_heat, &thread_data[i]);
    }

    //wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    //end timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken: %f seconds\n", time_taken);

    // Select the correct grid based on timesteps parity
    double **final_grid = (timesteps % 2 == 0) ? grid : new_grid;

    // Calculate RMSE vs serial baseline
    double **serial_grid = allocate_grid(rows, cols);
    double **serial_new_grid = allocate_grid(rows, cols);
    double rmse = -1.0;
    if (!serial_grid || !serial_new_grid) {
        fprintf(stderr, "Error: Failed to allocate serial baseline memory\n");
    } else {
        initialize_grid(serial_grid, rows, cols);
        initialize_grid(serial_new_grid, rows, cols);
        simulate_heat_serial(&serial_grid, &serial_new_grid, rows, cols, timesteps, alpha);
        rmse = calculate_rmse(final_grid, serial_grid, rows, cols);
    }

    printf("RMSE vs serial baseline: %.10e\n", rmse);

    //save results
    save_grid("../data/output_results/pthreads_output.txt", final_grid, rows, cols); //txt
    save_timing("../results/timing.csv", "pthreads", rows, cols, timesteps, num_threads, time_taken); //csv
    if (rmse >= 0.0) {
        save_rmse("../results/rmse.csv", "pthreads", rows, cols, timesteps, alpha, num_threads, rmse); //csv
    }

    //free memory
    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_data);
    free_grid(grid, rows);
    free_grid(new_grid, rows);
    if (serial_grid) free_grid(serial_grid, rows);
    if (serial_new_grid) free_grid(serial_new_grid, rows);

    printf("Simulation completed successfully.\n");
    return 0;
}