#pragma once

#include "mute/utils.h"

namespace mute
{
    namespace wave
    {
        float sine(float ph) { return mute::cos(ph); }
        float square(float ph) { return rephase(ph) < M_PI ? 1.0f : -1.0f; }
        float saw(float ph) { return -rephase(ph) / M_PI - 1.0f; }
        float tri(float ph) {
            float ph2 = rephase(ph);
            return ph2 < M_PI ? (2*saw(ph2) - 1) : (-2*saw(ph2) - 1);
        }
    }

    struct SineOscillator
    {
        float output = 0.0f;
        float phase = 0.0f;
        float frequency = 0.0f;
        float mod = 0.0f;

        void process(float sr)
        {
            phase = rephase(phase + mod * 2.f * M_PI, 2.0f * M_PI * frequency / sr);
            output = wave::sine(phase);
        }
    };

    struct LFO
    {
        enum class Mode
        {
            Sine,
            Square
        } mode;

        float output = 0.0f;
        float phase = 0.0f;
        float frequency = 0.0f;

        void process(float sr)
        {
            phase = rephase(phase, 2.0f * M_PI * frequency / sr);
            switch (mode)
            {
                case Mode::Sine:
                    output = wave::sine(phase) * 0.5f + 0.5f;
                    break;
                case Mode::Square:
                    output = wave::square(phase) * 0.5f + 0.5f;
                    break;
            }
        }
    };

    struct Oscillator
    {
        float output = 0.0f;
        float phase = 0.0f;
        float frequency;
        float wave = 0.0f;

        void process(float sr)
        {
            phase = rephase(phase, 2.0f * M_PI * frequency / sr);
            auto [a, b, c, d] = crossfade<4>(wave);
            output
                = a * wave::sine(phase)
                + b * wave::tri(phase)
                + c * wave::saw(phase)
                + d * wave::square(phase);
        }
    };

    struct SwarmOscillator
    {
        std::array<SineOscillator, 512> oscillators;
        float frequency = 0.0f;
        float output = 0.0f;
        float spread = 1.0f;

        void process(float sr)
        {
            int index = 0;
            output = 0.0f;
            for (auto& osc: oscillators)
            {
                osc.frequency = frequency * (1 + 2*index * spread);
                osc.process(sr);
                output += osc.output / float(oscillators.size());
                index++;
            }
        }
    };
}