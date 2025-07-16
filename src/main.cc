// #include "app.h"

// int main(int argc, char** argv)
// {
//     {
//         auto app = MuteApp();
//         app.run();
//     }
//     return 0;
// }

#include <cmath>
#include <algorithm>

#include <variant>
#include <string>
#include <array>
#include <vector>

#include <math_approx/math_approx.hpp>

// Constants
namespace mute
{
    static constexpr double Pi = 3.14159265358979323846264;
    static constexpr double Tau = 2 * Pi;
    static constexpr double Epsilon = 1e-6; // arbitrary as 1 microsec
}

// RSQRT
namespace mute
{
    template<typename To, typename From>
    struct unsafe_bit_cast_t {
        union {
            From from;
            To to;
        };
    };

    template<typename To, typename From>
    To unsafe_bit_cast(From from) {
        unsafe_bit_cast_t<To, From> u;
        u.from = from;
        return u.to;
    }


    static inline float fast_rsqrt_carmack(float x) {
        uint32_t i;
        float x2, y;
        const float threehalfs = 1.5f;
        y = x;
        i = unsafe_bit_cast<uint32_t, float>(y);
        i = 0x5f3759df - (i >> 1);
        y = unsafe_bit_cast<float, uint32_t>(i);
        x2 = x * 0.5f;
        y = y * (threehalfs - (x2 * y * y));
        return y;
    }

    static inline float fast_rsqrt_accurate(float fp0) {
        float _min = 1.0e-38;
        float _1p5 = 1.5;
        float fp1, fp2, fp3;

        uint32_t q = unsafe_bit_cast<uint32_t, float>(fp0);
        fp2 = unsafe_bit_cast<float, uint32_t>(0x5F3997BB - ((q >> 1) & 0x3FFFFFFF));
        fp1 = _1p5 * fp0 - fp0;
        fp3 = fp2 * fp2;
        if (fp0 < _min) {
            return fp0 > 0 ? fp2 : 1000.0f;
        }
        fp3 = _1p5 - fp1 * fp3;
        fp2 = fp2 * fp3;
        fp3 = fp2 * fp2;
        fp3 = _1p5 - fp1 * fp3;
        fp2 = fp2 * fp3;
        fp3 = fp2 * fp2;
        fp3 = _1p5 - fp1 * fp3;
        return fp2 * fp3;
    }
}

// Utils
namespace mute
{
    constexpr auto ipow(const auto& in, const std::integral auto& exponent)
    {
        auto out = (decltype(in)) 1;
        for (int i = 0; i < exponent; i++)
            out *= in;
        return out;
    }
    
    constexpr const auto& max(const auto& value, const auto& bound) { return value > bound ? value : bound; }
    constexpr const auto& min(const auto& value, const auto& bound) { return value < bound ? value : bound; }
    constexpr const auto& clamp(const auto& value, const auto& low, const auto& high) { return max(min(value, high), low); }
    constexpr const auto& clamp(const auto& value) { return clamp(value, static_cast<decltype(value)>(1), static_cast<decltype(value)>(-1)); }

    constexpr auto cos(auto x) { return math_approx::cos<5>(x); }
    constexpr auto sin(auto x) { return math_approx::sin<5>(x); }
    constexpr auto tan(auto x) { return math_approx::tan<3>(x); }

    constexpr auto exp(auto x) { return math_approx::exp<3>(x); }
    constexpr auto log(auto x) { return math_approx::log<3>(x); }
    constexpr auto pow(auto base, auto exponent) { return exp(exponent * log(base)); }

    constexpr auto sqrt(auto x) { return std::sqrt(x); }
    constexpr auto rsqrt(auto x) { return fast_rsqrt_carmack(x); }
}

namespace mute
{
    /// @brief Linearly interpolate value between a and b, using parameter t
    /// @param a left
    /// @param b right
    /// @param t parameter: [0, 1]
    /// @return something in between
    constexpr float lerp(const float& a, const float& b, float t) { return a * (1 - t) + b * t; }

    /// @brief Correct input to be in range [-pi, pi] ~[0, Tau[~
    /// @param phase radians
    /// @return 
    constexpr float rephase(float phase)
    {
        return math_approx::trig_detail::fast_mod_mpi_pi(phase);
        // return fmodf(phase, mute::Tau);
        // while (phase > Tau) phase -= Tau;
        // while (phase < 0) phase += Tau;
        // return phase;
    }

    inline float pitch2Hz(int pitch, float centerFrequency = 440)
    {
        return centerFrequency * mute::pow(2.f, (pitch - 69.f) / 12.f);
    }

