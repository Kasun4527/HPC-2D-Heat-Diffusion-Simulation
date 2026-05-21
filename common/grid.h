/*
 * grid.h - Grid allocation and initialization functions
 * 
 * Common functions for managing 2D temperature grids
 * used across all implementations.
 */

#ifndef GRID_H
#define GRID_H

/*
 * Allocate a 2D grid of doubles with the specified dimensions.
 * Returns a pointer to the grid, or NULL on failure.
 */
double** allocate_grid(int rows, int cols);

/*
 * Free a previously allocated 2D grid.
 */
void free_grid(double** grid, int rows);

/*
 * Initialize the grid with boundary conditions and initial values.
 * Boundaries are set to a fixed temperature (e.g., 100.0),
 * interior cells are initialized to 0.0.
 */
void initialize_grid(double** grid, int rows, int cols);

/*
 * Copy contents from source grid to destination grid.
 */
void copy_grid(double** dest, double** src, int rows, int cols);

/*
 * Save grid to a file.
 */
void save_grid(const char* filename, double** grid, int rows, int cols);

#endif // GRID_H
