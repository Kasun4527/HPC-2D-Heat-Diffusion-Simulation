#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$REPO_ROOT/results" "$REPO_ROOT/data/output_results"

echo "Running Serial Heat Diffusion Tests"
echo "===================================="

cd "$REPO_ROOT/serial"
make clean && make

for size in 500 1000 2000; do
    echo ""
    echo "Testing grid size: ${size}x${size}"
    ./heat_serial "$size" "$size" 1000 0.1
done

echo ""
echo "Serial tests completed."
