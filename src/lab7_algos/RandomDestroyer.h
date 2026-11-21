#ifndef RANDOMDESTROYER_H
#define RANDOMDESTROYER_H

#include "Destroyer.h"

class RandomDestroyer : public Destroyer {
    public:
        void destroy(const Problem& problem, std::vector<int>& solution, std::mt19937 rng, int amount) const override;
};

#endif