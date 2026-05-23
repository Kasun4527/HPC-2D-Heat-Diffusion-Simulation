#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$REPO_ROOT/results" "$REPO_ROOT/data/output_results"

echo "Running POSIX Threads Heat Diffusion Tests"
echo "==========================================="

cd "$REPO_ROOT/pthreads"
make clean && make

for threads in 1 2 4 8 16; do
    echo ""
    echo "Testing with $threads POSIX threads"
    ./heat_pthreads 1000 1000 1000 0.1 "$threads"
done

echo ""
echo "POSIX Threads tests completed."
