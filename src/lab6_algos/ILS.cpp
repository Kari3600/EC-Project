#include "ILS.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <utility>

static const int GUIDED_CANDIDATES = 10;   // guided: a replacement is drawn from this many nearest free nodes

Perturbation ILS::parsePerturbation(const std::string& text) {
    if (text == "randomMoves") return Perturbation::RandomMoves;
    if (text == "segment") return Perturbation::Segment;
    if (text == "doubleBridge") return Perturbation::DoubleBridge;
    if (text == "guided") return Perturbation::Guided;
    throw std::invalid_argument("Unknown perturbation '" + text + "' (randomMoves, segment, doubleBridge, guided)");
}

const char* ILS::perturbationName(Perturbation perturbation) {
    switch (perturbation) {
        case Perturbation::RandomMoves: return "randomMoves";
        case Perturbation::Segment: return "segment";
        case Perturbation::DoubleBridge: return "doubleBridge";
        case Perturbation::Guided: return "guided";
    }
    return "?";
}

int ILS::randomInt(int from, int to) {
    return std::uniform_int_distribution<int>(from, to)(rng);
}

int ILS::randomUnselected(const std::vector<bool>& selected) {
    // about half of the nodes are free, so rejection sampling needs ~2 draws
    while (true) {
        int node = randomInt(0, static_cast<int>(selected.size()) - 1);
        if (!selected[node]) {
            return node;
        }
    }
}

// k random moves, each with probability 1/2:
//   intra: reverse solution[i..j] (two-edges exchange),
//   inter: replace a random node of the cycle with a random node outside it
void ILS::randomMoves(std::vector<int>& solution, std::vector<bool>& selected) {
    const int m = static_cast<int>(solution.size());
    for (int t = 0; t < strength; ++t) {
        if (randomInt(0, 1) == 0) {
            // j - i < m - 1: reversing the whole cycle would not change it
            int i = randomInt(0, m - 2);
            int j = randomInt(i + 1, std::min(m - 1, i + m - 2));
            std::reverse(solution.begin() + i, solution.begin() + j + 1);
        } else {
            int pos = randomInt(0, m - 1);
            int newNode = randomUnselected(selected);
            selected[solution[pos]] = false;
            selected[newNode] = true;
            solution[pos] = newNode;
        }
    }
}

// replace k consecutive nodes of the cycle with k random nodes outside it
// (the removed nodes cannot come back, because they are still selected while drawing)
void ILS::segment(std::vector<int>& solution, std::vector<bool>& selected) {
    const int m = static_cast<int>(solution.size());
    const int freeCount = static_cast<int>(selected.size()) - m;
    const int k = std::min({strength, m, freeCount});
    const int start = randomInt(0, m - 1);

    std::vector<int> removed;
    for (int t = 0; t < k; ++t) {
        int pos = (start + t) % m;
        int newNode = randomUnselected(selected);
        selected[newNode] = true;
        removed.push_back(solution[pos]);
        solution[pos] = newNode;
    }
    for (int node : removed) {
        selected[node] = false;
    }
}

// k times: cut the cycle into A B C D at 3 random points and reconnect it as A C B D
// (3 edges change, no segment is reversed, so one two-edges move cannot undo it)
void ILS::doubleBridge(std::vector<int>& solution) {
    const int m = static_cast<int>(solution.size());
    for (int t = 0; t < strength; ++t) {
        int a = 0, b = 0, c = 0;
        while (!(a < b && b < c)) {
            int cuts[3] = {randomInt(1, m - 1), randomInt(1, m - 1), randomInt(1, m - 1)};
            std::sort(cuts, cuts + 3);
            a = cuts[0]; b = cuts[1]; c = cuts[2];
        }
        // [a, b) = B, [b, c) = C  ->  C B
        std::rotate(solution.begin() + a, solution.begin() + b, solution.begin() + c);
    }
}

// choose k nodes of the cycle at random with probability proportional to how much they cost:
//   cost[v] + d(prev, v) + d(v, next) - d(prev, next)  (what removing v from the cycle would save),
// and replace each with a random one of the GUIDED_CANDIDATES free nodes nearest to it
// (distance + cost of the new node)
void ILS::guided(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected) {
    const int m = static_cast<int>(solution.size());
    const int* costs = problem.getCosts();
    const int freeCount = static_cast<int>(selected.size()) - m;
    const int k = std::min({strength, m, freeCount});

    std::vector<double> weights(m);
    for (int pos = 0; pos < m; ++pos) {
        int prev = solution[(pos - 1 + m) % m], v = solution[pos], next = solution[(pos + 1) % m];
        int saving = costs[v] + problem.getDist(prev, v) + problem.getDist(v, next) - problem.getDist(prev, next);
        weights[pos] = std::max(saving, 1);   // rounded distances can make it slightly negative
    }

    std::vector<std::pair<int, int>> replacements;   // (position, new node)
    for (int t = 0; t < k; ++t) {
        int pos = std::discrete_distribution<int>(weights.begin(), weights.end())(rng);
        weights[pos] = 0;   // each position at most once

        std::vector<int> candidates;
        for (int node : nearest[solution[pos]]) {
            if (!selected[node]) {
                candidates.push_back(node);
                if (static_cast<int>(candidates.size()) == GUIDED_CANDIDATES) {
                    break;
                }
            }
        }
        int newNode = candidates[randomInt(0, static_cast<int>(candidates.size()) - 1)];
        selected[newNode] = true;   // not drawn again for another position
        replacements.push_back({pos, newNode});
    }
    for (auto [pos, newNode] : replacements) {
        selected[solution[pos]] = false;
        solution[pos] = newNode;
    }
}

void ILS::perturb(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected) {
    switch (perturbation) {
        case Perturbation::RandomMoves: randomMoves(solution, selected); break;
        case Perturbation::Segment: segment(solution, selected); break;
        case Perturbation::DoubleBridge: doubleBridge(solution); break;
        case Perturbation::Guided: guided(problem, solution, selected); break;
    }
}

std::vector<int> ILS::solve(const Problem& problem, int seed) {
    using Clock = std::chrono::steady_clock;
    const auto startTime = Clock::now();
    rng.seed(seed);

    const int n = problem.getSize();
    if (perturbation == Perturbation::Guided) {
        const int* costs = problem.getCosts();
        nearest.assign(n, {});
        for (int v = 0; v < n; ++v) {
            for (int u = 0; u < n; ++u) {
                if (u != v) nearest[v].push_back(u);
            }
            std::sort(nearest[v].begin(), nearest[v].end(), [&](int a, int b) {
                return problem.getDist(v, a) + costs[a] < problem.getDist(v, b) + costs[b];
            });
        }
    }

    // random starting solution improved by LS
    std::vector<int> x = localSearch->solve(problem, seed);
    int fx = problem.evaluate(x);
    trace.assign(1, fx);
    sameOptimum = 0;

    std::vector<bool> selected(n);
    while (std::chrono::duration<double, std::milli>(Clock::now() - startTime).count() < timeMs) {
        std::vector<int> y = x;
        std::fill(selected.begin(), selected.end(), false);
        for (int node : y) {
            selected[node] = true;
        }
        perturb(problem, y, selected);
        y = localSearch->improve(problem, std::move(y));

        const int fy = problem.evaluate(y);
        trace.push_back(fy);
        if (fy == fx) {
            ++sameOptimum;   // most likely LS went back to x (compared by objective value)
        }
        if (fy < fx) {
            x = std::move(y);
            fx = fy;
        }
    }
    return x;
}
