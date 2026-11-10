#include "MSLS.h"

#include <climits>
#include <utility>

std::vector<int> MSLS::solve(const Problem& problem, int seed) {
    std::vector<int> best;
    int bestCost = INT_MAX;

    for (int i = 0; i < iterations; ++i) {
        // Run `seed` (1, 2, ...) uses local search seeds 1..400, 401..800, ...
        // Each local search gets its own seed because RandomSolver calls srand(seed),
        // so a repeated seed would give the same starting solution.
        const int lsSeed = (seed - 1) * iterations + i + 1;

        // random starting solution + local search to a local optimum
        std::vector<int> solution = localSearch->solve(problem, lsSeed);

        const int cost = problem.evaluate(solution);
        if (cost < bestCost) {
            bestCost = cost;
            best = std::move(solution);
        }
    }
    return best;
}