    /// @brief Returns closest (positive or negative) value that is not zero
    /// @param value 
    /// @return value or Epsilon or -Epsilon
    constexpr inline float safe(float value)
    {
        constexpr auto limit = std::numeric_limits<float>::min();
        return value > 0 ? max(value, limit) : min(value, -limit);
    }

    inline float gain(float x, float k)
    {
        float a = 0.5 * mute::pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
        return (x < 0.5) ? a : 1.0 - a;
    }
}

// Mapping
namespace mute
{
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    struct LinearMapping
    {
        float low = 0, high = 1;
        float normalize(float in) const { return in * (high - low) + low; }
        float denormalize(float in) const { return (in - low) / (high - low); }
    };

    struct ExponentialMapping
    {
        float low = 0, high = 1, gamma = 1;
        float normalize(float in) const { return mute::pow(in, gamma) * (high - low) + low; }
        float denormalize(float in) const { return mute::pow((in - low) / (high - low), 1.f / gamma); }
    };

    struct SteppedMapping
    {
        int count = 1;
        float normalize(float in) const { return in / count; }
        float denormalize(float in) const { return std::floor(in * count); }
    };

    struct Mapping
    {
        using Variant = std::variant<SteppedMapping, LinearMapping, ExponentialMapping>;
        Variant variant = LinearMapping { 0.0f, 1.0f };

        float denormalize(float in) const
        {
            return std::visit(overloaded {
                [in](const auto& mapping) { return mapping.denormalize(in); }
            }, variant);
        }

        float normalize(float in) const
        {
            return std::visit(overloaded {
                [in](const auto& mapping) { return mapping.normalize(in); }
            }, variant);
        }
    };
}

// // Attrib
// namespace mute 
// {
//     struct Normalized { float value; };

//     struct Attrib
//     {
//         float value;
//         std::string name;
//         Mapping mapping;

//         Attrib& operator=(float v) { value = v; return *this; }
//         Attrib& operator=(Normalized v) { value = mapping.denormalize(v.value); return *this; }
//         operator float() const { return value; }
//         operator Normalized() const { return { mapping.normalize(value) }; }
//     };
// }

struct Phazor
{
    float frequency = 440.f; // 20, 192000
    float output = 0.f;

    void process(float sr)
    {
        output = mute::rephase(output + frequency * mute::Tau / mute::safe(sr));
    }
};

struct Mixer
{
    struct Channel
    {
        float volume = 1.0f;
        float input;
    };

    std::vector<Channel> channels;
    float output;

    void process()
    {
        output = 0.0f;
        for (auto& chan: channels)
            output += chan.input * chan.volume;
    }
};

float qFromDecay(float frequency, float decay, float sampleRate)
{
    auto rad = mute::pow(10.f, -3.f / (decay * sampleRate));
    auto BW = mute::log(rad) / -mute::Pi / (1.f / sampleRate);
    auto Q = frequency / BW;
    return Q;
}

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

struct EdgeDetector
{
    float threshold = 0.5;
    float input;

    bool rising = false;
    bool falling = false;
    bool up = false;

    void process(float sr)
    {
        bool before = up;
        up = input > threshold;
        rising = up && before != up;
        falling = !up && before != up;
    }
};

struct Waveloppe
{
    std::vector<float> points;
    float t = 1000;
    float time = 1;
    float skew = 1;
    float gate;
    float output;
    EdgeDetector edgeDetector;

    void trigger() { t = 0; }
    void process(float sr)
    {
        edgeDetector.input = gate;
        edgeDetector.process(sr);
        if (edgeDetector.rising)
            trigger();

        float fi = mute::gain(t, skew) / time * points.size();
        if (fi < int(points.size()) - 1)
        {
            int i1 = fi;
            int i2 = fi + 1;
            float frac = fi - i1;
            float p1 = points[i1];
            float p2 = points[i2];
            output = mute::lerp(p1, p2, frac);
        } else 
            output = 0;

        t += 1/sr;
        if (t > time)
        {
            t = time + 1;
            output = 0;
        }
    }

    static Waveloppe triangle(float time, float skew) { return Waveloppe { .points = { 0, 1, 0 }, .time = time, .skew = skew }; }
    static Waveloppe fall(float time, float skew) { return Waveloppe { .points = { 1, 0 }, .time = time, .skew = skew }; }
    static Waveloppe rise(float time, float skew) { return Waveloppe { .points = { 0, 1 }, .time = time, .skew = skew }; }
};

#include <fmt/printf.h>

#include "mute/driver.h"

#include "raylib.h"

static constexpr Color BackgroundColor = Color { 102, 102, 102, 255 };
static constexpr Color ShadowColor = Color { 0, 0, 0, 255 };
static constexpr Color ForegroundColor = Color { 245, 76, 0, 255 };
static constexpr Color HintColor = Color { 255, 255, 255, 255 };

