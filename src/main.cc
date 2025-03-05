#include "mute/dsp.h"
#include "mute/driver.h"
#include "mute/debug.h"
#include "mute/transport.h"

#include "mute/modules/enveloppe.h"
#include "mute/modules/sequencer.h"
#include "mute/modules/biquad.h"
#include "mute/modules/oscillator.h"
#include "mute/modules/perc.h"

#include <fmt/printf.h>
#include <fmt/ranges.h>

#include <map>
#include <ranges>
#include <span>
#include <chrono>

#include "mute/random.h"

#include <miniaudio.h>

struct Stereo
{
    float left, right;
};

auto sq(const auto& in) { return in * in; }

// struct MultiPercPatch
// {
//     float volume = 1;
//     static constexpr size_t TrackCount = 12;

//     struct State
//     {
//         std::array<mute::EuclideanSequencer, TrackCount> sequencers;
//         std::array<mute::Perc, TrackCount> percs;
//         std::array<float, TrackCount> pannings;
//     };

//     State current;
//     State morphs[2];

//     int swapper = 0;
//     float faderOrigin = 0;
//     float faderTarget = 1;
//     float t = 0;

//     mute::RandomNumberGenerator RNG = mute::RandomNumberGenerator::init(256*4, 0.8);

//     void rnd()
//     {
//         RNG.seed();

//         for (size_t i = 0; i < TrackCount; i++)
//         {
//             auto& morph = morphs[swapper];
//             auto& seq = morph.sequencers[i];
//             auto& perc = morph.percs[i];
//             auto& pan = morph.pannings[i];

//             // param.seq_length = RNG.next(4, 16);
//             // param.seq_increment = RNG.next(1, 15);
//             // param.seq_gatelen = 0.01;

//             // param.perc_env1_time = RNG.next(0.05, 1.0);
//             // param.perc_env1_skew = sq(RNG.next()) * 1.0 + 0.01;
//             // param.perc_env2_time = RNG.next(0.05, 1.7);
//             // param.perc_env2_skew = sq(RNG.next()) * 1.0 + 0.01;
            
//             // param.perc_pitchmod = sq(RNG.next());
//             // param.perc_noiseamt = sq(RNG.next());
//             // param.perc_filterenv = sq(RNG.next());

//             // param.perc_oscfreq = sq(RNG.next()) * 100 + 20;
//             // param.perc_noisefreq = sq(RNG.next()) * 1000 + 100;
//             // param.perc_filterfreq = sq(RNG.next()) * 100 + 20;

//             // param.perc_volume = mute::clamp(0.8 + 0.2 / i + sq(RNG.next(0.1, 0.5)), 0., 1.);

//             // param.pan = RNG.next(0.2, 0.7);

//             if (i == 0)
//             {
//                 float slr = RNG.next();
//                 float sir = RNG.next();

//                 if (slr < 0.3) seq.length = 4;
//                 else if (slr < 0.7) seq.length = 8;
//                 else if (slr < 1.) seq.length = 9;

//                 if (sir < 0.3) seq.increment = 1;
//                 else if (sir < 0.7) seq.increment = 2;
//                 else if (sir < 1.) seq.increment = 3;
//             } else {
//                 seq.length = RNG.next(4, 24);
//                 seq.increment = RNG.next(1, 15);
//             }
//             seq.gatelen = 0.01;

//             if (i == 0)
//             {
//                 perc.env1 = mute::Waveloppe::triangle(1.0, 1.0);
//                 perc.env1.time = RNG.next(0.3, 0.7);
//                 perc.env1.skew = sq(RNG.next()) * 1.0 + 0.01;
//                 perc.env2 = mute::Waveloppe::fall(1.0, 1.0);
//                 perc.env2.time = RNG.next(0.8, 1.7);
//                 perc.env2.skew = sq(RNG.next()) * 1.0 + 0.01;

//                 perc.pitchmod = sq(RNG.next(0., 0.1));
//                 perc.noiseamt = sq(RNG.next(0., 0.3));
//                 perc.filterenv = sq(RNG.next(0.1, 0.4));
//                 perc.filter.q = RNG.next(1., 4.0);
                
//                 perc.oscfreq = sq(RNG.next()) * 30 + 40;
//             } else {
//                 perc.env1 = mute::Waveloppe::triangle(1.0, 1.0);
//                 perc.env1.time = RNG.next(0.05, 0.9);
//                 perc.env1.skew = sq(RNG.next()) * 4.0 + 0.01;
//                 perc.env2 = mute::Waveloppe::triangle(1.0, 1.0);
//                 perc.env2.time = RNG.next(0.05, 1.7);
//                 perc.env2.skew = sq(RNG.next()) * 4.0 + 0.01;
                
//                 perc.pitchmod = sq(RNG.next(0.03, 2.0));
//                 perc.noiseamt = sq(RNG.next(0.1, 0.9));
//                 perc.filterenv = sq(RNG.next());
//                 perc.filter.q = RNG.next(0.5, 4.0);
                
//                 perc.oscfreq = sq(RNG.next()) * 100 + 20;
//             }

//             perc.noisefreq = sq(RNG.next()) * 4000 + 1000;
//             perc.filterfreq = sq(RNG.next()) * 100 + 20;

//             if (i == 0)
//             {
//                 perc.volume = 1.9;
//                 pan = 0.5;
//             }
//             else
//             {
//                 perc.volume = sq(RNG.next(0.04, 0.4));
//                 pan = RNG.next(0.0, 1.0);
//             }
//         }
//     }

//     ma_encoder encoder[TrackCount];

//     MultiPercPatch()
//     {
//         rnd();
//         swapper = 1 - swapper;
//         rnd();

//         for (int i = 0; i < int(TrackCount); i++)
//         {
//             ma_encoder_config config = ma_encoder_config_init(ma_encoding_format_wav, ma_format_s16, 2, 44100);
//             ma_result result = ma_encoder_init_file(fmt::format("track-{}.wav", i).c_str(), &config, &encoder[i]);
//             if (result != MA_SUCCESS) {
//                 mute::log::error(fmt::format("Error initializing WAV encoder {}", i));
//                 return;
//             }
//         }
//     }

