#!/bin/bash


echo "Running OpenMP Heat Diffusion Tests"
echo "===================================="

cd ../openmp


make clean && make


for threads in 1 2 4 8 16; do
    echo ""
    echo "Testing with $threads threads"
    ./heat_openmp 1000 1000 1000 0.1 $threads
done

echo ""
echo "OpenMP tests completed."
