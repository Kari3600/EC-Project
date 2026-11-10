#ifndef ILS_H
#define ILS_H

#include "../Solver.h"
#include "../Problem.h"
#include "../lab5_algos/ListOfMovesSolver.h"

#include <random>
#include <string>
#include <vector>

enum class Perturbation { RandomMoves, Segment, DoubleBridge, Guided };

// Iterated local search: x = LS(random), then until the time limit: y = LS(perturb(x)),
// x = y if y is better. The local search is ListOfMovesSolver (its initialSolver builds
// the random starting solution).
class ILS : public Solver {
    public:
        ILS(ListOfMovesSolver* localSearch, Perturbation perturbation, int strength, double timeMs)
            : localSearch(localSearch), perturbation(perturbation), strength(strength), timeMs(timeMs) {
            name = std::string("ILS_") + perturbationName(perturbation) + "_k=" + std::to_string(strength);
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"LS runs", static_cast<int>(trace.size())},
                    {"Same optimum", sameOptimum},
                    {"Trace", trace}};
        }

        static Perturbation parsePerturbation(const std::string& text);
        static const char* perturbationName(Perturbation perturbation);

    private:
        void perturb(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected);
        void randomMoves(std::vector<int>& solution, std::vector<bool>& selected);
        void segment(std::vector<int>& solution, std::vector<bool>& selected);
        void doubleBridge(std::vector<int>& solution);
        void guided(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected);

        int randomInt(int from, int to);   // uniform in [from, to]
        int randomUnselected(const std::vector<bool>& selected);

        ListOfMovesSolver* localSearch;
        Perturbation perturbation;
        int strength;
        double timeMs;
        std::string name;
        std::mt19937 rng;

        // guided perturbation: for every node, the other nodes sorted by distance + cost
        std::vector<std::vector<int>> nearest;

        // statistics of the last solve() call
        std::vector<int> trace;   // objective after every LS run (the first one is the LS from the random start)
        int sameOptimum = 0;      // LS runs after a perturbation that returned a solution as good as x
};

#endif