//     ~MultiPercPatch()
//     {
//         for (int i = 0; i < int(TrackCount); i++)
//             ma_encoder_uninit(&encoder[i]);
//     }

//     int accumBar = 0;

//     Stereo process(float sr, mute::Transport transport)
//     {
//         Stereo sum = {};

//         if (transport.measure.ticks.bar)
//             accumBar++;
        
//         if (accumBar > 8)
//         {
//             swapper = 1 - swapper;
//             rnd();
//             faderOrigin = 1 - faderOrigin;
//             faderTarget = 1 - faderTarget;
//             t = 0;
//             accumBar = 0;
//         }

//         // for (size_t i = 0; i < TrackCount; i++)
//         // {
//         //     auto& m = modules[0];
//         //     auto& seq = m.sequencers[i];
//         //     auto& perc = m.percs[i];
//         //     // auto& pan = m.pannings[i];
//         //     if (transport.measure.ticks._16th)
//         //         seq.next();

//         //     seq.process(sr);
//         //     perc.gate = seq.output;
//         //     perc.process(sr);
            
//         //     // TODO: panning
//         //     float out = mute::saturate(perc.output, 1.4);
//         //     sum.left += out;
//         //     sum.right += out;
//         // }

//         float fadeTime = (4. * 8.) / (transport.bpm / 60.);
//         float fader = mute::clamp(mute::lerp(faderOrigin, faderTarget, t / fadeTime), 0.f, 1.f);

//         for (size_t i = 0; i < TrackCount; i++)
//         {
//             auto& seq = current.sequencers[i];
//             auto& perc = current.percs[i];
//             auto& pan = current.pannings[i];

//             auto& seq0 = morphs[1 - swapper].sequencers[i];
//             auto& perc0 = morphs[1 - swapper].percs[i];
//             auto& pan0 = morphs[1 - swapper].pannings[i];

//             auto& seq1 = morphs[swapper].sequencers[i];
//             auto& perc1 = morphs[swapper].percs[i];
//             auto& pan1 = morphs[swapper].pannings[i];

//             if (transport.measure.ticks._16th)
//                 seq.next();

//             seq.length = mute::lerp(seq0.length, seq1.length, fader);
//             seq.increment = mute::lerp(seq0.increment, seq1.increment, fader);
//             seq.gatelen = mute::lerp(seq0.gatelen, seq1.gatelen, fader);

//             perc.env1.time = mute::lerp(perc0.env1.time, perc1.env1.time, fader);
//             perc.env1.skew = mute::lerp(perc0.env1.skew, perc1.env1.skew, fader);
//             perc.env2.time = mute::lerp(perc0.env2.time, perc1.env2.time, fader);
//             perc.env2.skew = mute::lerp(perc0.env2.skew, perc1.env2.skew, fader);
            
//             perc.pitchmod = mute::lerp(perc0.pitchmod, perc1.pitchmod, fader);
//             perc.noiseamt = mute::lerp(perc0.noiseamt, perc1.noiseamt, fader);
//             perc.filterenv = mute::lerp(perc0.filterenv, perc1.filterenv, fader);

//             perc.oscfreq = mute::lerp(perc0.oscfreq, perc1.oscfreq, fader);
//             perc.noisefreq = mute::lerp(perc0.noisefreq, perc1.noisefreq, fader);
//             perc.filterfreq = mute::lerp(perc0.filterfreq, perc1.filterfreq, fader);

//             perc.volume = mute::lerp(perc0.volume, perc1.volume, fader);

//             pan = mute::lerp(pan0, pan1, fader);

//             seq.process(sr);
//             perc.gate = seq.output;
//             perc.process(sr);
            
//             // TODO: panning
//             float out = mute::saturate(perc.output, 1.4);
//             float left = out * sqrt(1. - pan);
//             float right = out * sqrt(pan);
//             sum.left += left;
//             sum.right += right;

//             uint64_t framesWritten = 0;
//             int16_t lr16[2] =
//             {
//                 int16_t(left * (std::numeric_limits<int16_t>::max() - 1)),
//                 int16_t(right * (std::numeric_limits<int16_t>::max() - 1))
//             };
            
//             ma_result write_rez = ma_encoder_write_pcm_frames(&encoder[i], &lr16, 1, &framesWritten);
//             if (write_rez != MA_SUCCESS)
//                 mute::log::error("Error writing PCM frame");
//             // if (framesWritten > 0)
//             // {
//             //     mute::log::info(fmt::format("{} frames written !!", framesWritten));
//             // }
//         }

//         t += (1. / sr);
//         return sum;
//     }

//     void process(mute::AudioProcessData data, mute::Transport transport)
//     {
//         for (int f = 0; f < data.frames; f++)
//         {
//             transport.update(1. / data.sampleRate);

//             auto [left, right] = process(data.sampleRate, transport);
//             left = mute::clamp(volume * left, -1.f, 1.f);
//             right = mute::clamp(volume * right, -1.f, 1.f);
//             data.playback.buffer[f * data.playback.channels + 0] = left;
//             data.playback.buffer[f * data.playback.channels + 1] = right;
//         }
//     }
// };

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

int main(int argc, char** argv)
{
    std::unique_ptr<Mix> mix = nullptr;

    auto driver = mute::AudioDriver({
        .playback = {
            .channels = 2
        },
        .sampleRate = 44100,
        .bufferSize = 256,
        .callback = [&](mute::AudioProcessData data) { mix->process(data); }
    });

    mix = std::unique_ptr<Mix>(
        new Mix {
            .buffer = std::vector<float>(256*2),
            .transport = {
                .playing = true,
                .bpm = 140.f
            }
        });
    for (auto& p: mix->pp)
        p.mutate();

    driver.start();
    while (int c = getchar())
    {
        if (c == 'q')
            break;
        // if (c == 'r') 
        // {
        //     mix->mpp.rnd();
        // }
        // if (c == 'r') mix.osp.rnd();
        // if (c == 'd') mix.dp.rnd();
        // if (c == 'z') mix.osp.volume = 0.4 - mix.osp.volume;
        // if (c == 'e') mix.dp.volume = 1.0 - mix.dp.volume;
    }

    driver.stop();
    mix = nullptr;

    return 0;
}

