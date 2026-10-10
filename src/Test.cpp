#include "FileReader.cpp"
#include "Problem.cpp"
#include "./lab1_algos/RandomSolver.cpp"
#include "./lab1_algos/NearestNeighborI.cpp"
#include "./lab1_algos/NearestNeighborII.cpp"
#include "./lab1_algos/GreedyCycle.cpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int main() {
    try {
        const std::vector<std::string> instances = {
            "data/InstanceA.csv",
            "data/InstanceB.csv"
        };

        RandomSolver randomSolver;
        NearestNeighborI nearestNeighborI;
        NearestNeighborII nearestNeighborII;
        GreedyCycle greedyCycle;
        const std::vector<Solver*> solvers = {
            &randomSolver,
            &nearestNeighborI,
            &nearestNeighborII,
            &greedyCycle
            };

        for (const auto& path : instances) {
            std::ifstream file(path);
            if (!file) {
                throw std::runtime_error("Failed to open data file " + path);
            }
            Problem problem = readProblem(file);
            std::cout << "=== " << path << " (size " << problem.getSize() << ")" << std::endl;

            for (const auto& solver : solvers) {
                const int runs = problem.getSize();
                int minValue = 0;
                int maxValue = 0;
                long long sum = 0;
                std::vector<int> best;
                double totalMs = 0;

                for (int start = 0; start < runs; ++start) {
                    const auto startTime = std::chrono::steady_clock::now();
                    const std::vector<int> solution = solver->solve(problem, start);
                    const auto endTime = std::chrono::steady_clock::now();
                    totalMs += std::chrono::duration<double, std::milli>(endTime - startTime).count();

                    const int value = problem.evaluate(solution);

                    if (start == 0 || value < minValue) {
                        minValue = value;
                        best = solution;
                    }
                    if (start == 0 || value > maxValue) {
                        maxValue = value;
                    }
                    sum += value;
                }

                std::cout << solver->getName()
                          << ": min " << minValue
                          << ", max " << maxValue
                          << ", avg " << static_cast<double>(sum) / runs << std::endl;
                std::cout << "  time: total " << totalMs << " ms"
                          << ", avg per run " << totalMs / runs << " ms" << std::endl;
                std::cout << "  best: ";
                for (const int node : best) {
                    std::cout << node << " ";
                }
                std::cout << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
