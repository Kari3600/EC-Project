#include "RandomSolver.h"
#include <cstdlib>
#include <algorithm>

int* RandomSolver::solve(const Problem& problem) {
    const int size = problem.getSize();
    int* solution = new int[size];

    for (int i = 0; i < size; ++i) {
        solution[i] = i;
    }

    // Shuffle the solution array to create a random permutation
    for (int i = size - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(solution[i], solution[j]);
    }

    int minLength = problem.minLength();
    int* croppedSolution = new int[minLength];
    std::copy(solution, solution + minLength, croppedSolution);

    delete[] solution;
    return croppedSolution;
}