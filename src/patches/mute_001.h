#pragma once

#include "mute/debug.h"
#include "mute/transport.h"
#include "mute/driver.h"
#include "mute/random.h"

#include "mute/modules/enveloppe.h"
#include "mute/modules/sequencer.h"
#include "mute/modules/biquad.h"
#include "mute/modules/oscillator.h"
#include "mute/modules/perc.h"

#include <fmt/format.h>

#include <optional>

struct Stereo
{
    float left, right;
};

auto sq(const auto& in) { return in * in; }

inline float normalize(float in, float min, float max)
{
    return (in - min) / (max - min);
}

inline float denormalize(float in, float min, float max)
{
    return in * (max - min) + min;
}

template <typename T>
inline float getset(T* target, std::optional<float> n, float min, float max)
{
    if (n)
        *target = denormalize(*n, min, max);
    return normalize(*target, min, max);
}

struct PercPatch
{
    float volume = 1;
    float panning = 0.5;
    mute::EuclideanSequencer sequencer = {
        .increment = 3,
        .length = 8,
        .gatelen = 0.1
    };
    mute::Perc perc = {
        .env1 = mute::Waveloppe::triangle(0.3, 0.3),
        .env2 = mute::Waveloppe::fall(0.9, 0.3),
        .filterfreq = 60,
        .oscfreq = 60,
        .pitchmod = 0.02,
        .filterenv = 0.4,
        .noiseamt = 0.3
    };

    int paramCount() const { return 13; }

    float param(int index, std::optional<float> newvalue = {})
    {
        switch (index)
        {
            case 0: return getset(&volume, newvalue, 0.0f, 1.0f);
            case 1: return getset(&panning, newvalue, 0.0f, 1.0f);
            case 2: return getset(&sequencer.increment, newvalue, 1, 5);
            case 3: return getset(&sequencer.length, newvalue, 4, 8);
            case 4: return getset(&perc.env1.time, newvalue, 0.1, 0.4);
            case 5: return getset(&perc.env1.skew, newvalue, 0.01, 0.2);
            case 6: return getset(&perc.env2.time, newvalue, 0.2, 1.0);
            case 7: return getset(&perc.env2.skew, newvalue, 0.1, 0.8);
            case 8: return getset(&perc.filterfreq, newvalue, 20, 200);
            case 9: return getset(&perc.filter.q, newvalue, 0.8, 5.0);
            case 10: return getset(&perc.oscfreq, newvalue, 20, 200);
            case 11: return getset(&perc.pitchmod, newvalue, 0.0, 0.5);
            case 12: return getset(&perc.filterenv, newvalue, 0.0, 0.5);
            case 13: return getset(&perc.noiseamt, newvalue, 0.3, 1.0);
            default: return -1;
        }
    }

    Stereo process(float sr, mute::Transport transport)
    {
        Stereo sum = {};

        if (transport.measure.ticks._16th)
            sequencer.next();

        sequencer.process(sr);
        perc.gate = sequencer.output;
        perc.process(sr);
        
        float out = mute::saturate(perc.output, 2.4);
        float left = out * sqrt(1. - panning);
        float right = out * sqrt(panning);
        sum.left += left;
        sum.right += right;

        return sum;
    }

    void process(mute::AudioProcessData data, mute::Transport transport)
    {
        for (int f = 0; f < data.frames; f++)
        {
            transport.update(1. / data.sampleRate);

            auto [left, right] = process(data.sampleRate, transport);
            left = mute::clamp(volume * left, -1.f, 1.f);
            right = mute::clamp(volume * right, -1.f, 1.f);
            data.playback.buffer[f * data.playback.channels + 0] = left;
            data.playback.buffer[f * data.playback.channels + 1] = right;
        }
    }
};

template <typename Patch>
struct Mutator
{
    mute::RandomNumberGenerator RNG;
    Patch current, origin, target;

    Mutator()
    {
        int r = rand();
        RNG = mute::RandomNumberGenerator::init(target.paramCount(), 1.0, r);
        mutate();
        // mutate();
    }

    void mutate()
    {
        RNG.seed(rand());
        std::swap(origin, target);
        for (int i = 0; i < target.paramCount(); i++)
            target.param(i, RNG.next());
    }
    void lerp(float t)
    {
        for (int i = 0; i < current.paramCount(); i++)
            current.param(i, mute::lerp(origin.param(i), target.param(i), t));
    }

};

