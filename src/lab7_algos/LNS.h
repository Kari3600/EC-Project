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
            return {{"iterations", static_cast<int>(trace.size())},
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
        std::vector<int> trace;   // objective after every LS run (the first one is the LS from the random start)
        int sameOptimum = 0;      // LS runs after a perturbation that returned a solution as good as x
};

#endif