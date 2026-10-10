#ifndef NEARESTNEIGHBORII_H
#define NEARESTNEIGHBORII_H

#include "../Solver.h"
#include "../Problem.h"

class NearestNeighborII : public Solver {
    public:
        const char* getName() const override {
            return "NearestNeighborII";
        }
        std::vector<int> solve(const Problem& problem, int startNode) override;
};

#endif