#pragma once

#include <vector>

#include "mute/dsp.h"
#include "mute/modules/edgedetector.h"

namespace mute
{
    struct Stepquencer
    {
        static constexpr int MaxSteps = 128;
        std::array<float, MaxSteps> values;
        int index = MaxSteps;
        int length = 8;
        float output = 0;
        EdgeDetector gate;

        void reset() { index = MaxSteps; }
        void next() { index = (index + 1) % std::min(length, MaxSteps); }
        void process(float sr)
        {
            gate.process(sr);
            if (gate.rising)
                next();
            output = values[std::max(index, 0) % std::min(length, MaxSteps)];
        }
    };

    // struct NoteTrack
    // {
    //     struct Note
    //     {
    //         float pitch;
    //         float start = -1, length = -1;
    //     };
    //     struct Event
    //     {
    //         bool on;
    //         float pitch;
    //     };
        
    //     std::vector<Note> events;
    //     std::vector<Event> currentNotes;
    //     float t = 0;
    //     float length;
    //     bool loop = true;

    //     NoteTrack()
    //     {
    //         events.reserve(1024);
    //         currentNotes.reserve(32);
    //     }

    //     void noteon(float pitch)
    //     {
    //         for (auto& e: events)
    //         {
    //             if (e.pitch != pitch)
    //                 continue;

    //             if (e.start <= t && e.start + e.length > t)
    //                 e.length = t - e.start;

    //             events.push_back(Note { .pitch = pitch, .start = t, .length = -1 });
    //         }
    //     }
    //     void noteoff(float pitch)
    //     {
    //         for (auto& e: events)
    //         {
    //             if (e.pitch != pitch)
    //                 continue;

    //             if (e.length == -1)
    //                 e.length = t - e.start;
    //         }
    //     }

    //     void reset() { t = 0; }
    //     void process(float sr)
    //     {
    //         currentNotes.clear();
    //         for (auto& e: events)
    //             if (e.start <= t && e.start + e.length >= t)
    //                 currentNotes.push_back(e);

    //         t += 1 / sr;
    //         if (t > length)
    //             t -= length;
    //     }
    // };

    struct Sequencer
    {
        std::vector<float> values;
        int index = -1;
        
        float output;

        void reset() { index = -1; }
        void next() { index = (index + 1) % values.size(); }
        void process(float sr) { output = values[std::max(index, 0) % values.size()]; }
        
        // void randomize(int length, float low, float high)
        // {
        //     values.resize(length);
        //     for (auto& v: values)
        //         v = random(low, high);
        // }

        // static Sequencer init(int length, float low, float high)
        // {
        //     auto sq = Sequencer {};
        //     sq.values.resize(length);
        //     sq.randomize(low, high);
        //     return sq;
        // }

        // void randomize(float low, float high)
        // {
        //     for (auto& v: values)
        //         v = random(low, high);
        // }
    };

    struct GateSequencer
    {
        std::vector<bool> values;
        int index = -1;
        float output;
        float t = 0;
        float gatelen = 0.01;

        void reset() { index = -1; t = 0; }
        void next() { index = (index + 1) % values.size(); t = 0; }
        void process(float sr)
        {
            output = values[std::max(index, 0) % values.size()] ? 1. : 0.;
            t += 1/sr;
            if (t > gatelen)
                output = 0;
        }

        static GateSequencer init(int length)
        {
            auto sq = GateSequencer {};
            sq.values.resize(length);
            sq.randomize();
            return sq;
        }

        void randomize()
        {
            for (size_t i = 0; i < values.size(); i++)
                values[i] = true; // = random(0, 1) > 0.5;
        }
    };

    struct EuclideanSequencer
    {
        int length;
        int increment;
        float gatelen = 0.1;
        
        EdgeDetector gate;
        bool output;

        float t = 0;
        int index = -1;

        void reset()
        {
            index = length + 1;
            t = 0;
        }
        
        void next()
        {
            index = index < 0 ? 0 : index + increment;
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
}