PercPatch f2fkick() 
{
    return {
        .sequencer = { .increment = 1, .length = 4 },
        .perc = {
            .env1 = mute::Waveloppe::fall(0.5, 0.3),
            .env2 = mute::Waveloppe::fall(0.5, 0.8),
            .oscfreq = 30,
            .pitchmod = 0.01,
            .volume = 2.,
            .noiseamt = 0.3,
            .filterfreq = 100,
            .filterenv = 0.3
        }
    };
};

PercPatch gritf() 
{
    return {
        .sequencer = { .increment = 7, .length = 9 },
        .perc = {
            .env1 = mute::Waveloppe::fall(0.5, 0.4),
            .env2 = mute::Waveloppe::fall(0.5, 0.8),
            .oscfreq = 30,
            .pitchmod = 0.001,
            .volume = 0.8,
            .noiseamt = 0.3,
            .filterfreq = 100,
            .filterenv = 0.3
        }
    };
};

struct Mix
{
    PercPatch kp1 = f2fkick();
    PercPatch kp2 = gritf();
    std::array<Mutator<PercPatch>, 8> pp;
    float t;
    int count = 0;
    int selectedEncoder = 0;

    struct Encoders {
        static constexpr int NumEncoders = 2+8;
        ma_encoder ee[NumEncoders];
        std::array<int16_t, 256*2> buffer;

        Encoders()
        {
            for (int i = 0; i < NumEncoders; i++)
            {
                ma_encoder_config config = ma_encoder_config_init(ma_encoding_format_wav, ma_format_s16, 2, 44100);
                ma_result result = ma_encoder_init_file(fmt::format("track-{}.wav", i).c_str(), &config, &ee[i]);
                if (result != MA_SUCCESS) {
                    mute::log::error(fmt::format("Error initializing WAV encoder {}", i));
                    return;
                }
            }
        }
        ~Encoders()
        {
            for (int i = 0; i < NumEncoders; i++)
                ma_encoder_uninit(&ee[i]);
        }

        void write(int index, std::span<const float> values)
        {
            static constexpr float S16Max = std::numeric_limits<int16_t>::max();
            static constexpr float S16Min = std::numeric_limits<int16_t>::min();
            uint64_t framesWritten = 0;
            for (size_t k = 0; k < values.size(); k++)
                buffer[k] = values[k] * (values[k] > 0.f ? S16Max : -S16Min);
            ma_encoder_write_pcm_frames(&ee[index], &buffer, values.size() / 2, &framesWritten);
        }

        ma_encoder& operator[](int index) { return ee[index]; }
    } encoders;

    std::vector<float> buffer;
    mute::Transport transport;

    template <typename Patch>
    void process(mute::AudioProcessData data, Patch& patch)
    {
        namespace views = std::views;
        namespace ranges = std::ranges;

        mute::AudioProcessData pd = data;
        std::ranges::fill(buffer, 0);
        pd.playback.buffer = std::span { buffer.begin(), size_t(data.frames * data.playback.channels) };

        patch.process(pd, transport);
        encoders.write(selectedEncoder++, pd.playback.buffer);

        auto added = views::zip(data.playback.buffer, pd.playback.buffer)
            | views::transform([](auto zipped) { return mute::clamp(std::get<0>(zipped) + std::get<1>(zipped), -1.f, 1.f); });
        ranges::copy(added, data.playback.buffer.begin());
    }

    void process(mute::AudioProcessData data)
    {
        selectedEncoder = 0;
        for (auto& p: pp)
        {
            p.lerp(t);
            p.current.volume = mute::clamp(p.current.volume, 0.1f, 0.3f);
        }
        buffer.resize(data.playback.buffer.size());
        
        process(data, kp1);
        process(data, kp2);
        for (auto& p: pp)
            process(data, p.current);
            
        transport.update(data.frames / float(data.sampleRate));

        t += 16.0 * (60. / transport.bpm) * data.frames / float(data.sampleRate);
        t = mute::clamp(t, 0.f, 1.f);

        if (transport.measure.ticks.bar)
        {
            count++;
        }

        if (count > 0)
        {
            fmt::println("MUTATE");
            count = 0;
            t = 0.f;
            for (auto& p: pp)
                p.mutate();
        }
    }
};