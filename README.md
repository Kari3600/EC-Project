# Evolutionary Computation — Lab Project

Oliwier Necelman, Karol Okoń 

## Structure

```
data/              problem instances (x;y;cost per node)
src/               C++ code: instance reader, Problem, Solver interface, TestAlgo.cpp (single run)
src/lab1_algos/    Lab 1 methods: random, nearest neighbor I/II, greedy cycle
src/lab2_algos/    Lab 2 methods: 2-regret and weighted 2-regret (NN II, greedy cycle)
src/lab3_algos/    Lab 3 methods: local search (greedy/steepest, two-nodes/two-edges, random/greedy start)
results/           experiment results written by scripts/run_experiment.py (CSV)
scripts/           run_experiment.py (all runs), plot.py (figures)
scripts/lab2/      alpha_sweep.py (influence of the regret weight alpha)
reports/           lab reports (Markdown) and figures/
```

## Running

From the repository root:

```bash
mkdir -p out && g++ -std=c++17 -O2 src/TestAlgo.cpp -o out/test_algo   # build the single-run program
./out/test_algo GreedyCycle_2regret data/InstanceA.csv 5 --alpha 0.5   # single run, printed
./out/test_algo LS_greedy_edges_random data/InstanceA.csv --seed 7   # local search from a random start
./out/test_algo LS_steepest_edges_greedy data/InstanceA.csv 5 --init GreedyCycle_2regret   # from a greedy start
python3 scripts/run_experiment.py   # experiment, all methods -> results/lab3_*.csv
python3 scripts/plot.py --results results/lab3_best.csv --method-prefix LS_ --name lab3_plot   # figures -> reports/figures/
python3 scripts/lab2/alpha_sweep.py   # alpha study -> results/lab2_alpha_sweep.csv, reports/figures/lab2_alpha_sweep.png
(cd reports && pandoc lab3_report.md -o lab3_report.pdf --pdf-engine=xelatex)   # Markdown report -> PDF
```
Compiling the report needs pandoc and XeLaTeX (Ubuntu: `sudo apt install pandoc texlive-xetex texlive-fonts-recommended`).

Local search methods are named `LS_<greedy|steepest>_<nodes|edges>_<random|greedy>` (type of local
search, intra-route move, starting solution). With a greedy start, `--init` picks the construction
heuristic (weighted ones use alpha 0.5); `run_experiment.py` uses the best one from lab 2 for each
instance (`INIT_HEURISTICS`). For local search `test_algo` also prints the objective of the starting
solution and the number of steps (improving moves applied); `results/lab3_summary.csv` has the
steps statistics, and `results/lab3_best.csv` records which start (node or seed) gave the best solution.


## Reports

- Lab 1, greedy heuristics: [reports/lab1_report.pdf](reports/lab1_report.pdf)
- Lab 2, greedy regret heuristics: [reports/lab2_report.md](reports/lab2_report.md)
- Lab 3, local search: [reports/lab3_report.md](reports/lab3_report.md)
