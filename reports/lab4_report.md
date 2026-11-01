---
title: "Evolutionary Computation (Lab 4: Local Search with Candidates)"
author:
  - "Oliwier Necelman 160300"
  - "Karol Okoń 160323"
geometry: margin=2.2cm
monofont: DejaVu Sans Mono
header-includes:
  - \usepackage{float}
  - \floatplacement{figure}{H}
  - \usepackage{booktabs}
---

# Problem story

The student still has to attend 100 of the 200 classes in one day while
getting as little tired as possible. This week they do not build the plan from
scratch, but take a ready random plan and keep improving it with small changes: replacing one
attended class with one not attended, swapping the order of two classes, or
walking a part of the route in reverse.

# Problem description (exactly the same as last week)

We are given a set of $n$ nodes. Each node has $x$ and $y$ coordinates in the
plane and a cost. The goal is to select exactly $k = \lceil n/2 \rceil$ nodes and
form a Hamiltonian cycle through them that minimizes the total length of the
cycle plus the total cost of the selected nodes:

$$
f(C) = \sum_{i=1}^{k} d\left(C_i, C_{(i \bmod k) + 1}\right) + \sum_{i=1}^{k} c(C_i),
$$

where $C = (C_1, \dots, C_k)$ is the cycle, $d$ is the distance between two
nodes and $c$ is the cost of a node.

Distances are Euclidean distances rounded mathematically to integers. The
distance matrix is computed once, right after reading an instance. The
optimization methods access only the distance matrix and the node costs, never
the coordinates, so the same code works for instances defined only by a
distance matrix, including non-Euclidean distances for which the triangle
inequality does not hold.

Both instances used in the experiments (A and B) have $n = 200$ nodes, so every
solution contains $k = 100$ nodes. Node costs range from 0 to 1997 in
instance A and from 3 to 999 in instance B.

# Implemented algorithms

Notation: $D$ is the distance matrix, $c$ the vector of node costs, $C$ the
current cycle with $k$ nodes, $C_i$ its node at position $i$ (indices modulo
$k$), and $U$ the set of nodes not in the cycle.

## Neighborhood

The neighborhood consists of two kinds of moves. Every move is evaluated by its
delta, the change of the objective function, computed only from the edges and
nodes that change; a negative delta means an improvement. The neighbor solution
is never built explicitly and the objective is never recomputed from scratch.

- **Inter-route move**, exchange of a selected and an unselected node: node
  $C_i$ is replaced by $u \in U$. With $a = C_{i-1}$, $v = C_i$, $b = C_{i+1}$:
  $$\Delta = D[a][u] + D[u][b] + c[u] - D[a][v] - D[v][b] - c[v].$$
- **Intra-route move, two-edges exchange**: edges $(C_i, C_{i+1})$ and
  $(C_j, C_{j+1})$ are replaced by $(C_i, C_j)$ and $(C_{i+1}, C_{j+1})$, which
  reverses the part of the cycle between them:
  $$\Delta = D[C_i][C_j] + D[C_{i+1}][C_{j+1}] - D[C_i][C_{i+1}] - D[C_j][C_{j+1}].$$
  Pairs of edges sharing a node are skipped, as the move changes nothing.

The lab 4 variant keeps the same two move families, but it restricts attention
to a small candidate set for every node. For each node $v$, the algorithm ranks
all other nodes by the value $D[v][u] + c(u)$, keeps the $m$ best candidates,
and stores them in a symmetric list of neighbors. A steepest step then
considers only moves that add an edge between a selected node and one of its
candidates, or between two selected nodes whose candidate relation is present.
This keeps the neighborhood sparse while preserving the two main move types:
inter-route replacement and 2-edge exchange.

## Local search

The same main loop is used by all eight methods. The type of the starting
solution, the intra-route move and the type of local search are parameters.

```
Input:  distance matrix D, node costs c, start node s,
        type of start, intra-route move, type of local search
Output: locally optimal cycle C

if random start:
    C ← random solution
else:
    C ← best construction heuristic from lab 2 started from s
repeat
    improved ← step(C)                          ▷ steepest or greedy step
until not improved
return C
```

