#ifndef RANDOMSOLVER_H
#define RANDOMSOLVER_H

#include "../Solver.h"
#include "../Problem.h"

class RandomSolver : public Solver {
    public:
        const char* getName() const override {
            return "RandomSolver";
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) override;
};

#endif