#pragma once

#include "constants.h"
#include "mute/driver.h"
#include "mute/transport.h"
#include "mute/dsp.h"
#include "mute/random.h"
#include "mute/debug.h"

#include "mute/modules/oscillator.h"
#include "mute/modules/perc.h"
#include "mute/modules/sequencer.h"

#include "ui.h"

#include <raylib.h>
#include <fmt/printf.h>

static constexpr int DefaultSamplerate = 44100;
static constexpr int DefaultBuffersize = 256;

struct WavEncoder {
    ma_encoder mae;
    std::string name;
    std::array<int16_t, 256*2> buffer;

    WavEncoder() = default;
    WavEncoder(std::string_view name) { init(name); }
    ~WavEncoder() { uninit(); }

    bool init(std::string_view name)
    {
        this->name = name;
        ma_encoder_config config = ma_encoder_config_init(ma_encoding_format_wav, ma_format_s16, 2, 44100);
        ma_result result = ma_encoder_init_file(fmt::format("{}.wav", name).c_str(), &config, &mae);
        if (result != MA_SUCCESS) {
            mute::log::error(fmt::format("Error initializing WAV encoder '{}'", name));
            return false;
        }
        return true;
    }
    void uninit()
    {
        ma_encoder_uninit(&mae);
    }

    void write(std::span<const float> values)
    {
        static constexpr float S16Max = std::numeric_limits<int16_t>::max();
        static constexpr float S16Min = std::numeric_limits<int16_t>::min();
        uint64_t framesWritten = 0;
        for (size_t k = 0; k < values.size(); k++)
            buffer[k] = values[k] * (values[k] > 0.f ? S16Max : -S16Min);
        ma_encoder_write_pcm_frames(&mae, &buffer, values.size() / 2, &framesWritten);
    }
};

template <typename T, size_t Size>
struct HistoryBuffer
{
    std::array<T, Size> data;
    std::atomic<size_t> head = 0;
    size_t size = Size;

    void write(T&& v) { data[head++ % size] = std::forward<T>(v); }
    void write(const T& v) { data[head++ % size] = v; }
    void write(std::span<const T> values) { for (const auto& v: values) write(v); }

    void read(std::span<T> values) const
    {
        size_t offset = (head - values.size()) % size;
        for (auto& v: values)
            v = data[offset++ % size];
    }
};

struct OldPatch
{
    float volume = 1.0;
    float panning = 0.5;

    mute::EuclideanSequencer seq {
        .increment = 3,
        .length = 8,
        .gatelen = 0.9
    };
    mute::Perc perc {
        .noiseamt = 0.8,
        .pitchmod = 0.009,
        .filterfreq = 400,
        .filterenv = 0.03,
        .env1 = mute::Waveloppe::fall(0.4, 0.4),
        .env2 = mute::Waveloppe::triangle(0.4, 0.4),
    };
    mute::SineOscillator lfo { .frequency = 0.1f };
    mute::RandomNumberGenerator RNG { .chainSize = 32, .randomWalkSize = 2.0 };

    struct {
        float length;
        float increment;

        float lfo;
        float env1_time;
        float env1_skew;
        float env2_time;
        float env2_skew;
        float filterenv;
        float pitchmod;
        float filterfreq;
        float oscfreq;
        float noiseamt;
        float filterq;

        float tf;

        constexpr auto sq(const auto& in) { return in * in; }

        void randomize(mute::RandomNumberGenerator& RNG)
        {
            length = RNG.next(3, 16);
            increment = RNG.next(1, 8);

            lfo = RNG.next(0.03, 0.04);
            env1_time = RNG.next(0.1, 0.4);
            env1_skew = RNG.next(0.01, 1.3);
            env2_time = sq(RNG.next(0.1, 0.8));
            env2_skew = RNG.next(0.1, 1.7);
            filterenv = RNG.next(0.f, 0.3f);
            pitchmod = std::pow(RNG.next(0.f, 0.2f), 2.);
            filterfreq = RNG.next(50.f, 8200.f);
            oscfreq = std::pow(RNG.next(), 4.) * 200.f + 50;
            noiseamt = sq(RNG.next(0., 0.7));

            filterq = RNG.next(0.4, 3.0);
            tf = RNG.next();
        }
    } params[2];
    int side = 0;
    float t = 0;

    float output = 0;

