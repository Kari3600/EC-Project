---
title: "Evolutionary Computation (Lab 7: Large Neighborhood Search)"
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

# Problem description

Given $n$ nodes, each with coordinates and a cost, select exactly
$m = \lceil n/2 \rceil$ nodes and form a Hamiltonian cycle through them. The
objective is to minimize the cycle length plus the costs of the selected nodes:

$$
f(C) = \sum_{i=1}^{m} d\left(C_i, C_{(i \bmod m) + 1}\right)
       + \sum_{i=1}^{m} c(C_i),
$$

where $d$ is the distance between two nodes and $c$ is the node cost. Distances
are Euclidean distances rounded mathematically to integers. Both instances
contain $n=200$ nodes, so each solution selects $m=100$.

# Compared algorithms

## Large neighborhood search (LNS)

Large Neighborhood Search alternates destruction and reconstruction of a
current solution. At every iteration it copies the current cycle, removes
$r = 30$ nodes ($30\%$ of $m=100$), repairs it to a complete 100-node cycle,
optionally improves the repaired candidate with local search, and accepts the
candidate for the search only if it improves the best objective found.

```
procedure LNS(current, timeLimit):
    best <- current
    while elapsed time < timeLimit:
        candidate <- copy(current)
        destroy(candidate, r)
        repair(candidate)
        if postprocessor is enabled:
            postprocessor(candidate)
        if f(candidate) < f(best):
            current <- candidate
            best <- candidate
    return best
```

The search is initialized from a random solution of 100 nodes that is first
improved by `ListOfMovesSolver`. All six LNS variants use a limit of 1500 ms
and 20 independent runs, but the destroyer and repair/postprocessing stages
are configured differently as follows.

## Destroyers

### Random destroyer

The random destroyer removes $r$ selected nodes, one at a time, choosing a
position uniformly at random from the remaining cycle.

```
procedure RANDOM-DESTROY(C, r):
    repeat r times:
        p <- random(0, |C| - 1)
        C <- C without C[p]
```

### Random-chain destroyer

The random-chain destroyer removes one or five contiguous chains. Its first
implementation with one chain removes a chain of 30 nodes. For five chains,
the 30 removals are divided into five chains of six consecutive nodes. Every
chain starts at a position chosen uniformly at random; if the chain crosses
the end of the vector, the removal continues from the beginning.

```
procedure RANDOM-CHAIN-DESTROY(C, r, q):
    for i <- 1 to q:
        l <- r * i / q - r * (i - 1) / q
        s <- random(0, |C| - 1)
        C <- C with l nodes starting at s removed cyclically
```

The destruction therefore removes 30 nodes in each case, but a random-chain
destroyer keeps a chain of neighboring positions together while the random
destroyer scatters the removed nodes around the cycle.

## Repair and postprocessing

The repairer is always `GreedyCycle_2regret` with $\alpha=0.5$. The
destroyed solution contains only 70 nodes, so the repairer inserts nodes until
the cycle again has 100 nodes using the weighted two-regret criterion.

The repaired candidate is then tested in two variants: directly, or after
running `ListOfMovesSolver` as a postprocessor. The postprocessor uses the
steepest list-of-moves neighborhood from lab 5: it can replace a selected node
with an unselected node or exchange two edges of the cycle.

## Compared configurations

The experiment compares the random destroyer and the random-chain destroyer
with one and five chains, each with and without the local-search postprocessor.
Greedy cycle, random-start list-of-moves local search, and 400-start MSLS are
kept in the tables as baselines. ILS is also shown because it is the strongest
previous local-search method.

# Computational experiment

