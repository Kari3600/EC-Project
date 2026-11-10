"""Plot the convergence of ILS for every perturbation and strength k.

Reads the traces of the best run of each ILS configuration (results/lab6_trace.csv, the objective
after every LS run) and the averages over all runs (results/lab6_summary.csv), and saves one figure
per instance with a 2x2 grid of subplots, one per perturbation. In each subplot, every k has:
    small dots  objective of the solution after each LS run (f(y)),
    line        best objective so far (f(x)),
    star        LS run where the best solution of the run was found.
The x axis is the number of the LS run, so the length of a line is the number of LS runs done
within the time limit. The legend gives the average objective and LS runs over all runs.

Usage (from anywhere):
    python3 scripts/lab6/plot_convergence.py
    python3 scripts/lab6/plot_convergence.py --logx
"""

import argparse
import csv
import re
from itertools import accumulate
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # render to file, no window needed
import matplotlib.pyplot as plt

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

PERTURBATIONS = ["randomMoves", "segment", "doubleBridge", "guided"]
TITLES = {
    "randomMoves": "k random moves (two-edges / node exchange)",
    "segment": "k consecutive nodes replaced",
    "doubleBridge": "k double-bridges",
    "guided": "k costly nodes replaced by near ones",
}
# colorblind-safe, one color per k (in increasing order of k)
K_COLORS = ["#e69f00", "#0072b2", "#009e73", "#cc79a7"]
SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
METHOD_PATTERN = re.compile(r"ILS_perturbation=(\w+)_strength=(\d+)")


def parse_args():
    parser = argparse.ArgumentParser(description="Plot ILS convergence from run_experiment.py results.")
    parser.add_argument("--trace", type=Path, default=REPO_ROOT / "results" / "lab6_trace.csv")
    parser.add_argument("--summary", type=Path, default=REPO_ROOT / "results" / "lab6_summary.csv")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "reports" / "figures")
    parser.add_argument("--name", default="lab6_convergence",
                        help="figure name prefix, saved as <name>_<instance>.<format>")
    parser.add_argument("--format", default="png")
    parser.add_argument("--logx", action="store_true", help="logarithmic x axis")
    return parser.parse_args()


def read_csv(path):
    csv.field_size_limit(10**8)   # a trace has one value per LS run, often over 100k characters
    with open(path) as f:
        return list(csv.DictReader(f))


def main():
    args = parse_args()

    averages = {}   # (instance, perturbation, k) -> (avg objective, avg LS runs)
    for row in read_csv(args.summary):
        match = METHOD_PATTERN.fullmatch(row["method"])
        if match:
            key = (row["instance"], match.group(1), int(match.group(2)))
            averages[key] = (float(row["avg"]), float(row["avg_ls_runs"]))

    traces = {}   # instance -> perturbation -> [(k, seed, trace)]
    for row in read_csv(args.trace):
        match = METHOD_PATTERN.fullmatch(row["method"])
        if match:
            trace = [int(v) for v in row["trace"].split()]
            traces.setdefault(row["instance"], {}).setdefault(match.group(1), []).append(
                (int(match.group(2)), int(row["seed"]), trace))

    args.out_dir.mkdir(parents=True, exist_ok=True)
    for instance, by_perturbation in traces.items():
        all_traces = [t for runs in by_perturbation.values() for _, _, t in runs]
        y_low = min(min(t) for t in all_traces)
        y_high = max(t[0] for t in all_traces)   # every f(x) line is fully visible
        margin = 0.03 * (y_high - y_low)

        fig, axes = plt.subplots(2, 2, figsize=(13, 9), sharey=True, facecolor=SURFACE)
        for ax, perturbation in zip(axes.flat, PERTURBATIONS):
            ax.set_facecolor(SURFACE)
            runs = sorted(by_perturbation.get(perturbation, []))
            for color, (k, seed, trace) in zip(K_COLORS, runs):
                xs = range(1, len(trace) + 1)
                best_so_far = list(accumulate(trace, min))
                best_at = trace.index(best_so_far[-1])
                avg, avg_runs = averages.get((instance, perturbation, k), (float("nan"), float("nan")))

                ax.scatter(xs, trace, s=2, color=color, alpha=0.12, linewidths=0, rasterized=True)
                ax.plot(xs, best_so_far, color=color, linewidth=1.6,
                        label=f"k={k}: avg {avg:,.0f}, {avg_runs:,.0f} LS runs".replace(",", " "))
                ax.plot(best_at + 1, trace[best_at], marker="*", markersize=12, color=color,
                        markeredgecolor=TEXT_PRIMARY, markeredgewidth=0.6, linestyle="none")

            ax.set_title(TITLES[perturbation], color=TEXT_PRIMARY, fontsize=11)
            ax.set_ylim(y_low - margin, y_high + margin)
            if args.logx:
                ax.set_xscale("log")
            ax.set_xlabel("LS run", color=TEXT_SECONDARY)
            ax.set_ylabel("objective", color=TEXT_SECONDARY)
            ax.tick_params(colors=TEXT_SECONDARY, labelsize=9)
            ax.grid(color="#e4e3df", linewidth=0.6)
            for spine in ax.spines.values():
                spine.set_color("#d0cfca")
            ax.legend(fontsize=8.5, frameon=False, loc="upper right",
                      title="line: best so far (run with the best result)", title_fontsize=8)

        fig.suptitle(f"ILS convergence, {instance} (dots: objective after each LS run, star: best found)",
                     color=TEXT_PRIMARY, fontsize=13)
        fig.tight_layout()
        out_path = args.out_dir / f"{args.name}_{instance}.{args.format}"
        fig.savefig(out_path, dpi=150, facecolor=SURFACE)
        plt.close(fig)
        print(f"Saved {out_path}")


if __name__ == "__main__":
    main()
