#pragma once

#include "mute/math.h"

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