struct MIDIKeyboard
{
    //  1 3   6 8 10
    // 0 2 4 5 7 9 11 12
    // 
    //  z e   t y u
    // q s d f g h j k
    //
    //  w x -> octave shifts

    int octave = 4;
    std::array<bool, 13> status = {};
    std::vector<int> playedNotes = std::vector<int>(13);

    std::string_view keyNoteName(int key)
    {
        return std::array {
            "C", "C#",
            "D", "D#",
            "E",
            "F", "F#",
            "G", "G#",
            "A", "A#",
            "B",
            "C"
        }[key];
    }

    void process()
    {
        int index = 0;
        #define k(key) status[index++] = IsKeyPressed(KEY_ ## key)
        k(A); k(W); k(S); k(E); k(D); k(F); k(T); k(G); k(Y); k(H); k(U); k(J); k(K);
        #undef k

        if (IsKeyPressed(KEY_Z)) { octave--; fmt::println("-- octave down ({})", octave); }
        if (IsKeyPressed(KEY_X)) { octave++; fmt::println("-- octave up ({})", octave); }
        octave = std::clamp(octave, 0, 20);

        int pressedKeys = 0;
        for (int i = 0; i < (int) status.size(); i++)
        {
            if (status[i])
            {
                playedNotes.push_back(i + octave * 12);
                pressedKeys++;
            }
        }

        playedNotes.resize(pressedKeys);
    }
};

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

struct GrainGenerator
{

};

// #define USE_FILTERBANK

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
        Svf filter;
        float amplitude;
    };
    std::array<Partial, PartialsCount> partials;
#endif

    Waveloppe impulse = Waveloppe::triangle(0.01, 1.0);
    Waveloppe noiseEnv = Waveloppe::triangle(0.9, 0.3);
    Waveloppe env = Waveloppe::fall(0.1, 1.0);

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
            p.filter.q = qFromDecay(partialFrequency, decay, sr);
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

#include <fmt/std.h>

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
    EuclideanSequencer seq;
    std::string msg;
    EdgeDetector ed;

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

int main(int argc, char** argv)
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
    InitWindow(512, 512, "mute");
    SetTargetFPS(120);

    auto keyboard = MIDIKeyboard {};
    int lastTriggered = 0;
    auto synth = Polyphonic<Modal, 8> {};

    auto clk = Phazor { .frequency = 8.f };
    auto drummers = std::array {
        Drummer { synth, .freq = 37, { .length = 9, .increment = 5 }, .msg = "BONG" },
        Drummer { synth, .freq = 74, { .length = 23, .increment = 4 }, .msg = ">---bNgg" },
        Drummer { synth, .freq = 123, { .length = 37, .increment = 3 }, .msg = ">-->->>>->king" },
        Drummer { synth, .freq = 236, { .length = 38, .increment = 7 }, .msg = ">->->->>>>------bouW" },
        Drummer { synth, .freq = 778, { .length = 48, .increment = 9 }, .msg = ">>>>->>>->->->>>>>>>>iiiu" }
    };

    struct {
        Phazor A = Phazor { .frequency = 0.3f };
        Phazor B = Phazor { .frequency = 0.5f };
        Phazor C = Phazor { .frequency = 0.1f };
        Phazor D = Phazor { .frequency = 0.08f };
    } lfos;

    auto driver = mute::Driver {
        .configuration = {
            .playback = mute::Device { .channels = 2 },
            .bufferSize = 64,
            .sampleRate = 48000
        },
        .audioCallback = [&](mute::ProcessContext pc)
        {
            float sr = pc.sampleRate;

            for (int f = 0; f < pc.frames; f++)
            {
                float l = 0, r = 0;

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
                    // voice.process(sr);
                    // l += voice.output.left;
                    // r += voice.output.right;
                }

                synth.process(sr);
                l = synth.output.left;
                r = synth.output.right;

                float gain = 0.9f;
                l = mute::clamp(tanh(l * gain), -1.f, 1.f);
                r = mute::clamp(tanh(r * gain), -1.f, 1.f);
                pc.playback.buffer[2 * f + 0] = l;
                pc.playback.buffer[2 * f + 1] = r;
            }
        }
    };
    driver.init();
    driver.start();
    
    while (!WindowShouldClose())
    {
        keyboard.process();

        // for (const auto& note: keyboard.playedNotes)
        // {
        //     auto index = lastTriggered;
        //     modalVoices[index].trigger();
        //     modalVoices[index].frequency = mute::pitch2Hz(note);
        //     lastTriggered = (lastTriggered + 1) % modalVoices.size();
        // }
    
        BeginDrawing();
            ClearBackground(BackgroundColor);
        EndDrawing();
    }

    driver.stop();
    driver.uninit();

    CloseWindow();
    return 0;
}