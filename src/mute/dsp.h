#pragma once

#include <cmath>
#include <array>
#include <random>

namespace mute
{
    // ------------------------------------ CONSTANTS

    static constexpr double Pi = 3.14159265358979323846264;
    static constexpr double Tau = 2 * Pi;
    static constexpr double Epsilon = 1e-6; // arbitrary as 1 microsec

    // ------------------------------------ TRIGON

    /// @brief Fast cosine approximation
    /// @param x works between [-Tau, Tau]
    /// @return 
    constexpr inline auto cos(auto x) noexcept
    {
        using T = decltype(x);
        constexpr T tp = T(1) / T(Tau);
        x *= tp;
        x -= T(.25) + std::floor(x + T(.25));
        x *= T(16.) * (std::abs(x) - T(.5));
        #if EXTRA_PRECISION
        x += T(.225) * x * (std::abs(x) - T(1.));
        #endif
        return x;
    }

    /// @brief Fast sine approximation
    /// @param x works between [-Tau, Tau] (maybe, hahahha)
    /// @return 
    constexpr inline auto sin(auto x) noexcept { return cos(x - Pi / 2); }

    // ------------------------------------ UTILS

    constexpr const auto& max(const auto& value, const auto& bound) { return value > bound ? value : bound; }
    constexpr const auto& min(const auto& value, const auto& bound) { return value < bound ? value : bound; }
    constexpr const auto& clamp(const auto& value, const auto& low, const auto& high) { return max(min(value, high), low); }
    constexpr const auto& clamp(const auto& value) { return max(min(value, static_cast<decltype(value)>(1)), static_cast<decltype(value)>(-1)); }

    /// @brief Linearly interpolate value between a and b, using parameter t
    /// @param a left
    /// @param b right
    /// @param t parameter: [0, 1]
    /// @return something in between
    constexpr float lerp(const float& a, const float& b, float t) { return a * (1 - t) + b * t; }

    /// @brief Correct input to be in range [0, Tau[
    /// @param phase radians
    /// @return 
    constexpr float rephase(float phase)
    {
        while (phase > Tau) phase -= Tau;
        while (phase < 0) phase += Tau;
        return phase;
    }

    /// @brief Generate linearly mixed values from a single parameter
    /// @tparam ...Args 
    /// @param t crossfade parameter, [0, 1]
    /// @param ...interpolants pointers to crossfade values
    template <typename ... Args>
    constexpr void crossfaders(float t, Args* ... interpolants)
    {
        using Interp = std::common_type_t<Args...>;
        float tn = t * (sizeof...(Args) - 1);
        Interp* interps[] = { interpolants... };
        for (int i = 0; i < (int) sizeof...(Args); i++)
            *interps[i] = max(1 - std::abs(i - tn), 0.0f);
    }

    // ------------------------------------ AUDIO CONVERSIONS

    namespace frequency
    {
        // Maximum pleasure frequency range
        static constexpr double Low = 10;
        static constexpr double High = 96'000;
        constexpr auto denorm(const auto& in) { return in * (High - Low) + Low; }
        constexpr auto norm(const auto& in) { return (in - Low) / (High - Low); }

        // Safer frequency range according to that dude Nyquist
        static constexpr double LowSafe = 20;
        static constexpr double HighSafe = 22'000;   
        constexpr auto denormSafe(const auto& in) { return in * (HighSafe - LowSafe) + LowSafe; }
        constexpr auto normSafe(const auto& in) { return (in - LowSafe) / (HighSafe - LowSafe); }
    }

    /// @brief Convert from Hz to BPM
    /// @param in
    /// @return 
    constexpr float hz2bpm(float in) { return in * 60; }

    /// @brief Convert from BPM to Hz
    /// @param in 
    /// @return 
    constexpr float bpm2hz(float in) { return in / 60; }

    /// @brief Convert from midi pitch to actual frequency
    /// @param pitch in range [0, 256[
    /// @param centerFrequency in Hertz
    /// @return 
    inline float pitch2Hz(int pitch, float centerFrequency = 440)
    {
        return centerFrequency * std::pow(2, (pitch - 69) / 12);
    }

    /// @brief Returns closest (positive or negative) value that is not zero
    /// @param value 
    /// @return value or Epsilon or -Epsilon
    constexpr const auto& safe(const auto& value)
    {
        constexpr auto limit = std::numeric_limits<std::remove_cvref_t<decltype(value)>>::min();
        return value > 0 ? max(value, limit) : min(value, -limit);
    }

    /// @brief https://iquilezles.org/articles/functions/
    /// @param x input
    /// @param k gain strength [0, +inf]
    /// @return 
    inline float gain(float x, float k)
    {
        float a = 0.5 * std::pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
        return (x < 0.5) ? a : 1.0 - a;
    }

    // std::mt19937 RAND_ENGINE;

    // inline float random(float low, float high)
    // {
    //     using distribution = std::uniform_real_distribution<float>;
    //     return distribution(low, high)(RAND_ENGINE);
    // }

    inline float saturate(float in, float gain)
    {
        return tanh(gain * in);
    }

    // /// @brief Warp input 
    // /// @param x 
    // /// @param k 
    // /// @return 
    // auto warp(auto x, auto k)
    // {
    //     auto z = rephase(x) / Tau;
    //     auto a = 0.5 * std::pow((2.0 * (z < 0.5 ? z : (1 - z))), k);
    //     return 2 * Tau * (z < 0.5 ? a : 1.0 - a);
    // }

    // ------------------------------------ AUDIO

    enum class WaveShape
    {
        Sine,
        Triangle,
        Square,
        Sawtooth
    };

    inline float sawsine(float* phase, float frequency, float sampleRate, float k)
    {
        *phase = rephase(*phase + Tau * frequency / sampleRate);
        return sin(Tau*gain(fmodf(*phase/Tau, 1), k));
    }

    /// @brief Oscillator, advancing input phase
    /// @param phase inout, radians: [0, Tau[
    /// @param frequency Hz
    /// @param sampleRate Hz 
    /// @return Wobbling output
    constexpr float oscillate(float* phase, float frequency, float sampleRate, WaveShape shape = WaveShape::Sine)
    {
        float ph = *phase;
        *phase = rephase(*phase + Tau * frequency / sampleRate);
        switch (shape)
        {
            case WaveShape::Sine: return sin(ph);
            case WaveShape::Sawtooth: return -ph / Pi - 1;
            case WaveShape::Square: return ph < Pi ? 1 : -1;
            case WaveShape::Triangle: {
                return 0.f;
                // return ph < Pi ? (2*saw(ph) - 1) : (-2*saw(ph) - 1);
            }
        }
    }
}