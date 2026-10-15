#ifndef NEARESTNEIGHBORII_2REGRET_H
#define NEARESTNEIGHBORII_2REGRET_H

#include "../Solver.h"
#include "../Problem.h"

#include <sstream>
#include <string>

// Criterion to maximize: alpha * regret - (1 - alpha) * bestChange
// alpha = 1 is pure greedy 2-regret, alpha = 0.5 gives equal weights
// alpha = 0 gives the same results as NNII (with some exceptions when there are ties)
class NearestNeighborII_2regret : public Solver {
    public:
        explicit NearestNeighborII_2regret(double alpha = 0.5) : alpha(alpha) {
            std::ostringstream out;
            out << "NearestNeighborII_2regret_a" << alpha;   // e.g. ..._a1, ..._a0.5
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
