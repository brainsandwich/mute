#pragma once

#include <span>

namespace mute
{
    struct SineOscillator
    {
        float phase = 0;
        float frequency;
        float output;

        void process(float sr);
    };

    struct WavetableOscillator
    {
        std::span<float> data;
        float skew = 1;
        float frequency = 0;
        float phase = 0;
        float output;

        void process(float sr);

        static WavetableOscillator sine();
        static WavetableOscillator noise();
    };
}