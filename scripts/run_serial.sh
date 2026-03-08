#!/bin/bash
# Run serial implementation with various grid sizes

echo "Running Serial Heat Diffusion Tests"
echo "===================================="

cd ../serial

# Make sure the executable is built
make clean && make

# Test different grid sizes
for size in 500 1000 2000; do
    echo ""
    echo "Testing grid size: ${size}x${size}"
    ./heat_serial $size $size 1000 0.1
done

echo ""
echo "Serial tests completed."
