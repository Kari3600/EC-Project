---
title: "Evolutionary Computation (Lab 6: ILS perturbations)"
author:
  - "Oliwier Necelman 160300"
  - "Karol Okoń 160323"
geometry: margin=2.2cm
monofont: DejaVu Sans Mono
---

This document describes the three perturbations of iterated local search (ILS)
that were tested in lab 6 in addition to the one used in the main report
($k$ random moves, described there). The implementation is in
`src/lab6_algos/ILS.cpp`.

# Notation

$C = (C_0, \dots, C_{m-1})$ is the current cycle ($m = 100$ selected nodes),
indices are taken modulo $m$, $U$ is the set of nodes not in the cycle,
$D[a][b]$ is the distance between nodes $a$ and $b$, and $c[v]$ is the cost of
node $v$. `random(a, b)` is an integer drawn uniformly from $[a, b]$. Every
perturbation takes a strength $k$ and modifies $C$ and $U$ in place; afterwards
ILS runs the local search from the perturbed solution.

# Segment replacement (segment)

$k$ consecutive nodes of the cycle are replaced by $k$ random nodes outside the
cycle. The removed nodes are added to $U$ only after all replacements are
drawn, so they cannot come back in the same perturbation. The order of the
cycle is not changed, only the set of selected nodes in one part of it.

```
procedure SEGMENT(C, U, k):
    k <- min(k, m, |U|)
    s <- random(0, m - 1)
    R <- empty list                               ▷ removed nodes
    for t <- 0 to k - 1:
        p <- (s + t) mod m
        u <- random node of U
        U <- U - {u}
        append C[p] to R
        C[p] <- u
    U <- U + R
```

# Double-bridge (doubleBridge)

The cycle is cut at three random points into parts $A\,B\,C\,D$ and
reconnected as $A\,C\,B\,D$. Three edges change and no part is reversed, so
the change cannot be undone by one two-edges move (which always reverses a
part of the cycle). Unlike the other perturbations, it changes only the order
of the nodes, not the set of selected nodes. The strength $k$ is the number of
double-bridges applied one after another.

```
procedure DOUBLE-BRIDGE(C, k):
    repeat k times:
        draw a < b < c, three different values of random(1, m - 1)
        C <- C[0..a-1] + C[b..c-1] + C[a..b-1] + C[c..m-1]
```

# Guided replacement (guided)

$k$ nodes of the cycle are replaced, as in the segment replacement, but both
choices are guided by the objective function, while still random. A node is
removed with probability proportional to how much the objective would decrease
if it were removed from the cycle (its cost and its detour). Each removed node
is replaced by a random one of the 10 free nodes nearest to it, where the
distance includes the cost of the new node. The lists of nearest nodes are
computed once per ILS run.

```
procedure GUIDED(C, U, k):
    ▷ once per ILS run: N[v] <- all other nodes u sorted by D[v][u] + c[u]
    k <- min(k, m, |U|)
    for p <- 0 to m - 1:
        w[p] <- max(1, c[C[p]] + D[C[p-1]][C[p]] + D[C[p]][C[p+1]]
                        - D[C[p-1]][C[p+1]])
    S <- empty list                               ▷ chosen (position, new node)
    repeat k times:
        p <- position drawn with probability w[p] / sum(w)
        w[p] <- 0                                 ▷ each position at most once
        K <- first 10 nodes of N[C[p]] that are in U
        u <- random node of K
        U <- U - {u}
        append (p, u) to S
    for each (p, u) in S:
        U <- U + {C[p]}
        C[p] <- u
```