    void process(float sr, mute::Transport transport)
    {
        // if (transport.measure.ticks.bar)
        // {
        //     RNG.seed();
        //     params[side].randomize(RNG);
        //     side = 1 - side;
        //     t = 0;
        // }

        // lfo.process(sr);
        // float tl = std::pow(lfo.output * 0.5 + 0.5, 0.1);
        float tl = t;

        // float tl = t*t;

        lfo.frequency = mute::lerp(params[side].lfo, params[1 - side].lfo, tl);
        seq.length = mute::lerp(params[side].length, params[1 - side].length, 0.f);
        seq.increment = mute::lerp(params[side].increment, params[1 - side].increment, 0.f);
        perc.env1.time = mute::lerp(params[side].env1_time, params[1 - side].env1_time, tl);
        perc.env1.skew = mute::lerp(params[side].env1_skew, params[1 - side].env1_skew, tl);
        perc.env2.time = mute::lerp(params[side].env2_time, params[1 - side].env2_time, tl);
        perc.env2.skew = mute::lerp(params[side].env2_skew, params[1 - side].env2_skew, tl);
        perc.filterenv = mute::lerp(params[side].filterenv, params[1 - side].filterenv, tl);
        perc.pitchmod = mute::lerp(params[side].pitchmod, params[1 - side].pitchmod, tl);
        perc.filterfreq = mute::lerp(params[side].filterfreq, params[1 - side].filterfreq, tl);
        perc.oscfreq = mute::lerp(params[side].oscfreq, params[1 - side].oscfreq, 0);
        perc.noiseamt = mute::lerp(params[side].noiseamt, params[1 - side].noiseamt, tl);
        perc.filter.q = mute::lerp(params[side].filterq, params[1 - side].filterq, tl);

        float tf = mute::lerp(params[side].tf, params[1 - side].tf, tl);
        if (tf < 0.7)
            perc.env1.points = { 1, 0 };
        else if (tf < 0.9)
            perc.env1.points = { 0, 1, 0 };
        else if (tf < 1.) {
            // perc.env1.points = { 0, 0.3, 0.1, 1, 1, 0.4, 0., 0.8, 0 };
            perc.env1.points = { 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1 };
        }

        perc.env2.points = perc.env1.points;

        if (transport.measure.ticks._16th)
            seq.next();
        
        seq.process(sr);
        perc.gate = seq.output;
        perc.process(sr);
        output = mute::saturate(volume * perc.output, 0.8f + tf * 0.3f);

        transport.update(1.f / float(sr));
        // t += (1.f / sr) / (transport.bpm / 60.f);
    }
};

struct Steps
{
    int count;
};

struct Range
{
    float min;
    float max;
    float gamma;

    Range(float min, float max, float gamma = 1)
        : min(min)
        , max(max)
        , gamma(gamma) {}

    Range(const Steps& steps)
        : min(0)
        , max(steps.count - 1)
        , gamma(1) {}
};

struct Normalized { float value; };

enum class ProxyMode { Const, Mutable };

template <typename T>
static constexpr ProxyMode proxy_mode = std::is_const_v<T> ? ProxyMode::Const : ProxyMode::Mutable;

template <ProxyMode Mode = ProxyMode::Mutable>
struct Attrib
{
    using Pointer = std::conditional_t<Mode == ProxyMode::Const, const float*, float*>;
    std::string name;
    Pointer ptr;
    Range range;

    Attrib(std::string_view name, Pointer ptr, Range&& range)
        : name(name)
        , ptr(ptr)
        , range(range) {}

    Attrib& operator=(float v) requires(Mode == ProxyMode::Mutable) { *ptr = v; return *this; }
    Attrib& operator=(Normalized v) requires(Mode == ProxyMode::Mutable) { *ptr = denormalize(v.value); return *this; }

    operator float() const { return *ptr; }
    operator Normalized() const { return { normalize(*ptr) }; }

    float denormalize(float in) const
    {
        return std::pow(in, range.gamma) * (range.max - range.min) + range.min;
    }

    float normalize(float in) const
    {
        return std::pow((in - range.min) / (range.max - range.min), 1.f / range.gamma);
    }
};

template <typename Float>
Attrib(std::string_view, Float*, Range&&) -> Attrib<proxy_mode<Float>>;

template <ProxyMode Mode = ProxyMode::Mutable>
struct Module
{
    std::string name;
    std::vector<Attrib<Mode>> attributes;
    std::vector<Module<Mode>> modules;

    template <typename M>
    Module(M* m)
        : name(m->name)
    {
        auto attr = m->attributes();
        auto mods = m->modules();
        attributes = std::vector(std::begin(attr), std::end(attr));
        modules = std::vector(std::begin(mods), std::end(mods));
    }
};

template <typename M>
Module(M* m) -> Module<proxy_mode<M>>;

#define DeclareNoModules() \
    template <typename Self> \
    auto modules(this Self& self) \
    { \
        return std::array<Module<proxy_mode<Self>>, 0> {}; \
    }
   
#define DeclareNoAttributes() \
    template <typename Self> \
    auto attributes(this Self& self) \
    { \
        return std::array<Attrib<proxy_mode<Self>>, 0> {}; \
    }

#define DeclareModules(...) \
    template <typename Self> \
    auto modules(this Self& self) \
    { \
        return std::array { \
            __VA_ARGS__ \
        }; \
    }

