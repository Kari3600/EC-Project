#include "LocalSearchWithCandidates.h"

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

#include "../lab1_algos/RandomSolver.h"

std::vector<int> LocalSearchWithCandidates::getInitialSolution(const Problem& problem, int seed) const {
    if (initialSolver == nullptr) {
        throw std::logic_error(name + " needs a construction heuristic for the starting solution (--init)");
    }
    return initialSolver->solve(problem, seed);
}

// deltas: change of the objective after the move, negative = improvement
// replace solution[position] with newNode (not in the cycle)
int LocalSearchWithCandidates::interDelta(const Problem& problem, const std::vector<int>& solution, int position, int newNode) const {
    const int n = solution.size();
    const int prev = solution[(position - 1 + n) % n];
    const int next = solution[(position + 1) % n];
    const int oldNode = solution[position];
    const int* costs = problem.getCosts();

    return problem.getDist(prev, newNode) + problem.getDist(newNode, next) + costs[newNode]
         - problem.getDist(prev, oldNode) - problem.getDist(oldNode, next) - costs[oldNode];
}

// remove edges (i, i+1) and (j, j+1), add (i, j) and (i+1, j+1)
int LocalSearchWithCandidates::twoEdgesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const {
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
void LocalSearchWithCandidates::applyInter(std::vector<int>& solution, std::vector<bool>& selected, int position, int newNode) const {
    selected[solution[position]] = false;
    selected[newNode] = true;
    solution[position] = newNode;
}

// remove edges (i, i+1) and (j, j+1), add (i, j) and (i+1, j+1)
void LocalSearchWithCandidates::applyTwoEdges(std::vector<int>& solution, int i, int j) const {
    if (i > j) {
        std::swap(i, j);
    }
    std::reverse(solution.begin() + i + 1, solution.begin() + j + 1);
}

// best move from the whole neighbourhood (inter + intra)
bool LocalSearchWithCandidates::steepestStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected,
                               const std::vector<std::vector<int>>& candidateNeighbors) {
    const int n = solution.size();
    const int size = problem.getSize();
    std::vector<int> positionOf(size, -1);
    for (int position = 0; position < n; ++position) {
        positionOf[solution[position]] = position;
    }

    int bestDelta = 0;   // only improving moves (delta < 0) are kept
    bool bestIsInter = false;
    int bestI = -1;
    int bestJ = -1;

    for (int node = 0; node < size; ++node) {
        if (!selected[node]) {
            continue;
        }
        for (int neighbor : candidateNeighbors[node]) {
            if (!selected[neighbor]) {
                const int position = positionOf[node];
                const int previousPosition = (position - 1 + n) % n;
                const int nextPosition = (position + 1) % n;

                // Replacing either endpoint adjacent to node adds the candidate edge (node, neighbor).
                for (int replacementPosition : {previousPosition, nextPosition}) {
                    const int delta = interDelta(problem, solution, replacementPosition, neighbor);
                    if (delta < bestDelta) {
                        bestDelta = delta;
                        bestIsInter = true;
                        bestI = replacementPosition;
                        bestJ = neighbor;
                    }
                }
            } else if (node < neighbor) {
                const int nodePosition = positionOf[node];
                const int neighborPosition = positionOf[neighbor];
                const int previousNodePosition = (nodePosition - 1 + n) % n;
                const int previousNeighborPosition = (neighborPosition - 1 + n) % n;

                // Each of these 2-opt moves adds the candidate edge (node, neighbor).
                for (const auto& positions : {
                         std::pair<int, int>(nodePosition, neighborPosition),
                         std::pair<int, int>(previousNodePosition, previousNeighborPosition)}) {
                    const int delta = twoEdgesDelta(problem, solution, positions.first, positions.second);
                    if (delta < bestDelta) {
                        bestDelta = delta;
                        bestIsInter = false;
                        bestI = positions.first;
                        bestJ = positions.second;
                    }
                }
            }
        }
    }

    if (bestI == -1) {
        return false;   // local optimum
    }
    if (bestIsInter) {
        applyInter(solution, selected, bestI, bestJ);
    } else {
        applyTwoEdges(solution, bestI, bestJ);
    }
    return true;
}



std::vector<int> LocalSearchWithCandidates::solve(const Problem& problem, int seed) {
    rng.seed(std::rand());   // seeded by test_algo --seed, so every run is different but reproducible

    // we need to remember to count later how much time did it take compared to the whole algorithm
    std::vector<int> solution = getInitialSolution(problem, seed);
    initialSolution = solution;

    std::vector<bool> selected(problem.getSize(), false);
    for (int node : solution) {
        selected[node] = true;
    }

    const int size = problem.getSize();
    const int neighborCount = size > 0 ? std::min(std::max(candidatesCount, 0), size - 1) : 0;
    std::vector<std::vector<int>> candidateNeighbors(size);
    const int* costs = problem.getCosts();
    for (int node = 0; node < size; ++node) {
        std::vector<std::pair<long long, int>> rankedNeighbors;
        rankedNeighbors.reserve(size > 0 ? size - 1 : 0);
        for (int neighbor = 0; neighbor < size; ++neighbor) {
            if (neighbor != node) {
                rankedNeighbors.emplace_back(
                    static_cast<long long>(problem.getDist(node, neighbor)) + costs[neighbor],
                    neighbor);
            }
        }
        std::sort(rankedNeighbors.begin(), rankedNeighbors.end());
        for (int i = 0; i < neighborCount; ++i) {
            const int neighbor = rankedNeighbors[i].second;
            candidateNeighbors[node].push_back(neighbor);
            candidateNeighbors[neighbor].push_back(node);
        }
    }
    for (std::vector<int>& neighbors : candidateNeighbors) {
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
    }

    steps = 0;
    while (true) {
        bool improved = steepestStep(problem, solution, selected, candidateNeighbors);
        if (!improved) {
            break;
        }
        ++steps;
    }
    return solution;
}
