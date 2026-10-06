#include "Solver.h"
#include "Problem.h"

class RandomSolver : public Solver {
    public:
        int* solve(const Problem& problem) override;
};