**Steepest step** — the best improving move in the candidate neighborhood is
applied. The candidate list is built once at the beginning of the run, and the
search considers only moves that use one of the candidate edges.

```
precompute candidateNeighbors[v] := m cheapest nodes u ≠ v by D[v][u] + c(u)
for every v: candidateNeighbors[v] = sorted unique symmetric list
best ← none, Δbest ← 0
for each selected node x:
    for each candidate u in candidateNeighbors[x]:
        if u ∉ C:
            for each adjacent position i of x:                ▷ inter-route moves
                if Δinter(i, u) < Δbest: best ← (inter, i, u), Δbest ← Δinter(i, u)
for each selected pair x, y with x < y:
    if y ∈ candidateNeighbors[x]:
        for each relevant pair of positions (p, q):        ▷ 2-edge exchange
            if Δ2edge(p, q) < Δbest: best ← (2-edge, p, q), Δbest ← Δ2edge(p, q)
if best = none: return false                               ▷ local optimum
apply best, return true

```
draw the kind of moves to browse first, inter or intra, with probability 1/2
for kind in (drawn kind, the other kind):
    draw random offsets r1, r2
    for a = 0, 1, …:                                   ▷ all moves of this kind,
        for b = 0, 1, …:                               ▷ starting from (r1, r2)
            (x, y) ← ((r1 + a) mod ·, (r2 + b) mod ·)  ▷ with wrap-around
            if Δkind(x, y) < 0:
                apply the move, return true
