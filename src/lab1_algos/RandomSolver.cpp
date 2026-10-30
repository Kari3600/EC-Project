#include "RandomSolver.h"
#include <cstdlib>
#include <algorithm>
#include <vector>

std::vector<int> RandomSolver::solve(const Problem& problem, int seed) {
    std::srand(seed);
    const int size = problem.getSize();
    std::vector<int> solution(size);

    for (int i = 0; i < size; ++i) {
        solution[i] = i;
    }

    // Shuffle the solution array to create a random permutation
    for (int i = size - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(solution[i], solution[j]);
    }

    solution.resize(problem.minLength());
    return solution;
}