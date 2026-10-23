#include "LocalSearch.h"

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

#include "../lab1_algos/RandomSolver.h"

// construction heuristic chosen per instance (best one from the previous labs)
std::vector<int> LocalSearch::getInitialGreedy(const Problem& problem, int startNode) const {
    if (initialSolver == nullptr) {
        throw std::logic_error(name + " needs a construction heuristic for the starting solution (--init)");
    }
    return initialSolver->solve(problem, startNode);
}

std::vector<int> LocalSearch::getInitialRandom(const Problem& problem, int startNode) const {
    return RandomSolver().solve(problem, startNode);
}

// deltas: change of the objective after the move, negative = improvement
// replace solution[position] with newNode (not in the cycle)
int LocalSearch::interDelta(const Problem& problem, const std::vector<int>& solution, int position, int newNode) const {
    const int n = solution.size();
    const int prev = solution[(position - 1 + n) % n];
    const int next = solution[(position + 1) % n];
    const int oldNode = solution[position];
    const int* costs = problem.getCosts();

    return problem.getDist(prev, newNode) + problem.getDist(newNode, next) + costs[newNode]
         - problem.getDist(prev, oldNode) - problem.getDist(oldNode, next) - costs[oldNode];
}

// swap the nodes at positions i and j
int LocalSearch::twoNodesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const {
    const int n = solution.size();
    if (i == j) {
        return 0;
    }
    if ((j + 1) % n == i) {
        std::swap(i, j);   // neighbours: make j the one right after i
    }

    const int a = solution[i];
    const int b = solution[j];
    const int prevA = solution[(i - 1 + n) % n];
    const int nextB = solution[(j + 1) % n];

    if ((i + 1) % n == j) {   // neighbours: edge a-b stays, only the outer edges change
        return problem.getDist(prevA, b) + problem.getDist(a, nextB)
             - problem.getDist(prevA, a) - problem.getDist(b, nextB);
    }

    const int nextA = solution[(i + 1) % n];
    const int prevB = solution[(j - 1 + n) % n];
    return problem.getDist(prevA, b) + problem.getDist(b, nextA)
         + problem.getDist(prevB, a) + problem.getDist(a, nextB)
         - problem.getDist(prevA, a) - problem.getDist(a, nextA)
         - problem.getDist(prevB, b) - problem.getDist(b, nextB);
}


// remove edges (i, i+1) and (j, j+1), add (i, j) and (i+1, j+1)
int LocalSearch::twoEdgesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const {
    const int n = solution.size();
    if (i > j) {
        std::swap(i, j);
    }
    if (j - i < 2 || (i == 0 && j == n - 1)) {
        return 0;   // edges share a node, the move changes nothing
    }

    const int a = solution[i];
    const int aNext = solution[i + 1];
    const int b = solution[j];
    const int bNext = solution[(j + 1) % n];
    return problem.getDist(a, b) + problem.getDist(aNext, bNext)
         - problem.getDist(a, aNext) - problem.getDist(b, bNext);
}

// replace solution[position] with newNode (not in the cycle)
void LocalSearch::applyInter(std::vector<int>& solution, std::vector<bool>& selected, int position, int newNode) const {
    selected[solution[position]] = false;
    selected[newNode] = true;
    solution[position] = newNode;
}

// swap the nodes at positions i and j
void LocalSearch::applyTwoNodes(std::vector<int>& solution, int i, int j) const {
    std::swap(solution[i], solution[j]);
}

// remove edges (i, i+1) and (j, j+1), add (i, j) and (i+1, j+1)
void LocalSearch::applyTwoEdges(std::vector<int>& solution, int i, int j) const {
    if (i > j) {
        std::swap(i, j);
    }
    std::reverse(solution.begin() + i + 1, solution.begin() + j + 1);
}


// first improving move; random order of move types and random starting points
bool LocalSearch::greedyStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected) {
    const int n = solution.size();
    const int size = problem.getSize();

    auto tryInter = [&]() {
        const int startPosition = rng() % n;
        const int startNode = rng() % size;
        for (int a = 0; a < n; ++a) {
            const int position = (startPosition + a) % n;
            for (int b = 0; b < size; ++b) {
                const int node = (startNode + b) % size;
                if (selected[node]) {
                    continue;
                }
                if (interDelta(problem, solution, position, node) < 0) {
                    applyInter(solution, selected, position, node);
                    return true;
                }
            }
        }
        return false;
    };

    auto tryIntra = [&]() {
        const int startI = rng() % n;
        const int startJ = rng() % n;
        for (int a = 0; a < n; ++a) {
            const int i = (startI + a) % n;
            for (int b = 0; b < n; ++b) {
                const int j = (startJ + b) % n;
                if (i >= j) {
                    continue;   // every pair once
                }
                if (intra == IntraMove::Nodes) {
                    if (twoNodesDelta(problem, solution, i, j) < 0) {
                        applyTwoNodes(solution, i, j);
                        return true;
                    }
                } else {
                    if (twoEdgesDelta(problem, solution, i, j) < 0) {
                        applyTwoEdges(solution, i, j);
                        return true;
                    }
                }
            }
        }
        return false;
    };

    // random order of move types; if the first has no improving move, try the other
    if (rng() % 2 == 0) {
        return tryInter() || tryIntra();
    }
    return tryIntra() || tryInter();
}

// best move from the whole neighbourhood (inter + intra)
bool LocalSearch::steepestStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected) {
    const int n = solution.size();
    const int size = problem.getSize();

    int bestDelta = 0;   // only improving moves (delta < 0) are kept
    bool bestIsInter = false;
    int bestI = -1;
    int bestJ = -1;

    for (int position = 0; position < n; ++position) {
        for (int node = 0; node < size; ++node) {
            if (selected[node]) {
                continue;
            }
            const int delta = interDelta(problem, solution, position, node);
            if (delta < bestDelta) {
                bestDelta = delta;
                bestIsInter = true;
                bestI = position;
                bestJ = node;
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const int delta = (intra == IntraMove::Nodes)
                                  ? twoNodesDelta(problem, solution, i, j)
                                  : twoEdgesDelta(problem, solution, i, j);
            if (delta < bestDelta) {
                bestDelta = delta;
                bestIsInter = false;
                bestI = i;
                bestJ = j;
            }
        }
    }

    if (bestI == -1) {
        return false;   // local optimum
    }
    if (bestIsInter) {
        applyInter(solution, selected, bestI, bestJ);
    } else if (intra == IntraMove::Nodes) {
        applyTwoNodes(solution, bestI, bestJ);
    } else {
        applyTwoEdges(solution, bestI, bestJ);
    }
    return true;
}



std::vector<int> LocalSearch::solve(const Problem& problem, int startNode) {
    rng.seed(std::rand());   // seeded by test_algo --seed, so every run is different but reproducible

    // we need to remember to count later how much time did it take compared to the whole algorithm
    std::vector<int> solution = (start == StartType::Greedy)
                                    ? getInitialGreedy(problem, startNode)
                                    : getInitialRandom(problem, startNode);
    initialSolution = solution;

    std::vector<bool> selected(problem.getSize(), false);
    for (int node : solution) {
        selected[node] = true;
    }

    steps = 0;
    while (true) {
        bool improved = (search == SearchType::Greedy)
                            ? greedyStep(problem, solution, selected)
                            : steepestStep(problem, solution, selected);
        if (!improved) {
            break;
        }
        ++steps;
    }
    return solution;
}
