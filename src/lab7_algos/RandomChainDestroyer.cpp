#include "RandomChainDestroyer.h"

void RandomChainDestroyer::destroy(const Problem& problem, std::vector<int>& solution, std::mt19937& rng, int amount) const {
    for (int i = 0; i < chainCount; i++) {
        int chainLength = amount * (i+1) / chainCount - amount * i / chainCount;
        int len = solution.size();
        std::uniform_int_distribution<int> dist(0, len - 1);

        int start = dist(rng);

        if (start + chainLength <= len) {
            solution.erase(solution.begin() + start, solution.begin() + start + chainLength);
        } else {
            const int end = (start + chainLength) % len;

            solution.erase(solution.begin() + start, solution.end());
            solution.erase(solution.begin(), solution.begin() + end);
        }
    }
}