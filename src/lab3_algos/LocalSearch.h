#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "../Solver.h"
#include "../Problem.h"

#include <random>
#include <string>
#include <vector>

enum class SearchType { Greedy, Steepest };
enum class IntraMove { Nodes, Edges };
enum class StartType { Random, Greedy };

class LocalSearch : public Solver {
    public:
        LocalSearch(SearchType search, IntraMove intra, StartType start)
            : search(search), intra(intra), start(start) {
            name = std::string("LS_")
                 + (search == SearchType::Greedy ? "greedy" : "steepest")
                 + (intra == IntraMove::Nodes ? "_nodes" : "_edges")
                 + (start == StartType::Random ? "_random" : "_greedy");   // e.g. LS_greedy_edges_random
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int startNode) override;

    private:
        std::vector<int> getInitialGreedy(const Problem& problem, int startNode) const;
        std::vector<int> getInitialRandom(const Problem& problem, int startNode) const;

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
        StartType start;
        std::mt19937 rng;
        std::string name;
};

#endif
