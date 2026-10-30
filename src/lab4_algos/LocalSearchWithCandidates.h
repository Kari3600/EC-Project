#ifndef LOCALSEARCHWITHCANDIDATES_H
#define LOCALSEARCHWITHCANDIDATES_H

#include "../Solver.h"
#include "../Problem.h"

#include "../lab1_algos/RandomSolver.h"

#include <random>
#include <string>
#include <vector>

class LocalSearchWithCandidates : public Solver {
    public:
        LocalSearchWithCandidates(int candidatesCount)
            : candidatesCount(candidatesCount) {
            name = std::string("LS_")
                 + std::to_string(candidatesCount);
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"Steps", steps}};
        }

        // starting solution of the last solve() call
        const std::vector<int>& getInitialSolution() const {
            return initialSolution;
        }
        // number of improving moves applied in the last solve() call
        int getSteps() const {
            return steps;
        }

    private:
        std::vector<int> getInitialSolution(const Problem& problem, int seed) const;

        // deltas: change of the objective after the move, negative = improvement
        int interDelta(const Problem& problem, const std::vector<int>& solution, int position, int newNode) const;
        int twoEdgesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const;

        // moves: modify the solution in place
        void applyInter(std::vector<int>& solution, std::vector<bool>& selected, int position, int newNode) const;
        void applyTwoEdges(std::vector<int>& solution, int i, int j) const;

        bool steepestStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected,
                          const std::vector<std::vector<int>>& candidateNeighbors);

        int candidatesCount;
        std::mt19937 rng;
        std::string name;
        Solver* initialSolver = new RandomSolver();
        std::vector<int> initialSolution;
        int steps = 0;
};

#endif
