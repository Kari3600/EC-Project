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
        LocalSearch(SearchType search, IntraMove intra)
            : search(search), intra(intra){
            name = std::string("LS_")
                 + (search == SearchType::Greedy ? "greedy" : "steepest")
                 + (intra == IntraMove::Nodes ? "_nodes" : "_edges");
        }
        const char* getName() const override {
            return name.c_str();
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) override;
        // local search from the given solution (used by ILS); updates getSteps()
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"Steps", steps}};
        }

        // number of improving moves applied in the last solve() call
        int getSteps() const {
            return steps;
        }

    private:
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
