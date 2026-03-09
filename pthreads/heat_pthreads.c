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

    //allocate memory for the grid
    double **grid = allocate_grid(rows, cols);
    double **new_grid = allocate_grid(rows, cols);

    //initialize the grid
    init_grid(grid, rows, cols);

    //create threads
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

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

        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }

    //wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    //end timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken: %f seconds\n", time_taken);

    //free memory
    free_grid(grid, rows);
    free_grid(new_grid, rows);
    free(threads);
    free(thread_data);
    pthread_barrier_destroy(&barrier);

    return 0;
}