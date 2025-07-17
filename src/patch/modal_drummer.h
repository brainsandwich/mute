#pragma once

#include "mute/dsp/core.h"
#include "mute/dsp/filter.h"
#include "mute/dsp/enveloppe.h"
#include "mute/dsp/sequencer.h"

// #define USE_FILTERBANK

namespace patch
{

    struct Modal
    {
        static constexpr auto PartialsCount = 64;

        float frequency = 100.f; // 20, 192000
        float decay = 0.7f; // 0 10
        
        // Impulse
        float stiffness = 0.001f; // 0.001f, 0.5f

        // Grains
        float noiseAmount = 0.01f; // 0.0f, 1.f

        // Partials
        float partialCompression = 1.0f; // 0.1, 2.0
        float partialSpacing = 1.0f; // 0.1f, 10.f

        // Amplitude
        float tilt = 0.1f; // 0.0001f, 1.0f
        float sine = 0.9f; // 0, 1
        float sinefreq = 9.0f; // 1, 20
        float sinephase = 0.f; // 0, 1

        // Input/Output
        float gate;
        struct {
            float left = 0, right = 0;
        } output;

    #if defined(USE_FILTERBANK)
        struct {
            SvfBank<PartialsCount> filter;
            std::array<float, PartialsCount> amplitude;
        } partials;
    #else
        struct Partial
        {
            mute::dsp::Svf filter;
            float amplitude;
        };
        std::array<Partial, PartialsCount> partials;
    #endif

        mute::dsp::Waveloppe impulse = mute::dsp::Waveloppe::triangle(0.01, 1.0);
        mute::dsp::Waveloppe noiseEnv = mute::dsp::Waveloppe::triangle(0.9, 0.3);
        mute::dsp::Waveloppe env = mute::dsp::Waveloppe::fall(0.1, 1.0);

        void trigger()
        {
            env.trigger();
            impulse.trigger();
            noiseEnv.trigger();
    #if defined(USE_FILTERBANK)
            partials.filter.reset();
    #else
            for (auto& p: partials)
                p.filter.reset();
    #endif
        }

        void process(float sr)
        {
            // int numPartials = partials.size();
            float fundamental = frequency;
            output = {};
            impulse.time = stiffness;
            impulse.process(sr);
            env.process(sr);
            noiseEnv.process(sr);

            // float grainChance = rand() / double(RAND_MAX);
            float noiseInput = noiseAmount * (rand() / double(RAND_MAX));
                // = grainChance < noiseAmount
                // ? rand() / double(RAND_MAX)
                // : 0.f;

            float impulseInput = impulse.output;
            float input = impulseInput + noiseInput * noiseEnv.output;

    #if defined(USE_FILTERBANK)
            for (int index = 0; index < PartialsCount; index++)
            {
                float partialFrequency = fundamental * (1 + mute::pow(index * partialSpacing, (float) partialCompression));
                if (partialFrequency > sr)
                    continue;

                // float s = (0.5 + 0.5 * mute::cos(sinefreq * mute::Tau * (index / float(PartialsCount)) + sinephase));
                float amp = mute::exp(-tilt * index); // * mute::lerp(1.0f, s, sine);
                partials.filter.amplitude[index] = amp;
                partials.filter.frequency[index] = partialFrequency;
                partials.filter.q[index] = qFromDecay(partialFrequency, decay, sr);
            }

            partials.filter.input = input;
            partials.filter.process(sr);
            output.left = partials.filter.bpout;
            output.right = partials.filter.bpout;
    #else
            for (int index = 0; index < PartialsCount; index++)
            {
                auto& p = partials[index];
                float partialFrequency = fundamental * (1 + mute::pow(index * partialSpacing, (float) partialCompression));
                if (partialFrequency > sr)
                    continue;

                float s = abs(mute::cos(sinefreq * mute::Tau * (index / float(PartialsCount)) + sinephase));
                p.amplitude = mute::exp(-tilt * index) * mute::lerp(1.0f, s, sine);
                p.filter.frequency = partialFrequency;
                p.filter.q = mute::dsp::qFromDecay(partialFrequency, decay, sr);
                p.filter.input = input;
                p.filter.process(sr);
                float mout = p.filter.bp * p.amplitude;
                float pan = index == 0 ? 0.5 : (index % 2);
                float panleft = mute::sqrt(pan);
                float panright = mute::sqrt(1 - pan);

                output.left += panleft * mout;
                output.right += panright * mout;
            }
    #endif
        }

