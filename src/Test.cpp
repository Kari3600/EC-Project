#include "FileReader.cpp"
#include "Problem.cpp"
#include "RandomSolver.cpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        std::ifstream file("data\\InstanceA.csv");
        if (!file) {
            throw std::runtime_error("Failed to open data file");
        }
        Problem problem = readProblem(file);
        std::cout << "Problem size: " << problem.getSize() << std::endl;

        RandomSolver solver;
        int* solution = solver.solve(problem);

        std::cout << "Random solution: ";
        for (int i = 0; i < problem.getSize(); ++i) {
            std::cout << solution[i] << " ";
        }
        std::cout << std::endl;
        delete[] solution;
    } catch (const std::exception& e) {
        std::cerr << "Error reading problem: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}