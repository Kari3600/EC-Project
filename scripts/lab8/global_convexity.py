"""Global convexity (fitness-similarity correlation) tests for lab 8.

Reads the 1000 local optima of every instance from results/lab8_results.json (written by
scripts/run_experiment.py --source tests/lab8_test.py) and the very good solution of every
instance (the best ILS solution from results/lab6_best.csv). For every local optimum it computes
three similarities with two measures (common edges, common selected nodes):
    average similarity to all the other 999 local optima,
    similarity to the best of the 1000 local optima (the best itself is left out),
    similarity to the very good (ILS) solution.
Saves one figure per instance with a 3x2 grid of scatter plots (objective vs similarity) to
reports/figures/lab8_<instance>.png and prints a LaTeX table of the correlation coefficients.

Usage (from anywhere):
    python3 scripts/lab8/global_convexity.py
"""

import argparse
import csv
import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # render to file, no window needed
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import spearmanr

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

POINT_COLOR = "#2a78d6"
TREND_COLOR = "#52514e"
SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRID_COLOR = "#e4e3df"

VERSIONS = ["Average similarity to all local optima",
            "Similarity to the best of 1000 local optima",
            "Similarity to the very good (ILS) solution"]
MEASURES = ["Common edges", "Common selected nodes"]


def parse_args():
    parser = argparse.ArgumentParser(description="Fitness-similarity correlations of local optima.")
    parser.add_argument("--results", type=Path, default=REPO_ROOT / "results" / "lab8_results.json")
    parser.add_argument("--good", type=Path, default=REPO_ROOT / "results" / "lab6_best.csv",
                        help="CSV with very good solutions; the best one of every instance is used")
    parser.add_argument("--data-dir", type=Path, default=REPO_ROOT / "data")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "reports" / "figures")
    parser.add_argument("--dpi", type=int, default=200)
    return parser.parse_args()


def read_instance(path):
    nodes = []
    with open(path) as f:
        for line in f:
            if line.strip():
                x, y, cost = map(int, line.strip().split(";"))
                nodes.append((x, y, cost))
    return nodes


def evaluate(nodes, solution):
    """Objective as in Problem::evaluate: rounded Euclidean cycle length + node costs."""
    total = 0
    for a, b in zip(solution, solution[1:] + solution[:1]):
        total += int(math.floor(math.hypot(nodes[a][0] - nodes[b][0], nodes[a][1] - nodes[b][1]) + 0.5))
        total += nodes[a][2]
    return total


def edge_set(solution):
    """Undirected edges of the cycle, including the edge from the last node back to the first."""
    return {(min(a, b), max(a, b)) for a, b in zip(solution, solution[1:] + solution[:1])}


def common_edges(a, b):
    return len(a["edges"] & b["edges"])


def common_nodes(a, b):
    return len(a["nodes"] & b["nodes"])


def describe(solution, objective):
    return {"solution": solution, "objective": objective,
            "nodes": set(solution), "edges": edge_set(solution)}


def load_local_optima(path):
    with open(path) as f:
        data = json.load(f)
    (test_label, instances), = data.items()
    return {instance: [describe(chain[-1]["result"]["solution"], chain[-1]["result"]["objective"])
                       for chain in chains]
            for instance, chains in instances.items()}


def load_good_solutions(path):
    best = {}
    with open(path) as f:
        for row in csv.DictReader(f):
            value = int(row["value"])
            if row["instance"] not in best or value < best[row["instance"]][1]:
                best[row["instance"]] = (row["method"], value, list(map(int, row["solution"].split())))
    return best


def similarities(optima, good, measure):
    """Returns, for the three versions, the (objectives, similarities) pairs to plot."""
    n = len(optima)
    objectives = np.array([o["objective"] for o in optima])

    matrix = np.zeros((n, n))
    for i in range(n):
        for j in range(i + 1, n):
            matrix[i, j] = matrix[j, i] = measure(optima[i], optima[j])
    average = matrix.sum(axis=1) / (n - 1)

    best = int(np.argmin(objectives))
    others = [i for i in range(n) if i != best]   # the best itself would have 100% similarity
    to_best = matrix[best, others]

    to_good = np.array([measure(o, good) for o in optima])

    return [(objectives, average), (objectives[others], to_best), (objectives, to_good)]


