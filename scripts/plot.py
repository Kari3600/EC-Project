"""Plot the best solution of each method on each instance.

Reads the best solutions written by scripts/run_experiment.py (results/lab1_best.csv) and the node
coordinates/costs from data/<instance>.csv, and saves one figure per instance with
a 2x2 grid of subplots (one per method), named <name>_<instance>.<format>.

Usage (from anywhere):
    python3 scripts/plot.py
    python3 scripts/plot.py --out-dir reports/figures --name lab1_plot
"""

import argparse
import csv
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # render to file, no window needed
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, Normalize
from matplotlib.lines import Line2D

REPO_ROOT = Path(__file__).resolve().parent.parent

# Sequential blue ramp (light -> dark) for node cost; lightest steps are skipped
# so cheap nodes stay visible on the white background.
COST_CMAP = LinearSegmentedColormap.from_list(
    "cost", ["#9ec5f4", "#5598e7", "#2a78d6", "#1c5cab", "#104281", "#0d366b"]
)
SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
EDGE_COLOR = "#52514e"


def parse_args():
    parser = argparse.ArgumentParser(description="Plot best solutions from run_experiment.py results.")
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "reports" / "figures",
                        help="directory to save the figure in (default: reports/figures)")
    parser.add_argument("--name", default="lab1_plot",
                        help="file name prefix; the instance name is appended (default: lab1_plot)")
    parser.add_argument("--format", default="png", help="image format, e.g. png, pdf, svg (default: png)")
    parser.add_argument("--results", type=Path, default=REPO_ROOT / "results" / "lab1_best.csv",
                        help="best solutions CSV written by run_experiment.py (default: results/lab1_best.csv)")
    parser.add_argument("--data-dir", type=Path, default=REPO_ROOT / "data",
                        help="directory with instance CSV files (default: data)")
    parser.add_argument("--dpi", type=int, default=200, help="resolution for raster formats (default: 200)")
    parser.add_argument("--method-prefix", default="",
                        help="plot only methods whose name starts with this, e.g. LS_ (default: all)")
    return parser.parse_args()


def read_instance(path):
    """Return lists of x, y, cost for each node (rows like '2944;497;1607')."""
    xs, ys, costs = [], [], []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            x, y, cost = (int(v) for v in line.replace(",", ";").split(";"))
            xs.append(x)
            ys.append(y)
            costs.append(cost)
    return xs, ys, costs


def read_best(path, method_prefix=""):
    """Return {instance: [(method, value, [nodes...]), ...]} keeping file order."""
    best = {}
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            if not row["method"].startswith(method_prefix):
                continue
            nodes = [int(v) for v in row["solution"].split()]
            best.setdefault(row["instance"], []).append((row["method"], int(row["value"]), nodes))
    return best


def plot_solution(ax, xs, ys, costs, nodes, norm):
    selected = set(nodes)
    others = [i for i in range(len(xs)) if i not in selected]

    # Path through the selected nodes (solid) and the edge closing the cycle,
    # from the last node back to the first (dotted)
    ax.plot([xs[i] for i in nodes], [ys[i] for i in nodes],
            color=EDGE_COLOR, linewidth=1, zorder=1)
    ax.plot([xs[nodes[-1]], xs[nodes[0]]], [ys[nodes[-1]], ys[nodes[0]]],
            color=EDGE_COLOR, linewidth=1.2, linestyle=":", zorder=1)

    # Not selected: same cost colors, just smaller
    ax.scatter([xs[i] for i in others], [ys[i] for i in others],
               c=[costs[i] for i in others], cmap=COST_CMAP, norm=norm,
               s=10, linewidths=0, zorder=2)

    # Selected: larger, with a thin surface-colored ring so overlapping points separate
    ax.scatter([xs[i] for i in nodes], [ys[i] for i in nodes],
               c=[costs[i] for i in nodes], cmap=COST_CMAP, norm=norm,
               s=30, edgecolors=SURFACE, linewidths=0.8, zorder=3)

    ax.set_aspect("equal")
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_color("#d9d8d4")


def main():
    args = parse_args()
    best = read_best(args.results, args.method_prefix)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    legend_items = [
        Line2D([], [], marker="o", linestyle="", markersize=6, color="#2a78d6",
               markeredgecolor=SURFACE, label="selected node"),
        Line2D([], [], marker="o", linestyle="", markersize=3.5, color="#2a78d6",
               label="not selected node"),
        Line2D([], [], color=EDGE_COLOR, linewidth=1, label="cycle"),
        Line2D([], [], color=EDGE_COLOR, linewidth=1.2, linestyle=":",
               label="closing edge (last → first node)"),
    ]

    for instance, solutions in best.items():
        xs, ys, costs = read_instance(args.data_dir / f"{instance}.csv")
        norm = Normalize(vmin=min(costs), vmax=max(costs))

        cols = 2
        rows = (len(solutions) + cols - 1) // cols
        fig, axes = plt.subplots(rows, cols, figsize=(11, 2.8 * rows + 1.0),
                                 squeeze=False, layout="constrained", facecolor=SURFACE)
        axes = axes.flatten()

        for ax, (method, value, nodes) in zip(axes, solutions):
            ax.set_facecolor(SURFACE)
            plot_solution(ax, xs, ys, costs, nodes, norm)
            ax.set_title(f"{method}\nobjective {value:,}".replace(",", " "),
                         fontsize=10, color=TEXT_PRIMARY)
        for ax in axes[len(solutions):]:
            ax.set_visible(False)

        colorbar = fig.colorbar(plt.cm.ScalarMappable(norm=norm, cmap=COST_CMAP),
                                ax=axes.tolist(), shrink=0.8, pad=0.01)
        colorbar.set_label("node cost", color=TEXT_SECONDARY)
        colorbar.outline.set_visible(False)
        colorbar.ax.tick_params(colors=TEXT_SECONDARY, labelsize=8)

        fig.suptitle(instance, fontsize=12, color=TEXT_PRIMARY)
        fig.legend(handles=legend_items, loc="outside lower center", ncol=4,
                   frameon=False, fontsize=9, labelcolor=TEXT_SECONDARY)

        out_path = args.out_dir / f"{args.name}_{instance}.{args.format}"
        fig.savefig(out_path, dpi=args.dpi, facecolor=SURFACE)
        plt.close(fig)
        print(f"Saved {out_path}")


if __name__ == "__main__":
    main()
