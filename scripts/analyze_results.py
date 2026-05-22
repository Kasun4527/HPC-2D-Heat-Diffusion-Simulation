#!/usr/bin/env python3
"""
Generate HPC metrics and plots from simulation CSV outputs.

Inputs:
  results/timing.csv
  results/rmse.csv

Outputs:
  results/metrics.csv
  results/plots/time_<problem>.png
  results/plots/speedup_<problem>.png
  results/plots/efficiency_<problem>.png
  results/plots/rmse_<problem>.png
"""

from __future__ import annotations

import csv
import math
from collections import defaultdict
from pathlib import Path
from statistics import mean


REPO_ROOT = Path(__file__).resolve().parents[1]
RESULTS_DIR = REPO_ROOT / "results"
PLOTS_DIR = RESULTS_DIR / "plots"
TIMING_CSV = RESULTS_DIR / "timing.csv"
RMSE_CSV = RESULTS_DIR / "rmse.csv"
METRICS_CSV = RESULTS_DIR / "metrics.csv"


def require_matplotlib():
    try:
        import matplotlib.pyplot as plt
    except ImportError as exc:
        raise SystemExit(
            "matplotlib is required for plots. Install it with: "
            "python3 -m pip install matplotlib"
        ) from exc
    return plt


def read_timing_rows():
    if not TIMING_CSV.exists():
        raise SystemExit(f"Missing timing input: {TIMING_CSV}")

    rows = []
    with TIMING_CSV.open(newline="") as file:
        for row in csv.DictReader(file):
            rows.append(
                {
                    "implementation": row["implementation"],
                    "rows": int(row["rows"]),
                    "cols": int(row["cols"]),
                    "timesteps": int(row["timesteps"]),
                    "parallelism": int(row["parallelism"]),
                    "time_seconds": float(row["time_seconds"]),
                }
            )
    return rows


def read_rmse_rows():
    if not RMSE_CSV.exists():
        return []

    rows = []
    with RMSE_CSV.open(newline="") as file:
        for row in csv.DictReader(file):
            rows.append(
                {
                    "implementation": row["implementation"],
                    "rows": int(row["rows"]),
                    "cols": int(row["cols"]),
                    "timesteps": int(row["timesteps"]),
                    "alpha": float(row["alpha"]),
                    "parallelism": int(row["parallelism"]),
                    "rmse": float(row["rmse"]),
                }
            )
    return rows


def aggregate_timing(rows):
    grouped = defaultdict(list)
    for row in rows:
        key = (
            row["implementation"],
            row["rows"],
            row["cols"],
            row["timesteps"],
            row["parallelism"],
        )
        grouped[key].append(row["time_seconds"])

    aggregated = []
    for key, values in sorted(grouped.items()):
        implementation, grid_rows, cols, timesteps, parallelism = key
        aggregated.append(
            {
                "implementation": implementation,
                "rows": grid_rows,
                "cols": cols,
                "timesteps": timesteps,
                "parallelism": parallelism,
                "time_seconds": mean(values),
                "runs": len(values),
            }
        )
    return aggregated


def aggregate_rmse(rows):
    grouped = defaultdict(list)
    for row in rows:
        key = (
            row["implementation"],
            row["rows"],
            row["cols"],
            row["timesteps"],
            row["alpha"],
            row["parallelism"],
        )
        grouped[key].append(row["rmse"])

    aggregated = []
    for key, values in sorted(grouped.items()):
        implementation, grid_rows, cols, timesteps, alpha, parallelism = key
        aggregated.append(
            {
                "implementation": implementation,
                "rows": grid_rows,
                "cols": cols,
                "timesteps": timesteps,
                "alpha": alpha,
                "parallelism": parallelism,
                "rmse": mean(values),
            }
        )
    return aggregated


