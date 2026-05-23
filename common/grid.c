/*
 * grid.c - Grid allocation and initialization implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"

double** allocate_grid(int rows, int cols) {
    // Allocate array of row pointers
    double** grid = (double**)malloc(rows * sizeof(double*));
    if (!grid) return NULL;
    
    // Allocate contiguous memory for all elements
    double* data = (double*)malloc(rows * cols * sizeof(double));
    if (!data) {
        free(grid);
        return NULL;
    }
    
    // Set up row pointers
    for (int i = 0; i < rows; i++) {
        grid[i] = &data[i * cols];
    }
    
    return grid;
}

void free_grid(double** grid, int rows) {
    (void)rows;
    if (!grid) return;
    
    // Free contiguous data block
    if (grid[0]) free(grid[0]);
    
    // Free row pointers
    free(grid);
}

void initialize_grid(double** grid, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Set boundaries to fixed temperature
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) {
                grid[i][j] = 100.0;
            } else {
                // Interior cells start at 0
                grid[i][j] = 0.0;
            }
        }
    }
}

void copy_grid(double** dest, double** src, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        memcpy(dest[i], src[i], cols * sizeof(double));
    }
}

void save_grid(const char* filename, double** grid, int rows, int cols) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "# Grid dimensions: %d x %d\n", rows, cols);
    
    // Save a sample of the grid (center and corners) to keep file size manageable
    int sample_size = (rows < 10) ? rows : 10;
    
    for (int i = 0; i < sample_size && i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%.6f ", grid[i][j]);
        }
        fprintf(file, "\n");
    }
    
    if (rows > sample_size) {
        fprintf(file, "# ... (middle rows omitted) ...\n");
        for (int i = rows - sample_size; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                fprintf(file, "%.6f ", grid[i][j]);
            }
            fprintf(file, "\n");
        }
    }
    
    fclose(file);
}
