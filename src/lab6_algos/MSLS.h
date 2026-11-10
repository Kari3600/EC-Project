#ifndef MSLS_H
#define MSLS_H

#include "../Solver.h"
#include "../Problem.h"

#include <string>
#include <vector>

// Multiple start local search: runs the given local search `iterations` times, each time from
// a different random starting solution, and returns the best local optimum found.
// The local search must build its own starting solution with RandomSolver.
class MSLS : public Solver {
    public:
        MSLS(Solver* localSearch, int iterations = 400)
            : localSearch(localSearch), iterations(iterations) {
            name = std::string("MSLS_") + localSearch->getName();
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"LS runs", iterations}};
        }

    private:
        Solver* localSearch;
        int iterations;
        std::string name;
};

#endif
