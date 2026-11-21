#ifndef SOLVER_H
#define SOLVER_H

#include "Problem.h"

#include <random>
#include <vector>
#include <map>
#include <string>
#include <any>

class Solver {
    public:
        virtual const char* getName() const = 0;
        virtual void solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) = 0;
        virtual std::map<std::string, std::any> getLastRunStatistics() const {
            return {};
        };
};

#endif