// #include <fmt/printf.h>
// #include <fmt/ranges.h>
// #include <raylib.h>

// #include <ranges>

// #include "mute/audio.h"
// #include "mute/dsp.h"
// // #include "mute/modules/noise.h"
// // #include "mute/modules/adsr.h"
// // #include "mute/modules/oscillator.h"

// namespace mute
// {
//     // /// @brief Meta enveloppe, using only one "time" parameter to drive a full ADSR
//     // /// 1. decay progresses first
//     // /// 2. then some attack is added
//     // /// 3. sustain and release come last
//     // ///
//     // /// - at time == 0.1, mostly decay, in ~0.3sec
//     // /// - at time == 0.5, 0.125sec attack, 0.833sec decay, sustain at 0.125, complete in 1.47sec
//     // /// - at time == 1, complete rise and fall time is 2.5sec
//     // struct MetaEnv
//     // {
//     //     float time; // normalized 0 to 1
//     //     float gate;
//     //     float output;
//     //     ADSR adsr;

//     //     void process(float sr)
//     //     {
//     //         time = mute::clamp(time, 0.0f, 1.0f);
//     //         adsr.a = 0.5*time*time;
//     //         adsr.d = sqrt(time);
//     //         adsr.s = time*time*time;
//     //         // adsr.d = 2.0f * adsr.s;
//     //         adsr.r = 2.0f * adsr.s;
//     //         // adsr.r = sqrt(time);
//     //         adsr.gate = gate;
//     //         adsr.process(sr);

//     //         output = adsr.output;
//     //     }
//     // };

//     struct Trigger
//     {
//         float last = 0.0f;
//         float input = 0.0f;
//         bool output = false;

//         Trigger& operator=(float in) { input = in; return *this; }

//         void process()
//         {
//             output = false;
//             if (input > 0.6f && last < 0.4f)
//             {
//                 output = true;
//             }
//             last = input;
//         }
//     };

//     struct AD
//     {
//         float a;
//         float d;
//         float gate;
//         float output;
//         float t = 0.0f;
//         bool gateup = false;

//         void process(float sr)
//         {
//             output = 0.0f;

//             // bool triggered = false;
//             if (gate > 0.5f && !gateup)
//             {
//                 gateup = true;
//                 t = 0.0f;
//             }
//             if (gate < 0.5f && gateup)
//                 gateup = false;

//             if (t < a) output = t / max(a, Epsilon);
//             else if (t < a + d) output = (-(t - a) / d) + 1;

//             t = min(t + 1.0f / sr, a + d + Epsilon);
//         }
//     };

//     float ads(float params[3], float gate, float* t, float sr)
//     {
//         const float& a = params[0];
//         const float& d = params[1];
//         const float& s = params[2];
        
//         float dt = 1 / safe(sr);
//         float origin = *t < a ? 0 : 1;
//         float target = *t < a ? gate : gate * s;
//         float time = *t < a ? (*t / a) : ((*t - a) / d);

//         float output = lerp(origin, target, *t);
//         *t = clamp(*t + dt, 0.0f, a + d + Epsilon);
//         return output;
//     }

//     struct GateSequencer
//     {
//         Trigger clock;
//         float output;

//         int index = -1;
//         float t = 0.0f;
//         float gatelen = 0.1f;
//         std::vector<bool> pattern;

//         void next()
//         {
//             index = (index + 1) % pattern.size();
//             t = 0.0f;
//         }

//         void process(float sr)
//         {
//             clock.process();
//             if (clock.output)
//             {
//                 next();
//             }

//             output = 0.0f;
//             if (pattern[index % pattern.size()])
//             {
//                 t += 1.f / sr;
//                 output = t < gatelen;
//             }
//         }
//     };

//     struct KickDrum
//     {
//         float phase = 0;
//         float t = 0;

//         void trigger() { t = 0; phase = 0; }
//         float process(float sr, float decay, float fx1, float fx2)
//         {
//             float freq = fx1;
//             float pitchmod = fx2 * 2.f - 1.f;

//             return oscillate(&phase, freq, sr);
//         }
//     };

//     struct Perc
//     {
//         enum class Mode
//         {
//             KickDrum
//         } mode;

//         union {
//             KickDrum kickdrum;
//         } models;

//         float volume;
//         float decay;
//         float fx1, fx2;

//         void trigger() {}

//         void process(float sr)
//         {

//         }
//     };
// }

// int main(int argc, char** argv)
// {
//     InitWindow(800, 600, "mute");
//     SetTargetFPS(60);

//     auto audio = mute::Audio(mute::AudioConfiguration {
//         .bufferSize = 128,
//         .sampleRate = 44100,
//         .playback = { .channels = 2 }
//     });

//     // clock -> drum sequencer -> multiple drum sounds
//     // drum sound: "noise source" + env (ad ?) + filter
//     // drum set: mixed drum sounds
//     // additional: send effects
//     // mix: drum set + returns / compress + saturate

//     // A: one drum sound
//     auto noise = mute::PrimeCluster {};
//     auto lfo1 = mute::Oscillator { .frequency = 1.0f };
//     auto lfo2 = mute::Oscillator { .frequency = 1.8f };
//     // auto env = mute::MetaEnv { .time = 0.4f };
//     auto env = mute::AD { .a = 0.f, .d = 0.03f };

