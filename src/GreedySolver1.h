#ifndef GREEDYSOLVER1_H
#define GREEDYSOLVER1_H

#include "Solver.h"
#include "Problem.h"

class GreedySolver1 : public Solver {
    public:
        const char* getName() const override {
            return "GreedySolver1";
        }
        int* solve(const Problem& problem) override;
};

#endif