#ifndef GREEDYCYCLE_H
#define GREEDYCYCLE_H

#include "../Solver.h"
#include "../Problem.h"

class GreedyCycle : public Solver {
    public:
        const char* getName() const override {
            return "GreedyCycle";
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
};

#endif