Each method was run 20 times per instance (seeds 1--20). The greedy weighted
2-regret heuristic was run once from every possible starting node, the random
list-of-moves local search was run with 200 seeds, and MSLS was run with 400
local-search iterations in each of 20 seeds. ILS and all LNS variants use a
1500 ms time limit per run. Lower objective values are better; the tables show
the average and the minimum--maximum range over the runs.

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
Greedy Cycle with weighted 2regret                                                & 73\,350 & 72\,011 & 75\,532 & 57\,771 & 52\,906 & 63\,326
Local Search with list of moves                                                   & 76\,235 & 73\,561 & 80\,874 & 52\,510 & 50\,333 & 56\,533
Multiple starts Local Search                                                      & 76\,550 & 74\,022 & 79\,354 & 52\,703 & 50\,709 & 54\,687
Iterative Local Search                                                            & 71\,563 & 71\,484 & 71\,716 & 48\,267 & 48\,150 & 48\,532
Large Neighborhood Search, random destroyer, no postprocessor                     & 72\,225 & 71\,786 & 72\,880 & 49\,183 & 48\,357 & 50\,491
Large Neighborhood Search, random destroyer, local search postprocessor           & 72\,114 & 71\,697 & 72\,831 & 49\,069 & 48\,218 & 50\,192
Large Neighborhood Search, 1 random chain destroyer, no postprocessor             & 72\,142 & 71\,815 & 72\,895 & 49\,967 & 49\,263 & 50\,920
Large Neighborhood Search, 1 random chain destroyer, local search postprocessor   & 72\,117 & 71\,788 & 72\,897 & 49\,576 & 48\,574 & 50\,237
Large Neighborhood Search, 5 random chains destroyer, no postprocessor            & 71\,840 & 71\,728 & 72\,438 & 48\,814 & 48\,271 & 49\,893
Large Neighborhood Search, 5 random chains destroyer, local search postprocessor  & 71\,718 & 71\,697 & 71\,767 & 48\,834 & 48\,260 & 49\,808
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
Greedy Cycle with weighted 2regret                                                & 1.475 & 1.250 & 2.524 & 1.433 & 1.201 & 2.279
Local Search with list of moves                                                   & 4.435 & 2.829 & 7.504 & 4.323 & 3.048 & 10.187
Multiple starts Local Search                                                      & 1575.642 & 1363.588 & 1800.761 & 1521.501 & 1245.772 & 1764.623
Iterative Local Search                                                            & 1500.170 & 1500.002 & 1500.496 & 1500.227 & 1500.017 & 1500.864
Large Neighborhood Search, random destroyer, no postprocessor                     & 1500.324 & 1500.043 & 1500.676 & 1500.308 & 1500.078 & 1500.445
Large Neighborhood Search, random destroyer, local search postprocessor           & 1500.384 & 1500.128 & 1500.806 & 1500.380 & 1500.030 & 1501.154
Large Neighborhood Search, 1 random chain destroyer, no postprocessor             & 1500.366 & 1500.010 & 1501.010 & 1500.429 & 1500.066 & 1501.842
Large Neighborhood Search, 1 random chain destroyer, local search postprocessor   & 1500.352 & 1500.051 & 1500.696 & 1500.411 & 1500.077 & 1500.989
Large Neighborhood Search, 5 random chains destroyer, no postprocessor            & 1500.249 & 1500.031 & 1500.550 & 1500.355 & 1500.002 & 1501.103
Large Neighborhood Search, 5 random chains destroyer, local search postprocessor  & 1500.290 & 1500.034 & 1500.747 & 1500.257 & 1500.018 & 1500.641
\bottomrule 
\end{tabular} 
\end{table}

\begin{table}[H] 
\centering 
\small 
\textbf{Steps}\\[6pt] 
\begin{tabular}{l r@{\ (}r@{ -- }r@{)\hspace{1.5em}} r@{\ (}r@{ -- }r@{)}} 
\toprule 
Method & \multicolumn{3}{c}{Instance A} & \multicolumn{3}{c}{Instance B} \\ 
\cmidrule(lr){2-4} \cmidrule(l){5-7} 
& \multicolumn{3}{c}{av (min -- max)} & \multicolumn{3}{c}{av (min -- max)} \\ 
\midrule
Greedy Cycle with weighted 2regret                                               
Local Search with list of moves                                                  
Multiple starts Local Search                                                     
Iterative Local Search                                                            & 3\,358 & 3\,271 & 3\,465 & 3\,402 & 3\,285 & 3\,515
Large Neighborhood Search, random destroyer, no postprocessor                     & 2\,453 & 2\,033 & 2\,688 & 2\,312 & 2\,006 & 2\,543
Large Neighborhood Search, random destroyer, local search postprocessor           & 2\,079 & 1\,863 & 2\,201 & 2\,003 & 1\,783 & 2\,173
Large Neighborhood Search, 1 random chain destroyer, no postprocessor             & 2\,414 & 2\,179 & 2\,610 & 2\,482 & 2\,225 & 2\,617
Large Neighborhood Search, 1 random chain destroyer, local search postprocessor   & 2\,117 & 1\,951 & 2\,317 & 2\,102 & 1\,947 & 2\,267
Large Neighborhood Search, 5 random chains destroyer, no postprocessor            & 2\,356 & 2\,223 & 2\,530 & 2\,322 & 2\,217 & 2\,531
Large Neighborhood Search, 5 random chains destroyer, local search postprocessor  & 2\,105 & 1\,999 & 2\,191 & 2\,096 & 2\,015 & 2\,204
\bottomrule 
\end{tabular} 
\end{table} 

