/*
 * POSIX Threads Heat Diffusion Simulation
 *
 * Shared-memory implementation of a 2D heat diffusion model using pthreads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "../common/grid.h"
#include "../common/utils.h"

typedef struct {
    double ***grid;
    double ***new_grid;
    int thread_id;
    int start_row;
    int end_row;
    int cols;
    int timesteps;
    double alpha;
    pthread_barrier_t *barrier;
} ThreadData;

static void *compute_heat(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int t = 0; t < data->timesteps; t++) {
        double **grid = *(data->grid);
        double **new_grid = *(data->new_grid);

        for (int i = data->start_row; i < data->end_row; i++) {
            for (int j = 1; j < data->cols - 1; j++) {
                new_grid[i][j] = grid[i][j] + data->alpha * (
                    grid[i-1][j] + grid[i+1][j] +
                    grid[i][j-1] + grid[i][j+1] -
                    4.0 * grid[i][j]
                );
            }
        }

        pthread_barrier_wait(data->barrier);

        if (data->thread_id == 0) {
            double **temp = *(data->grid);
            *(data->grid) = *(data->new_grid);
            *(data->new_grid) = temp;
        }

        pthread_barrier_wait(data->barrier);
    }

    return NULL;
}

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

    if (num_threads < 1) {
        fprintf(stderr, "Error: Number of threads must be at least 1\n");
        return 1;
    }

    printf("POSIX Threads Heat Diffusion Simulation\n");
    printf("Grid size: %d x %d\n", rows, cols);
    printf("Timesteps: %d\n", timesteps);
    printf("Alpha: %f\n", alpha);
    printf("Threads: %d\n\n", num_threads);

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

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));

    if (!threads || !thread_data) {
        fprintf(stderr, "Error: Failed to allocate thread metadata\n");
        pthread_barrier_destroy(&barrier);
        free(threads);
        free(thread_data);
        free_grid(grid, rows);
        free_grid(new_grid, rows);
        return 1;
    }

    int interior_rows = rows - 2;
    int base_rows = interior_rows / num_threads;
    int extra_rows = interior_rows % num_threads;
    int next_start = 1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        int assigned_rows = base_rows + (i < extra_rows ? 1 : 0);

        thread_data[i].grid = &grid;
        thread_data[i].new_grid = &new_grid;
        thread_data[i].thread_id = i;
        thread_data[i].start_row = next_start;
        thread_data[i].end_row = next_start + assigned_rows;
        thread_data[i].cols = cols;
        thread_data[i].timesteps = timesteps;
        thread_data[i].alpha = alpha;
        thread_data[i].barrier = &barrier;

        pthread_create(&threads[i], NULL, compute_heat, &thread_data[i]);
        next_start += assigned_rows;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    double **serial_grid = allocate_grid(rows, cols);
    double **serial_new_grid = allocate_grid(rows, cols);
    if (!serial_grid || !serial_new_grid) {
        fprintf(stderr, "Error: Failed to allocate serial baseline memory\n");
        pthread_barrier_destroy(&barrier);
        free(threads);
        free(thread_data);
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

    printf("Execution time: %.6f seconds\n", elapsed);
    printf("RMSE vs serial baseline: %.10e\n", rmse);

    save_grid("../data/output_results/pthreads_output.txt", grid, rows, cols);
    save_timing("../results/timing.csv", "pthreads", rows, cols, timesteps,
                num_threads, elapsed);
    save_rmse("../results/rmse.csv", "pthreads", rows, cols, timesteps, alpha,
              num_threads, rmse);

    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_data);
    free_grid(grid, rows);
    free_grid(new_grid, rows);
    free_grid(serial_grid, rows);
    free_grid(serial_new_grid, rows);

    printf("Simulation completed successfully.\n");
    return 0;
}
