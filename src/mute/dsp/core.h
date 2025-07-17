#pragma once

#include "mute/math.h"
#include "mute/util.h"

namespace mute::dsp
{
    struct EdgeDetector
    {
        float threshold = 0.5;
        float input;

        bool rising = false;
        bool falling = false;
        bool up = false;

        void process(float sr)
        {
            bool before = up;
            up = input > threshold;
            rising = up && before != up;
            falling = !up && before != up;
        }
    };

    struct Phazor
    {
        float frequency = 440.f; // 20, 192000
        float output = 0.f;

        void process(float sr)
        {
            output = mute::rephase(output + frequency * mute::Tau / mute::safe(sr));
        }
    };
}