The iteration counts above are counted by the LNS implementation. The initial
solution is not included as an iteration; every later entry is the result of
destroying, repairing, and optionally postprocessing one candidate.

## Destroyer and postprocessor comparison

The five-chain destroyer gives the best LNS averages on both instances. With
the local-search postprocessor it obtains 71\,718 on average on instance A and
48\,834 on instance B. The random destroyer and one-chain destroyer perform
slightly worse. The postprocessor consistently improves the averages for both
the random and one-chain destroyers, and it reduces the dispersion of the
five-chain configuration on instance A.

# Visualization of the best solutions 
 
Each panel shows the best solution of one method from the tables. Node color 
encodes the node cost (darker means more expensive). Selected nodes are drawn 
larger, the solid line shows the cycle, and the dotted line marks the edge from 
the last node back to the first. 

![Best solutions for InstanceA](C:\Users\karol\Documents\Google Drive\PUT\S7\EC\Project\reports\figures/lab7_plot_InstanceA.png) 

![Best solutions for InstanceB](C:\Users\karol\Documents\Google Drive\PUT\S7\EC\Project\reports\figures/lab7_plot_InstanceB.png) 

# Best solutions 
 
The following are the best solutions found by each tested LNS destroyer and
postprocessing configuration. Node numbers are 0-based, as in the data and
source code.

## Instance A

**Large Neighborhood Search, random destroyer, no postprocessor** (objective 71,786) 

```
 76 132 180  28 102 185  35  51  16  67 144 140  53 179  52 177  36 131  33   9
 54   8 142 164 186 136 184 178  29  55 101   1 169   3  50  93  47 130 159 146
170 153 193  78  88 107  64  57 182 138 151  42  89 113 106 150 174 176  72 134
 84  58 158 192 181 114 135  73  45  87 141  30 187 189   2 171  92  69 127  97
168 116 149  63  11  49 160  83 123 100 152 108  60  31  48  80  91 157  71   4
``` 

**Large Neighborhood Search, random destroyer, local search postprocessor** (objective 71,697) 

```
 28 102 185  35  51  16  67 144 140  53 179  52 177  36 131  33   9  54   8 142
164 186 136 184 178  29 101 147  47   1 169   3  50  93 130 159 146 170 153 193
 78  88 107  64  57 182 138 151  42  89 113 106 150 174 176  72 134  84  58 158
192 181 114 135  73  45  87 141  30 187 189   2 171  92  69 127  97 168 116 149
 63  11  49 160  83 123 100 152 108  60  31  48  80  91 157  71   4  76 132 180
``` 

**Large Neighborhood Search, 1 random chain destroyer, no postprocessor** (objective 71,815) 

```
123  83 160  97 127 161  30 187 171   2 189 141  87  45  73 135 114 181 192 158
 58  84 134  72 176 174 150 106 113  89  42 151 138 182  57  64 107  88  78 193
153 170 146 159 130  93   3   1 169  50  47  37 147 101  29 178 184 136 186 164
142   8  54   9  33 131  36 177  52 179  53 140 144  67  16  51  35 185 102  28
180 132  76   4  71 157  91  80  48  31  60 116 149  63 168  11  49 108 152 100
``` 

