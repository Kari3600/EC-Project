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
#include <map>

static void printAny(std::any any) {
    if (int* val = std::any_cast<int>(&any)) {
        std::cout << *val;
    } else {
        throw "Unsupported printing type: "+std::string(any.type().name());
    }
}

static std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return text;
}

static float asFloat(std::string text) {
    try {
        return std::stof(text);
    } catch (std::invalid_argument) {
        throw std::invalid_argument("'"+text+"' cannot be parsed as float.");
    }
}

static int asInt(std::string text) {
    try {
        return std::stoi(text);
    } catch (std::invalid_argument) {
        throw std::invalid_argument("'"+text+"' cannot be parsed as int.");
    }
}

static std::string stripOfQuotes(std::string text) {
    int size = text.size();
    if (text.at(0) == text.at(size-1)) {
        if (text.at(0) == '"' || text.at(0) == '\'') {
            return text.substr(1,size-2);
        }
    }
    return text;
}

static std::string stripLeadingDashes(std::string str) {
    str.erase(0, str.find_first_not_of('-'));
    return str;
}

struct ProgramArgs {
    std::map<std::string, std::string> args;
    std::vector<std::string> unnamedArgs;

    std::string get(int pos) {
        if (pos >= unnamedArgs.size()) {
            throw std::invalid_argument("expected at least " + std::to_string(pos+1) + " arguments, found " + std::to_string(unnamedArgs.size()));
        }
        return unnamedArgs[pos];
    }

    std::string get(std::string name) {
        auto it = args.find(name);
        if (it == args.end()) {
            throw std::invalid_argument("argument '" + name + "' was expected but not found");
        }
        return it->second;
    }

    std::string getOrDefault(int pos, std::string defaultValue) {
        if (pos >= unnamedArgs.size()) {
            return defaultValue;
        }
        return unnamedArgs[pos];
    }

    std::string getOrDefault(std::string name, std::string defaultValue) {
        auto it = args.find(name);
        if (it == args.end()) {
            return defaultValue;
        }
        return it->second;
    }

    ProgramArgs(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            char* arg = argv[i];
            if (arg[0] == '-') {
                std::string argName = stripLeadingDashes(std::string(arg));
                std::string argValue;
                if (++i == argc) {
                    argValue = "true";
                } else {
                    char* arg2 = argv[i];
                    if (arg2[0] == '-') {
                        argValue = "true";
                        i--;
                    } else {
                        argValue = stripOfQuotes(std::string(arg2));
                    }
                }
                args[argName] = argValue;
            } else {
                unnamedArgs.push_back(stripOfQuotes(std::string(arg)));
            }
        }
    }
};

Solver* createSolver(std::string solverName, ProgramArgs& args) {
    if (solverName == "RandomSolver") {
        return new RandomSolver();
    } else if (solverName == "GreedyCycle") {
        return new GreedyCycle();
    } else if (solverName == "NearestNeighborI") {
        return new NearestNeighborI();
    } else if (solverName == "NearestNeighborII") {
        return new NearestNeighborII();
    } else if (solverName == "GreedyCycle_2regret") {
        float alpha = asFloat(args.getOrDefault("alpha", "0.5"));
        return new GreedyCycle_2regret(alpha);
    } else if (solverName == "NearestNeighborII_2regret") {
        float alpha = asFloat(args.getOrDefault("alpha", "0.5"));
        return new NearestNeighborII_2regret(alpha);
    } else if (solverName == "LocalSearch") {
        SearchType searchType = (args.get("searchType") == "Steepest") ? SearchType::Steepest : SearchType::Greedy;
        IntraMove intraMove = (args.get("intraMove") == "Nodes") ? IntraMove::Nodes : IntraMove::Edges;
        Solver* initialSolver = createSolver(args.get("initialSolver"), args); 
        return new LocalSearch(searchType, intraMove, initialSolver);
    } else {
        throw std::invalid_argument("Solver '" + solverName + "' does not exist.");
    }
}

int main(int argc, char* argv[]) {
    ProgramArgs args(argc, argv);
    Solver* solver = createSolver(args.get(0), args);
    
    int seed = asInt(args.getOrDefault("seed", "0"));
    std::string instancePath = args.get(1);
    std::ifstream file(instancePath);
    if (!file) {
        throw std::runtime_error("Failed to open data file " + instancePath);
    }
    Problem problem = readProblem(file);

    const auto startTime = std::chrono::steady_clock::now();
    const std::vector<int> solution = solver->solve(problem, seed);
    const auto endTime = std::chrono::steady_clock::now();

    std::cout << "Solution: ";
    for (auto val : solution) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "Objective: " << problem.evaluate(solution) << std::endl;
    std::cout << "Time:      "
                << std::chrono::duration<double, std::milli>(endTime - startTime).count()
                << " ms" << std::endl;
    for (auto [statName, statValue] : solver->getLastRunStatistics()) {
        std::cout << statName << ": ";
        printAny(statValue);
        std::cout << std::endl;
    }

    return 0;
}