#pragma once

#include <math_approx/math_approx.hpp>

namespace mute
{
    static constexpr double Pi = 3.14159265358979323846264;
    static constexpr double Tau = 2 * Pi;
    static constexpr double Epsilon = 1e-6; // arbitrary as 1 microsec

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

    constexpr auto ipow(const auto& in, const std::integral auto& exponent)
    {
        auto out = (decltype(in)) 1;
        for (int i = 0; i < exponent; i++)
            out *= in;
        return out;
    }

    constexpr auto cos(auto x) { return math_approx::cos<5>(x); }
    constexpr auto sin(auto x) { return math_approx::sin<5>(x); }
    constexpr auto tan(auto x) { return math_approx::tan<3>(x); }

    constexpr auto acos(auto x) { return math_approx::acos<3>(x); }
    constexpr auto asin(auto x) { return math_approx::asin<3>(x); }
    constexpr auto atan(auto x) { return math_approx::atan<3>(x); }
    constexpr auto atanh(auto x) { return math_approx::atanh<3>(x); }

    constexpr auto cosh(auto x) { return math_approx::cosh<3>(x); }
    constexpr auto sinh(auto x) { return math_approx::sinh<3>(x); }
    constexpr auto tanh(auto x) { return math_approx::tanh<3>(x); }

    constexpr auto sigmoid(auto x) { return math_approx::sigmoid<3>(x); }
    constexpr auto exp(auto x) { return math_approx::exp<3>(x); }
    constexpr auto log(auto x) { return math_approx::log<3>(x); }
    constexpr auto pow(auto base, auto exponent) { return exp(exponent * log(base)); }

    constexpr auto sqrt(auto x) { return std::sqrt(x); }
    constexpr auto rsqrt(auto x) { return fast_rsqrt_carmack(x); }
}