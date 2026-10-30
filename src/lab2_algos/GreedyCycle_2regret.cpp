#include "GreedyCycle_2regret.h"

#include <vector>
#include <climits>

std::vector<int> GreedyCycle_2regret::solve(const Problem& problem, int seed) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    std::vector<int> solution(minLength);

    std::vector<bool> visited(size, false);
    int startNode = seed % size;
    solution[0] = startNode;
    visited[startNode] = true;

    // 2nd node: same as GreedyCycle (with one node there is only one position, so no regret)
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

    for (int i = 2; i < minLength; ++i) { //iterations- in each we add 1 node to the cycle
        int injectNode = -1; //node with the best criterion
        int injectPose = -1;
        double bestCriterion = 0;

        int cycleSize = i;

        for (int candidate = 0; candidate < size; ++candidate) { //iterating over nodes beyond current cycle
            if (visited[candidate]) {
                continue;
            }

            int bestScore = INT_MAX;// we want to minimise
            int bestScore2nd = INT_MAX;
            int injectPoseCurrent = -1;

            for (int beforeNode = 0; beforeNode < cycleSize; ++beforeNode) { //every edge of the cycle
                int afterNode = (beforeNode + 1) % cycleSize;
                int currentScore = problem.getDist(solution[beforeNode], solution[afterNode]);

                const int score = problem.getDist(solution[beforeNode], candidate) +
                                        problem.getDist(candidate, solution[afterNode]) +
                                        problem.getCosts()[candidate] -
                                        currentScore;

                // calculating 2 best scores for a candidate for regret calculation
                if (score < bestScore) {
                    bestScore2nd = bestScore;
                    bestScore = score;
                    injectPoseCurrent = afterNode;
                } else if (score < bestScore2nd) {
                    bestScore2nd = score;
                }
            }

            int regret = bestScore2nd - bestScore; //we want to maximise
            double optimisationCriterion = alpha * regret - (1 - alpha) * bestScore;
            if (injectNode == -1 || optimisationCriterion > bestCriterion) {
                bestCriterion = optimisationCriterion;
                injectNode = candidate;
                injectPose = injectPoseCurrent;
            }
        }

        // injecting new node before solution[injectPosition]
        for (int j = cycleSize; j > injectPose; --j) {
            solution[j] = solution[j - 1];
        }

        solution[injectPose] = injectNode;
        visited[injectNode] = true;
    }
    return solution;
}
