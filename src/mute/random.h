#pragma once

#include "mute/dsp.h"

#include <random>

namespace mute
{
    struct RandomNumberGenerator
    {
        size_t chainSize;
        float randomWalkSize;
        std::mt19937 engine;
        std::vector<float> chain;
        size_t index = 0;

        void seed(uint64_t s)
        {
            index = 0;
            engine.seed(s);
            chain.clear();
            chain.reserve(chainSize);
            auto distribution = std::uniform_real_distribution<float>(0, 1);
            for (size_t i = 0; i < chainSize; i++)
                chain.push_back(distribution(engine));
        }

        void seed()
        {
            index = 0;
            auto distribution = std::uniform_real_distribution<float>(-randomWalkSize, randomWalkSize);
            for (size_t i = 0; i < chainSize; i++)
            {
                chain[i] += distribution(engine);
                if (chain[i] > 1) chain[i] -= chain[i] - 1;
                if (chain[i] < 0) chain[i] += -chain[i];
            }
        }

        float next() { return chain[index++]; }
        float next(float min, float max) { return next() * (max - min) + min; }

        static RandomNumberGenerator init(size_t chainSize, float randomWalkSize, uint64_t seed = 42234)
        {
            RandomNumberGenerator rng { .chainSize = chainSize, .randomWalkSize = randomWalkSize };
            rng.seed(seed);
            return rng;
        }
    };

    extern RandomNumberGenerator RNG;
}