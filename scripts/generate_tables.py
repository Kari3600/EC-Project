import json

from pathlib import Path
import argparse
import importlib.util as importutil

REPO_ROOT = Path(__file__).resolve().parent.parent

def parse_args():
    parser = argparse.ArgumentParser(description="Plot best solutions from run_experiment.py results.")
    parser.add_argument("--source", type=Path, default=REPO_ROOT / "tests" / "lab7_test.py",
                        help="test source file (default: tests/lab7_test.py)")
    parser.add_argument("--data", type=Path, default=REPO_ROOT / "results",
                        help="data source file or path (default: results)")
    return parser.parse_args()

def generate_statistics_table(title, data, value_mapper, vf = lambda x: str(x)):
    maxlen = max(map(len, data.keys()))
    print("\\begin{table}[H] \n" +
          "\\centering \n" +
          "\\small \n" +
          "\\textbf{" + title + "}\\\\[6pt] \n" +
          "\\begin{tabular}{l r@{\ (}r@{ -- }r@{)\\hspace{1.5em}} r@{\ (}r@{ -- }r@{)}} \n" +
          "\\toprule \n" +
          "Method & \\multicolumn{3}{c}{Instance A} & \\multicolumn{3}{c}{Instance B} \\\\ \n" +
          "\\cmidrule(lr){2-4} \cmidrule(l){5-7} \n" +
          "& \\multicolumn{3}{c}{av (min -- max)} & \\multicolumn{3}{c}{av (min -- max)} \\\\ \n" +
          "\\midrule")
    for test_label, instances in data.items():
        print(f"{test_label: <{maxlen+1}}", end="")
        for instance, test_cases in instances.items():
            min_score = float('inf')
            max_score = 0
            avg_score = 0
            score_count = 0
            for test_case in test_cases:
                chain_last = test_case[-1]
                score = value_mapper(chain_last["result"])
                if score == None: continue
                min_score = min(min_score, score)
                max_score = max(max_score, score)
                avg_score += score
                score_count += 1
            if score_count == 0: continue
            avg_score /= score_count
            avg_score = int(avg_score)
            print(f" & {vf(avg_score)} & {vf(min_score)} & {vf(max_score)}", end="")
        print("")
    print("\\bottomrule \n" +
          "\\end{tabular} \n" +
          "\\end{table} \n")

def main():
    args = parse_args()

    source_file = args.source
    spec = importutil.spec_from_file_location(
        source_file.stem, source_file
    )
    module = importutil.module_from_spec(spec)
    spec.loader.exec_module(module)

    data_file = args.data
    if data_file.is_dir():
        data_file = data_file / f"{module.NAME}_results.json"

    with open(data_file, "r") as file:


        data = json.load(file)
        
        # Generate objective table
        generate_statistics_table("Objective function", data, lambda d: d["objective"], vf = lambda v: f"{v:,}".replace(",","\\,"))

        # Generate time table
        generate_statistics_table("Running time [ms]", data, lambda d: d["time"]/1000, vf = lambda v: f"{v:.3f}")

        for label, value_mapper, vf in module.measurements():
            generate_statistics_table(label, data, value_mapper, vf)

if __name__ == "__main__":
    main()