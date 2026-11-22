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
import json
import subprocess
from pathlib import Path
import importlib
import importlib.util as importutil

REPO_ROOT = Path(__file__).resolve().parent.parent

def parse_args():
    parser = argparse.ArgumentParser(description="Run all algorithms on all instances via test_algo.")
    parser.add_argument("--source", type=Path, default=REPO_ROOT / "tests" / "lab7_test.py",
                        help="source of the test")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "results",
                        help="path to the output directory")
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

def run_once(binary, algorithm, instance, initialSolution, params):
    """Run test_algo once and return a dict with the objective, time in ms, solution and the
    statistics the method prints (None when it does not print them): objective of the starting
    solution, number of improving moves (local search), number of LS runs, number of LS runs that
    returned to the same optimum and the objective after every LS run (ILS)."""
    command = [str(binary), algorithm, f'"{str(instance)}"']
    if (len(initialSolution) > 0):
        command.append(",".join(map(lambda v: str(v), initialSolution)))
    for param_name, param_value in params.items() :
        if param_value == None: continue
        command += ["--"+param_name, str(param_value)]
    command += ["--outFormat", "json"]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"{' '.join(command)} failed: {result.stderr.strip()}")

    return json.loads(result.stdout)

def main():
    args = parse_args()
    
    source_file = args.source
    spec = importutil.spec_from_file_location(
        source_file.stem, source_file
    )
    module = importutil.module_from_spec(spec)
    spec.loader.exec_module(module)

    if not args.no_build:
        build(args.bin)

    args.out_dir.mkdir(parents=True, exist_ok=True)

    results_path = args.out_dir / f"{module.NAME}_results.json"    

    with open(results_path, "w", newline="") as results_file:
        results_file.write("{\n")
        for test_label, test_data in module.generate():
            print(f"Test case: {test_label}")
            results_file.write('"'+test_label+'": {\n')
            for instance in args.instances:
                instance_name = instance.stem
                results_file.write('"'+instance_name+'": [\n')
                for test_case in test_data:
                    data = []
                    solution = test_case[0]
                    data.append({"result": {"solution": solution}})
                    for n in range(1,len(test_case)):
                        algorithm, params = test_case[n]
                        run = run_once(args.bin, algorithm, instance, solution, params)
                        data.append({
                            "algorithm": algorithm,
                            "params": params,
                            "result": run
                        })
                        solution = run["solution"]
                    json.dump(data, results_file)
                    results_file.write(",\n")
                results_file.seek(results_file.tell() - 2)
                results_file.write("\n],\n")
            results_file.seek(results_file.tell() - 2)
            results_file.write("\n},\n")
        results_file.seek(results_file.tell() - 2)
        results_file.write("}")
    

        


if __name__ == "__main__":
    main()
