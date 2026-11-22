#include "RandomDestroyer.h"

void RandomDestroyer::destroy(const Problem& problem, std::vector<int>& solution, std::mt19937& rng, int amount) const {
    for (int i = 0; i < amount; ++i) {
        std::uniform_int_distribution<int> dist(0, solution.size() - 1);

        int index = dist(rng);

        solution.erase(solution.begin() + index);
    }
}