/*
 * utils.c - Utility function implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "utils.h"

void save_timing(const char* filename, const char* implementation,
                 int rows, int cols, int timesteps, int parallelism, 
                 double time) {
    // Check if file exists to determine if we need to write header
    struct stat st;
    int file_exists = (stat(filename, &st) == 0);
    
    FILE* file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    // Write header if this is a new file
    if (!file_exists) {
        fprintf(file, "implementation,rows,cols,timesteps,parallelism,time_seconds\n");
    }
    
    // Write timing data
    fprintf(file, "%s,%d,%d,%d,%d,%.6f\n", 
            implementation, rows, cols, timesteps, parallelism, time);
    
    fclose(file);
}

double calculate_rmse(double** grid1, double** grid2, int rows, int cols) {
    double sum_squared_error = 0.0;
    int count = 0;
    
    // Calculate RMSE for interior cells only (boundaries are fixed)
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            double diff = grid1[i][j] - grid2[i][j];
            sum_squared_error += diff * diff;
            count++;
        }
    }
    
    return sqrt(sum_squared_error / count);
}

void save_rmse(const char* filename, const char* implementation,
               int rows, int cols, double rmse) {
    // Check if file exists to determine if we need to write header
    struct stat st;
    int file_exists = (stat(filename, &st) == 0);
    
    FILE* file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    // Write header if this is a new file
    if (!file_exists) {
        fprintf(file, "implementation,rows,cols,rmse\n");
    }
    
    // Write RMSE data
    fprintf(file, "%s,%d,%d,%.10e\n", implementation, rows, cols, rmse);
    
    fclose(file);
}

int parse_config(const char* filename, int* rows, int* cols, 
                 int* timesteps, double* alpha) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open config file %s\n", filename);
        return -1;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Parse key-value pairs
        char key[64];
        if (sscanf(line, "%s", key) == 1) {
            if (strcmp(key, "rows") == 0) {
                sscanf(line, "%*s %d", rows);
            } else if (strcmp(key, "cols") == 0) {
                sscanf(line, "%*s %d", cols);
            } else if (strcmp(key, "timesteps") == 0) {
                sscanf(line, "%*s %d", timesteps);
            } else if (strcmp(key, "alpha") == 0) {
                sscanf(line, "%*s %lf", alpha);
            }
        }
    }
    
    fclose(file);
    return 0;
}