#define DeclareAttributes(...) \
    template <typename Self> \
    auto attributes(this Self& self) \
    { \
        return std::array { \
            __VA_ARGS__ \
        }; \
    }

struct Oscillator
{
    std::string name = "oscillator";
    float phase = 0;
    float output = 0;
    float frequency = 440;
    float waveshape = 0;

    DeclareAttributes(
        Attrib("frequency", &self.frequency, Range(10, 192000, 2)),
        Attrib("waveshape", &self.waveshape, Steps(3)),
    )
    DeclareNoModules()

    void process(float sr)
    {
        phase = mute::rephase(phase + mute::Tau * frequency / sr);
        output = mute::sin(phase);
    }
};

struct SwarmOscillator
{
    std::string name = "swarm-oscillator";
    std::array<Oscillator, 4> oscillators;

    DeclareNoAttributes()
    DeclareModules(
        Module(&self.oscillators[0]),
        Module(&self.oscillators[1]),
        Module(&self.oscillators[2]),
        Module(&self.oscillators[3])
    )
};

struct Patch
{
    SwarmOscillator osc;

    void print()
    {
        fmt::println("{}", osc.name);
        for (const auto& attr: osc.attributes())
            fmt::println("\t- {}: {}", attr.name, ((Normalized) attr).value);
        for (const auto& mod: osc.modules())
        {
            fmt::println("\t- {}:", mod.name);
            for (const auto& attr: mod.attributes)
                fmt::println("\t\t- {}: {}", attr.name, ((Normalized) attr).value);
        }
    }

    Patch()
    {
        print();
    }

    void process(float sr, mute::Transport transport)
    {
        
    }
};

struct MuteApp
{
    UI ui;
    struct {
        mute::Driver driver;
        mute::Transport transport;
        HistoryBuffer<float, 4 * (1<<16) * 2> history;

        Patch patch;
        WavEncoder encoder;
    } audio;

    MuteApp()
    {
        {
            SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
            InitWindow(1400, 700, "mute");
            SetTargetFPS(120);

            audio.history.size = (2*65536) * 2;
            ui.font120 = LoadFontEx(PPSupplyMonoRegular_FontPath, 120, nullptr, 0);
            ui.font30 = LoadFontEx(PPSupplyMonoRegular_FontPath, 30, nullptr, 0);
            ui.cursor = {
                .dpi = GetWindowScaleDPI().x
            };
            ui.app = this;
        }

        {
            audio.transport.bpm = 150.f;
            audio.transport.playing = true;
            audio.driver = mute::Driver {
                .configuration = {
                    .playback = mute::Device {
                        .channels = 2
                    },    
                    .sampleRate = 44100,
                    .bufferSize = 128,
                },
                .audioCallback = [&](mute::ProcessContext data) { renderAudio(data); }
            };
            audio.driver.init();
            audio.driver.start();
        }
    }

    ~MuteApp()
    {
        audio.driver.stop();
        audio.driver.uninit();
        CloseWindow();
    }

    int run()
    {
        while (!WindowShouldClose())
        {
            // PollInputEvents();

            if (IsKeyPressed(KEY_Z)) { audio.history.size /= 2; fmt::println("zoom"); }
            if (IsKeyPressed(KEY_E)) { audio.history.size *= 2; fmt::println("dezoom"); }
            // if (IsKeyPressed(KEY_A)) { randomize(); fmt::println("randomize"); }

            // if (IsKeyDown(KEY_U))
            // {
            //     audio.common_t = mute::clamp(audio.common_t + 0.01f, 0.f, 1.f);
            //     for (auto& p: audio.patches)
            //         p.t = audio.common_t;
            // }
            // if (IsKeyDown(KEY_J))
            // {
            //     audio.common_t = mute::clamp(audio.common_t - 0.01f, 0.f, 1.f);
            //     for (auto& p: audio.patches)
            //         p.t = audio.common_t;
            // }
            
            BeginDrawing();
                ClearBackground(BackgroundColor);
                renderUI();
            EndDrawing();
        }
        return 0;
    }

    void renderUI()
    {
        ui.draw();
    }

    void renderAudio(mute::ProcessContext data)
    {
        for (int f = 0; f < data.frames; f++)
        {
            float left = 0.f;
            float right = 0.f;
            
            audio.patch.process(data.sampleRate, audio.transport);

            left = mute::clamp(tanh(left * 1.0f), -1.f, 1.f);
            right = mute::clamp(tanh(right * 1.0f), -1.f, 1.f);

            data.playback.buffer[f * 2 + 0] = left;
            data.playback.buffer[f * 2 + 1] = right;
            audio.transport.update(1.f / float(data.sampleRate));
        }

        audio.history.write(data.playback.buffer);
        // audio.encoders[0].write(data.playback.buffer);
    }
};