#pragma once

#include <vector>

#include "mute/dsp/core.h"

namespace mute::dsp
{
    struct Waveloppe
    {
        std::vector<float> points;
        float t = 1000;
        float time = 1;
        float skew = 1;
        float gate;
        float output;
        EdgeDetector edgeDetector;

        void trigger() { t = 0; }
        void process(float sr)
        {
            edgeDetector.input = gate;
            edgeDetector.process(sr);
            if (edgeDetector.rising)
                trigger();

            float fi = mute::gain(t, skew) / time * points.size();
            if (fi < int(points.size()) - 1)
            {
                int i1 = fi;
                int i2 = fi + 1;
                float frac = fi - i1;
                float p1 = points[i1];
                float p2 = points[i2];
                output = mute::lerp(p1, p2, frac);
            } else 
                output = 0;

            t += 1/sr;
            if (t > time)
            {
                t = time + 1;
                output = 0;
            }
        }

        static Waveloppe triangle(float time, float skew) { return Waveloppe { .points = { 0, 1, 0 }, .time = time, .skew = skew }; }
        static Waveloppe fall(float time, float skew) { return Waveloppe { .points = { 1, 0 }, .time = time, .skew = skew }; }
        static Waveloppe rise(float time, float skew) { return Waveloppe { .points = { 0, 1 }, .time = time, .skew = skew }; }
    };
}