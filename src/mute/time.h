#pragma once

#include <cstdint>
#include <chrono>

namespace mute
{
    struct Time
    {
        int64_t value = 0;

        constexpr Time() = default;

        constexpr Time(double v) { value = int64_t(v * 1e9); }
        constexpr Time(int64_t v) { value = v; }
        constexpr operator double() const { return s(); }
        constexpr operator int64_t() const { return value; }

        template <typename Clock, typename Duration>
        constexpr Time(std::chrono::time_point<Clock, Duration> tp)
        {
            value = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
        }

        template <typename Rep, typename Period>
        constexpr Time(std::chrono::duration<Rep, Period> d)
        {
            value = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
        }

        template <typename Clock, typename Duration>
        constexpr operator std::chrono::time_point<Clock, Duration>() const
        {
            return std::chrono::time_point<Clock>(std::chrono::nanoseconds(value));
        }

        template <typename Rep, typename Period>
        constexpr operator std::chrono::duration<Rep, Period>() const
        {
            using Duration = std::chrono::duration<Rep, Period>;
            return std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(value));
        }

        constexpr double h() const { return s() / 3600; }
        constexpr double m() const { return s() / 60; }
        constexpr double s() const { return double(value) / 1e9; }
        constexpr double ms() const { return double(value) / 1e6; }
        constexpr double us() const { return double(value) / 1e3; }
        constexpr double ns() const { return double(value); }

        static constexpr Time h(double v) { return Time::s(v * 3600); }
        static constexpr Time m(double v) { return Time::s(v * 60); }
        static constexpr Time s(double v) { return Time(int64_t(v * 1e9)); }
        static constexpr Time ms(double v) { return Time(int64_t(v * 1e6)); }
        static constexpr Time us(double v) { return Time(int64_t(v * 1e3)); }
        static constexpr Time ns(double v) { return Time(int64_t(v)); }

        static constexpr Time zero() { return Time(0ll); }

        static Time now();
        static Time programStartTime();
        static Time programElapsedTime();

        friend constexpr auto operator<=>(const Time& left, const Time& right) { return left.value <=> right.value; }

        constexpr Time& operator+=(const Time& other) { value += other.value; return *this; }
        constexpr Time& operator-=(const Time& other) { value -= other.value; return *this; }
        constexpr Time& operator*=(double scalar) { value *= scalar; return *this; }
        constexpr Time& operator/=(double scalar) { value /= scalar; return *this; }
        constexpr Time operator-() const { return Time(-value); }

        friend constexpr Time operator+(const Time& left, const Time& right) { Time res = left; res += right; return res; }
        friend constexpr Time operator-(const Time& left, const Time& right) { Time res = left; res -= right; return res; }
        friend constexpr Time operator*(const Time& left, double scalar) { Time res = left; res *= scalar; return res; }
        friend constexpr Time operator*(double scalar, const Time& right) { Time res = right; res *= scalar; return res; }
        friend constexpr Time operator/(const Time& left, double scalar) { Time res = left; res /= scalar; return res; }
    };
}