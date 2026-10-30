"""Run the computational experiment by calling out/test_algo for every run.

Each greedy method is started once from every node of every instance; the random
method is run RANDOM_RUNS times with different seeds. Local search with a random
start is run RANDOM_RUNS times with different seeds; with a greedy start it is
started once from every node, building the starting solution with the heuristic
from INIT_HEURISTICS for that instance. Results are written in the format read by
scripts/plot.py:
    <out-dir>/<name>_summary.csv   instance,method,min,max,avg,avg_time_ms,min_time_ms,max_time_ms,
                                   avg_steps,min_steps,max_steps
    <out-dir>/<name>_best.csv      instance,method,value,start,seed,initial_value,solution
For local search, start/seed/initial_value describe the starting solution of the best run, and
the steps columns count the improving moves applied (empty for the other methods).

Usage (from anywhere):
    python3 scripts/run_experiment.py                  # -> results/lab3_*.csv
    python3 scripts/run_experiment.py --name lab2
"""

import argparse
import csv
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

RANDOM_RUNS = 200

ALGORITHMS = [
    ("RandomSolver", {}, lambda size: range(1, RANDOM_RUNS + 1)),
    ("NearestNeighborI", {}, lambda size: range(size)),
    ("NearestNeighborII", {}, lambda size: range(size)),
    ("GreedyCycle", {}, lambda size: range(size)),
    ("NearestNeighborII_2regret", {"alpha": 1.0}, lambda size: range(size)),
    ("NearestNeighborII_2regret", {"alpha": 0.5}, lambda size: range(size)),
    ("GreedyCycle_2regret", {"alpha": 1.0}, lambda size: range(size)),
    ("GreedyCycle_2regret", {"alpha": 0.5}, lambda size: range(size)),
]

for search in ["Greedy", "Steepest"]:
    for intra in ["Nodes", "Edges"]:
        ALGORITHMS.append(("LocalSearch", {"searchType": search, "intraMove": intra, "initialSolver": "RandomSolver"}, lambda size: range(1, RANDOM_RUNS + 1)))
        ALGORITHMS.append(("LocalSearch", {"searchType": search, "intraMove": intra, "initialSolver": "GreedyCycle_2regret", "alpha": 0.5}, lambda size: range(size)))

def parse_args():
    parser = argparse.ArgumentParser(description="Run all algorithms on all instances via test_algo.")
    parser.add_argument("--name", default="lab3",
                        help="prefix of the result files (default: lab3)")
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


def run_once(binary, algorithm, instance, seed, params):
    """Run test_algo once and return (method name, objective, time in ms, solution,
    objective of the starting solution, number of improving moves); the last two are None
    if the method is not local search."""
    command = [str(binary), algorithm, f'"{str(instance)}"', '--seed', str(seed)]
    for param_name, param_value in params.items() :
        command += ["--"+param_name, str(param_value)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"{' '.join(command)} failed: {result.stderr.strip()}")

    fields = {}
    for line in result.stdout.splitlines():
        key, _, value = line.partition(":")
        fields[key.strip()] = value.strip()
    return (int(fields["Objective"]),
            float(fields["Time"].split()[0]),
            [int(v) for v in fields["Solution"].split()],
            int(fields["Initial"]) if "Initial" in fields else None,
            int(fields["Steps"]) if "Steps" in fields else None)


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
        summary.writerow(["instance", "method", "min", "max", "avg",
                          "avg_time_ms", "min_time_ms", "max_time_ms",
                          "avg_steps", "min_steps", "max_steps"])
        best_writer.writerow(["instance", "method", "value", "seed", "solution"])

        for instance in args.instances:
            instance_name = instance.stem
            size = instance_size(instance)
            print(f"=== {instance_name} (size {size})", flush=True)

            for algorithm, params, seeds_getter in ALGORITHMS:
                values, times, steps = [], [], []
                best, method = None, algorithm
                for paramName, paramValue in params.items():
                    method += f"_{paramName}={paramValue}"
                seeds = seeds_getter(size)
                for seed in seeds:
                    value, time_ms, solution, initial, step_count = run_once(
                        args.bin, algorithm, instance, seed, params)
                    values.append(value)
                    times.append(time_ms)
                    if step_count is not None:
                        steps.append(step_count)
                    if best is None or value < best[0]:
                        best = (value, seed, solution)

                avg = sum(values) / len(values)
                avg_time = sum(times) / len(times)
                summary.writerow([instance_name, method, min(values), max(values), f"{avg:.2f}",
                                  f"{avg_time:.4f}", f"{min(times):.4f}", f"{max(times):.4f}",
                                  f"{sum(steps) / len(steps):.2f}" if steps else "",
                                  min(steps) if steps else "", max(steps) if steps else ""])
                best_value, best_seed, best_solution = best
                best_writer.writerow([instance_name, method, best_value, best_seed,
                                      " ".join(map(str, best_solution))])
                print(f"{method}: min {min(values)}, max {max(values)}, avg {avg:.2f}, "
                      f"avg time {avg_time:.4f} ms ({len(seeds)} runs)", flush=True)

    print(f"Results saved to {summary_path} and {best_path}")


if __name__ == "__main__":
    main()
