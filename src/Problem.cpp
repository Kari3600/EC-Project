#include "Problem.h"

#include <cstddef>
#include <stdexcept>
#include <string>

Problem::Problem(std::size_t size, int** weights, int* costs)
        : size(size),
          weights(weights),
          costs(costs) {}

const int Problem::getSize() const {
    return static_cast<int>(size);
}

const int Problem::minLength() const {
    return (static_cast<int>(size)+1) / 2;
}

const int* const* Problem::getWeights() const {
    return weights;
}

const int* Problem::getCosts() const {
    return costs;
}

int Problem::getDist(int from, int to) const {
    return weights[from][to];
}

int Problem::evaluate(const std::vector<int>& solution) const {
    const int length = static_cast<int>(solution.size());
    if (length != minLength()) {
        throw std::invalid_argument("Wrong solution length: " + std::to_string(length) +
                                    ", expected " + std::to_string(minLength()));
    }

    int total = 0;
    for (int i = 0; i < length; ++i) {
        const int from = solution[i];
        const int to = solution[(i + 1) % length];
        total += weights[from][to] + costs[from];
    }
    return total;
}