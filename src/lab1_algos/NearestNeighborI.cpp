#include "NearestNeighborI.h"

#include <vector>

void NearestNeighborI::solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    const int insize = solution.size();

    std::vector<bool> visited(size, false);

    for (int node : solution) {
        visited[node] = true;
    }

    int currentNode = solution[insize-1];

    solution.resize(problem.minLength());

    for (int i = insize; i < minLength; ++i) {
        int nextNode = -1;
        int bestScore = 0;

        for (int candidate = 0; candidate < size; ++candidate) {
            if (visited[candidate]) {
                continue;
            }

            const int score = problem.getDist(currentNode, candidate) +
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
}