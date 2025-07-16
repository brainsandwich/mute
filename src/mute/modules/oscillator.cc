#include "mute/dsp.h"
#include "mute/modules/oscillator.h"

#include <random>

namespace mute
{
    void SineOscillator::process(float sr)
    {
        phase = rephase(phase + Tau * frequency / sr);
        output = mute::sin(phase);
    }

    void WavetableOscillator::process(float sr)
    {
        phase = mute::rephase(phase + mute::Tau * frequency / sr);
        float pn = mute::gain(phase / mute::Tau, skew);
        float pos = pn * (data.size() - 1);
        int i1 = int(pos) % data.size();
        int i2 = (i1 + 1) % data.size();
        float frac = pos - i1;

        float d1 = data[i1];
        float d2 = data[i2];
        output = lerp(d1, d2, frac);
    }

    struct WaveTables
    {
        static constexpr size_t Size = 1024;
        std::array<float, Size> SineWave;
        std::array<float, Size> WhiteNoise;
        std::array<float, Size> TriangleWave;
        std::array<float, Size> SawWave;
        std::array<float, Size> SquareWave;

        WaveTables()
        {
            float tc = -1.f;
            for (int i = 0; i < int(Size); i++)
            {
                SineWave[i] = std::sin(i * Tau / double(Size));
                SawWave[i] = float(std::abs(-i) % Size) / float(Size) * 2 - 1;
                SquareWave[i] = i < int(Size / 2) ? 1.f : -1.f;
                TriangleWave[i] = tc;
                if (i < int(Size / 2))
                    tc = tc + (2.f / float(Size / 2));
                if (i >= int(Size / 2))
                    tc = tc - (2.f / float(Size / 2));
            }

            std::random_device rng;
            std::uniform_real_distribution distribution(-1.f, 1.f);
            for (auto& v: WhiteNoise)
                v = distribution(rng);
        }
    } WT;

    WavetableOscillator WavetableOscillator::sine() { return WavetableOscillator { .data = WT.SineWave }; }
    WavetableOscillator WavetableOscillator::triangle() { return WavetableOscillator { .data = WT.TriangleWave }; }
    WavetableOscillator WavetableOscillator::square() { return WavetableOscillator { .data = WT.SquareWave }; }
    WavetableOscillator WavetableOscillator::sawtooth() { return WavetableOscillator { .data = WT.SawWave }; }
    WavetableOscillator WavetableOscillator::noise() { return WavetableOscillator { .data = WT.WhiteNoise }; }
}