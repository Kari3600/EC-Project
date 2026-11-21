#include "RandomSolver.h"
#include <algorithm>
#include <vector>
#include <random>

void RandomSolver::solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) {
    const int size = problem.getSize();

    if (solution.size() == 0) {
        solution.resize(problem.getSize());
        std::iota(solution.begin(), solution.end(), 0);
        std::shuffle(solution.begin(), solution.end(), rng);
        solution.resize(problem.minLength());
    } else {
        std::vector<bool> used(size, false);

        for (int x : solution) {
            used[x] = true;
        }

        // Collect unused numbers
        std::vector<int> available;
        available.reserve(size - solution.size());

        for (int i = 0; i < size; ++i) {
            if (!used[i])
                available.push_back(i);
        }

        // Randomize available numbers
        std::shuffle(available.begin(), available.end(), rng);

        // Insert random numbers at random positions
        for (int i = solution.size(); i < problem.minLength(); ++i) {
            std::uniform_int_distribution<size_t> dist(0, solution.size());
            size_t pos = dist(rng);

            solution.insert(solution.begin() + pos, available[i]);
        }
    }
}