#pragma once

#include <cstddef>

class Problem {
public:
    Problem(std::size_t size, int** weights, int* costs)
        : size(size),
          weights(weights),
          costs(costs) {}

    int getSize() const {
        return static_cast<int>(size);
    }

    const int* const* getWeights() const {
        return weights;
    }

    const int* getCosts() const {
        return costs;
    }

    int getWeight(int index) const;

private:
    const std::size_t size;
    int** const weights;
    int* const costs;
};