def plot_panel(ax, x, y, title, r, rho):
    ax.set_facecolor(SURFACE)
    ax.scatter(x, y, s=9, color=POINT_COLOR, alpha=0.45, linewidths=0)
    slope, intercept = np.polyfit(x, y, 1)
    xs = np.array([x.min(), x.max()])
    ax.plot(xs, slope * xs + intercept, color=TREND_COLOR, linewidth=1.5, linestyle="--")
    ax.set_title(title, fontsize=9.5, color=TEXT_PRIMARY, loc="left")
    ax.text(0.98, 0.95, f"Pearson r = {r:.3f}\nSpearman ρ = {rho:.3f}", transform=ax.transAxes,
            ha="right", va="top", fontsize=8.5, color=TEXT_PRIMARY,
            bbox=dict(boxstyle="round,pad=0.3", facecolor="white", edgecolor=GRID_COLOR))
    ax.grid(True, color=GRID_COLOR, linewidth=0.6)
    ax.set_axisbelow(True)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(TEXT_SECONDARY)
    ax.tick_params(colors=TEXT_SECONDARY, labelsize=8)
    ax.set_xlabel("Objective function", fontsize=8.5, color=TEXT_SECONDARY)


def latex_number(v):
    return f"{v:.3f}".replace("-", "$-$")


def main():
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    optima = load_local_optima(args.results)
    good = load_good_solutions(args.good)

    correlations = {}   # (version, measure) -> {instance: (r, rho)}
    for instance, instance_optima in optima.items():
        nodes = read_instance(args.data_dir / f"{instance}.csv")
        method, value, solution = good[instance]
        assert evaluate(nodes, solution) == value, "very good solution does not match its objective"
        for o in instance_optima:
            assert evaluate(nodes, o["solution"]) == o["objective"]
        good_solution = describe(solution, value)
        print(f"{instance}: {len(instance_optima)} local optima, "
              f"best {min(o['objective'] for o in instance_optima)}, very good solution {value} ({method})")

        fig, axes = plt.subplots(3, 2, figsize=(11, 12.5), facecolor="white")
        for col, (measure_name, measure) in enumerate([(MEASURES[0], common_edges), (MEASURES[1], common_nodes)]):
            for row, (x, y) in enumerate(similarities(instance_optima, good_solution, measure)):
                r = float(np.corrcoef(x, y)[0, 1])
                rho = float(spearmanr(x, y).statistic)
                correlations.setdefault((VERSIONS[row], measure_name), {})[instance] = (r, rho)
                ax = axes[row, col]
                plot_panel(ax, x, y, f"{VERSIONS[row]}\n{measure_name.lower()}", r, rho)
                ax.set_ylabel(("Average number of " if row == 0 else "Number of ") + measure_name.lower(),
                              fontsize=8.5, color=TEXT_SECONDARY)
        fig.suptitle(f"Fitness-similarity correlation of 1000 greedy local optima, {instance}",
                     fontsize=12, color=TEXT_PRIMARY)
        fig.tight_layout(rect=(0, 0, 1, 0.98))
        out = args.out_dir / f"lab8_{instance}.png"
        fig.savefig(out, dpi=args.dpi)
        plt.close(fig)
        print(f"saved {out}")

    instances = list(optima)
    print()
    print("\\begin{table}[H] \n"
          "\\centering \n"
          "\\small \n"
          "\\textbf{Correlation coefficients between the objective function and similarity}\\\\[6pt] \n"
          "\\begin{tabular}{l l r r r r} \n"
          "\\toprule \n"
          "Similarity & Measure & \\multicolumn{2}{c}{Instance A} & \\multicolumn{2}{c}{Instance B} \\\\ \n"
          "\\cmidrule(lr){3-4} \\cmidrule(l){5-6} \n"
          "& & Pearson & Spearman & Pearson & Spearman \\\\ \n"
          "\\midrule")
    for (version, measure), values in correlations.items():
        cells = "".join(f" & {latex_number(values[i][0])} & {latex_number(values[i][1])}" for i in instances)
        print(f"{version} & {measure.lower()}{cells} \\\\")
    print("\\bottomrule \n"
          "\\end{tabular} \n"
          "\\end{table}")


if __name__ == "__main__":
    main()