//     auto clock = mute::LFO { .frequency = 4*mute::bpm2hz(120.0f), .mode = mute::LFO::Mode::Square };
//     auto seq = mute::GateSequencer {
//         .gatelen = 0.01f,
//         .pattern = { 1, 0, 0, 0, 1, 1, 0, 0 }
//     };
//     seq.pattern.resize(16);
//     auto randomizeSequence = [&](){
//         for (int i = 0; i < (int) seq.pattern.size(); i++)
//             seq.pattern[i] = (rand() > RAND_MAX / 2);
//         fmt::println("seq: {}", fmt::join(seq.pattern | std::views::transform([](auto v) { return v ? "X" : "_"; }) , ", "));
//     };

//     audio.audioCallback = [&](mute::AudioProcessData data)
//     {
//         float sr = data.sampleRate;
//         int chans = data.playback.channels;
//         memset(data.playback.buffer, 0, data.playback.channels * data.frames * sizeof(float));

//         for (int f = 0; f < data.frames; f++)
//         {
//             clock.process(sr);
//             seq.clock = clock.output;

//             lfo1.process(sr);
//             lfo2.process(sr);
//             noise.process(sr);
//             noise.noiseAmount = 0.1f * (lfo1.output * 0.5f + 0.5f) + 0.01f;
//             noise.freqMult = 0.2f * lfo2.output + 1.0f;
            
//             seq.process(sr);
//             env.gate = seq.output;
//             env.process(sr);
//             float output = noise.output * env.output;
//             output *= 0.8f;

//             for (int c = 0; c < chans; c++)
//                 data.playback.buffer[f * chans + c] = output;
//         }

//     };
//     audio.start();

//     while (!WindowShouldClose())
//     {
//         PollInputEvents();
//         if (IsKeyPressed(KEY_Z))
//             randomizeSequence();

//         SwapScreenBuffer();
//     }

//     audio.stop();
//     CloseWindow();

//     return 0;
// }














// #include <array>
// #include <vector>
// #include <span>
// #include <optional>
// #include <algorithm>
// #include <ranges>
// #include <unordered_set>
// #include <queue>
// #include <stack>

// #include <fmt/printf.h>

// namespace mute
// {
//     template <typename Sample>
//     struct AudioBlock
//     {
//         static constexpr size_t MaxChannels = 16;
//         std::array<std::span<Sample>, MaxChannels> samples;
//         int channels;
//     };

//     struct ProcessContext
//     {
//         AudioBlock<const float> in;
//         AudioBlock<float> out;
//     };

//     struct PrepareInfo
//     {
//         int sampleRate;
//         int bufferSize;
//         int channels;
//     };

//     struct Processor
//     {
//         std::string name;
//         std::vector<Processor*> sources;

//         Processor(std::string_view name): name(name) {}
        
//         virtual void prepare(const PrepareInfo&) = 0;
//         virtual void process(const ProcessContext&) = 0;
//     };

//     struct Passthrough: Processor
//     {
//         Passthrough(std::string_view name): Processor(name) {}

//         void prepare(const PrepareInfo&) override {}
//         void process(const ProcessContext&) override {}
//     };

//     template <typename T> struct NodeSiblingAccessor {};
//     // static const Node* sibling(const Node* node, int index) { return nullptr; }

//     template <typename T> struct NodeChildAccessor {};
//     // static const Node* child(const Node* node, int index) { return nullptr; }
//     // static int children(Node* node) { return 0; }

//     template <typename T> struct NodeParentAccessor {};
//     // static const Node* parent(const Node* node) { return nullptr; }

//     template <typename T> struct NodeIndexAccessor {};
//     // static int index(Node* node) { return 0; }

//     template <typename T> struct NodeChildMutator {};
//     // static void insert(Node* node, int index, Node* child) {}
//     // static void remove(Node* node, int index) {}

//     template <typename T> struct NodeTypeAccessor {};
//     // using Node = T;

//     template <std::derived_from<Processor> P>
//     struct NodeTypeAccessor<P>
//     {
//         using Node = Processor;
//     };

//     template <std::derived_from<Processor> P>
//     struct NodeChildAccessor<P>
//     {
//         static Processor* child(Processor* node, int index) { return node->sources[index]; }
//         static int children(Processor* node) { return node->sources.size(); }
//     };

//     template <std::derived_from<Processor> P>
//     struct NodeChildMutator<P>
//     {
//         static void insert(Processor* node, int index, Processor* child) { node->sources.insert(node->sources.begin() + index, child); }
//         static void remove(Processor* node, int index) { node->sources.erase(node->sources.begin() + index); }
//     };

//     template <typename T>
//     struct NodeManipulator:
//           NodeTypeAccessor<T>
//         , NodeSiblingAccessor<T>
//         , NodeChildAccessor<T>
//         , NodeParentAccessor<T>
//         , NodeIndexAccessor<T>
//         , NodeChildMutator<T>
//     {};

//     enum class GraphTraversal
//     {
//         DepthFirstPreOrder,
//         DepthFirstInOrder,
//         DepthFirstPostOrder,
//         BreadthFirst
//     };

//     template <GraphTraversal Traversal>
//     struct TraversalHelper
//     {
//         template <typename T, typename OutputIterator>
//         static void traverse(T* root, OutputIterator out)
//         {
//             using Manipulator = NodeManipulator<T>;
            
//             if constexpr (Traversal == GraphTraversal::DepthFirstPreOrder)
//                 *out++ = root;

//             for (int i = 0; i < Manipulator::children(root); i++)
//                 traverse(Manipulator::child(root, i), out);

//             if constexpr (Traversal == GraphTraversal::DepthFirstPostOrder)
//                 *out++ = root;
//         }
//     };

//     template <>
//     struct TraversalHelper<GraphTraversal::BreadthFirst>
//     {
//         template <typename T, typename OutputIterator>
//         static void traverse(T* root, OutputIterator out)
//         {
//             using Manipulator = NodeManipulator<T>;
//             using Node = Manipulator::Node;
//             using History = std::stack<Node*>; 