**Large Neighborhood Search, 1 random chain destroyer, local search postprocessor** (objective 71,788) 

```
  4  71 157  91  80  48  31  60 108 152 100 123  83 160  49  11  63 149 116 168
 97 127 161  30 187 171   2 189 141  87  45  73 135 114 181 192 158  58  84 134
 72 176 174 150 106 113  89  42 151 138 182  57  64 107  88  78 193 153 170 146
159 130  93  50   3 169   1  47  37 147 101  29 178 184 136 186 164 142   8  54
  9  33 131  36 177  52 179  53 140 144  67  16  51  35 185 102  28 180 132  76
``` 

**Large Neighborhood Search, 5 random chains destroyer, no postprocessor** (objective 71,728) 

```
 80  91 157  71   4  76 132 180  28 102 185  35  51  16  67 144 140  53 179  52
177  36 131  33   9  54   8 174  85 142 164 186 136 184 178  29 101   1 169   3
 50  93  47 130 159 146 170 153 193  78  88 107  64  57 182 138 151  42  89 113
106 150 176  72 134  84  58 158 192 181 114 135  73  45  87 141  30 187 189   2
171  92  69 127  97 160  83 123 100 152 108  49  11 168  63 149 116  60  31  48
``` 

**Large Neighborhood Search, 5 random chains destroyer, local search postprocessor** (objective 71,697) 

```
 16  51  35 185 102  28 180 132  76   4  71 157  91  80  48  31  60 108 152 100
123  83 160  49  11  63 149 116 168  97 127  69  92 171   2 189 187  30 141  87
 45  73 135 114 181 192 158  58  84 134  72 176 174 150 106 113  89  42 151 138
182  57  64 107  88  78 193 153 170 146 159 130  93  50   3 169   1  47 147 101
 29 178 184 136 186 164 142   8  54   9  33 131  36 177  52 179  53 140 144  67
``` 

## Instance B

**Large Neighborhood Search, random destroyer, no postprocessor** (objective 48,357) 

```
171  17  74  20   2  14 153  90 129 127  80 192 162  54  78  10 114 196  61 105
 94  81 187  70 190 104  76 185  57  89 156  48  96 198 121  42 147 116  37 146
 13 149  28 145  65 178 103 132  29   6 197  36  62 175 134  45 157  60  40  71
155 115 170 125  26 154 140  47  38  21  30 183 189  69 160 172 136 133 120  72
 46 184   3 151  98  73  59  85  51 167 181 152  34  24  67 179  82  87  88  99
``` 

**Large Neighborhood Search, random destroyer, local search postprocessor** (objective 48,218) 

```
 17  74  20   2  14 153  90 129  80 127  10 114 196  61 105  94  81 190  70 187
 54 192 162  76 185  57  89 156  48  96 198 121  42 147 116  37 146  13 149  28
145  65 178 103 132  29   6 197  36  62 175 134  45 157  60  40  71 155 115 170
177 125  26 154 140  47  38  21  30 183  69 160 172 136 133 120 117  72  46 184
  3 151  27  66  67  24  34 152 181 167  73  59  85  51  82  87 179  88  99 171
``` 

**Large Neighborhood Search, 1 random chain destroyer, no postprocessor** (objective 49,263) 

```
152 181 167  73  59  85  51 179  82  87  88  99 171  17  74  20   2  14 153  90
129 127  80 192 162  54  78  10 114 196 187  70 190  77  57  89 156  48  96 198
121 130 191  30  21  38  47 140 154  97 115 170 125 177 158 149  28 145  65 178
103 132  29   6 197  36  62 175 134  45 157  60  71 155  40  33  64 124  69 160
172 136 133 120   8 117  72  46 184   3 122 151  98  27 135  76  66  67  24  34
``` 

**Large Neighborhood Search, 1 random chain destroyer, local search postprocessor** (objective 48,574) 

```
157  45 134 175  62  36 197 132  29   6 180 115 170 125 158 145  28 149  13 146
 37 116 147  42 121 198  96  48 156  89  57 185  76 104 190  70 187 169  81  94
105  61 196 114  10  78  54 162 192  80 127 129  90 153  14   2  20  74  17 171
 99  88 179  87  82  51  85  59  73 167 181 152  67  24  34  98 151   3 184  46
 72 117 120 133 136 172 160  69 189 183  30  21  38 154 140  47  64  40  71  60
``` 

