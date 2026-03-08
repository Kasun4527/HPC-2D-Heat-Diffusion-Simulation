/*
 * utils.h - Utility functions for timing and accuracy evaluation
 * 
 * Common utility functions used across all implementations.
 */

#ifndef UTILS_H
#define UTILS_H

/*
 * Save timing results to a CSV file.
 * Appends a new row with: implementation, grid_rows, grid_cols, 
 * timesteps, num_threads/processes, execution_time
 */
void save_timing(const char* filename, const char* implementation,
                 int rows, int cols, int timesteps, int parallelism, 
                 double time);

/*
 * Calculate Root Mean Square Error (RMSE) between two grids.
 * Used to verify numerical accuracy against the serial baseline.
 */
double calculate_rmse(double** grid1, double** grid2, int rows, int cols);

/*
 * Save RMSE results to a CSV file.
 */
void save_rmse(const char* filename, const char* implementation,
               int rows, int cols, double rmse);

/*
 * Parse configuration from input file.
 * Returns 0 on success, -1 on failure.
 */
int parse_config(const char* filename, int* rows, int* cols, 
                 int* timesteps, double* alpha);

#endif // UTILS_H
