"""Compare the weight alpha of the weighted 2-regret heuristics (lab 2).

Criterion maximized by the methods: alpha * regret - (1 - alpha) * bestChange.
For every alpha, each method is started once from every node of every instance.

Writes:
    <out-dir>/lab2_alpha_sweep.csv          instance,method,alpha,min,max,avg
    <figure-dir>/lab2_alpha_sweep.png       best objective vs alpha, one panel per instance

Usage (from anywhere):
    python3 scripts/lab2/alpha_sweep.py
    python3 scripts/lab2/alpha_sweep.py --alphas 0 0.25 0.5 0.75 1
    python3 scripts/lab2/alpha_sweep.py --plot-only    # redraw the figure from the CSV
"""

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from run_experiment import REPO_ROOT, build, instance_size, run_once  # noqa: E402

import matplotlib  # noqa: E402

matplotlib.use("Agg")  # render to file, no window needed
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402
from matplotlib.ticker import FuncFormatter  # noqa: E402

METHODS = ["NearestNeighborII_2regret", "GreedyCycle_2regret"]
# Categorical slots 1-2 (blue, orange), validated for colorblind separation
COLORS = {"NearestNeighborII_2regret": "#2a78d6", "GreedyCycle_2regret": "#eb6834"}
LABELS = {"NearestNeighborII_2regret": "NN II 2-regret", "GreedyCycle_2regret": "Greedy cycle 2-regret"}
SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRID = "#e6e5e1"


def parse_args():
    parser = argparse.ArgumentParser(description="Best objective of the 2-regret methods for different alpha.")
    parser.add_argument("--alphas", type=float, nargs="+",
                        default=[round(i / 10, 1) for i in range(11)],
                        help="alpha values to test (default: 0, 0.1, ..., 1)")
    parser.add_argument("--instances", type=Path, nargs="+",
                        default=[REPO_ROOT / "data" / "InstanceA.csv", REPO_ROOT / "data" / "InstanceB.csv"],
                        help="instance files (default: data/InstanceA.csv data/InstanceB.csv)")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "results",
                        help="directory for the CSV (default: results)")
    parser.add_argument("--figure-dir", type=Path, default=REPO_ROOT / "reports" / "figures",
                        help="directory for the figure (default: reports/figures)")
    parser.add_argument("--bin", type=Path, default=REPO_ROOT / "out" / "test_algo",
                        help="test_algo binary (default: out/test_algo)")
    parser.add_argument("--no-build", action="store_true",
                        help="do not compile src/TestAlgo.cpp before running")
    parser.add_argument("--plot-only", action="store_true",
                        help="do not run anything, only redraw the figure from the existing CSV")
    return parser.parse_args()


def run_sweep(args):
    """Return rows (instance, method, alpha, min, max, avg)."""
    rows = []
    for instance in args.instances:
        size = instance_size(instance)
        for method in METHODS:
            for alpha in args.alphas:
                values = [run_once(args.bin, method, alpha, instance, start)[1] for start in range(size)]
                row = (instance.stem, method, alpha, min(values), max(values), sum(values) / len(values))
                rows.append(row)
                print(f"{row[0]} {method} alpha={alpha}: min {row[3]}, max {row[4]}, avg {row[5]:.2f}",
                      flush=True)
    return rows


def plot(rows, instances, out_path):
    fig, axes = plt.subplots(1, len(instances), figsize=(5.5 * len(instances), 4),
                             squeeze=False, layout="constrained", facecolor=SURFACE)
    for ax, instance in zip(axes[0], instances):
        ax.set_facecolor(SURFACE)
        for method in METHODS:
            points = [(alpha, best) for inst, m, alpha, best, _, _ in rows if inst == instance and m == method]
            alphas = [a for a, _ in points]
            bests = [b for _, b in points]
            ax.plot(alphas, bests, color=COLORS[method], linewidth=2, marker="o", markersize=6,
                    markeredgecolor=SURFACE, markeredgewidth=1.5, label=LABELS[method], zorder=3)
            # Ring around the best alpha of each method (exact values are in the CSV / report table)
            best_alpha, best_value = min(points, key=lambda p: p[1])
            ax.scatter([best_alpha], [best_value], s=160, facecolors="none",
                       edgecolors=COLORS[method], linewidths=1.5, zorder=4)

        ax.set_title(instance, fontsize=11, color=TEXT_PRIMARY)
        ax.set_xlabel("alpha (weight of regret)", color=TEXT_SECONDARY)
        ax.set_ylabel("best objective (lower is better)", color=TEXT_SECONDARY)
        ax.grid(axis="y", color=GRID, linewidth=0.8)
        ax.set_axisbelow(True)
        ax.tick_params(colors=TEXT_SECONDARY, labelsize=8)
        ax.yaxis.set_major_formatter(FuncFormatter(lambda v, _: f"{v:,.0f}".replace(",", " ")))
        for side in ("top", "right"):
            ax.spines[side].set_visible(False)
        for side in ("left", "bottom"):
            ax.spines[side].set_color("#d9d8d4")

    handles, labels = axes[0][0].get_legend_handles_labels()
    handles.append(Line2D([], [], marker="o", linestyle="", markersize=10, markerfacecolor="none",
                          markeredgecolor=TEXT_SECONDARY, markeredgewidth=1.5))
    labels.append("best alpha of the method")
    fig.legend(handles, labels, loc="outside lower center", ncol=len(handles),
               frameon=False, fontsize=9, labelcolor=TEXT_SECONDARY)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=200, facecolor=SURFACE)
    plt.close(fig)


def read_csv(path):
    with open(path, newline="") as f:
        return [(r["instance"], r["method"], float(r["alpha"]), int(r["min"]), int(r["max"]), float(r["avg"]))
                for r in csv.DictReader(f)]


def main():
    args = parse_args()
    csv_path = args.out_dir / "lab2_alpha_sweep.csv"

    if args.plot_only:
        rows = read_csv(csv_path)
    else:
        if not args.no_build:
            build(args.bin)
        rows = run_sweep(args)
        args.out_dir.mkdir(parents=True, exist_ok=True)
        with open(csv_path, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["instance", "method", "alpha", "min", "max", "avg"])
            for instance, method, alpha, best, worst, avg in rows:
                writer.writerow([instance, method, alpha, best, worst, f"{avg:.2f}"])

    figure_path = args.figure_dir / "lab2_alpha_sweep.png"
    plot(rows, [instance.stem for instance in args.instances], figure_path)
    print(f"Saved {csv_path} and {figure_path}")


if __name__ == "__main__":
    main()
