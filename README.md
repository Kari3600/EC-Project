# Evolutionary Computation — Lab Project

Oliwier Necelman, Karol Okoń 

## Structure

```
data/              problem instances (x;y;cost per node)
src/               C++ code: instance reader, Problem, Solver interface, TestAlgo.cpp (single run)
src/lab1_algos/    Lab 1 methods: random, nearest neighbor I/II, greedy cycle
src/lab2_algos/    Lab 2 methods: 2-regret and weighted 2-regret (NN II, greedy cycle)
results/           experiment results written by scripts/run_experiment.py (CSV)
scripts/           run_experiment.py (all runs), plot.py (figures)
reports/           lab reports (Markdown) and figures/
```

## Running

From the repository root:

```bash
mkdir -p out && g++ -std=c++17 -O2 src/TestAlgo.cpp -o out/test_algo   # build the single-run program
./out/test_algo GreedyCycle_2regret data/InstanceA.csv 5 --alpha 0.5   # single run, printed
python3 scripts/run_experiment.py --name lab2   # experiment -> results/lab2_*.csv
python3 scripts/plot.py --results results/lab2_best.csv --name lab2_plot   # figures -> reports/figures/
(cd reports && pandoc report.md -o report.pdf --pdf-engine=xelatex)   # reports/report.md -> reports/report.pdf
```
Compiling the report needs pandoc and XeLaTeX (Ubuntu: `sudo apt install pandoc texlive-xetex texlive-fonts-recommended`).


## Reports

- Lab 1, greedy heuristics: [reports/lab1_report.pdf](reports/lab1.pdf)
