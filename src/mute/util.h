#pragma once

#include "mute/math.h"

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

    /// @brief Returns closest (positive or negative) value that is not zero
    /// @param value 
    /// @return value or Epsilon or -Epsilon
    constexpr inline float safe(float value)
    {
        constexpr auto limit = std::numeric_limits<float>::min();
        return value > 0 ? max(value, limit) : min(value, -limit);
    }

    /// @brief https://iquilezles.org/articles/functions/
    /// @param x input
    /// @param k gain strength [0, +inf]
    /// @return 
    inline float gain(float x, float k)
    {
        float a = 0.5 * mute::pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
        return (x < 0.5) ? a : 1.0 - a;
    }

    /// @brief Convert from midi pitch to actual frequency
    /// @param pitch in range [0, 256[
    /// @param centerFrequency in Hertz
    /// @return 
    inline float midiNoteFrequency(int pitch, float centerFrequency = 440)
    {
        return centerFrequency * mute::pow(2.f, (pitch - 69.f) / 12.f);
    }
}