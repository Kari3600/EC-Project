#include "MSLS.h"

#include <climits>
#include <utility>

void MSLS::solve(const Problem& problem, std::vector<int>& best, std::mt19937& rng) {
    std::vector<int> initialSolution = best;
    int bestCost = INT_MAX;

    for (int i = 0; i < iterations; ++i) {
        std::vector<int> solution = initialSolution;
        localSearch->solve(problem, solution, rng);

        const int cost = problem.evaluate(solution);
        if (cost < bestCost) {
            bestCost = cost;
            best = std::move(solution);
        }
    }
}
