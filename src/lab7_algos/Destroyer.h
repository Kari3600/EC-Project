#ifndef DESTROYER_H
#define DESTROYER_H

#include "../Problem.h"
#include <random>
#include <vector>

class Destroyer {
    public:
        virtual void destroy(const Problem& problem, std::vector<int>& solution, std::mt19937 rng, int amount) const = 0;
        virtual ~Destroyer() = default;
};

#endif