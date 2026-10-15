#include "NearestNeighborII_2regret.h"

#include <vector>
#include <climits>

std::vector<int> NearestNeighborII_2regret::solve(const Problem& problem, int startNode) {
    const int size = problem.getSize();
    const int minLength = problem.minLength();
    std::vector<int> solution(minLength);

    int currentNode = startNode;
    std::vector<bool> visited(size, false);
    solution[0] = currentNode;
    visited[currentNode] = true;

    for (int i = 1; i < minLength; ++i) { //iterations- in each we add 1 node to solution
        int injectNode = -1; //highest regret node
        double bestCriterion = 0;
        int injectPose = -1;
        
        for (int candidate = 0; candidate < size; ++candidate) { //iterating over nodes beyond current sol
            if (visited[candidate]) {
                continue;
            }
            int bestScore = INT_MAX;// we want to minimise
            int bestScore2nd = INT_MAX;
            int injectPoseCurrent = -1;


            for (int injectGap = -1; injectGap < i; injectGap++){
                int score = 0;

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
                // calculating 2 best scores for a candidate for regret calculation
                if (score < bestScore){
                    bestScore2nd = bestScore;
                    bestScore = score;
                    injectPoseCurrent = injectGap;
                } else if (score < bestScore2nd){
                    bestScore2nd = score;
                }
            }
            int regret = bestScore2nd - bestScore; //we want to maximise
            double optimisationCriterion = alpha * regret - (1 - alpha) * bestScore;
            if (injectNode == -1 || optimisationCriterion > bestCriterion){ // if we found new node with best regret 
                bestCriterion = optimisationCriterion;
                injectNode = candidate;
                injectPose = injectPoseCurrent;
            }
        
        
        }
        // injectiong new node
        const int pos = injectPose + 1;         
        for (int j = i; j > pos; --j) {          
            solution[j] = solution[j - 1];
        }
        solution[pos] = injectNode;
        visited[injectNode] = true;
    }

    return solution;
}