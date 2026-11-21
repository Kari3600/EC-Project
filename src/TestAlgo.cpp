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
#include "./lab4_algos/LocalSearchWithCandidates.cpp"
#include "./lab5_algos/ListOfMovesSolver.cpp"
#include "./lab6_algos/MSLS.cpp"
#include "./lab6_algos/ILS.cpp"
#include "./lab7_algos/LNS.cpp"
#include "./lab7_algos/RandomDestroyer.cpp"
#include "./lab7_algos/RandomChainDestroyer.cpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <experimental/iterator>

static void printAny(std::any any) {
    if (int* val = std::any_cast<int>(&any)) {
        std::cout << *val;
    } else if (auto* values = std::any_cast<std::vector<int>>(&any)) {
        std::cout << "[";
        std::copy(values->begin(), values->end(), std::experimental::make_ostream_joiner(std::cout, ", "));
        std::cout << "]";
    } else {
        throw "Unsupported printing type: "+std::string(any.type().name());
    }
}

static std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return text;
}

static float asFloat(const std::string& text) {
    try {
        return std::stof(text);
    } catch (std::invalid_argument) {
        throw std::invalid_argument("'"+text+"' cannot be parsed as float.");
    }
}

static int asInt(const std::string& text) {
    try {
        return std::stoi(text);
    } catch (std::invalid_argument) {
        throw std::invalid_argument("'"+text+"' cannot be parsed as int.");
    }
}

std::vector<int> asIntVector(const std::string& str) {
    std::vector<int> result;
    size_t start = 0;

    while (start < str.size()) {
        size_t end = str.find(',', start);
        if (end == std::string::npos)
            end = str.size();

        int value = 0;

        for (size_t i = start; i < end; ++i) {
            unsigned char c = static_cast<unsigned char>(str[i]);

            if (!std::isdigit(c))
                throw std::invalid_argument("Invalid character");

            if (value > (INT_MAX - (c - '0')) / 10)
                throw std::out_of_range("Integer overflow");

            value = value * 10 + (c - '0');
        }

        result.push_back(value);
        start = end + 1;
    }

    if (!str.empty() && str.back() == ',')
        throw std::invalid_argument("Trailing comma");

    return result;
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

    bool contains(std::string name) {
        auto it = args.find(name);
        return (it != args.end());
    }

    const std::string get(int pos) {
        if (pos >= unnamedArgs.size()) {
            throw std::invalid_argument("expected at least " + std::to_string(pos+1) + " arguments, found " + std::to_string(unnamedArgs.size()));
        }
        return unnamedArgs[pos];
    }

    const std::string get(std::string name) {
        auto it = args.find(name);
        if (it == args.end()) {
            throw std::invalid_argument("argument '" + name + "' was expected but not found");
        }
        return it->second;
    }

    const std::string getOrDefault(int pos, std::string defaultValue) {
        if (pos >= unnamedArgs.size()) {
            return defaultValue;
        }
        return unnamedArgs[pos];
    }

    const std::string getOrDefault(std::string name, std::string defaultValue) {
        auto it = args.find(name);
        if (it == args.end()) {
            return defaultValue;
        }
        return it->second;
    }

    void buildArgs(std::vector<std::string>& vector) {
        for (auto i = vector.begin(); i != vector.end(); i++) {
            std::string arg = *i;
            if (arg[0] == '-') {
                std::string argName = stripLeadingDashes(arg);
                std::string argValue;
                if (++i == vector.end()) {
                    argValue = "true";
                } else {
                    std::string arg2 = *i;
                    if (arg2[0] == '-') {
                        argValue = "true";
                        i--;
                    } else {
                        argValue = stripOfQuotes(arg2);
                    }
                }
                args[argName] = argValue;
            } else {
                unnamedArgs.push_back(stripOfQuotes(arg));
            }
        }
    }



    ProgramArgs(const std::string& str) {
        std::vector<std::string> result;
        std::istringstream iss(str);
        std::string token;

        while (iss >> token)
            result.push_back(token);
        
        buildArgs(result);
    }

    ProgramArgs(int argc, char* argv[]) {
        std::vector<std::string> result;
        result.reserve(argc);

        for (int i = 1; i < argc; ++i)
            result.emplace_back(argv[i]);
        
        buildArgs(result);
    }

    ProgramArgs(std::vector<std::string>& argvec) {
        buildArgs(argvec);
    }
};

Destroyer* createDestroyer(ProgramArgs& args) {
    std::string destroyerName = args.get(0);
    if (destroyerName == "random") {
        return new RandomDestroyer();
    } else if (destroyerName == "randomChain") {
        int chainCount = asInt(args.get("chainCount"));
        return new RandomChainDestroyer(chainCount);
    } else {
        throw std::invalid_argument("Destroyer '" + destroyerName + "' does not exist.");
    }
}

