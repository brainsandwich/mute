#pragma once

#include "mute/dsp.h"

namespace mute
{
    // https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
    struct BiquadFilter
    {
        enum class Mode
        {
            LowPass,
            HighPass,
            BandPassQ,      // peak at Q
            BandPassPeak,   // constant 0db peak gain
            Notch,
            AllPass,

            PeakingEQ,
            LowShelf,
            HighShelf
        } mode = Mode::LowPass;

        struct {
            float in[2] = {};
            float out[2] = {};
        } samples;

        float input;
        float output;
        float frequency;
        float gain;             // PeakingEQ and Shelves
        union {
            float q;
            float bandwidth;
            float slope;
        };

        void process(float sr)
        {
            float w0 = 2.0f * Pi * frequency / mute::safe(sr);
            float cw0 = mute::cos(w0);
            float sw0 = mute::sin(w0);

            // float alphaq = sw0 / 2.0f * q;                                                      // Alpha from Q factor
            // float alphabw = sw0 * std::sinh((std::log(2.f) / 2.f) * bandwidth * w0 / sw0);      // Alpha from BW factor, BPF, Notch or Peaking EQ
            // float alphas = sw0 / 2.0f * std::sqrt((A + 1.0f / A) * (1 / slope - 1.0f) + 2.f);   // Alpha from shelf slope

            float a0, a1, a2;
            float b0, b1, b2;

            if (mode <= Mode::AllPass)
            {
                float Q = 1.f / mute::safe(q);
                float alpha = sw0 / 2.0f * Q;

                switch (mode)
                {
                    case Mode::LowPass:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = (1.f - cw0) / 2.f;
                        b1 = 1.f - cw0;
                        b2 = (1.f - cw0) / 2.f;
                        break;

                    case Mode::HighPass:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = (1.f + cw0) / 2.f;
                        b1 = -(1.f + cw0);
                        b2 = (1.f + cw0) / 2.f;
                        break;

                    case Mode::BandPassQ:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = q * alpha;
                        b1 = 0.f;
                        b2 = -q * alpha;
                        break;

                    case Mode::BandPassPeak:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = alpha;
                        b1 = 0.f;
                        b2 = -alpha;
                        break;

                    case Mode::Notch:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = 1.f;
                        b1 = -2.f * cw0;
                        b2 = 1.f;
                        break;

                    case Mode::AllPass:
                        a0 = 1.f + alpha;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha;
                        b0 = 1.f - alpha;
                        b1 = -2.f * cw0;
                        b2 = 1.f + alpha;
                        break;

                    default:
                        break;
                }
            }

            if (mode >= Mode::PeakingEQ)
            {
                float A = std::pow(10.0f, gain / 40.0f);
                float alpha = sw0 / 2.0f * std::sqrt((A + 1.0f / A) * (1 / slope - 1.0f) + 2.f);
                float as2 = 2.f * alpha * std::sqrt(A);

                switch (mode)
                {
                    case Mode::PeakingEQ:
                        a0 = 1.f + alpha / A;
                        a1 = -2.f * cw0;
                        a2 = 1.f - alpha / A;
                        b0 = 1.f + alpha * A;
                        b1 = -2.f * cw0;
                        b2 = 1.f - alpha * A;
                        break;

                    case Mode::LowShelf:
                        a0 = (A + 1.f) + (A - 1.f) * cw0 + as2;
                        a1 = -2.f * ((A - 1.f) + (A + 1.f) * cw0);
                        a2 = (A + 1.f) + (A - 1.f) * cw0 - as2;
                        b0 = A * ((A + 1.f) - (A - 1.f) * cw0 + as2);
                        b1 = 2.f * A * ((A - 1.f) - (A + 1.f) * cw0);
                        b2 = A * ((A + 1.f) - (A - 1.f) * cw0 - as2);
                        break;

                    case Mode::HighShelf:
                        a0 = (A + 1.f) - (A - 1.f) * cw0 + as2;
                        a1 = 2.f * ((A - 1.f) - (A + 1.f) * cw0);
                        a2 = (A + 1.f) + (A - 1.f) * cw0 - as2;
                        b0 = A * ((A + 1.f) + (A - 1.f) * cw0 + as2);
                        b1 = -2.f * A * ((A - 1.f) + (A + 1.f) * cw0);
                        b2 = A * ((A + 1.f) + (A - 1.f) * cw0 - as2);
                        break;

                    default:
                        break;
                }
            }

            output = (b0 * input
                + b1 * samples.in[0] + b2 * samples.in[1]
                - a1 * samples.out[0] - a2 * samples.out[1]) / a0;

            // TODO: At the transition between one stage to the other,
            // apply a non-linear transform to color the sound (i.e.
            // saturation, hysteresis, folding, etc.)
            samples.in[1] = samples.in[0];
            samples.in[0] = input;
            samples.out[1] = samples.out[0];
            samples.out[0] = output;
        }
    };
}