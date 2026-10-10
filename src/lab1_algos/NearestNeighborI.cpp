#include "NearestNeighborI.h"

#include <vector>

int* GreedySolver1::solve(const Problem& problem) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    int* solution = new int[minLength];

    int currentNode = 0;
    std::vector<bool> visited(size, false);
    solution[0] = currentNode;
    visited[currentNode] = true;

    for (int i = 1; i < minLength; ++i) {
        int nextNode = -1;
        int bestScore = 0;

        for (int candidate = 0; candidate < size; ++candidate) {
            if (visited[candidate]) {
                continue;
            }

            const int score = problem.getWeight(currentNode)[candidate] +
                                    problem.getCosts()[candidate];
            if (nextNode == -1 || score < bestScore) {
                nextNode = candidate;
                bestScore = score;
            }
        }

        solution[i] = nextNode;
        visited[nextNode] = true;
        currentNode = nextNode;
    }

    return solution;
}