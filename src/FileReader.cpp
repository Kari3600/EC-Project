#include "FileReader.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::array<int, 3>> readCSV(std::istream& str) {
    std::vector<std::array<int, 3>> rows;
    std::string line;
    std::size_t lineNumber = 0;

    if (!str) {
        throw std::runtime_error("Input stream is not valid");
    }

    while (std::getline(str, line)) {
        ++lineNumber;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        std::istringstream row(line);
        std::array<int, 3> values{};
        char firstDelimiter;
        char secondDelimiter;

        if (!(row >> values[0] >> firstDelimiter >> values[1] >> secondDelimiter >> values[2]) ||
            (firstDelimiter != ',' && firstDelimiter != ';') ||
            secondDelimiter != firstDelimiter) {
            throw std::runtime_error("Invalid triplet on line " + std::to_string(lineNumber));
        }

        row >> std::ws;
        if (!row.eof()) {
            throw std::runtime_error("Unexpected content on line " + std::to_string(lineNumber));
        }

        rows.push_back(values);
    }

    return rows;
}

Problem readProblem(std::istream& str) {
    const auto triplets = readCSV(str);
    const std::size_t size = triplets.size();

    int* costs = new int[size];
    int** weights = new int*[size];
    for (std::size_t index = 0; index < size; ++index) {
        weights[index] = new int[size];
        costs[index] = triplets[index][2];
    }

    for (std::size_t from = 0; from < size; ++from) {
        for (std::size_t to = 0; to < size; ++to) {
            const double deltaX = static_cast<double>(triplets[from][0]) - triplets[to][0];
            const double deltaY = static_cast<double>(triplets[from][1]) - triplets[to][1];
            weights[from][to] = static_cast<int>(std::lround(std::hypot(deltaX, deltaY)));
        }
    }

    return Problem(size, weights, costs);
}