//             std::unordered_set<Node*> traversed;
//             History history;
//             history.push(root);

//             while (!history.empty())
//             {
//                 Node* current = history.top();
//                 history.pop();
//                 traversed.emplace(current);
//                 *out++ = current;

//                 for (int i = Manipulator::children(current) - 1; i >= 0; i--)
//                 {
//                     auto child = Manipulator::child(current, i);
//                     history.push(child);
//                 }
//             }
//         }
//     };

//     template <GraphTraversal Traversal, typename T, typename OutputIterator>
//     void traverse(T* root, OutputIterator out)
//     {
//         TraversalHelper<Traversal>::traverse(root, out);
//     }

//     struct DepthFirstPreOrder_t {} DepthFirstPreOrder;
//     struct DepthFirstInOrder_t {} DepthFirstInOrder;
//     struct DepthFirstPostOrder_t {} DepthFirstPostOrder;
//     struct BreadthFirst_t {} BreadthFirst;

//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstPreOrder_t) { traverse<GraphTraversal::DepthFirstPreOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstInOrder_t) { traverse<GraphTraversal::DepthFirstInOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstPostOrder_t) { traverse<GraphTraversal::DepthFirstPostOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, BreadthFirst_t) { traverse<GraphTraversal::BreadthFirst>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out) { traverse<GraphTraversal::DepthFirstPreOrder>(root, out); }
// }

// int main(int argc, char** argv)
// {
//     auto gena = mute::Passthrough("gena");
//     auto genb = mute::Passthrough("genb");
//     auto mixab = mute::Passthrough("mixab");
//     mixab.sources = { &gena, &genb };

//     auto genc = mute::Passthrough("genc");
//     auto gainc = mute::Passthrough("gainc");
//     gainc.sources = { &genc };

//     auto mixabc = mute::Passthrough("mixabc");
//     mixabc.sources = { &mixab, &gainc };

//     std::vector<bool> done;
//     std::vector<bool> run;
//     std::vector<int> depcount;
//     std::vector<mute::Processor*> ordered;
//     mute::traverse(&mixabc, std::back_inserter(ordered), mute::DepthFirstPostOrder);

//     done.resize(ordered.size());
//     run.resize(ordered.size());
//     depcount.resize(ordered.size());

//     for (auto i = 0ull; i < ordered.size(); i++)
//     {
//         auto& node = ordered[i];
//         auto depends = node->sources
//             | std::views::transform([](mute::Processor* node) { return node->name; });

//         fmt::println("eval {}: {}, depends on {{{}}}"
//             , i, node->name
//             , fmt::join(depends, ", "));

//         depcount[i] = node->sources.size();
//     }

//     while (!std::ranges::all_of(done, [](auto v) { return v == true; }))
//     {
//         int group = 0;
//         for (auto i = 0ull; i < ordered.size(); i++)
//         {
//             auto& node = ordered[i];
//             if (depcount[i] == 0)
//             {
//                 fmt::println("[{}] processing {}", group, node->name);
//                 done[i] = true; 

//                 for (auto k = 0ull; k < ordered.size(); k++)
//                 {
//                     auto& target = ordered[k];
//                     if (std::ranges::distance(target->sources
//                         | std::views::filter([node](auto ss) { return ss == node; })) > 0)
//                     {
//                         depcount[k]--;
//                     }
//                 }
//             }
//         }
//         group++;
//     }

//     return 0;
// }















// #include <libremidi/libremidi.hpp>
// #include <fmt/printf.h>
// #include <raylib.h>

// #include "audio.h"
// #include "utils.h"
// // #include "midi.h"

// #include "modules/oscillator.h"
// #include "modules/noise.h"
// #include "modules/adsr.h"

// #include <cassert>

// template <size_t N>
// constexpr auto init_array(const auto& value)
// {
//     std::array<std::remove_cvref_t<decltype(value)>, N> result;
//     for (size_t i = 0; i < N; i++)
//         result[i] = value;
//     return result;
// }

// template <size_t N, typename Fn>
// constexpr auto init_array_fn(Fn&& fn)
// {
//     std::array<std::remove_cvref_t<std::invoke_result_t<Fn, size_t>>, N> result;
//     for (size_t i = 0; i < N; i++)
//         result[i] = fn(i);
//     return result;
// }

// template <size_t N>
// struct Wavetable
// {
//     static constexpr uint32_t size = N;
//     static constexpr uint32_t mask = size - 1;

//     std::array<float, N> data;

//     constexpr Wavetable(float (*fn)(float phase))
//         : data(init_array_fn<N>([fn](size_t i) { return fn(2.0f * M_PI * float(i) / float(size)); })) {}

//     float value(float phase) const
//     {
//         float index = float(N) * phase / (2.0f * M_PI);
//         int i1 = uint32_t(index) & mask;
//         int i2 = (i1 + 1) & mask;
//         float frac = index - uint32_t(index);

//         float a = data[i1];
//         float b = data[i2];

//         return mute::lerp(a, b, frac);
//     }
// };

// struct Oscillator
// {
//     Wavetable<1024> wavetable = Wavetable<1024>([](float phase) { return std::sin(phase); });

//     float phase;
//     float increment;
//     float frequency;

//     float process(float sr)
//     {
//         phase = mute::rephase(phase, 2.0f * M_PI * frequency / sr);
//         return wavetable.value(phase);
//     }
// };

// template <int OscillatorCount>
// struct SwarmOscillator
// {
//     std::array<float, OscillatorCount> phase = {};
//     std::array<float, OscillatorCount> increment = {};
//     std::array<float, OscillatorCount> amplitude = init_array<OscillatorCount>(1.0f);

//     float frequency = 100.0f;
//     float spread = 1.0f;

