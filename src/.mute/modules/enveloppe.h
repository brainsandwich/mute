#pragma once

#include "mute/dsp.h"
#include "mute/modules/edgedetector.h"

namespace mute
{
    struct AHDSR
    {
        // parameters
        float a, h, d, s, r;
        bool gated = true;
        bool looped = false;
        
        // input
        float gate = 0;
        
        // output
        float output = 0;

        // state
        float t = 0;

        void process(float sr)
        {
            bool gateon = gate > 0.5;
            float dt = 1 / safe(sr);

            if (gateon)
            {
                float origin = 0, target = 0, t0 = 0;
                if (t < a)
                {
                    origin = 0;
                    target = 1;
                    t0 = t / safe(a);
                }
                else if (t < a + h)
                {
                    origin = 1;
                    target = 1;
                }
                else
                {
                    origin = 1;
                    target = s;
                    t0 = (t - a - h) / safe(d); // safe(1 - s);
                }
                
                t = clamp(t + dt, 0.0f, float(a + h + d + Epsilon));
                output = lerp(origin, target, t0);
                if (looped && t > a + h + d)
                    t = 0.0f;
            } else 
            {
                float slope = dt / r;
                output = clamp(output - slope, 0.0f, 1.0f);
                t = output * a;
            }
        }
    };

    struct Waveloppe
    {
        std::vector<float> points;
        float t = 1000;
        float time = 1;
        float skew = 1;
        float gate;
        float output;
        mute::EdgeDetector edgeDetector;

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
                output = lerp(p1, p2, frac);
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