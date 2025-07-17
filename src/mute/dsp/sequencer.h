#pragma once

#include "mute/dsp/core.h"

namespace mute::dsp
{
    struct EuclideanSequencer
    {
        int length;
        int increment;
        float gatelen = 0.1;
        
        EdgeDetector gate;
        bool output = false;

        float t = 0;
        int index = -1;

        void reset()
        {
            index = -1;
            t = 0;
            output = false;
        }
        
        void next()
        {
            index = index < 0 ? 0 : (index + increment);
            if (index > length)
            {
                index -= length;
                output = true;
                t = 0;
            } else
                output = false;
        }

        void process(float sr)
        {
            gate.process(sr);
            if (gate.rising)
                next();

            t += 1./sr;
            if (t > gatelen)
                output = false;
        }
    };
}