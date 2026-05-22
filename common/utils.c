/*
 * utils.c - Utility function implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "grid.h"
#include "utils.h"

static void make_directory(const char* path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

void ensure_output_directories(void) {
    make_directory("../results");
    make_directory("../data");
    make_directory("../data/output_results");
}

void save_timing(const char* filename, const char* implementation,
                 int rows, int cols, int timesteps, int parallelism,
                 double time) {
    struct stat st;
    int file_exists = (stat(filename, &st) == 0);

    FILE* file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }

    if (!file_exists) {
        fprintf(file, "implementation,rows,cols,timesteps,parallelism,time_seconds\n");
    }

    fprintf(file, "%s,%d,%d,%d,%d,%.6f\n",
            implementation, rows, cols, timesteps, parallelism, time);

    fclose(file);
}

void simulate_heat_serial(double*** grid, double*** new_grid,
                          int rows, int cols, int timesteps, double alpha) {
    for (int t = 0; t < timesteps; t++) {
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                (*new_grid)[i][j] = (*grid)[i][j] + alpha * (
                    (*grid)[i-1][j] + (*grid)[i+1][j] +
                    (*grid)[i][j-1] + (*grid)[i][j+1] -
                    4.0 * (*grid)[i][j]
                );
            }
        }

        double** temp = *grid;
        *grid = *new_grid;
        *new_grid = temp;
    }
}

double calculate_rmse(double** grid1, double** grid2, int rows, int cols) {
    double sum_squared_error = 0.0;
    int count = 0;

    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            double diff = grid1[i][j] - grid2[i][j];
            sum_squared_error += diff * diff;
            count++;
        }
    }

    return count > 0 ? sqrt(sum_squared_error / count) : 0.0;
}

void save_rmse(const char* filename, const char* implementation,
               int rows, int cols, int timesteps, double alpha,
               int parallelism, double rmse) {
    struct stat st;
    int file_exists = (stat(filename, &st) == 0);

    FILE* file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }

    if (!file_exists) {
        fprintf(file, "implementation,rows,cols,timesteps,alpha,parallelism,rmse\n");
    }

    fprintf(file, "%s,%d,%d,%d,%.6f,%d,%.10e\n",
            implementation, rows, cols, timesteps, alpha, parallelism, rmse);

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
        if (line[0] == '#' || line[0] == '\n') continue;

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
