#pragma once

namespace mute
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
}