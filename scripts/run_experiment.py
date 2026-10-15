"""Run the computational experiment by calling out/test_algo for every run.

Each greedy method is started once from every node of every instance; the random
method is run RANDOM_RUNS times with different seeds. Results are written in the
format read by scripts/plot.py:
    <out-dir>/<name>_summary.csv   instance,method,min,max,avg,avg_time_ms
    <out-dir>/<name>_best.csv      instance,method,value,solution

Usage (from anywhere):
    python3 scripts/run_experiment.py                  # -> results/lab2_*.csv
    python3 scripts/run_experiment.py --name lab3
"""

import argparse
import csv
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# (algorithm name in test_algo, alpha or None); to add a method, add a line here
ALGORITHMS = [
    ("RandomSolver", None),
    ("NearestNeighborI", None),
    ("NearestNeighborII", None),
    ("GreedyCycle", None),
    ("NearestNeighborII_2regret", 1.0),
    ("NearestNeighborII_2regret", 0.5),
    ("GreedyCycle_2regret", 1.0),
    ("GreedyCycle_2regret", 0.5),
]
RANDOM_RUNS = 200


def parse_args():
    parser = argparse.ArgumentParser(description="Run all algorithms on all instances via test_algo.")
    parser.add_argument("--name", default="lab2",
                        help="prefix of the result files (default: lab2)")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "results",
                        help="directory for the result files (default: results)")
    parser.add_argument("--instances", type=Path, nargs="+",
                        default=[REPO_ROOT / "data" / "InstanceA.csv", REPO_ROOT / "data" / "InstanceB.csv"],
                        help="instance files (default: data/InstanceA.csv data/InstanceB.csv)")
    parser.add_argument("--bin", type=Path, default=REPO_ROOT / "out" / "test_algo",
                        help="test_algo binary (default: out/test_algo)")
    parser.add_argument("--no-build", action="store_true",
                        help="do not compile src/TestAlgo.cpp before running")
    return parser.parse_args()


def build(binary):
    binary.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["g++", "-std=c++17", "-O2", str(REPO_ROOT / "src" / "TestAlgo.cpp"),
                    "-o", str(binary)], check=True)


def instance_size(path):
    with open(path) as f:
        return sum(1 for line in f if line.strip())


def run_once(binary, algorithm, alpha, instance, start, seed=None):
    """Run test_algo once and return (method name, objective, time in ms, solution)."""
    command = [str(binary), algorithm, str(instance), str(start)]
    if alpha is not None:
        command += ["--alpha", str(alpha)]
    if seed is not None:
        command += ["--seed", str(seed)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"{' '.join(command)} failed: {result.stderr.strip()}")

    fields = {}
    for line in result.stdout.splitlines():
        key, _, value = line.partition(":")
        fields[key.strip()] = value.strip()
    return (fields["Algorithm"],
            int(fields["Objective"]),
            float(fields["Time"].split()[0]),
            [int(v) for v in fields["Solution"].split()])


def main():
    args = parse_args()
    if not args.no_build:
        build(args.bin)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    summary_path = args.out_dir / f"{args.name}_summary.csv"
    best_path = args.out_dir / f"{args.name}_best.csv"

    with open(summary_path, "w", newline="") as summary_file, open(best_path, "w", newline="") as best_file:
        summary = csv.writer(summary_file)
        best_writer = csv.writer(best_file)
        summary.writerow(["instance", "method", "min", "max", "avg", "avg_time_ms"])
        best_writer.writerow(["instance", "method", "value", "solution"])

        for instance in args.instances:
            instance_name = instance.stem
            size = instance_size(instance)
            print(f"=== {instance_name} (size {size})", flush=True)

            for algorithm, alpha in ALGORITHMS:
                # Random: RANDOM_RUNS different seeds (glibc treats seed 0 like 1, so start at 1);
                # greedy methods: one run from every start node
                if algorithm == "RandomSolver":
                    runs = [(0, seed) for seed in range(1, RANDOM_RUNS + 1)]
                else:
                    runs = [(start, None) for start in range(size)]

                values, times = [], []
                best_value, best_solution, method = None, None, algorithm
                for start, seed in runs:
                    method, value, time_ms, solution = run_once(args.bin, algorithm, alpha, instance, start, seed)
                    values.append(value)
                    times.append(time_ms)
                    if best_value is None or value < best_value:
                        best_value, best_solution = value, solution

                avg = sum(values) / len(values)
                avg_time = sum(times) / len(times)
                summary.writerow([instance_name, method, min(values), max(values),
                                  f"{avg:.2f}", f"{avg_time:.4f}"])
                best_writer.writerow([instance_name, method, best_value, " ".join(map(str, best_solution))])
                print(f"{method}: min {min(values)}, max {max(values)}, avg {avg:.2f}, "
                      f"avg time {avg_time:.4f} ms ({len(runs)} runs)", flush=True)

    print(f"Results saved to {summary_path} and {best_path}")


if __name__ == "__main__":
    main()
