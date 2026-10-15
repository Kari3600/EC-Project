#ifndef GREEDYCYCLE_2REGRET_H
#define GREEDYCYCLE_2REGRET_H

#include "../Solver.h"
#include "../Problem.h"

#include <sstream>
#include <string>

// Criterion to maximize: alpha * regret - (1 - alpha) * bestChange
// alpha = 1 is pure greedy 2-regret, alpha = 0.5 gives equal weights
// alpha = 0 gives the same results as GreedyCycle (with some exceptions when there are ties)
class GreedyCycle_2regret : public Solver {
    public:
        explicit GreedyCycle_2regret(double alpha = 0.5) : alpha(alpha) {
            std::ostringstream out;
            out << "GreedyCycle_2regret_a" << alpha;   // e.g. ..._a1, ..._a0.5
            name = out.str();
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int startNode) override;

    private:
        double alpha;
        std::string name;
};

#endif
