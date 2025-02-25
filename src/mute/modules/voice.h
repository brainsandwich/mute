#pragma once

#include "mute/modules/oscillator.h"
#include "mute/modules/biquad.h"
#include "mute/modules/adsr.h"

namespace mute
{
    struct Voice
    {
        float gate;
        float pitch;    // Hz
        float wave;
        float cutoff;
        float resonance;
        float volume = 1.0f;

        Oscillator osc;
        BiquadFilter filter;
        ADSR env1 { .a = 0.f, .d = 0.02f, .s = 1.0f, .r = 0.02f };
        ADSR env2 { .a = 0.f, .d = 0.02f, .s = 1.0f, .r = 0.02f };

        struct EnvMod
        {
            float amp;
            struct {
                float freq;
                float wave;
            } osc;
            struct {
                float freq;
                float q;
            } filter;
        } em1, em2;

        float output;

        void process(float sr)
        {
            env1.gate = env2.gate = gate;
            env1.process(sr);
            env2.process(sr);

            osc.frequency = pitch
                + em1.osc.freq * env1.output
                + em1.osc.freq * env2.output;
            osc.wave = wave
                + em1.osc.wave * env1.output
                + em2.osc.wave * env2.output;
            osc.process(sr);

            filter.input = osc.output;
            filter.frequency = cutoff
                + em1.filter.freq * env1.output
                + em2.filter.freq * env2.output;
            filter.q = resonance
                + em1.filter.q * env1.output
                + em2.filter.q * env2.output;
            filter.process(sr);

            output = filter.output;

            float amp = volume
                * (em1.amp * env1.output
                + em2.amp * env2.output);
            output = amp * output;
        }
    };
}