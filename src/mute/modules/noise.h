#pragma once

#include "mute/modules/oscillator.h"

#include <array>

namespace mute
{
    template <typename T>
    constexpr T lcg(const T& x, const T& a, const T& c, const T& m)
    {
        return (a * x + c) % m;
    }

    constexpr auto mmixLCG(unsigned long long in)
    {
        return lcg(in, 6364136223846793005ull, 1442695040888963407ull, 0ull - 1);
    }
}

namespace mute
{
    struct WhiteNoise
    {
        float output = 0.f;
        unsigned long long seed = 0xdaC0FFEEdeadbeefull;
        
        void process(float)
        {
            output = (double(seed) / double(0ull - 1)) * 2.f - 1.f;
            seed = mmixLCG(seed);
        }
    };

    // struct PrimeCluster
    // {
    //     WhiteNoise noise;
    //     std::array<SineOscillator, 16> oscillators;
    //     std::array<int, 16> primes = {
    //         53,
    //         127,
    //         199,
    //         283,
    //         383,
    //         467,
    //         577,
    //         661,
    //         769,
    //         877,
    //         983,
    //         1087,
    //         1193,
    //         1297,
    //         1429,
    //         1523
    //     };
    //     float noiseAmount = 0.0f;
    //     float freqMult = 1.0f;
    //     float output = 0.0f;

    //     void process(float sr)
    //     {
    //         noise.process();
    //         output = 0.0f;
    //         int index = 0;
    //         for (auto& osc: oscillators)
    //         {
    //             osc.frequency = primes[index] * freqMult;
    //             osc.mod = noiseAmount * noise.output;
    //             osc.process(sr);
    //             output += osc.output;
    //             index++;
    //         }
    //         output /= float(oscillators.size());
    //     }
    // };

    // struct FibonCluster
    // {
    //     WhiteNoise noise;
    //     std::array<SineOscillator, 16> oscillators;
    //     std::array<int, 16> primes = {
    //         53,
    //         127,
    //         199,
    //         283,
    //         383,
    //         467,
    //         577,
    //         661,
    //         769,
    //         877,
    //         983,
    //         1087,
    //         1193,
    //         1297,
    //         1429,
    //         1523
    //     };
    //     float noiseAmount = 0.0f;
    //     float freqMult = 1.0f;
    //     float output = 0.0f;

    //     void process(float sr)
    //     {
    //         noise.process();
    //         output = 0.0f;
    //         int index = 0;
    //         for (auto& osc: oscillators)
    //         {
    //             osc.frequency = primes[index] * freqMult;
    //             osc.mod = noiseAmount * noise.output;
    //             osc.process(sr);
    //             output += osc.output;
    //             index++;
    //         }
    //         output /= float(oscillators.size());
    //     }
    // };
}