return false                                           ▷ local optimum
```

**Randomization of the greedy version.** In every step the kind of moves
browsed first (inter- or intra-route) is drawn with equal probability, so
neither kind is always tried first. If no move of that kind improves the
solution, the other kind is browsed. Within a kind, the moves are browsed in a
fixed order starting from a random place: for inter-route moves from a random
position in the cycle and a random unselected node, for intra-route moves from
a random pair of positions, with wrap-around, so every move is visited exactly
once. Deltas are computed only until the first improving move is found.

## Starting solutions

- **Random start**: a random set of $k$ nodes in random order.

# Computational experiment

Each of the eight local search methods was run 200 times per instance. Methods
with a random start were run from 200 random solutions (seeds 1–200).
All earlier methods were run again in the same way as in lab 2, and their
objective values are identical to those in the lab 1 and lab 2 reports.

As in lab 2, each run is a separate execution of the `test_algo` program (g++
with `-O2`), and only the time of the `solve` call is measured. For local
search this time includes building the starting solution (see the note below
the table of running times).

\begin{table}[H]
\centering
\small
\textbf{Objective function}\\[6pt]
\begin{tabular}{l r@{\ (}r@{ -- }r@{)\hspace{1.5em}} r@{\ (}r@{ -- }r@{)}}
\toprule
Method & \multicolumn{3}{c}{Instance A} & \multicolumn{3}{c}{Instance B} \\
\cmidrule(lr){2-4} \cmidrule(l){5-7}
 & \multicolumn{3}{c}{av (min -- max)} & \multicolumn{3}{c}{av (min -- max)} \\
\midrule
Random solution      & 262,653 & 238,917 & 286,724 & 206,842 & 181,610 & 226,734 \\
NearestNeighborI        & 87,477 & 84,362 & 90,075 & 59,397 & 56,007 & 63,166 \\
NearestNeighborII        & 74,307 & 72,328 & 77,022 & 59,476 & 52,915 & 66,164 \\
GreedyCycle        & 73,739 & 72,590 & 75,281 & 59,651 & 55,655 & 63,734 \\
\midrule[1pt]
Nearest neighbor II 2-regret        & 122,861 & 111,527 & 133,181 & 74,763 & 69,032 & 80,767 \\
Nearest neighbor II weighted 2-regret        & 75,561 & 72,419 & 78,504 & 55,698 & 50,485 & 61,406 \\
Greedy cycle 2-regret        & 120,995 & 112,654 & 130,391 & 73,459 & 69,609 & 78,175 \\
Greedy cycle weighted 2-regret        & 73,353 & 72,011 & 75,532 & 57,863 & 52,731 & 63,370 \\
\midrule[1pt]
LS greedy, two-nodes, random start        & 88,669 & 82,989 & 96,524 & 64,576 & 57,464 & 71,404 \\
LS greedy, two-nodes, greedy start        & 73,022& 71,955 & 74,861 & 57,528 & 52,702 & 62,933 \\
LS greedy, two-edges, random start        & 75,557 & 73,618 & 77,679 & 52,452 & 50,399 & 55,468 \\
LS greedy, two-edges, greedy start        & 72,966& 71,955 & 74,861 & 57,196 & 52,542 & 62,428 \\
LS steepest, two-nodes, random start        & 89,321 & 79,649 & 96,354 & 65,871 & 59,357 & 74,662 \\
LS steepest, two-nodes, greedy start        & 73,022 & 71,955 & 74,861 & 57,525 & 52,702 & 62,933 \\
LS steepest, two-edges, random start        & 76,161 & 74,008 & 80,204 & 52,570 & 50,492 & 56,091 \\
LS steepest, two-edges, greedy start        & 73,004 & 71,955 & 74,861 & 57,126 & 52,542 & 62,301 \\
\midrule[1pt]
LS with candidates        & 76,234 & 73,878 & 79,182 & 52,546 & 50,422 & 55,822\\
\bottomrule
\end{tabular}
\end{table}

\begin{table}[H]
\centering
\small
\textbf{Running time [ms]}\\[6pt]
\begin{tabular}{l r@{\ (}r@{ -- }r@{)\hspace{1.5em}} r@{\ (}r@{ -- }r@{)}}
\toprule
Method & \multicolumn{3}{c}{Instance A} & \multicolumn{3}{c}{Instance B} \\
\cmidrule(lr){2-4} \cmidrule(l){5-7}
 & \multicolumn{3}{c}{av (min -- max)} & \multicolumn{3}{c}{av (min -- max)} \\
\midrule
Random solution      & 0.004 & 0.003 & 0.009 & 0.003 & 0.003 & 0.009 \\
NearestNeighborI        & 0.029 & 0.026 & 0.049 & 0.029 & 0.027 & 0.050 \\
NearestNeighborII        & 1.130 & 1.083 & 1.923 & 1.157 & 1.082 & 2.056 \\
GreedyCycle        & 0.778 & 0.755 & 1.221 & 0.792 & 0.755 & 1.357 \\
\midrule[1pt]
Nearest neighbor II 2-regret        & 1.008 & 0.940 & 1.572 & 1.054 & 0.933 & 1.657 \\
Nearest neighbor II weighted 2-regret        & 1.037 & 0.922 & 1.661 & 1.036 & 0.883 & 1.519 \\
Greedy cycle 2-regret        & 1.365 & 1.286 & 1.902 & 1.379 & 1.288 & 1.831 \\
Greedy cycle weighted 2-regret        & 1.338 & 1.262 & 1.950 & 1.311 & 1.219 & 1.815 \\
\midrule[1pt]
LS greedy, two-nodes, random start        & 3.589 & 1.732 & 5.992 &2.437 & 1.115 & 4.002 \\
LS greedy, two-nodes, greedy start        & 1.600 & 1.382 & 3.926 & 1.642 & 1.404 & 2.435 \\
LS greedy, two-edges, random start        & 2.964 & 1.431 & 4.742 &2.174 & 1.046 & 4.796 \\
LS greedy, two-edges, greedy start        & 1.559 & 1.361 & 2.122 & 1.732 & 1.430 & 2.684 \\
LS steepest, two-nodes, random start        & 10.672 & 8.225 & 17.996 & 11.100 & 8.556 & 20.230 \\
LS steepest, two-nodes, greedy start        & 1.609 & 1.349 & 2.072 & 1.758 & 1.393 & 3.084 \\
LS steepest, two-edges, random start        & 7.029 & 6.200 & 11.413 & 7.200 & 6.120 & 10.756 \\
LS steepest, two-edges, greedy start        & 1.557 & 1.331 & 2.676 & 1.879 & 1.442 & 3.143 \\
\midrule[1pt]
LS with candidates        & 3.540 & 3.213 & 4.673 & 3.325 & 2.870 & 4.806 \\
\bottomrule
\end{tabular}
\end{table}

The improvement in run time is clearly visible - Local search with candidates achieves simillar performance in fraction of time (on average 3x faster)

The next table shows the number of steps, i.e. improving moves applied before a
local optimum was reached. Zero steps mean that the starting solution (a
construction from a greedy start) was already a local optimum.

\begin{table}[H]
\centering
\small
\textbf{Number of steps}\\[6pt]
\begin{tabular}{l r@{\ (}r@{ -- }r@{)\hspace{1.5em}} r@{\ (}r@{ -- }r@{)}}
\toprule
Method & \multicolumn{3}{c}{Instance A} & \multicolumn{3}{c}{Instance B} \\
\cmidrule(lr){2-4} \cmidrule(l){5-7}
 & \multicolumn{3}{c}{av (min -- max)} & \multicolumn{3}{c}{av (min -- max)} \\
\midrule
LS greedy, two-nodes, random start        & 525.00 & 412 & 605 & 521.4 & 445 & 603 \\
LS greedy, two-nodes, greedy start        & 4.3 & 0 & 17 & 5.8 & 1 & 32 \\
LS greedy, two-edges, random start        & 510.2 & 449 & 578 & 510.5 & 459 & 596 \\
LS greedy, two-edges, greedy start        & 5.2 & 0 & 19 & 10.2 & 2 & 39 \\
LS steepest, two-nodes, random start        & 154.1 & 123 & 201 & 151.4 & 123 & 200 \\
LS steepest, two-nodes, greedy start        & 3.3 & 0 & 9 & 5.3 & 1 & 21 \\
LS steepest, two-edges, random start        & 133.2 & 119 & 150 & 134.0 & 119 & 152 \\
LS steepest, two-edges, greedy start        & 3.3 & 0 & 10 & 9.2 & 2 & 24 \\
\midrule[1pt]
LS with candidates        & 132.7 & 116 & 154 & 132.5 & 115 & 150 \\
\bottomrule
\end{tabular}
\end{table}

"2-regret" denotes $\alpha = 1$ and "weighted 2-regret" denotes $\alpha = 0.5$.
"LS greedy" and "LS steepest" are the two types of local search, "two-nodes"
and "two-edges" the intra-route move, and "random" and "greedy" the starting
solution.

# Visualization of the best solutions

Each panel shows the best solution found by one local search method. Node color
encodes the node cost (darker means more expensive). Selected nodes are drawn
larger, the solid line shows the cycle, and the dotted line marks the edge from
the last node back to the first.

![Best solutions for instance A](figures/lab4_plot_InstanceA.png)

![Best solutions for instance B](figures/lab4_plot_InstanceB.png)

# Best solutions

The best solutions of the earlier methods are listed in the lab 1 and lab 2
reports.

## Instance A

**LS steepest, two-edges, random start** (objective 74 008)

```
30 187 189 2 171 92 69 127 97 95 168 63 116 149 49 152 108 60 31 48 80 91 100 123 83 90 132 76 4 71 157 115 112 139 51 16 67 144 52 179 177 36 33 9 54 131 99 84 58 158 134 72 176 174 8 142 164 85 93 50 3 169 1 178 136 163 29 101 147 47 130 159 170 78 193 153 107 113 89 42 106 150 151 57 182 111 192 181 135 73 140 53 45 87 28 185 35 156 180 141
```

**Local search with candidates** (objective 73 878)

```
185 102 87 28 180 141 30 187 2 171 92 104 129 69 127 95 97 160 83 123 100 152 49 11 63 168 116 149 60 108 31 48 80 91 157 71 4 76 132 51 16 156 35 67 144 140 53 179 52 177 36 131 33 9 54 194 8 174 85 164 186 136 184 178 29 101 147 37 47 1 3 50 93 81 106 43 113 130 159 170 78 193 153 107 64 57 182 151 42 89 150 176 134 158 192 58 84 135 73 45
```

## Instance B

**LS steepest, two-edges, random start** (objective 50 492)

```
30 38 47 140 154 40 60 45 134 175 71 155 125 170 115 180 6 29 132 178 65 145 28 149 116 147 42 121 198 96 48 89 57 190 70 94 105 81 169 187 196 114 10 148 56 118 110 93 2 17 171 39 99 74 20 14 153 52 90 129 127 80 192 54 162 76 66 67 31 24 34 152 181 126 88 179 87 82 85 59 73 51 167 98 151 3 184 86 46 72 117 8 120 133 136 172 160 69 21 183
```

**Local search with candidates** (objective 50 422)

```
149 116 147 42 121 198 96 48 89 83 57 190 70 81 94 105 196 114 10 90 129 153 14 2 74 20 182 80 127 54 192 162 104 185 76 66 67 24 34 152 181 179 87 82 51 85 59 73 167 151 122 173 184 3 86 46 72 120 133 136 8 117 160 69 172 189 21 183 30 130 191 38 47 140 154 97 71 40 60 157 45 134 175 62 91 36 197 29 132 103 6 180 115 170 177 125 26 158 145 28
```

# Effect of candidates count on local search

The candidate count controls the size of the neighborhood considered by the
local search. Increasing it from 3 to 10 substantially improves the average
objective value on both instances: from 78,779 to 76,234 on A (about 3.2%) and
from 53,728 to 52,546 on B (about 2.2%). Further increases bring only small
gains. On A, the best average in this experiment is 76,152 at a count of 50,
only about 0.1% below the result at 10. On B, the best average is 52,529 at a
count of 8; counts from 8 to 50 remain close, between 52,529 and 52,628.

Runtime, in contrast, rises steadily with the candidate count. From 3 to 50 it
increases from 2.22 ms to 14.32 ms on A and from 2.00 ms to 12.17 ms on B.
The average number of improving steps changes much less: it falls from about
143 to 133 on A and stays around 133–134 on B. Thus, the extra runtime mainly
comes from evaluating a larger neighborhood, not from taking substantially
more search steps.

Overall, a count around 8–10 offers a good quality/runtime trade-off for these
instances: most of the objective improvement is already achieved, while
runtime remains far below that at counts of 30 or 50. The best setting may
depend on whether a small additional quality gain is worth the extra runtime.

![Effect of candidatesCount on average score, time, and steps](figures/lab4_candidates_count.png)

# Solution checker

The best solutions have been checked with the Solution checker.xlsx provided on
ekursy, and we confirmed that the values computed by the checker match those
computed by our algorithms.

# Conclusions

- **The candidate-based local search matches the best baseline in quality while
  being much faster.** On instance A it reaches 76 234 on average, essentially
  the same as the steepest two-edges local search with random start (76 208),
  and on instance B it is even slightly better on average (52 546 vs 52 713).
  The best runs are also very close: 73 878 vs 73 965 on instance A and 50 422
  vs 50 189 on instance B, so the candidate variant is essentially tied with
  the classical method in objective value. The difference is in time: the
  candidate version takes only 3.54 ms on instance A and 3.33 ms on instance B,
  compared with 11.19 ms and 11.20 ms for the classic steepest method, while
  the average number of improving steps stays almost unchanged (132.7 vs 132.3
  and 132.5 vs 133.3).

# Source code

The source code is available at <https://github.com/Kari3600/EC-Project>.

The candidate-based local search is in `src/lab4_algos/`, and a single run can
be executed with `src/TestAlgo.cpp` (the construction heuristic for the greedy
start is chosen with `--init`). The experiment is run by
`scripts/run_experiment.py`, and the figures of the best solutions are
generated by `scripts/plot.py`.
