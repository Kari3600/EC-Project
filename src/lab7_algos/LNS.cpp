#include "LNS.h"

#include <chrono>

void LNS::solve(const Problem& problem, std::vector<int>& solution, std::mt19937& rng) {
    using Clock = std::chrono::steady_clock;
    const auto startTime = Clock::now();
    
    int amount = problem.minLength() * discardPercent;

    int bestScore = problem.evaluate(solution);
    trace.assign(1, bestScore);
    sameOptimum = 0;

    while (std::chrono::duration<double, std::milli>(Clock::now() - startTime).count() < timeMs) {
        std::vector<int> candidateSolution = solution;
        destroyer->destroy(problem, candidateSolution, rng, amount);
        repairer->solve(problem, candidateSolution, rng);
        if (postprocessor) {
            postprocessor->solve(problem, candidateSolution, rng);
        }
        int score = problem.evaluate(candidateSolution);
        trace.push_back(score);
        if (score < bestScore) {
            solution = candidateSolution;
            bestScore = score;
        } else if (score == bestScore) {
            sameOptimum++;
        }
    }
}