        void process(float sr, int frames, float* out)
        {
            for (int f = 0; f < frames; f++)
            {
                process(sr);
                *out++ = output.left;
                *out++ = output.right;
            }
        }
    };

    template <typename Synth, int Polyphony>
    struct Polyphonic
    {
        std::array<Synth, Polyphony> voices;
        int currentVoice = 0;
        struct {
            float left, right;
        } output;

        void trigger(float freq)
        {
            auto& v = voices[currentVoice];
            v.trigger();
            v.frequency = freq;
            currentVoice = (currentVoice + 1) % voices.size();
        }

        void process(float sr)
        {
            output = {};
            for (auto& v: voices)
            {
                v.process(sr);
                output.left += v.output.left;
                output.right += v.output.right;
            }
        }
    };

    template <typename Synth>
    struct Drummer
    {
        Synth& synth;
        float freq;
        mute::dsp::EuclideanSequencer seq;
        std::string msg;
        mute::dsp::EdgeDetector ed;

        void process(float clkin, float sr)
        {
            seq.gate.input = clkin;
            seq.process(sr);
            ed.input = seq.output;
            ed.process(sr);

            if (ed.rising)
            {
                fmt::println("{}", msg);
                synth.trigger(freq);
            }
        }
    };

    struct ModalDrummer
    {
        using Synth = Polyphonic<Modal, 8>;

        float gain = 0.9f;

        mute::dsp::Phazor clk = { .frequency = 8.f };

        Synth synth = {};

        std::array<Drummer<Synth>, 5> drummers = {
            Drummer { synth, .freq = 37, { .length = 9, .increment = 5 }, .msg = "BONG" },
            Drummer { synth, .freq = 74, { .length = 23, .increment = 4 }, .msg = ">---bNgg" },
            Drummer { synth, .freq = 123, { .length = 37, .increment = 3 }, .msg = ">-->->>>->king" },
            Drummer { synth, .freq = 236, { .length = 38, .increment = 7 }, .msg = ">->->->>>>------bouW" },
            Drummer { synth, .freq = 778, { .length = 48, .increment = 9 }, .msg = ">>>>->>>->->->>>>>>>>iiiu" }
        };

        struct {
            mute::dsp::Phazor A = { .frequency = 0.3f };
            mute::dsp::Phazor B = { .frequency = 0.5f };
            mute::dsp::Phazor C = { .frequency = 0.1f };
            mute::dsp::Phazor D = { .frequency = 0.08f };
        } lfos;

        struct {
            float left, right;
        } output;

        void process(float sr)
        {
            clk.process(sr);

            for (auto& d: drummers)
                d.process(clk.output, sr);

            lfos.A.process(sr);
            lfos.B.process(sr);
            lfos.C.process(sr);
            lfos.D.process(sr);

            for (int i = 0; i < (int) synth.voices.size(); i++)
            {
                auto& voice = synth.voices[i];
                voice.decay = 1.0f + 0.8f * mute::cos(lfos.D.output + 0.74);
                voice.stiffness = 0.01 + 0.002 * mute::cos(lfos.A.output + 0.77);
                voice.tilt = 0.8f + 0.1f * mute::cos(lfos.B.output);
                voice.partialSpacing = (4.7f + 3.f * mute::cos(lfos.D.output)) * voice.env.output + 1.6f;
                voice.sinephase = 0.2*mute::cos(lfos.A.output);
                voice.partialCompression = 1.0 + 0.7 * mute::cos(lfos.C.output);
            }

            synth.process(sr);

            output.left = mute::clamp(mute::tanh(synth.output.left * gain), -1.f, 1.f);
            output.right = mute::clamp(mute::tanh(synth.output.right * gain), -1.f, 1.f);
        }
    };

    static constexpr const char* PatchName = "Modal Drummer";
    static constexpr const char* PatchCreationDate = "17.07.25";
    static constexpr const char* PatchDesc =
R"(
A polyphonic Modal synthesizer is triggered by an array of Euclidean sequencers.
The Synthesizer has a controlled polyphony (at compilation time) ; on my Mac M1 Pro
it's okay up until about 12 voices max. Each voice uses a bandpass filter bank of
128 modes, pinged by a mix of triangle impulse (Waveloppe) and a noise generator
(simple rand() generator).
)";
}