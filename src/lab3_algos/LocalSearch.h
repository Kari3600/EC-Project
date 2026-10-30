#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "../Solver.h"
#include "../Problem.h"

#include <random>
#include <string>
#include <vector>

enum SearchType { Greedy, Steepest };
enum IntraMove { Nodes, Edges };

class LocalSearch : public Solver {
    public:
        LocalSearch(SearchType search, IntraMove intra, Solver* initialSolver)
            : search(search), intra(intra), initialSolver(initialSolver) {
            name = std::string("LS_")
                 + (search == SearchType::Greedy ? "greedy" : "steepest")
                 + (intra == IntraMove::Nodes ? "_nodes" : "_edges")
                 + (initialSolver->getName());   // e.g. LS_greedy_edges_random
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
        int twoNodesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const;
        int twoEdgesDelta(const Problem& problem, const std::vector<int>& solution, int i, int j) const;

        // moves: modify the solution in place
        void applyInter(std::vector<int>& solution, std::vector<bool>& selected, int position, int newNode) const;
        void applyTwoNodes(std::vector<int>& solution, int i, int j) const;
        void applyTwoEdges(std::vector<int>& solution, int i, int j) const;

        bool greedyStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected);
        bool steepestStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected);

        SearchType search;
        IntraMove intra;
        std::mt19937 rng;
        std::string name;
        Solver* initialSolver;
        std::vector<int> initialSolution;
        int steps = 0;
};

#endif