**Large Neighborhood Search, 5 random chains destroyer, no postprocessor** (objective 48,271) 

```
 51  82  87 179  88  99 171  17  74  20   2  14 153  90 129 127  80 192 162  54
 78  10 114 196  61 105  94  81  70 190 104  76 185  57  89 156  48  96 198 121
 42 147 116  37 146  13 149  28 145  65 178 103 132  29   6 197  36  62 175 134
 45 157  60  40  71 155 115 170 125  26 154 140  47  38  21  30 183 189  69 160
172 136 133 120  72  46 184   3 151  27  66  67  24  34 152 181 167  73  59  85
``` 

**Large Neighborhood Search, 5 random chains destroyer, local search postprocessor** (objective 48,260) 

```
104 190  70  81  94 105  61 196 114  10  78  54 162 192  80 127 129  90 153  14
  2  20  74  17 171  99  88 179  87  82  51  85  59  73 167 181 152  34  24  67
 66  27 151   3 184  46  72 117 120 133 136 172 160  69 183  30  21  38  47 140
154  26 125 170 115 155  71  40  60 157  45 134 175  62  36 197   6  29 132 103
178  65 145  28 149  13 146  37 116 147  42 121 198  96  48 156  89  57 185  76
``` 

# Solution checker

The objective values in the report are computed with `Problem::evaluate`, which
adds Euclidean cycle length to the costs of the 100 selected nodes. Before
submission, the listed cycles can also be verified in the Solution
checker.xlsx spreadsheet by checking that they contain 100 node numbers, with
no duplicates and no number outside 0--199.

# Conclusions

- **LNS improves on the tested random local-search baseline.** The 5-chain
  configuration with postprocessing averages 71\,718 on instance A, while
  one random local-search run averages 76\,235. It also improves instance B
  from 52\,510 to 48\,834.
- **Five-chain destruction is generally more effective than one chain or
  random destruction.** It averages 71\,840 on instance A without
  postprocessing and 48\,814 on instance B, both better than the corresponding
  random and one-chain configurations.
- **Local-search postprocessing is beneficial for random and one-chain
  destruction.** It lowers the averages from 72\,225 to 72\,114 and from
  49\,183 to 49\,069 for random destruction, and from 72\,142 to 72\,117 and
  from 49\,967 to 49\,576 for one-chain destruction.
- **Postprocessing has a smaller and mixed effect for five chains.** It improves
  instance A from 71\,840 to 71\,718, but weakens the instance-B average from
  48\,814 to 48\,834. The best observed solution on instance B is still the
  configuration without postprocessing (48\,271).
- **ILS remains the strongest tested approach.** It averages 71\,563 on
  instance A and 48\,267 on instance B, compared with 71\,718 and 48\,814 for
  the best 5-chain LNS configurations in the respective primary comparison.
- **The time budget is the dominant stopping condition.** All LNS and ILS runs
  take approximately 1500 ms, while MSLS takes about 1576 ms on instance A and
  1522 ms on instance B. LNS performs roughly 2000--2450 candidates in the
  fixed time, whereas the first random local-search run takes only a few ms.

# Source code and data

LNS is implemented in `src/lab7_algos/LNS.cpp`, and the random and random-chain
destroyers are implemented in `src/lab7_algos/RandomDestroyer.cpp` and
`src/lab7_algos/RandomChainDestroyer.cpp`. Repair is performed with
`src/lab2_algos/GreedyCycle_2regret.cpp`, and optional postprocessing uses
`src/lab5_algos/ListOfMovesSolver.cpp`.

The experimental configurations are defined in `tests/lab7_test.py`. Raw
results are stored in `results/lab7_results.json`, and the tables and best
solution listings can be regenerated with
`python3 scripts/generate_tables.py --data results/lab7_results.json` and
`python3 scripts/plot_best_solution.py`. The visualization is generated in
`reports/figures/lab7_plot_InstanceA.png` and
`reports/figures/lab7_plot_InstanceB.png`.
