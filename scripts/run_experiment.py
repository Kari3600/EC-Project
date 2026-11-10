"""Run the computational experiment by calling out/test_algo for every run.

Each greedy method is started once from every node of every instance; the random
method is run RANDOM_RUNS times with different seeds. Local search with a random
start is run RANDOM_RUNS times with different seeds; with a greedy start it is
started once from every node, building the starting solution with the heuristic
from INIT_HEURISTICS for that instance. Results are written in the format read by
scripts/plot.py:
    <out-dir>/<name>_summary.csv   instance,method,min,max,avg,avg_time_ms,min_time_ms,max_time_ms,
                                   avg_steps,min_steps,max_steps,avg_ls_runs,min_ls_runs,max_ls_runs,
                                   avg_same_optimum_pct,min_same_optimum_pct,max_same_optimum_pct
    <out-dir>/<name>_best.csv      instance,method,value,seed,solution
    <out-dir>/<name>_trace.csv     instance,method,seed,trace   (ILS only, best run of each configuration)
The steps columns count the improving moves applied by local search; ls_runs is the number of LS
runs of ILS and same_optimum_pct the percentage of LS runs after a perturbation that came back to
a solution as good as the current one (empty for the other methods).

Usage (from anywhere):
    python3 scripts/run_experiment.py                  # -> results/lab3_*.csv
    python3 scripts/run_experiment.py --name lab2
    python3 scripts/run_experiment.py --name lab6 --only MSLS ILS   # lab 6 -> results/lab6_*.csv
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

ALGORITHMS.append(("LocalSearchWithCandidates", {"candidatesCount": 10}, lambda size: range(1, RANDOM_RUNS + 1)))
ALGORITHMS.append(("ListOfMovesSolver", {"initialSolver": "RandomSolver"}, lambda size: range(1, RANDOM_RUNS + 1)))

# lab 6: one MSLS run = best of 400 steepest LS (edges) runs from random starts
MSLS_RUNS = 20
ALGORITHMS.append(("MSLS", {"iterations": 400, "localSearch": "ListOfMovesSolver",
                            "initialSolver": "RandomSolver"},
                   lambda size: range(1, MSLS_RUNS + 1)))

# lab 6: ILS with ListOfMovesSolver, stopped after the average running time of MSLS on the instance
# (avg_time_ms of MSLS in results/lab6_summary.csv); a dict value is chosen per instance and is
# not part of the method name
ILS_RUNS = 20
ILS_TIME_MS = {"InstanceA": 1466, "InstanceB": 1423}
ILS_STRENGTHS = {
    "randomMoves": [2, 4, 8, 16, 24, 32],
    "segment": [2, 4, 8, 16],
    "doubleBridge": [1, 2, 3, 4],
    "guided": [2, 4, 8, 16, 24, 32],
}
for perturbation, strengths in ILS_STRENGTHS.items():
    for strength in strengths:
        ALGORITHMS.append(("ILS", {"perturbation": perturbation, "strength": strength, "timeMs": ILS_TIME_MS},
                           lambda size: range(1, ILS_RUNS + 1)))


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
    parser.add_argument("--only", nargs="+",
                        help="run only these algorithms (names as in ALGORITHMS, e.g. MSLS)")
    return parser.parse_args()


def build(binary):
    binary.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["g++", "-std=c++17", "-O2", str(REPO_ROOT / "src" / "TestAlgo.cpp"),
                    "-o", str(binary)], check=True)


def instance_size(path):
    with open(path) as f:
        return sum(1 for line in f if line.strip())


def run_once(binary, algorithm, instance, seed, params):
    """Run test_algo once and return a dict with the objective, time in ms, solution and the
    statistics the method prints (None when it does not print them): objective of the starting
    solution, number of improving moves (local search), number of LS runs, number of LS runs that
    returned to the same optimum and the objective after every LS run (ILS)."""
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
    def optional_int(key):
        return int(fields[key]) if key in fields else None

    return {
        "value": int(fields["Objective"]),
        "time_ms": float(fields["Time"].split()[0]),
        "solution": [int(v) for v in fields["Solution"].split()],
        "initial": optional_int("Initial"),
        "steps": optional_int("Steps"),
        "ls_runs": optional_int("LS runs"),
        "same_optimum": optional_int("Same optimum"),
        "trace": [int(v) for v in fields["Trace"].split()] if "Trace" in fields else None,
    }


def stats_columns(values, digits=2):
    """avg, min, max of a list as CSV cells (empty if the list is empty)."""
    if not values:
        return ["", "", ""]
    return [f"{sum(values) / len(values):.{digits}f}", min(values), max(values)]


def main():
    args = parse_args()
    if not args.no_build:
        build(args.bin)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    summary_path = args.out_dir / f"{args.name}_summary.csv"
    best_path = args.out_dir / f"{args.name}_best.csv"
    trace_path = args.out_dir / f"{args.name}_trace.csv"

    trace_rows = []
    with open(summary_path, "w", newline="") as summary_file, open(best_path, "w", newline="") as best_file:
        summary = csv.writer(summary_file)
        best_writer = csv.writer(best_file)
        summary.writerow(["instance", "method", "min", "max", "avg",
                          "avg_time_ms", "min_time_ms", "max_time_ms",
                          "avg_steps", "min_steps", "max_steps",
                          "avg_ls_runs", "min_ls_runs", "max_ls_runs",
                          "avg_same_optimum_pct", "min_same_optimum_pct", "max_same_optimum_pct"])
        best_writer.writerow(["instance", "method", "value", "seed", "solution"])


        for instance in args.instances:
            instance_name = instance.stem
            size = instance_size(instance)
            print(f"=== {instance_name} (size {size})", flush=True)

            for algorithm, params, seeds_getter in ALGORITHMS:
                if args.only and algorithm not in args.only:
                    continue
                values, times, steps, ls_runs, same_pct = [], [], [], [], []
                best, method = None, algorithm
                run_params = {}
                for paramName, paramValue in params.items():
                    if isinstance(paramValue, dict):   # per-instance value, e.g. the ILS time limit
                        run_params[paramName] = paramValue[instance_name]
                    else:
                        run_params[paramName] = paramValue
                        method += f"_{paramName}={paramValue}"
                seeds = seeds_getter(size)
                for seed in seeds:
                    run = run_once(args.bin, algorithm, instance, seed, run_params)
                    value = run["value"]
                    values.append(value)
                    times.append(run["time_ms"])
                    if run["steps"] is not None:
                        steps.append(run["steps"])
                    if run["ls_runs"] is not None:
                        ls_runs.append(run["ls_runs"])
                        if run["same_optimum"] is not None and run["ls_runs"] > 1:
                            # the first LS run starts from a random solution, not from a perturbation
                            same_pct.append(100 * run["same_optimum"] / (run["ls_runs"] - 1))
                    if best is None or value < best["value"]:
                        best = {**run, "seed": seed}

                avg = sum(values) / len(values)
                avg_time = sum(times) / len(times)
                summary.writerow([instance_name, method, min(values), max(values), f"{avg:.2f}",
                                  f"{avg_time:.4f}", f"{min(times):.4f}", f"{max(times):.4f}",
                                  *stats_columns(steps), *stats_columns(ls_runs),
                                  *[f"{v:.2f}" if isinstance(v, float) else v for v in stats_columns(same_pct)]])
                best_writer.writerow([instance_name, method, best["value"], best["seed"],
                                      " ".join(map(str, best["solution"]))])
                if best["trace"] is not None:
                    trace_rows.append([instance_name, method, best["seed"],
                                       " ".join(map(str, best["trace"]))])
                print(f"{method}: min {min(values)}, max {max(values)}, avg {avg:.2f}, "
                      f"avg time {avg_time:.4f} ms ({len(seeds)} runs)", flush=True)

    print(f"Results saved to {summary_path} and {best_path}")
    if trace_rows:
        # objective after every LS run of the best run of each ILS configuration; the first value is
        # the LS from the random start, the best solution so far is the running minimum
        with open(trace_path, "w", newline="") as trace_file:
            trace_writer = csv.writer(trace_file)
            trace_writer.writerow(["instance", "method", "seed", "trace"])
            trace_writer.writerows(trace_rows)
        print(f"Traces of the best ILS runs saved to {trace_path}")


if __name__ == "__main__":
    main()
