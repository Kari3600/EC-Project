#include "ListOfMovesSolver.h"

#include <vector>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <iterator>

#include "../lab1_algos/RandomSolver.h"

int ListOfMovesSolver::interDelta(const Problem& problem, int x1, int x2, int x3, int x4) const {
    return problem.getDist(x1, x3) + problem.getDist(x2, x4)
         - problem.getDist(x1, x2) - problem.getDist(x3, x4);
}

int ListOfMovesSolver::intraDelta(const Problem& problem, int x1, int x2, int prev, int next) const {
    return problem.getCosts()[x1] - problem.getCosts()[x2]
         + problem.getDist(prev, x1) + problem.getDist(x1, next)
         - problem.getDist(prev, x2) - problem.getDist(x2, next);
}

std::vector<int>::iterator ListOfMovesSolver::safeNext(std::vector<int>& vector, std::vector<int>::iterator it) const {
    auto next = std::next(it);
    if (next == vector.end()) {
        return vector.begin();
    }
    return next;
}

std::vector<int>::iterator ListOfMovesSolver::safePrev(std::vector<int>& vector, std::vector<int>::iterator it) const {
    if (it == vector.begin()) {
        return std::prev(vector.end());
    }
    return std::prev(it);
}

void ListOfMovesSolver::generateIntraMoves(
    const Problem& problem,
    std::vector<int>& solution,
    std::vector<bool>& selected,
    std::set<Data,Compare>& listOfMoves,
    std::vector<int>::iterator pos
) {
    auto prev = safePrev(solution, pos);
    auto next = safeNext(solution, pos);
    for (int j = 0; j < problem.getSize(); ++j) {
        if (j == *pos) {
            continue;
        }
        if (!selected[j]) {
            int delta = intraDelta(problem, j, *pos, *prev, *next);
            if (delta < 0) {
                listOfMoves.insert({false, j, *pos, *prev, *next, delta});
            }
        }
    }
}

void ListOfMovesSolver::generateInterMoves(
    const Problem& problem,
    std::vector<int>& solution,
    std::set<Data,Compare>& listOfMoves,
    std::vector<int>::iterator node1,
    std::vector<int>::iterator node2,
    bool ignorePrevious
) {
    for (auto node3 = ignorePrevious ? safeNext(solution, node2) : solution.begin(); node3 != solution.end(); ++node3) {
        if (!ignorePrevious && (node3 == node1 || node3 == node2)) {
            continue;
        }
        auto node4 = safeNext(solution, node3);
        if (!ignorePrevious && (node4 == node1 || node4 == node2)) {
            continue;
        }
        int delta = interDelta(problem, *node1, *node2, *node3, *node4);
        if (delta < 0) {
            listOfMoves.insert({true, *node1, *node2, *node3, *node4, delta});
        } 
        delta = interDelta(problem, *node1, *node2, *node4, *node3);
        if (delta < 0) {
            listOfMoves.insert({true, *node1, *node2, *node4, *node3, delta});
        }
    }
}


