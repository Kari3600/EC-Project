#include "GreedyCycle.h"

#include <vector>

std::vector<int> GreedyCycle::solve(const Problem& problem, int startNode) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    std::vector<int> solution(minLength);

    std::vector<bool> visited(size, false);
    solution[0] = startNode;
    visited[startNode] = true;

    int nextNode = -1;
    int bestScore = 0;
    for (int candidate = 0; candidate < size; ++candidate) {
        if (visited[candidate]) {
            continue;
        }

        const int score = problem.getDist(solution[0], candidate) +
                          problem.getDist(candidate, solution[0]) +
                          problem.getCosts()[candidate];

        if (nextNode == -1 || score < bestScore) {
            nextNode = candidate;
            bestScore = score;
        }
    }
    solution[1] = nextNode;
    visited[nextNode] = true;

    for (int i = 2; i < minLength; ++i) {
        int nextNode = -1;
        int nextPosition = -1;
        int bestScore = 0;

        int cycleSize = i;

        for (int beforeNode = 0; beforeNode < cycleSize; ++beforeNode) {
            int afterNode = (beforeNode + 1) % cycleSize;
            int currentScore = problem.getDist(solution[beforeNode], solution[afterNode]);
            for (int candidate = 0; candidate < size; ++candidate) {
                if (visited[candidate]) {
                    continue;
                }

                const int score = problem.getDist(solution[beforeNode], candidate) +
                                        problem.getDist(candidate, solution[afterNode]) +
                                        problem.getCosts()[candidate] -
                                        currentScore;

                if (nextNode == -1 || score < bestScore) {
                    nextNode = candidate;
                    nextPosition = afterNode;
                    bestScore = score;
                }
            }
        }

        for (int j = cycleSize; j > nextPosition; --j) {
            solution[j] = solution[j - 1];
        }

        solution[nextPosition] = nextNode;
        visited[nextNode] = true;
    }
    return solution;
}