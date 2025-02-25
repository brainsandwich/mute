#pragma once

#include "mute/dsp.h"

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
}