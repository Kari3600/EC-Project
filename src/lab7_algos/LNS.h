#ifndef LNS_H
#define LNS_H

#include "../Solver.h"
#include "../Problem.h"

#include "Destroyer.h"

#include <sstream>
#include <string>

class LNS : public Solver {
    public:
        LNS(Destroyer* destroyer, Solver* repairer, Solver* postprocessor, double discardPercent, double timeMs)
            : destroyer(destroyer), repairer(repairer), postprocessor(postprocessor), discardPercent(discardPercent), timeMs(timeMs) {
            name = std::string("LNS");
        }
        const char* getName() const override {
            return name.c_str();
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) override;
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"iterations", static_cast<int>(trace.size()) - 1},   // main loop iterations
                    {"same_optimum", sameOptimum},
                    {"trace", trace}};
        }

    private:
        Destroyer* destroyer;
        Solver* repairer;
        Solver* postprocessor;
        double discardPercent;
        double timeMs;

        std::string name;

        // statistics of the last solve() call
        std::vector<int> trace;   // objective of the starting solution, then of every candidate (one per main loop iteration)
        int sameOptimum = 0;      // candidates as good as the current solution
};

#endif