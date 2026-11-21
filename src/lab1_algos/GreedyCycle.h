#ifndef GREEDYCYCLE_H
#define GREEDYCYCLE_H

#include "../Solver.h"
#include "../Problem.h"

class GreedyCycle : public Solver {
    public:
        const char* getName() const override {
            return "GreedyCycle";
        }
        void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& seed) override;
};

#endif