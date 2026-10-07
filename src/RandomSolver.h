#ifndef RANDOMSOLVER_H
#define RANDOMSOLVER_H

#include "Solver.h"
#include "Problem.h"

class RandomSolver : public Solver {
    public:
        const char* getName() const override {
            return "RandomSolver";
        }
        int* solve(const Problem& problem) override;
};

#endif