#ifndef SOLVER_H
#define SOLVER_H

#include "Problem.h"

#include <vector>

class Solver {
    public:
        virtual const char* getName() const = 0;
        virtual std::vector<int> solve(const Problem& problem, int startNode) = 0;
};

#endif