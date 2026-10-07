#include "FileReader.cpp"
#include "Problem.cpp"
#include "RandomSolver.cpp"
#include "GreedySolver1.cpp"
#include "GreedyCycle.cpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

int main() {
    try {
        std::ifstream file("data\\InstanceA.csv");
        if (!file) {
            throw std::runtime_error("Failed to open data file");
        }
        Problem problem = readProblem(file);
        std::cout << "Problem size: " << problem.getSize() << std::endl;

        RandomSolver randomSolver;
        GreedySolver1 greedySolver1;
        GreedyCycle greedyCycle;
        const std::vector<Solver*> solvers = {
            &randomSolver,
            &greedySolver1,
            &greedyCycle
        };

        for (const auto& solver : solvers) {
            int* solution = solver->solve(problem);
            std::cout << solver->getName() << " solution: ";
            for (int i = 0; i < problem.minLength(); ++i) {
                std::cout << solution[i] << " ";
            }
            std::cout << std::endl;
            delete[] solution;
        }

        
    } catch (const std::exception& e) {
        std::cerr << "Error reading problem: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}