//     void process(float* out, int frames, float sr)
//     {
//         for (int f = 0; f < frames; f++)
//         {
//             float sum = 0.0f;
//             for (int i = 0; i < OscillatorCount; i++)
//             {
//                 auto fq = frequency * (1.0f + i * spread);
//                 increment[i] = 2.0f * M_PI * fq / sr;
//                 amplitude[i] = 1.0f / float(i + 1);
//                 if (fq > sr / 2.0f)
//                     amplitude[i] = 0.0f;

//                 phase[i] += increment[i];
//                 if (phase[i] > 2.0f * M_PI) phase[i] -= 2.0f * M_PI;
//                 if (phase[i] < 0.0f) phase[i] += 2.0f * M_PI;
//                 float output = mute::fast::cos(phase[i]);
//                 sum += output * amplitude[i];
//             }
//             *out++ = sum;
//         }
//     }
// };

// template <int OscillatorCount>
// struct SwarmWaveOscillator
// {
//     Wavetable<1024> wavetable = Wavetable<1024>([](float phase) { return std::sin(phase); });
//     std::array<float, OscillatorCount> phase = {};

//     float frequency = 100.0f;
//     float spread = 1.0f;

//     float process(float sr)
//     {
//         const float twopi = 2.0f * M_PI;
//         const float twopi_sr = twopi / sr;
//         float out = 0.0f;

//         for (int i = 0; i < OscillatorCount; i++)
//         {
//             auto fq = frequency * (1.0f + i * spread);
//             float increment = fq * twopi_sr;
//             float amplitude = 1.0f / float(i + 1);
//             if (fq > sr / 2.0f)
//                 amplitude = 0.0f;

//             phase[i] = mute::rephase(phase[i], increment);
//             out += wavetable.value(phase[i]) * amplitude;
//         }
//         return out;
//     }
// };

// #include <complex>

// template <int OscillatorCount>
// struct SwarmOscillator2
// {
//     std::array<std::complex<float>, OscillatorCount> value = init_array<OscillatorCount>(std::complex<float>(1));
//     std::array<std::complex<float>, OscillatorCount> multiplier = init_array<OscillatorCount>(std::complex<float>(1));
//     std::array<float, OscillatorCount> amplitude = init_array<OscillatorCount>(1.0f);

//     float frequency = 100.0f;
//     float sampleRate = 44100.0f;

//     void setFrequency(float fq)
//     {
//         frequency = fq;
//         for (int i = 0; i < OscillatorCount; i++)
//         {
//             float f = frequency * (1.0f + i);
//             float increment = 2.0f * M_PI * (f / sampleRate);
//             multiplier[i] = { std::cos(increment), std::sin(increment) };
//             amplitude[i] = 1.0f / float(i + 1);
//             if (f > sampleRate / 2.0f)
//                 amplitude[i] = 0.0f;
//         }
//     }

//     void process(float* out, int frames)
//     {
//         for (int f = 0; f < frames; f++)
//         {
//             float sum = 0.0f;
//             for (int i = 0; i < OscillatorCount; i++)
//             {
//                 value[i] *= multiplier[i];
//                 sum += value[i].imag() * amplitude[i];
//             }
//             *out++ = sum;
//         }
//     }
// };

// namespace mute
// {
//     constexpr float scale(float normalized, float min, float max) { return (max - min) * normalized + min; }
//     constexpr float sq(float in) { return in * in; }

//     float random() { return float(rand()) / float(RAND_MAX); }
//     float random(float max) { return scale(random(), 0.0f, max); }
//     float random(float min, float max) { return scale(random(), min, max); }
// }

// int main(int argc, char** argv)
// {
//     auto audio = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2, .deviceName = "Haut-parleurs MacBook Pro" },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });


//     auto source = SwarmWaveOscillator<512> {};
//     auto lfo = Oscillator { .frequency = 0.3f };
//     // source.sampleRate = audio.internalConfiguration.sampleRate;
//     // std::array<float, 512> audioBuffer = {};

//     audio.audioCallback = [&](mute::AudioProcessData data)
//     {
//         // source.process(audioBuffer.data(), data.frames);
//         // source.process(audioBuffer.data(), data.frames, data.sampleRate);

//         for (int f = 0; f < data.frames; f++)
//         {
//             source.spread = 1.0f + lfo.process(data.sampleRate);
//             float out = source.process(data.sampleRate);
//             // float& out = audioBuffer[f];
//             out = std::clamp(out, -1.0f, 1.0f);
//             out *= 0.4f;
//             if (!isfinite(out))
//                 out = 0.0f;

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = out;
//         }
//     };

//     audio.start();
//     InitWindow(800, 600, "mute");

//     while (!WindowShouldClose())
//     {
//         if (IsKeyPressed(KEY_Z) || IsKeyPressedRepeat(KEY_Z))
//         {
//             auto freq = mute::random(20.0f, 1000.0f);
//             // source.setFrequency(freq);
//             source.frequency = freq;
//             fmt::println("FREQ: {:.4f}Hz", freq);
//         }

//         BeginDrawing();
//         EndDrawing();
//     }

//     CloseWindow();
//     audio.stop();
//     return 0;
// }

// namespace mute
// {
//     struct RisingEdgeDetector
//     {
//         float output;
//         float last = 0.f;
//         bool active = false;

//         void process(float in)
//         {
//             if (!active && in > last)
//             {
//                 active = true;
//                 last = in;
//                 output = 1.0f;
//                 return;
//             }

//             if (active && in < last)
//                 active = false;
            
//             last = in;
//             output = 0.0f;
//         }
//     };

//     struct Clock
//     {
//         int counter = 0;
//         int division = 4;
//         std::function<void()> ontick;

//         void reset() { counter = 0; }
//         void tick()
//         {
//             if (counter == 0)
//                 ontick();

//             counter = (counter + 1) % division;
//         }
//     };
// }

// int main(int argc, char** argv)
// {
//     auto context = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2 },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });

//     if (!context.valid())
//         exit(1);

//     mute::MidiIn midi;
//     midi.open("Arturia BeatStepPro");
    