Solver* createSolver(ProgramArgs& args) {
    std::string solverName = args.get(0);
    if (solverName == "RandomSolver") {
        return new RandomSolver();
    } else if (solverName == "NearestNeighborI") {
        return new NearestNeighborI();
    } else if (solverName == "NearestNeighborII") {
        return new NearestNeighborII();
    } else if (solverName == "GreedyCycle") {
        return new GreedyCycle();
    } else if (solverName == "GreedyCycle_2regret") {
        float alpha = asFloat(args.getOrDefault("alpha", "0.5"));
        return new GreedyCycle_2regret(alpha);
    } else if (solverName == "NearestNeighborII_2regret") {
        float alpha = asFloat(args.getOrDefault("alpha", "0.5"));
        return new NearestNeighborII_2regret(alpha);
    } else if (solverName == "LocalSearch") {
        SearchType searchType = (args.get("searchType") == "Steepest") ? SearchType::Steepest : SearchType::Greedy;
        IntraMove intraMove = (args.get("intraMove") == "Nodes") ? IntraMove::Nodes : IntraMove::Edges;
        return new LocalSearch(searchType, intraMove);
    } else if (solverName == "LocalSearchWithCandidates") {
        int candidatesCount = asInt(args.getOrDefault("candidatesCount", "10"));
        return new LocalSearchWithCandidates(candidatesCount);
    } else if (solverName == "ListOfMovesSolver") {
        return new ListOfMovesSolver();
    } else if (solverName == "MSLS") {
        int iterations = asInt(args.getOrDefault("iterations", "400"));
        ProgramArgs localSearchArgs(args.get("localSearch"));
        Solver* localSearch = createSolver(localSearchArgs);
        return new MSLS(localSearch, iterations);
    } else if (solverName == "ILS") {
        ProgramArgs localSearchArgs(args.get("localSearch"));
        Solver* localSearch = createSolver(localSearchArgs);
        Perturbation perturbation = ILS::parsePerturbation(args.get("perturbation"));
        int strength = asInt(args.get("strength"));
        double timeMs = asFloat(args.get("timeMs"));
        return new ILS(localSearch, perturbation, strength, timeMs);
    } else if (solverName == "LNS") {
        ProgramArgs destroyerArgs(args.get("destroyer"));
        Destroyer* destroyer = createDestroyer(destroyerArgs);
        ProgramArgs repairerArgs(args.get("repairer"));
        Solver* repairer = createSolver(repairerArgs);
        Solver* postprocessor = nullptr;
        if (args.contains("postprocessor")) {
            ProgramArgs postprocessorArgs = ProgramArgs(args.get("postprocessor"));
            postprocessor = createSolver(postprocessorArgs);
        }
        double discardPercent = asFloat(args.get("discardPercent"));
        double timeMs = asFloat(args.get("timeMs"));
        return new LNS(destroyer, repairer, postprocessor, discardPercent, timeMs);
    } else {
        throw std::invalid_argument("Solver '" + solverName + "' does not exist.");
    }
}

int main(int argc, char* argv[]) {
    ProgramArgs args(argc, argv);
    Solver* solver = createSolver(args);
    
    int seed = asInt(args.getOrDefault("seed", "0"));
    std::mt19937 rng(seed);
    std::vector<int> solution = asIntVector(args.getOrDefault(2, ""));

    std::string instancePath = args.get(1);
    std::ifstream file(instancePath);
    if (!file) {
        throw std::runtime_error("Failed to open data file " + instancePath);
    }
    Problem problem = readProblem(file);

    const auto startTime = std::chrono::steady_clock::now();
    solver->solve(problem, solution, rng);
    const auto endTime = std::chrono::steady_clock::now();

    std::string outFormat = args.getOrDefault("outFormat", "plain");
    if (outFormat == "plain") {
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
    } else if (outFormat == "json") {
        std::cout << "{" << std::endl;
        std::cout << "    \"solution\": ";
        printAny(solution);
        std::cout << "," << std::endl;
        std::cout << "    \"objective\": " << problem.evaluate(solution) << "," << std::endl;
        for (auto [statName, statValue] : solver->getLastRunStatistics()) {
            std::cout << "    \"" << statName << "\": ";
            printAny(statValue);
            std::cout << "," << std::endl;
        }
        std::cout << "    \"time\": "<< std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << std::endl;
        std::cout << "}" << std::endl;
    } else {
        throw std::invalid_argument("outFormat "+outFormat+" does not exist.");
    }
    

    return 0;
}