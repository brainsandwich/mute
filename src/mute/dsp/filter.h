#pragma once

#include "mute/math.h"
#include "mute/util.h"

namespace mute::dsp
{
    float qFromDecay(float frequency, float decay, float sampleRate)
    {
        auto rad = mute::pow(10.f, -3.f / (decay * sampleRate));
        auto BW = mute::log(rad) / -mute::Pi / (1.f / sampleRate);
        auto Q = frequency / BW;
        return Q;
    }

    // https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
    struct Biquad
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

    // Extracted from full Biquad filter code
    struct BPF
    {
        float input;
        float output;
        float frequency;
        float q;

        struct {
            float in[2] = {};
            float out[2] = {};
        } samples;

        struct {
            float a1 = 0;
            float a2 = 0;
            float b0 = 0;
        } coeffs;

        void computeCoefficients(float sr)
        {
            float w0 = 2.0f * mute::Pi * frequency / sr;
            float cw0 = mute::cos(w0);
            float sw0 = mute::sin(w0);
            float alpha = sw0 * q / 2.0f;
            float beta = 1 + alpha;

            coeffs.a1 = (-2.f * cw0) / beta;
            coeffs.a2 = (1.f - alpha) / beta;
            coeffs.b0 = (q * alpha) / beta;
        }

        void reset()
        {
            samples = {};
            coeffs = {};
        }

        void process(float sr)
        {
            computeCoefficients(sr);

            output = coeffs.b0 * (input - samples.in[1])
                - coeffs.a1 * samples.out[0] - coeffs.a2 * samples.out[1];

            output = mute::clamp(output, -1.f, 1.f);

            // TODO: At the transition between one stage to the other,
            // apply a non-linear transform to color the sound (i.e.
            // saturation, hysteresis, folding, etc.)
            samples.in[1] = samples.in[0];
            samples.in[0] = input;
            samples.out[1] = samples.out[0];
            samples.out[0] = output;
        }
    };

    struct Svf
    {
        float frequency = 100.0f; // 20, 192000
        float q = 1.f; // 0.1f, 10000.f

        float input;
        float hp, bp, lp;
        
        float g, r, h;
        float state_1 = 0, state_2 = 0;

        void set_f_q(float f, float q)
        {
            g = mute::tan(f * mute::Pi);
            r = 1.f / q;
            h = 1.f / (1.f + r * g + g * g);
        }

        void reset()
        {
            state_1 = state_2 = 0;
        }

        void process(float sr)
        {
            set_f_q(frequency / sr, q);
            hp = (input - r * state_1 - g * state_1 - state_2) * h;
            bp = g * hp + state_1;
            state_1 = g * hp + bp;
            lp = g * bp + state_2;
            state_2 = g * bp + lp;
        }
    };

    template <size_t N>
    struct SvfBank
    {
        static constexpr auto Modes = N;

        float input;
        float hpout;
        float bpout;
        float lpout;

        std::array<float, Modes> frequency;
        std::array<float, Modes> q;
        std::array<float, Modes> amplitude;

        std::array<float, Modes> hp;
        std::array<float, Modes> bp;
        std::array<float, Modes> lp;

        std::array<float, Modes> g;
        std::array<float, Modes> r;
        std::array<float, Modes> h;

        std::array<float, Modes> state_1;
        std::array<float, Modes> state_2;

        void reset()
        {
            state_1 = state_2 = {};
        }

        static void compute_grh(float f, float q, float* g, float* r, float* h)
        {
            *g = mute::tan(f * mute::Pi);
            *r = 1.f / q;
            *h = 1.f / (1.f + (*r) * (*g) + (*g) * (*g));
        }

        void process(float sr)
        {
            hpout = bpout = lpout = 0;
            for (size_t k = 0; k < Modes; k++)
            {
                float freq = frequency[k];
                float amp = amplitude[k];
                if (freq < 0.5f * sr && amp > mute::Epsilon)
                {
                    compute_grh(freq / sr, q[k], &g[k], &r[k], &h[k]);
                    hp[k] = (input - r[k] * state_1[k] - g[k] * state_1[k] - state_2[k]) * h[k];
                    bp[k] = g[k] * hp[k] + state_1[k];
                    state_1[k] = g[k] * hp[k] + bp[k];
                    lp[k] = g[k] * bp[k] + state_2[k];
                    state_2[k] = g[k] * bp[k] + lp[k];

                    hpout += amp * hp[k];
                    bpout += amp * bp[k];
                    lpout += amp * lp[k];
                }
            }
        }
    };
}