//     mute::Voice voice {
//         .em1.amp = 1.0f,
//         .volume = 1.0f,
//         .cutoff = mute::pitchToFrequency(127)
//     };

//     midi.on.noteon = [&](auto chan, auto note, auto velo)
//     {
//         if (chan == 1)
//         {
//             voice.pitch = mute::pitchToFrequency(note);
//             voice.gate = 1.0f;
//             voice.em2.amp = 0.5f * (velo / 127.0f);
//         }
//     };

//     midi.on.noteoff = [&](auto chan, auto note)
//     {
//         if (chan == 1)
//         {
//             if (voice.pitch == mute::pitchToFrequency(note))
//             {
//                 voice.gate = 0.0f;
//             }
//         }
//     };

//     auto setVolume = [&](float v)
//     {
//         voice.volume = v;
//         fmt::println(" VOL | {:.2f}", voice.volume);
//     };
//     auto setEnveloppe = [&](mute::ADSR& env, float a, float d, float s, float r)
//     {
//         env.a = a;
//         env.d = d;
//         env.s = s;
//         env.r = r;
//         fmt::println("ADSR | {:.4f}ms / {:.4f}ms \\ {:.2f} — {:.4f}ms \\"
//             , 1e3*env.a
//             , 1e3*env.d
//             , env.s
//             , 1e3*env.r);
//     };
//     auto setEnveloppeMod = [&](mute::Voice::EnvMod& mod, float pitch, float wave, float cutoff, float res)
//     {
//         mod.osc.freq = pitch;
//         mod.osc.wave = wave;
//         mod.filter.freq = cutoff;
//         mod.filter.q = res;
//         fmt::println(" MOD | {:.2f} {:.2f} {:.2f}Hz {:.2f}"
//             , mod.osc.freq
//             , mod.osc.wave
//             , mod.filter.freq
//             , mod.filter.q);
//     };
//     auto setWave = [&](float wave)
//     {
//         voice.wave = wave;
//         fmt::println(" OSC | w{:.2f}", wave);
//     };
//     auto setFilter = [&](float cutoff, float resonance)
//     {
//         voice.cutoff = cutoff;
//         voice.resonance = resonance;
//         fmt::println("FILT | {:.2f}Hz {:.2f}", cutoff, resonance);
//     };
//     auto setFilterMode = [&](mute::BiquadFilter::Mode mode)
//     {
//         auto last = voice.filter.mode;
//         if (last != mode)
//         {
//             voice.filter.mode = mode;
//             std::string modestr;
//             switch (mode)
//             {
//                 case mute::BiquadFilter::Mode::LowPass: modestr = "LowPass"; break;
//                 case mute::BiquadFilter::Mode::HighPass: modestr = "HighPass"; break;
//                 case mute::BiquadFilter::Mode::BandPassQ: modestr = "BandPassQ"; break;
//                 case mute::BiquadFilter::Mode::BandPassPeak: modestr = "BandPassPeak"; break;
//                 case mute::BiquadFilter::Mode::Notch: modestr = "Notch"; break;
//                 case mute::BiquadFilter::Mode::AllPass: modestr = "AllPass"; break;
//                 case mute::BiquadFilter::Mode::PeakingEQ: modestr = "PeakingEQ"; break;
//                 case mute::BiquadFilter::Mode::LowShelf: modestr = "LowShelf"; break;
//                 case mute::BiquadFilter::Mode::HighShelf: modestr = "HighShelf"; break;
//                 default:
//                     break;
//             }
//             fmt::println("FILT | mode {}", modestr);
//         }
//     };

//     midi.on.cc = [&](auto chan, auto control, auto value)
//     {
//         // fmt::println("CC {:#x} {}", control, value);
//         float fvalue = value / 127.0f;
//         auto& env1 = voice.env1;
//         auto& env2 = voice.env2;
//         auto& em2 = voice.em2;
//         switch (control)
//         {
//             case 0x72: setVolume(std::pow(fvalue, 2.0f)); break;
//             case 0x12: setWave(fvalue); break;
//             case 0x13: setFilter(mute::pitchToFrequency(value), voice.resonance); break;
//             case 0x10: setFilter(voice.cutoff, fvalue * 20.f); break;

//             case 0x0a: setEnveloppe(env1, std::pow(fvalue, 2.0f), env1.d, env1.s, env1.r); break;
//             case 0x4a: setEnveloppe(env1, env1.a, std::pow(fvalue, 2.0f), env1.s, env1.r); break;
//             case 0x47: setEnveloppe(env1, env1.a, env1.d, fvalue, env1.r); break;
//             case 0x4c: setEnveloppe(env1, env1.a, env1.d, env1.s, std::pow(fvalue, 2.0f)); break;

//             // TODO: find actual CC 
//             case 0x4d: setEnveloppe(env2, std::pow(fvalue, 2.0f), env2.d, env2.s, env2.r); break;
//             case 0x5d: setEnveloppe(env2, env2.a, std::pow(fvalue, 2.0f), env2.s, env2.r); break;
//             case 0x49: setEnveloppe(env2, env2.a, env2.d, fvalue, env2.r); break;
//             case 0x4b: setEnveloppe(env2, env2.a, env2.d, env2.s, std::pow(fvalue, 2.0f)); break;

//             case 0x11: setEnveloppeMod(em2, mute::pitchToFrequency(value), em2.osc.wave, em2.filter.freq, em2.filter.q); break;
//             case 0x5b: setEnveloppeMod(em2, em2.osc.freq, fvalue, em2.filter.freq, em2.filter.q); break;
//             case 0x4f: setEnveloppeMod(em2, em2.osc.freq, em2.osc.wave, mute::pitchToFrequency(value), em2.filter.q); break;
//             case 0x48: setFilterMode(mute::BiquadFilter::Mode(fvalue * 9)); break;

//             default:
//                 break;
//         }
//     };

//     // midi.on.clock = [&]()
//     // {
//     //     clock.tick();
//     // };

