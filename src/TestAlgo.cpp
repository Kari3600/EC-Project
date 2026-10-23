// Run a single algorithm on a single instance from the terminal.
//
// Usage: test_algo <algorithm> <instance.csv> [startNode] [--alpha A] [--seed S] [--init H]
// Example: ./out/test_algo GreedyCycle data/InstanceA.csv 5
//          ./out/test_algo NearestNeighborII_2regret data/InstanceA.csv 5 --alpha 0.3
//          ./out/test_algo RandomSolver data/InstanceA.csv --seed 7
//          ./out/test_algo LS_greedy_edges_random data/InstanceA.csv --seed 7
//          ./out/test_algo LS_steepest_edges_greedy data/InstanceA.csv 5 --init GreedyCycle_2regret

#include "FileReader.cpp"
#include "Problem.cpp"
#include "./lab1_algos/RandomSolver.cpp"
#include "./lab1_algos/NearestNeighborI.cpp"
#include "./lab1_algos/NearestNeighborII.cpp"
#include "./lab1_algos/GreedyCycle.cpp"
#include "./lab2_algos/NearestNeighborII_2regret.cpp"
#include "./lab2_algos/GreedyCycle_2regret.cpp"
#include "./lab3_algos/LocalSearch.cpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Algorithm that takes a weight alpha (--alpha); created after the arguments are read
struct WeightedAlgorithm {
    std::string name;
    std::function<std::shared_ptr<Solver>(double alpha)> create;
};

static std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return text;
}

