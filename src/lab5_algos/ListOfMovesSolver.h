#ifndef LISTOFMOVESSOLVER_H
#define LISTOFMOVESSOLVER_H

#include "../Solver.h"
#include "../Problem.h"

#include "../lab1_algos/RandomSolver.h"

#include <string>
#include <vector>
#include <iterator>
#include <set>

struct Data {
    bool inter;
    int x1;
    int x2;
    int x3;
    int x4;
    int delta;
};

struct Compare {
    bool operator()(const Data& a, const Data& b) const {
        return std::tie(a.delta, a.x1, a.x2, a.x3, a.x4) < std::tie(b.delta, b.x1, b.x2, b.x3, b.x4);
    }
};

class ListOfMovesSolver : public Solver {
    public:
        ListOfMovesSolver(Solver* initialSolver)
            : initialSolver(initialSolver) {
            name = std::string("LMS_")
                 + initialSolver->getName();
        }
        const char* getName() const override {
            return name.c_str();
        }
        std::vector<int> solve(const Problem& problem, int seed) override;
        // local search from the given solution (used by ILS); updates getSteps()
        std::vector<int> improve(const Problem& problem, std::vector<int> solution);
        std::map<std::string, std::any> getLastRunStatistics() const override {
            return {{"Steps", steps}};
        }

        // starting solution of the last solve() call
        const std::vector<int>& getInitialSolution() const {
            return initialSolution;
        }
        // number of improving moves applied in the last solve() call
        int getSteps() const {
            return steps;
        }

    private:
        // deltas: change of the objective after the move, negative = improvement
        int interDelta(const Problem& problem, int x1, int x2, int x3, int x4) const;
        int intraDelta(const Problem& problem, int x1, int x2, int prev, int next) const;

        std::vector<int>::iterator safeNext(std::vector<int>& vector, std::vector<int>::iterator it) const;
        std::vector<int>::iterator safePrev(std::vector<int>& vector, std::vector<int>::iterator it) const;

        void generateIntraMovesLeaving(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected, std::set<Data,Compare>& listOfMoves, std::vector<int>::iterator pos);
        void generateIntraMovesEntering(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected, std::set<Data,Compare>& listOfMoves, int entering);
        void generateInterMoves(const Problem& problem, std::vector<int>& solution, std::set<Data,Compare>& listOfMoves, std::vector<int>::iterator node1, std::vector<int>::iterator node2, bool ignorePrevious = false);

        // moves: modify the solution in place
        void applyInter(std::vector<int>& solution, std::vector<bool>& selected, int position, int newNode) const;
        void applyIntra(std::vector<int>& solution, int i, int j) const;

        bool steepestStep(const Problem& problem, std::vector<int>& solution, std::vector<bool>& selected,
                          const std::vector<std::vector<int>>& candidateNeighbors);

        std::string name;
        Solver* initialSolver;
        std::vector<int> initialSolution;
        int steps = 0;
};

#endif
