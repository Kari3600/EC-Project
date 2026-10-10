#ifndef GREEDYCYCLE_H
#define GREEDYCYCLE_H

#include "../Solver.h"

class GreedyCycle : public Solver {
    public:
        const char* getName() const override {
            return "GreedyCycle";
        }
        int* solve(const Problem& problem) override;
};

#endif