def compute_metrics(timing_rows):
    serial_times = {}
    for row in timing_rows:
        if row["implementation"] == "serial":
            key = (row["rows"], row["cols"], row["timesteps"])
            current = serial_times.get(key)
            if current is None or row["time_seconds"] < current:
                serial_times[key] = row["time_seconds"]

    metrics = []
    for row in timing_rows:
        problem_key = (row["rows"], row["cols"], row["timesteps"])
        serial_time = serial_times.get(problem_key)
        if serial_time is None:
            speedup = math.nan
            efficiency = math.nan
        else:
            speedup = serial_time / row["time_seconds"]
            efficiency = speedup / row["parallelism"] * 100.0

        metrics.append(
            {
                **row,
                "serial_time_seconds": serial_time if serial_time is not None else math.nan,
                "speedup": speedup,
                "efficiency_percent": efficiency,
            }
        )
    return metrics


def write_metrics(metrics):
    RESULTS_DIR.mkdir(exist_ok=True)
    fieldnames = [
        "implementation",
        "rows",
        "cols",
        "timesteps",
        "parallelism",
        "runs",
        "time_seconds",
        "serial_time_seconds",
        "speedup",
        "efficiency_percent",
    ]
    with METRICS_CSV.open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for row in metrics:
            writer.writerow(row)


def problem_label(row):
    return f"{row['rows']}x{row['cols']}_t{row['timesteps']}"


def group_by_problem(rows):
    grouped = defaultdict(list)
    for row in rows:
        grouped[problem_label(row)].append(row)
    return grouped


def plot_metric(plt, rows, metric, ylabel, title, output_path):
    by_impl = defaultdict(list)
    for row in rows:
        if math.isnan(row.get(metric, math.nan)):
            continue
        by_impl[row["implementation"]].append(row)

    if not by_impl:
        return

    plt.figure(figsize=(8, 5))
    for implementation, impl_rows in sorted(by_impl.items()):
        impl_rows = sorted(impl_rows, key=lambda item: item["parallelism"])
        x_values = [row["parallelism"] for row in impl_rows]
        y_values = [row[metric] for row in impl_rows]
        plt.plot(x_values, y_values, marker="o", linewidth=2, label=implementation)

    plt.xlabel("Parallelism (threads/processes/total workers)")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=200)
    plt.close()


def plot_rmse(plt, rmse_rows):
    by_problem = group_by_problem(rmse_rows)
    for label, rows in sorted(by_problem.items()):
        rows = [row for row in rows if row["implementation"] != "serial"]
        if not rows:
            continue

        rows = sorted(rows, key=lambda item: (item["implementation"], item["parallelism"]))
        names = [
            f"{row['implementation']}\n{row['parallelism']}"
            for row in rows
        ]
        values = [row["rmse"] for row in rows]

        plt.figure(figsize=(8, 5))
        plt.bar(names, values)
        plt.xlabel("Implementation and parallelism")
        plt.ylabel("RMSE vs serial")
        plt.title(f"Accuracy Comparison ({label})")
        plt.grid(True, axis="y", linestyle="--", alpha=0.4)
        plt.tight_layout()
        plt.savefig(PLOTS_DIR / f"rmse_{label}.png", dpi=200)
        plt.close()


def generate_plots(metrics, rmse_rows):
    plt = require_matplotlib()
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)

    for label, rows in sorted(group_by_problem(metrics).items()):
        plot_metric(
            plt,
            rows,
            "time_seconds",
            "Execution time (seconds)",
            f"Execution Time ({label})",
            PLOTS_DIR / f"time_{label}.png",
        )
        plot_metric(
            plt,
            rows,
            "speedup",
            "Speedup",
            f"Speedup vs Serial ({label})",
            PLOTS_DIR / f"speedup_{label}.png",
        )
        plot_metric(
            plt,
            rows,
            "efficiency_percent",
            "Efficiency (%)",
            f"Parallel Efficiency ({label})",
            PLOTS_DIR / f"efficiency_{label}.png",
        )

    plot_rmse(plt, rmse_rows)


def main():
    timing_rows = read_timing_rows()
    rmse_rows = aggregate_rmse(read_rmse_rows())
    aggregated_timing = aggregate_timing(timing_rows)
    metrics = compute_metrics(aggregated_timing)

    write_metrics(metrics)
    generate_plots(metrics, rmse_rows)

    print(f"Wrote metrics: {METRICS_CSV}")
    print(f"Wrote plots to: {PLOTS_DIR}")


if __name__ == "__main__":
    main()
