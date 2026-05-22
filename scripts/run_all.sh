#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$REPO_ROOT/results" "$REPO_ROOT/data/output_results"

rm -f "$REPO_ROOT/results/timing.csv" \
      "$REPO_ROOT/results/rmse.csv" \
      "$REPO_ROOT/results/metrics.csv"

"$SCRIPT_DIR/run_serial.sh"
"$SCRIPT_DIR/run_openmp.sh"
"$SCRIPT_DIR/run_mpi.sh"
"$REPO_ROOT/run_hybrid.sh"

python3 "$SCRIPT_DIR/analyze_results.py"

echo ""
echo "All benchmark runs and analysis completed."