std::vector<int> ListOfMovesSolver::solve(const Problem& problem, int seed) {
    // we need to remember to count later how much time did it take compared to the whole algorithm
    std::vector<int> solution = initialSolver->solve(problem, seed);
    initialSolution = solution;
    steps = 0;

    std::vector<bool> selected(problem.getSize(), false);
    for (int node : solution) {
        selected[node] = true;
    }

    std::set<Data,Compare> listOfMoves;

    for (int i = 0; i < solution.size(); ++i) {
        auto pos = std::next(solution.begin(), i);
        generateIntraMoves(problem, solution, selected, listOfMoves, pos);
    }

    for (int i = 0; i < solution.size(); ++i) {
        auto node1 = std::next(solution.begin(), i);
        auto node2 = safeNext(solution, node1);
        generateInterMoves(problem, solution, listOfMoves, node1, node2, true);
    }

    while (!listOfMoves.empty()) {
        // std::cout << "Candidates: " << listOfMoves.size() << std::endl;

        // std::cout << "Current soltuion: ";
        // for (auto val : solution) {
        //     std::cout << val << " ";
        // }
        // std::cout << std::endl;

        const Data bestMove = *listOfMoves.begin();
        listOfMoves.erase(listOfMoves.begin());
        if (!bestMove.inter) {
            const int x1 = bestMove.x1;
            const int x2 = bestMove.x2;
            if (!selected[x2]) {
                // x2 is no longer in the solution, so the move is no longer valid
                continue;
            }
            if (selected[x1]) {
                // x1 is already in the solution, so the move is no longer valid
                continue;
            }
            auto pos = std::find(solution.begin(), solution.end(), x2);
            auto prev = safePrev(solution, pos);
            auto next = safeNext(solution, pos);

            if (!(*prev == bestMove.x3 && *next == bestMove.x4 || *prev == bestMove.x4 && *next == bestMove.x3)) {
                // either x3 or x4 is no longer in the solution, so the move is no longer valid
                continue;
            }

            selected[x2] = false;
            selected[x1] = true;
            *pos = x1;
            // for (auto it = listOfMoves.begin(); it != listOfMoves.end(); ) {
            //     if (!it->inter) {
            //         if (it->x1 == x1 || it->x2 == x2) {
            //             it = listOfMoves.erase(it);
            //             continue;
            //         }
            //     } else {
            //         if (it->x1 == x2 || it->x2 == x2 || it->x3 == x2 || it->x4 == x2) {
            //             it = listOfMoves.erase(it);
            //             continue;
            //         }
            //     }
            //     ++it;
            // }
            generateIntraMoves(problem, solution, selected, listOfMoves, prev);
            generateIntraMoves(problem, solution, selected, listOfMoves, pos);
            generateIntraMoves(problem, solution, selected, listOfMoves, next);
            generateInterMoves(problem, solution, listOfMoves, prev, pos);
            generateInterMoves(problem, solution, listOfMoves, pos, next);
        } else {
            const int x1 = bestMove.x1;
            const int x2 = bestMove.x2;
            const int x3 = bestMove.x3;
            const int x4 = bestMove.x4;
            bool flag = false;
            auto pos1 = std::find(solution.begin(), solution.end(), x1);
            auto pos3 = std::find(solution.begin(), solution.end(), x3);
            if (pos1 == solution.end() || pos3 == solution.end()) {
                // either x1 or x3 is no longer in the solution, so the move is no longer valid
                continue;
            }
            auto pos2 = safeNext(solution, pos1);
            if (*pos2 != x2) {
                pos2 = safePrev(solution, pos1);
                flag = !flag;
                if (*pos2 != x2) {
                    // either x2 is no longer in the solution or is not next to x1, so the move is no longer valid
                    continue;
                }
                std::swap(pos1, pos2);
            }
            auto pos4 = safeNext(solution, pos3);
            if (*pos4 != x4) {
                pos4 = safePrev(solution, pos3);
                flag = !flag;
                if (*pos4 != x4) {
                    // either x4 is no longer in the solution or is not next to x3, so the move is no longer valid
                    continue;
                }
                std::swap(pos3, pos4);
            }
            if (flag) {
                // this move would break cycle into two separate cycles, so we cannot apply it
                const int delta = interDelta(problem, *pos1, *pos2, *pos3, *pos4);
                if (delta < 0) {
                    listOfMoves.insert({true, *pos1, *pos2, *pos3, *pos4, delta});
                }
                continue;
            }
            if (pos2 > pos4) {
                std::swap(pos1, pos3);
                std::swap(pos2, pos4);
            }
            std::reverse(pos2, pos4);
            // for (auto it = listOfMoves.begin(); it != listOfMoves.end(); ) {
            //     if (!isIntraMove(it->move)) {
            //         const int y1 = (it->move >> 48) & 0xFFFF;
            //         const int y2 = (it->move >> 32) & 0xFFFF;
            //         const int y3 = (it->move >> 16) & 0xFFFF;
            //         const int y4 = it->move & 0xFFFF;
            //         if (y1 == x1 && y2 == x2
            //          || y2 == x1 && y1 == x2
            //          || y3 == x3 && y4 == x4
            //          || y4 == x3 && y3 == x4
            //          || y1 == x3 && y2 == x4
            //          || y2 == x3 && y1 == x4
            //          || y3 == x1 && y4 == x2
            //          || y4 == x1 && y3 == x2) {
            //             it = listOfMoves.erase(it);
            //             continue;
            //         }
            //     }
            //     ++it;
            // }
            generateIntraMoves(problem, solution, selected, listOfMoves, pos1);
            generateIntraMoves(problem, solution, selected, listOfMoves, pos2);
            generateIntraMoves(problem, solution, selected, listOfMoves, pos3);
            generateIntraMoves(problem, solution, selected, listOfMoves, pos4);
            generateInterMoves(problem, solution, listOfMoves, pos1, pos2);
            generateInterMoves(problem, solution, listOfMoves, pos3, pos4);
        }
        ++steps;
    }

    return solution;
}