static double parseAlpha(const std::string& text) {
    try {
        std::size_t used = 0;
        const double value = std::stod(text, &used);
        if (used == text.size()) {
            return value;
        }
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("--alpha must be a number, got: " + text);
}

int main(int argc, char* argv[]) {
    RandomSolver randomSolver;
    NearestNeighborI nearestNeighborI;
    NearestNeighborII nearestNeighborII;
    GreedyCycle greedyCycle;
    // All 8 local search variants, e.g. LS_greedy_edges_random
    std::vector<LocalSearch> localSearches;
    for (const SearchType search : {SearchType::Greedy, SearchType::Steepest}) {
        for (const IntraMove intra : {IntraMove::Nodes, IntraMove::Edges}) {
            for (const StartType start : {StartType::Random, StartType::Greedy}) {
                localSearches.emplace_back(search, intra, start);
            }
        }
    }
    // To add an algorithm: include its .cpp above and add it to one of the lists
    std::vector<Solver*> solvers = {
        &randomSolver,
        &nearestNeighborI,
        &nearestNeighborII,
        &greedyCycle
    };
    for (LocalSearch& localSearch : localSearches) {
        solvers.push_back(&localSearch);
    }
    const std::vector<WeightedAlgorithm> weightedAlgorithms = {
        {"NearestNeighborII_2regret",
         [](double alpha) { return std::make_shared<NearestNeighborII_2regret>(alpha); }},
        {"GreedyCycle_2regret",
         [](double alpha) { return std::make_shared<GreedyCycle_2regret>(alpha); }}
    };
    const double defaultAlpha = 0.5;

    try {
        // Split arguments into positional ones and the --alpha / --seed / --init options
        std::vector<std::string> positional;
        double alpha = defaultAlpha;
        bool alphaGiven = false;
        std::string initName;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--alpha") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--alpha needs a value");
                }
                alpha = parseAlpha(argv[++i]);
                alphaGiven = true;
            } else if (arg == "--seed") {
                // Seed for rand() (RandomSolver, local search); without it every run gives the same random solution
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--seed needs a value");
                }
                std::srand(static_cast<unsigned>(std::stoul(argv[++i])));
            } else if (arg == "--init") {
                // Construction heuristic for local search with a greedy start (weighted ones use the default alpha)
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--init needs an algorithm name");
                }
                initName = argv[++i];
            } else {
                positional.push_back(arg);
            }
        }

        if (positional.size() < 2 || positional.size() > 3) {
            std::cerr << "Usage: " << argv[0]
                      << " <algorithm> <instance.csv> [startNode] [--alpha A] [--seed S] [--init H]\n"
                      << "Algorithms:";
            for (const auto& solver : solvers) {
                std::cerr << " " << solver->getName();
            }
            for (const auto& weighted : weightedAlgorithms) {
                std::cerr << " " << weighted.name << " (--alpha, default " << defaultAlpha << ")";
            }
            std::cerr << std::endl;
            return 1;
        }
        const std::string& algorithmName = positional[0];
        const std::string& instancePath = positional[1];

        // Find an algorithm by name; a weighted one is created with the given alpha and kept in owner
        // (shared_ptr destroys the real type, Solver has no virtual destructor)
        auto findSolver = [&](const std::string& name, double alphaValue,
                              std::shared_ptr<Solver>& owner) -> Solver* {
            for (const auto& candidate : solvers) {
                if (toLower(candidate->getName()) == toLower(name)) {
                    return candidate;
                }
            }
            for (const auto& weighted : weightedAlgorithms) {
                if (toLower(weighted.name) == toLower(name)) {
                    owner = weighted.create(alphaValue);
                    return owner.get();
                }
            }
            throw std::invalid_argument("Unknown algorithm: " + name);
        };

        std::shared_ptr<Solver> weightedSolver;
        Solver* solver = findSolver(algorithmName, alpha, weightedSolver);
        if (alphaGiven && weightedSolver == nullptr) {
            throw std::invalid_argument(algorithmName + " does not take --alpha");
        }

        // Local search with a greedy start builds its starting solution with the --init heuristic
        LocalSearch* localSearch = dynamic_cast<LocalSearch*>(solver);
        std::shared_ptr<Solver> initialOwner;
        Solver* initialSolver = nullptr;
        if (localSearch != nullptr && localSearch->needsInitialSolver()) {
            if (initName.empty()) {
                throw std::invalid_argument(algorithmName + " needs --init <construction heuristic>");
            }
            initialSolver = findSolver(initName, defaultAlpha, initialOwner);
            if (dynamic_cast<LocalSearch*>(initialSolver) != nullptr) {
                throw std::invalid_argument("--init must be a construction heuristic, not " + initName);
            }
            localSearch->setInitialSolver(initialSolver);
        } else if (!initName.empty()) {
            throw std::invalid_argument(algorithmName + " does not take --init");
        }

        std::ifstream file(instancePath);
        if (!file) {
            throw std::runtime_error("Failed to open data file " + instancePath);
        }
        Problem problem = readProblem(file);

        const int startNode = positional.size() == 3 ? std::stoi(positional[2]) : 0;
        if (startNode < 0 || startNode >= problem.getSize()) {
            throw std::invalid_argument("startNode must be between 0 and " +
                                        std::to_string(problem.getSize() - 1));
        }

        const auto startTime = std::chrono::steady_clock::now();
        const std::vector<int> solution = solver->solve(problem, startNode);
        const auto endTime = std::chrono::steady_clock::now();

        std::cout << "Algorithm: " << solver->getName() << std::endl;
        std::cout << "Instance:  " << instancePath << " (size " << problem.getSize() << ")" << std::endl;
        std::cout << "Start:     " << startNode << std::endl;
        std::cout << "Objective: " << problem.evaluate(solution) << std::endl;
        std::cout << "Time:      "
                  << std::chrono::duration<double, std::milli>(endTime - startTime).count()
                  << " ms" << std::endl;
        if (localSearch != nullptr) {
            // Starting solution of the local search (built inside solve, so included in Time)
            if (initialSolver != nullptr) {
                std::cout << "Init:      " << initialSolver->getName() << std::endl;
            }
            std::cout << "Initial:   " << problem.evaluate(localSearch->getInitialSolution()) << std::endl;
            std::cout << "Steps:     " << localSearch->getSteps() << std::endl;
        }
        std::cout << "Solution: ";
        for (const int node : solution) {
            std::cout << " " << node;
        }
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
