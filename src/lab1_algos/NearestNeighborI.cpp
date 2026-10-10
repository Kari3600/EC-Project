#include "NearestNeighborI.h"

#include <vector>

std::vector<int> NearestNeighborI::solve(const Problem& problem, int startNode) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    std::vector<int> solution(minLength);

    int currentNode = startNode;
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

    return solution;
}