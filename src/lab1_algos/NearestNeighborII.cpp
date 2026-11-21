#include "NearestNeighborII.h"

#include <vector>

void NearestNeighborII::solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    const int insize = solution.size();

    std::vector<bool> visited(size, false);

    for (int node : solution) {
        visited[node] = true;
    }

    solution.resize(problem.minLength());

    for (int i = insize; i < minLength; ++i) {
        int injectNode = -1;
        int injectPose = -2;
        int bestScore = 0;
        
        for (int injectGap = -1; injectGap < i; injectGap++){
            for (int candidate = 0; candidate < size; ++candidate) {

                int score = 0;
                if (visited[candidate]) {
                    continue;
                }

                if(injectGap == -1){
                    score = problem.getDist(candidate, solution[0])
                                    + problem.getCosts()[candidate];
                }
                else if (injectGap == i - 1)
                {
                    score = problem.getDist(candidate, solution[i - 1])
                                    + problem.getCosts()[candidate];
                }
                else{
                    int node1 = solution[injectGap];
                    int node2 = solution[injectGap + 1];
                    score = problem.getDist(node1, candidate)
                                + problem.getDist(candidate, node2)
                                + problem.getCosts()[candidate]
                                - problem.getDist(node1, node2);
                }
                if (injectNode == -1 || score < bestScore) {
                    injectNode = candidate;
                    injectPose = injectGap;
                    bestScore = score;
                }
            }
        }

        const int pos = injectPose + 1;         
        for (int j = i; j > pos; --j) {          
            solution[j] = solution[j - 1];
        }
        solution[pos] = injectNode;
        visited[injectNode] = true;
    }
}