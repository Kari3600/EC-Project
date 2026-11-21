#ifndef NEARESTNEIGHBORI_H
#define NEARESTNEIGHBORI_H

#include "../Solver.h"
#include "../Problem.h"

class NearestNeighborI : public Solver {
    public:
        const char* getName() const override {
            return "NearestNeighborI";
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) override;
};

#endif