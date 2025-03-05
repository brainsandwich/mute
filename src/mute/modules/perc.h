#pragma once

#include "mute/modules/edgedetector.h"
#include "mute/modules/oscillator.h"
#include "mute/modules/biquad.h"
#include "mute/modules/enveloppe.h"
#include "mute/modules/noise.h"

namespace mute
{
    struct Perc
    {
        float volume = 1;

        float pitchmod = 0.004;
        float noiseamt = 0.1;
        float filterenv = 0;

        float oscfreq = 40;
        float noisefreq = 100;
        float filterfreq = 30;

        mute::EdgeDetector triggerDetector;
        mute::WavetableOscillator osc = mute::WavetableOscillator::sine();
        // mute::WavetableOscillator noise = mute::WavetableOscillator::noise();
        mute::WhiteNoise noise;
        mute::BiquadFilter filter { .mode = BiquadFilter::Mode::LowPass, .q = 1 };
        mute::Waveloppe env1 = mute::Waveloppe::fall(0.4, 0.4);
        mute::Waveloppe env2 = mute::Waveloppe::fall(0.7, 0.7);

        float gate;
        float output;

        void trigger()
        {
            osc.phase = 0;
            // noise.phase = 0;
            env1.trigger();
            env2.trigger();
        }

        void process(float sr)
        {
            triggerDetector.input = gate;
            triggerDetector.process(sr);
            if (triggerDetector.rising)
                trigger();

            env1.process(sr);
            env2.process(sr);

            float fm = 0.5 * sr * pitchmod * env1.output;
            osc.frequency = oscfreq + fm;
            osc.process(sr);

            // noise.frequency = noisefreq;
            noise.process(sr);

            float mix1 = osc.output + noise.output * noiseamt;
            filter.frequency = filterfreq + 0.4 * sr * filterenv * env1.output;
            filter.input = mix1;
            filter.process(sr);

            output = volume * filter.output * env2.output;
        }
    };
}