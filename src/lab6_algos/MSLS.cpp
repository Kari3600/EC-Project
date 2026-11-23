#include "MSLS.h"

#include <algorithm>
#include <climits>
#include <numeric>
#include <utility>

void MSLS::solve(const Problem& problem, std::vector<int>& best, std::mt19937& rng) {
    int bestCost = INT_MAX;

    for (int i = 0; i < iterations; ++i) {
        // every local search starts from its own new random solution
        std::vector<int> solution(problem.getSize());
        std::iota(solution.begin(), solution.end(), 0);
        std::shuffle(solution.begin(), solution.end(), rng);
        solution.resize(problem.minLength());

        localSearch->solve(problem, solution, rng);

        const int cost = problem.evaluate(solution);
        if (cost < bestCost) {
            bestCost = cost;
            best = std::move(solution);
        }
    }
}
