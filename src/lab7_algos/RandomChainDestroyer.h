#ifndef RANDOMCHAINDESTROYER_H
#define RANDOMCHAINDESTROYER_H

#include "Destroyer.h"

class RandomChainDestroyer : public Destroyer {
    public:
        RandomChainDestroyer(int chainCount): chainCount(chainCount) {};
        void destroy(const Problem& problem, std::vector<int>& solution, std::mt19937& rng, int amount) const override;
    private:
        int chainCount;
};

#endif