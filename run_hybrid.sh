#!/bin/bash
# Run hybrid (MPI + OpenMP) implementation with various configurations

echo "Running Hybrid (MPI + OpenMP) Heat Diffusion Tests"
echo "=================================================="

cd ../hybrid

# Make sure the executable is built
make clean && make

# Test different MPI process and thread combinations
echo ""
echo "Testing 2 MPI processes with 4 threads each"
mpirun -np 2 ./heat_hybrid 1000 1000 1000 0.1 4

echo ""
echo "Testing 4 MPI processes with 2 threads each"
mpirun -np 4 ./heat_hybrid 1000 1000 1000 0.1 2

echo ""
echo "Testing 2 MPI processes with 8 threads each"
mpirun -np 2 ./heat_hybrid 1000 1000 1000 0.1 8

echo ""
echo "Hybrid tests completed."
