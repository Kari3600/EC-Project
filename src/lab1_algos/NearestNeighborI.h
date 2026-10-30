#ifndef NEARESTNEIGHBORI_H
#define NEARESTNEIGHBORI_H

#include "../Solver.h"
#include "../Problem.h"

class NearestNeighborI : public Solver {
    public:
        const char* getName() const override {
            return "NearestNeighborI";
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
};

#endif