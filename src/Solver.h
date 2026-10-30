#ifndef SOLVER_H
#define SOLVER_H

#include "Problem.h"

#include <vector>
#include <map>
#include <string>
#include <any>

class Solver {
    public:
        virtual const char* getName() const = 0;
        virtual std::vector<int> solve(const Problem& problem, int seed) = 0;
        virtual std::map<std::string, std::any> getLastRunStatistics() const {
            return {};
        };
};

#endif