#include "Problem.h"

#include <cstddef>

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

const int* Problem::getWeight(int index) const {
    return weights[index];
}