#pragma once

#include "mute/time.h"

#include <cstdint>

namespace mute
{
    struct Measure
    {
        union {
            struct {
                uint32_t _128th : 3;    // div 16, div 32 (??)
                uint32_t _32th : 2;
                uint32_t _16th : 4;     // div 16
                uint32_t bars : (32-4-5);
            };
            uint32_t value;
        };
        struct {
            bool bar : 1 = false;
            bool _16th : 1 = false;
            bool _32th : 1 = false;
            bool _128th : 1 = false;
        } ticks;
        float fractional = 0;

        void reset() { value = 0; fractional = 0; }

        void increment(float bpm, float deltaTime)
        {
            ticks = {};

            const float bps = bpm/60.f;
            const float spb = 1.f/bps;
            const float spb128 = spb/128;
            fractional += deltaTime;

            uint32_t prevbars = bars;
            uint32_t prev16th = _16th;
            uint32_t prev32th = _32th;

            while (fractional > spb128)
            {
                fractional -= spb128;
                value++;
                ticks._128th = true;
            }

            if (prevbars != bars) ticks.bar = true;
            if (prev16th != _16th) ticks._16th = true;
            if (prev32th != _32th) ticks._32th = true;
        }
    };

    struct Transport
    {
        bool playing = false;
        float bpm = 0;
        Time time = {};
        Measure measure = {};

        void reset() { time = {}; measure = {}; }

        void update(float bpm, float deltaTime)
        {
            if (!playing)
                return;

            this->bpm = bpm;
            time += Time::s(deltaTime);
            measure.increment(bpm, deltaTime);
        }
        void update(float deltaTime)
        {
            update(bpm, deltaTime);
        }
    };
}