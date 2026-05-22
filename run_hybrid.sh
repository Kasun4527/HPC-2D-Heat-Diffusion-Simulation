#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

mkdir -p "$REPO_ROOT/results" "$REPO_ROOT/data/output_results"

echo "Running Hybrid (MPI + OpenMP) Heat Diffusion Tests"
echo "=================================================="

cd "$REPO_ROOT/hybrid"
make clean && make

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
