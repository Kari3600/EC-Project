#ifndef NEARESTNEIGHBORII_H
#define NEARESTNEIGHBORII_H

#include "../Solver.h"
#include "../Problem.h"

class NearestNeighborII : public Solver {
    public:
        const char* getName() const override {
            return "NearestNeighborII";
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) override;
};

#endif