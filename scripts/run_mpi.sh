#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$REPO_ROOT/results" "$REPO_ROOT/data/output_results"

echo "Running MPI Heat Diffusion Tests"
echo "================================="

cd "$REPO_ROOT/mpi"
make clean && make

for procs in 1 2 4 8; do
    echo ""
    echo "Testing with $procs MPI processes"
    mpirun -np "$procs" ./heat_mpi 1000 1000 1000 0.1
done

echo ""
echo "MPI tests completed."
