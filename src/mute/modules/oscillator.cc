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

        WaveTables()
        {
            for (int i = 0; i < int(Size); i++)
                SineWave[i] = std::sin(i * Tau / double(Size));

            std::random_device rng;
            std::uniform_real_distribution distribution(-1, 1);
            for (auto& v: WhiteNoise)
                v = distribution(rng);
        }
    } WT;

    WavetableOscillator WavetableOscillator::sine() { return WavetableOscillator { .data = WT.SineWave }; }
    WavetableOscillator WavetableOscillator::noise() { return WavetableOscillator { .data = WT.WhiteNoise }; }
}