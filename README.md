# Evolutionary Computation — Lab Project

Oliwier Necelman, Karol Okoń 

## Structure

```
data/              problem instances (x;y;cost per node)
src/               C++ code: instance reader, Problem, Solver interface, Test.cpp (experiment)
src/lab1_algos/    Lab 1 methods: random, nearest neighbor I/II, greedy cycle
results/           experiment results written by Test.cpp (CSV)
scripts/plot.py    plots the best solutions from results/
reports/           lab reports (Markdown) and figures/
```

## Running

From the repository root:

```bash
mkdir -p out && g++ -std=c++17 -O2 src/Test.cpp -o out/test && ./out/test   # experiment -> results/
python3 scripts/plot.py   # figures -> reports/figures/
```

## Reports

- Lab 1, greedy heuristics: [reports/lab1.md](reports/lab1.md)
