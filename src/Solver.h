#ifndef SOLVER_H
#define SOLVER_H

#include "Problem.h"

class Solver {
    public:
        virtual const char* getName() const = 0;
        virtual int* solve(const Problem& problem) = 0;
};

#endif