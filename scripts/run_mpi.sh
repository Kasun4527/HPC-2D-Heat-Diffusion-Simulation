#!/bin/bash


echo "Running MPI Heat Diffusion Tests"
echo "================================="

cd ../mpi


make clean && make


for procs in 1 2 4 8; do
    echo ""
    echo "Testing with $procs MPI processes"
    mpirun -np $procs ./heat_mpi 1000 1000 1000 0.1
done

echo ""
echo "MPI tests completed."
