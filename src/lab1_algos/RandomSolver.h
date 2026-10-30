#ifndef RANDOMSOLVER_H
#define RANDOMSOLVER_H

#include "../Solver.h"
#include "../Problem.h"

class RandomSolver : public Solver {
    public:
        const char* getName() const override {
            return "RandomSolver";
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
};

#endif