//     // midi.on.stop = [&]()
//     // {
//     //     clock.reset();
//     // };

//     // clock.ontick = [&]()
//     // {
        
//     // };

//     context.audioCallback = [&](mute::AudioProcessData data)
//     {
//         auto sr = data.sampleRate;
//         midi.consumeMessages();
        
//         for (int f = 0; f < data.frames; f++)
//         {
//             // voice.pitch = mute::pitchToFrequency(35);
//             // voice.cutoff = mute::pitchToFrequency(100);
//             // voice.gate = 1.0f;
//             voice.process(sr);
//             float output = voice.output;

//             output = std::tanh(output);
//             output = std::clamp(output, -1.f, 1.f);

//             if (!std::isfinite(output))
//                 output = 0.0f;

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = output;            
//         }
//     };

//     context.start();
// #if defined(__APPLE__)
//     CFRunLoopRun();
// #else
//     getchar();
// #endif
//     context.stop();
    
//     return 0;
// }

// int main(int argc, char** argv)
// {
//     auto context = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2 },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });

//     if (!context.valid())
//         exit(1);

//     mute::MidiIn midi;
//     midi.open("Arturia BeatStepPro");

//     mute::Sequencerz sequencerz;
//     sequencerz.randomize(mute::Sequencerz::ALL);

//     mute::Oscillator osc { .frequency = 420.0f };
//     mute::ADSR env {
//         .a = 0.0f,
//         .d = 0.0f,
//         .s = 1.0f,
//         .r = 0.0f
//     };
//     mute::Clock clock {};

//     float wfmod = 0.0f;
//     // float wvmod = 0.0f;

//     midi.on.noteon = [&](auto chan, auto note, auto velo)
//     {
//         if (chan == 1)
//         {
//             osc.frequency = mute::pitchToFrequency(note);
//             env.gate = 1.0f;
//             wfmod = velo / 127.0f;
//             // wvmod = velo / 127.0f;
//         }

//         if (chan == 16)
//         {
//             switch (note)
//             {
//                 case 0x2c: sequencerz.randomize(mute::Sequencerz::BASIC); break;
//                 case 0x2d: sequencerz.randomize(mute::Sequencerz::EXT); break;
//                 default: break;
//             }
//         }
//     };

//     midi.on.noteoff = [&](auto chan, auto note)
//     {
//         if (chan == 1)
//         {
//             if (osc.frequency == mute::pitchToFrequency(note))
//                 env.gate = 0.0f;
//         }
//     };

//     float wavefolding = 1.0f;
//     float volume = 1.0f;
//     auto setVolume = [&](float v)
//     {
//         volume = v;
//         fmt::println(" VOL | {:.2f}", volume);
//     };
//     auto setEnveloppe = [&](float a, float d, float s, float r)
//     {
//         env.a = a;
//         env.d = d;
//         env.s = s;
//         env.r = r;
//         fmt::println("ADSR | {:.4f}ms / {:.4f}ms \\ {:.2f} — {:.4f}ms \\"
//             , 1e3*env.a
//             , 1e3*env.d
//             , env.s
//             , 1e3*env.r);
//     };
//     auto setOscParams = [&](float wave, float folding)
//     {
//         osc.wave = wave;
//         wavefolding = folding;
//         fmt::println(" OSC | w{:.2f} f{:.2f}", wave, folding);
//     };

//     midi.on.cc = [&](auto chan, auto control, auto value)
//     {
//         // fmt::println("CC {:#x} {}", control, value);
//         float fvalue = value / 127.0f;
//         switch (control)
//         {
//             case 0x72: setVolume(std::pow(fvalue, 2.0f)); break;
//             case 0x12: setOscParams(fvalue, wavefolding); break;
//             case 0x13: setOscParams(osc.wave, 10.0f * fvalue); break;
//             case 0x10:
//                 break;

//             case 0x0a: setEnveloppe(std::pow(fvalue, 2.0f), env.d, env.s, env.r); break;
//             case 0x4a: setEnveloppe(env.a, std::pow(fvalue, 2.0f), env.s, env.r); break;
//             case 0x47: setEnveloppe(env.a, env.d, fvalue, env.r); break;
//             case 0x4c: setEnveloppe(env.a, env.d, env.s, std::pow(fvalue, 2.0f)); break;

//             default:
//                 break;
//         }
//     };

//     midi.on.clock = [&]()
//     {
//         clock.tick();
//     };

//     midi.on.stop = [&]()
//     {
//         sequencerz.reset();
//         clock.reset();
//     };

//     clock.ontick = [&]()
//     {
//         sequencerz.next();
//     };

//     context.audioCallback = [&](mute::AudioProcessData data)
//     {
//         auto sampleRate = data.sampleRate;
//         midi.consumeMessages();
        
//         for (int f = 0; f < data.frames; f++)
//         {
//             sequencerz.process(sampleRate);
//             // osc.frequency = sequencerz.pitch.output();
//             // osc.wave = sequencerz.wave.output();
//             // env.gate = sequencerz.tg.output;
//             // osc.frequency = mute::pitchToFrequency(midi.notes[15].pitch);
//             // env.gate = midi.notes[15].on ? 1.0f : 0.0f;
//             osc.process(sampleRate);
//             env.process(sampleRate);
//             // osc.output *= mute::mvg(midi.notes[15].velocity);
//             // osc.output *= midi.notes[15].on ? 1.0f : 0.0f;
//             osc.output *= env.output;
//             // osc.output *= float(sequencerz.velocity.output());
//             osc.output = sin((wavefolding * (1.0f + 0.5f * wfmod)) * osc.output);
//             osc.output = mute::saturate(osc.output);
//             osc.output *= volume;
//             osc.output = mute::clamp(osc.output, -1.0f, 1.0f);

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = osc.output;            
//         }
//     };

//     context.start();
// #if defined(__APPLE__)
//     CFRunLoopRun();
// #else
//     getchar();
// #endif
//     context.stop();
    
//     return 0;
// }