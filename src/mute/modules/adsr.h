#pragma once

#include <numeric>

#include "mute/utils.h"

namespace mute
{
    // struct AD
    // {
    //     static constexpr float Epsilon = std::numeric_limits<float>::min();

    //     enum class Segment
    //     {
    //         Attack,
    //         Decay
    //     } segment = Segment::Decay;

    //     float a, d;
    //     float gate;
    //     float output = 0.0f;

    //     void process(float sr)
    //     {
    //         // const float dt = 1.f / sr;
    //     }
    // };

    // struct ADSR
    // {
    //     static constexpr float Epsilon = std::numeric_limits<float>::min();

    //     enum class Segment
    //     {
    //         Attack,
    //         Decay,
    //         Sustain,
    //         Release
    //     } segment = Segment::Release;

    //     float a, d, s, r;
    //     float gate;
    //     float output = 0.0f;

    //     void process(float sr)
    //     {
    //         const float dt = 1.f / sr;
    //         const float aslope = dt / mute::max(a, Epsilon);
    //         const float dslope = -dt / mute::max(d / std::max(1.0f - s, Epsilon), Epsilon);
    //         const float rslope = -dt / mute::max(r, Epsilon);

    //         const float atarget = 1.0f;
    //         const float dtarget = s;
    //         const float starget = s;
    //         const float rtarget = 0.0f;

    //         bool gateon = gate > 0.5f;
    //         float slope = 0.0f;
    //         float target = 0.0f;

    //         // auto lastSegment = segment;
    //         switch (segment)
    //         {
    //             case Segment::Attack:
    //                 segment = gateon ? Segment::Attack : Segment::Release;
    //                 // segment = (gateon && output >= atarget) ? Segment::Decay : Segment::Attack;
    //                 break;
    //             case Segment::Decay:
    //                 segment = gateon ? Segment::Decay : Segment::Release;
    //                 // segment = (gateon && output <= dtarget) ? Segment::Sustain : Segment::Decay;
    //                 break;
    //             case Segment::Sustain:
    //                 segment = gateon ? Segment::Sustain : Segment::Release;
    //                 break;
    //             case Segment::Release:
    //                 segment = gateon ? Segment::Attack : Segment::Release;
    //                 break;
    //         }

    //         // if (segment != lastSegment)
    //         //     fmt::println("ADSR Segment {} -> {}", (int) lastSegment, (int) segment);

    //         switch (segment)
    //         {
    //             case Segment::Attack:
    //                 slope = aslope;
    //                 target = atarget;
    //                 break;
    //             case Segment::Decay:
    //                 slope = dslope;
    //                 target = dtarget;
    //                 break;
    //             case Segment::Sustain:
    //                 slope = 0.0f;
    //                 target = starget;
    //                 break;
    //             case Segment::Release:
    //                 slope = rslope;
    //                 target = rtarget;
    //                 break;
    //         }
            
    //         output += slope;
    //         output = slope > 0.0f
    //             ? mute::min(output, target)
    //             : mute::max(output, target);

    //         // lastSegment = segment;
    //         if (output == target)
    //         {
    //             switch (segment)
    //             {
    //                 case Segment::Attack: segment = Segment::Decay; break;
    //                 case Segment::Decay: segment = Segment::Sustain; break;
    //                 case Segment::Sustain: break;
    //                 case Segment::Release: break;
    //             }
    //         }
    //         // if (segment != lastSegment)
    //         //     fmt::println("ADSR Segment {} -> {}", (int) lastSegment, (int) segment);
    //     }
    // };
}