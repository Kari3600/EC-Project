#ifndef PROBLEM_H
#define PROBLEM_H

#include <cstdlib>
#include <vector>

class Problem {
    public:
        Problem(std::size_t size, int** weights, int* costs);
        const int getSize() const;
        const int minLength() const;
        const int* const* getWeights() const;
        const int* getCosts() const;
        const int* getWeight(int index) const;
        int evaluate(const std::vector<int>& solution) const;

    private:
        const std::size_t size;
        int** const weights;
        int* const costs;
};

#endif