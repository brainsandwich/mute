#include "mute/dsp.h"
#include "mute/driver.h"
#include "mute/enveloppe.h"

#include <fmt/printf.h>
#include <fmt/ranges.h>

#include <ranges>
#include <span>

namespace mute
{    
    struct Patch
    {
        virtual ~Patch() = default;
        virtual void process(AudioProcessData data) = 0;
    };

    struct PatchGroup
    {
        AudioBuffer buffer = AudioBuffer::init(1024);
        std::vector<std::unique_ptr<Patch>> patches;
        std::vector<float> volumes;

        PatchGroup() = default;
        PatchGroup(const PatchGroup&) = delete;
        PatchGroup(PatchGroup&&) = default;

        PatchGroup& add(std::unique_ptr<Patch>&& patch)
        {
            patches.emplace_back(std::move(patch));
            volumes.emplace_back(0.0f);
            return *this;
        }

        template <std::derived_from<Patch> P>
        PatchGroup& add()
        {
            add(std::make_unique<P>());
            return *this;
        }

        void process(AudioProcessData data)
        {
            AudioProcessData patchData = data;
            buffer.resize(data.frames * data.playback.channels);
            patchData.playback.buffer = buffer;

            for (int i = 0; i < (int) patches.size(); i++)
            {
                buffer *= 0.0f;
                patches[i]->process(patchData);
                buffer *= volumes[i];
                data.playback.buffer += buffer;
            }
        }
    };

    struct Oscillator
    {
        float phase = 0;
        float frequency;
        float output;
        float k = 1;

        void process(float sr)
        {
            output = mute::sawsine(&phase, frequency, sr, k);
            // output = mute::oscillate(&phase, frequency, sr);
        }
    };

    struct Sequencer
    {
        std::vector<float> values;
        int index = -1;
        
        float output;

        void next() { index = (index + 1) % values.size(); }
        void process(float sr) { output = values[std::max(index, 0) % values.size()]; }
        void randomize(int length, float low, float high) {
            values.resize(length);
            for (auto& v: values)
                v = random(low, high);
        }

        static Sequencer init(int length, float low, float high)
        {
            auto sq = Sequencer {};
            sq.values.resize(length);
            sq.randomize(low, high);
            return sq;
        }

        void randomize(float low, float high)
        {
            for (auto& v: values)
                v = random(low, high);
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

    struct Clock
    {
        float input = 0.0f;
        float bpm = 0.0f;
        float t = 1000;
        bool output = false;

        void reset() { t = 1000; }

        void process(float sr)
        {
            output = false;
            if (bpm > 0)
            {
                const float bps = bpm/60.f;
                const float spb = 1.f/bps;
                const float spb128 = spb/128.f;
                t += sr;
                if (t > spb128)
                {
                    t -= spb128;
                    output = true;
                }
            }
        }
    };

    struct Transport
    {
        bool playing;
        float bpm;
    };
}

struct OscSequencePatch: mute::Patch
{
    mute::Oscillator clock { .frequency = 8.0f };
    mute::EdgeDetector clockDetector;

    mute::Oscillator osc1;
    mute::Oscillator osc2;
    mute::AHDSR env = { .a = 0.001, .d = 0.3, .s = 0.3, .r = 0.1 };

    mute::Sequencer pitchseq = mute::Sequencer::init(16, 20, 400);
    mute::Sequencer gateseq = mute::Sequencer::init(16, 1, 1);
    mute::Sequencer fmseq = mute::Sequencer::init(16, 0.1, 10);

    int seqcount = 0;

    float process(float sr)
    {
        clock.process(sr);
        clockDetector.input = clock.output;
        clockDetector.process(sr);

        if (clockDetector.rising)
        {
            pitchseq.next();
            gateseq.next();
            fmseq.next();
            // if (pitchseq.index == 0)
            //     seqcount++;

            // if (seqcount > 2)
            // {
            //     pitchseq.randomize(20, 400);
            //     gateseq.randomize(0, 1);
            //     fmseq.randomize(0, 1);
            //     seqcount = 0;
            // }
        }
        pitchseq.process(sr);
        gateseq.process(sr);
        fmseq.process(sr);

        osc1.frequency = pitchseq.output * 0.5f;
        osc1.process(sr);

        osc2.k = 1.0;
        osc2.frequency = pitchseq.output + 0.04 * sr * osc1.output * fmseq.output;
        osc2.process(sr);

        env.gate = gateseq.output;
        env.process(sr);

        float out = osc2.output * env.output;
        out = mute::saturate(out, 2.f);
        out = mute::clamp(out, -1.f, 1.f);
        return out;
    }

    void process(mute::AudioProcessData data) override
    {
        for (int f = 0; f < data.frames; f++)
        {
            float out = process(data.sampleRate);
            for (int c = 0; c < data.playback.channels; c++)
                data.playback.buffer[f * data.playback.channels + c] = out;
        }
    }
};

#include "mute/debug.h"

int main(int argc, char** argv)
{
    auto patchGroup = mute::PatchGroup();
    patchGroup.add<OscSequencePatch>();

    auto driver = mute::AudioDriver({
        .playback = {
            .channels = 2
        },
        .sampleRate = 44100,
        .bufferSize = 256,
        .callback = [&](mute::AudioProcessData data) { patchGroup.process(data); }
    });

    driver.start();
    getchar();
    driver.stop();

    return 0;
}

// #include <fmt/printf.h>
// #include <fmt/ranges.h>
// #include <raylib.h>

// #include <ranges>

// #include "mute/audio.h"
// #include "mute/dsp.h"
// // #include "mute/modules/noise.h"
// // #include "mute/modules/adsr.h"
// // #include "mute/modules/oscillator.h"

// namespace mute
// {
//     // /// @brief Meta enveloppe, using only one "time" parameter to drive a full ADSR
//     // /// 1. decay progresses first
//     // /// 2. then some attack is added
//     // /// 3. sustain and release come last
//     // ///
//     // /// - at time == 0.1, mostly decay, in ~0.3sec
//     // /// - at time == 0.5, 0.125sec attack, 0.833sec decay, sustain at 0.125, complete in 1.47sec
//     // /// - at time == 1, complete rise and fall time is 2.5sec
//     // struct MetaEnv
//     // {
//     //     float time; // normalized 0 to 1
//     //     float gate;
//     //     float output;
//     //     ADSR adsr;

//     //     void process(float sr)
//     //     {
//     //         time = mute::clamp(time, 0.0f, 1.0f);
//     //         adsr.a = 0.5*time*time;
//     //         adsr.d = sqrt(time);
//     //         adsr.s = time*time*time;
//     //         // adsr.d = 2.0f * adsr.s;
//     //         adsr.r = 2.0f * adsr.s;
//     //         // adsr.r = sqrt(time);
//     //         adsr.gate = gate;
//     //         adsr.process(sr);

//     //         output = adsr.output;
//     //     }
//     // };

//     struct Trigger
//     {
//         float last = 0.0f;
//         float input = 0.0f;
//         bool output = false;

//         Trigger& operator=(float in) { input = in; return *this; }

//         void process()
//         {
//             output = false;
//             if (input > 0.6f && last < 0.4f)
//             {
//                 output = true;
//             }
//             last = input;
//         }
//     };

//     struct AD
//     {
//         float a;
//         float d;
//         float gate;
//         float output;
//         float t = 0.0f;
//         bool gateup = false;

//         void process(float sr)
//         {
//             output = 0.0f;

//             // bool triggered = false;
//             if (gate > 0.5f && !gateup)
//             {
//                 gateup = true;
//                 t = 0.0f;
//             }
//             if (gate < 0.5f && gateup)
//                 gateup = false;

//             if (t < a) output = t / max(a, Epsilon);
//             else if (t < a + d) output = (-(t - a) / d) + 1;

//             t = min(t + 1.0f / sr, a + d + Epsilon);
//         }
//     };

//     float ads(float params[3], float gate, float* t, float sr)
//     {
//         const float& a = params[0];
//         const float& d = params[1];
//         const float& s = params[2];
        
//         float dt = 1 / safe(sr);
//         float origin = *t < a ? 0 : 1;
//         float target = *t < a ? gate : gate * s;
//         float time = *t < a ? (*t / a) : ((*t - a) / d);

//         float output = lerp(origin, target, *t);
//         *t = clamp(*t + dt, 0.0f, a + d + Epsilon);
//         return output;
//     }

//     struct GateSequencer
//     {
//         Trigger clock;
//         float output;

//         int index = -1;
//         float t = 0.0f;
//         float gatelen = 0.1f;
//         std::vector<bool> pattern;

//         void next()
//         {
//             index = (index + 1) % pattern.size();
//             t = 0.0f;
//         }

//         void process(float sr)
//         {
//             clock.process();
//             if (clock.output)
//             {
//                 next();
//             }

//             output = 0.0f;
//             if (pattern[index % pattern.size()])
//             {
//                 t += 1.f / sr;
//                 output = t < gatelen;
//             }
//         }
//     };

//     struct KickDrum
//     {
//         float phase = 0;
//         float t = 0;

//         void trigger() { t = 0; phase = 0; }
//         float process(float sr, float decay, float fx1, float fx2)
//         {
//             float freq = fx1;
//             float pitchmod = fx2 * 2.f - 1.f;

//             return oscillate(&phase, freq, sr);
//         }
//     };

//     struct Perc
//     {
//         enum class Mode
//         {
//             KickDrum
//         } mode;

//         union {
//             KickDrum kickdrum;
//         } models;

//         float volume;
//         float decay;
//         float fx1, fx2;

//         void trigger() {}

//         void process(float sr)
//         {

//         }
//     };
// }

// int main(int argc, char** argv)
// {
//     InitWindow(800, 600, "mute");
//     SetTargetFPS(60);

//     auto audio = mute::Audio(mute::AudioConfiguration {
//         .bufferSize = 128,
//         .sampleRate = 44100,
//         .playback = { .channels = 2 }
//     });

//     // clock -> drum sequencer -> multiple drum sounds
//     // drum sound: "noise source" + env (ad ?) + filter
//     // drum set: mixed drum sounds
//     // additional: send effects
//     // mix: drum set + returns / compress + saturate

//     // A: one drum sound
//     auto noise = mute::PrimeCluster {};
//     auto lfo1 = mute::Oscillator { .frequency = 1.0f };
//     auto lfo2 = mute::Oscillator { .frequency = 1.8f };
//     // auto env = mute::MetaEnv { .time = 0.4f };
//     auto env = mute::AD { .a = 0.f, .d = 0.03f };

//     auto clock = mute::LFO { .frequency = 4*mute::bpm2hz(120.0f), .mode = mute::LFO::Mode::Square };
//     auto seq = mute::GateSequencer {
//         .gatelen = 0.01f,
//         .pattern = { 1, 0, 0, 0, 1, 1, 0, 0 }
//     };
//     seq.pattern.resize(16);
//     auto randomizeSequence = [&](){
//         for (int i = 0; i < (int) seq.pattern.size(); i++)
//             seq.pattern[i] = (rand() > RAND_MAX / 2);
//         fmt::println("seq: {}", fmt::join(seq.pattern | std::views::transform([](auto v) { return v ? "X" : "_"; }) , ", "));
//     };

//     audio.audioCallback = [&](mute::AudioProcessData data)
//     {
//         float sr = data.sampleRate;
//         int chans = data.playback.channels;
//         memset(data.playback.buffer, 0, data.playback.channels * data.frames * sizeof(float));

//         for (int f = 0; f < data.frames; f++)
//         {
//             clock.process(sr);
//             seq.clock = clock.output;

//             lfo1.process(sr);
//             lfo2.process(sr);
//             noise.process(sr);
//             noise.noiseAmount = 0.1f * (lfo1.output * 0.5f + 0.5f) + 0.01f;
//             noise.freqMult = 0.2f * lfo2.output + 1.0f;
            
//             seq.process(sr);
//             env.gate = seq.output;
//             env.process(sr);
//             float output = noise.output * env.output;
//             output *= 0.8f;

//             for (int c = 0; c < chans; c++)
//                 data.playback.buffer[f * chans + c] = output;
//         }

//     };
//     audio.start();

//     while (!WindowShouldClose())
//     {
//         PollInputEvents();
//         if (IsKeyPressed(KEY_Z))
//             randomizeSequence();

//         SwapScreenBuffer();
//     }

//     audio.stop();
//     CloseWindow();

//     return 0;
// }














// #include <array>
// #include <vector>
// #include <span>
// #include <optional>
// #include <algorithm>
// #include <ranges>
// #include <unordered_set>
// #include <queue>
// #include <stack>

// #include <fmt/printf.h>

// namespace mute
// {
//     template <typename Sample>
//     struct AudioBlock
//     {
//         static constexpr size_t MaxChannels = 16;
//         std::array<std::span<Sample>, MaxChannels> samples;
//         int channels;
//     };

//     struct ProcessContext
//     {
//         AudioBlock<const float> in;
//         AudioBlock<float> out;
//     };

//     struct PrepareInfo
//     {
//         int sampleRate;
//         int bufferSize;
//         int channels;
//     };

//     struct Processor
//     {
//         std::string name;
//         std::vector<Processor*> sources;

//         Processor(std::string_view name): name(name) {}
        
//         virtual void prepare(const PrepareInfo&) = 0;
//         virtual void process(const ProcessContext&) = 0;
//     };

//     struct Passthrough: Processor
//     {
//         Passthrough(std::string_view name): Processor(name) {}

//         void prepare(const PrepareInfo&) override {}
//         void process(const ProcessContext&) override {}
//     };

//     template <typename T> struct NodeSiblingAccessor {};
//     // static const Node* sibling(const Node* node, int index) { return nullptr; }

//     template <typename T> struct NodeChildAccessor {};
//     // static const Node* child(const Node* node, int index) { return nullptr; }
//     // static int children(Node* node) { return 0; }

//     template <typename T> struct NodeParentAccessor {};
//     // static const Node* parent(const Node* node) { return nullptr; }

//     template <typename T> struct NodeIndexAccessor {};
//     // static int index(Node* node) { return 0; }

//     template <typename T> struct NodeChildMutator {};
//     // static void insert(Node* node, int index, Node* child) {}
//     // static void remove(Node* node, int index) {}

//     template <typename T> struct NodeTypeAccessor {};
//     // using Node = T;

//     template <std::derived_from<Processor> P>
//     struct NodeTypeAccessor<P>
//     {
//         using Node = Processor;
//     };

//     template <std::derived_from<Processor> P>
//     struct NodeChildAccessor<P>
//     {
//         static Processor* child(Processor* node, int index) { return node->sources[index]; }
//         static int children(Processor* node) { return node->sources.size(); }
//     };

//     template <std::derived_from<Processor> P>
//     struct NodeChildMutator<P>
//     {
//         static void insert(Processor* node, int index, Processor* child) { node->sources.insert(node->sources.begin() + index, child); }
//         static void remove(Processor* node, int index) { node->sources.erase(node->sources.begin() + index); }
//     };

//     template <typename T>
//     struct NodeManipulator:
//           NodeTypeAccessor<T>
//         , NodeSiblingAccessor<T>
//         , NodeChildAccessor<T>
//         , NodeParentAccessor<T>
//         , NodeIndexAccessor<T>
//         , NodeChildMutator<T>
//     {};

//     enum class GraphTraversal
//     {
//         DepthFirstPreOrder,
//         DepthFirstInOrder,
//         DepthFirstPostOrder,
//         BreadthFirst
//     };

//     template <GraphTraversal Traversal>
//     struct TraversalHelper
//     {
//         template <typename T, typename OutputIterator>
//         static void traverse(T* root, OutputIterator out)
//         {
//             using Manipulator = NodeManipulator<T>;
            
//             if constexpr (Traversal == GraphTraversal::DepthFirstPreOrder)
//                 *out++ = root;

//             for (int i = 0; i < Manipulator::children(root); i++)
//                 traverse(Manipulator::child(root, i), out);

//             if constexpr (Traversal == GraphTraversal::DepthFirstPostOrder)
//                 *out++ = root;
//         }
//     };

//     template <>
//     struct TraversalHelper<GraphTraversal::BreadthFirst>
//     {
//         template <typename T, typename OutputIterator>
//         static void traverse(T* root, OutputIterator out)
//         {
//             using Manipulator = NodeManipulator<T>;
//             using Node = Manipulator::Node;
//             using History = std::stack<Node*>; 

//             std::unordered_set<Node*> traversed;
//             History history;
//             history.push(root);

//             while (!history.empty())
//             {
//                 Node* current = history.top();
//                 history.pop();
//                 traversed.emplace(current);
//                 *out++ = current;

//                 for (int i = Manipulator::children(current) - 1; i >= 0; i--)
//                 {
//                     auto child = Manipulator::child(current, i);
//                     history.push(child);
//                 }
//             }
//         }
//     };

//     template <GraphTraversal Traversal, typename T, typename OutputIterator>
//     void traverse(T* root, OutputIterator out)
//     {
//         TraversalHelper<Traversal>::traverse(root, out);
//     }

//     struct DepthFirstPreOrder_t {} DepthFirstPreOrder;
//     struct DepthFirstInOrder_t {} DepthFirstInOrder;
//     struct DepthFirstPostOrder_t {} DepthFirstPostOrder;
//     struct BreadthFirst_t {} BreadthFirst;

//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstPreOrder_t) { traverse<GraphTraversal::DepthFirstPreOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstInOrder_t) { traverse<GraphTraversal::DepthFirstInOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, DepthFirstPostOrder_t) { traverse<GraphTraversal::DepthFirstPostOrder>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out, BreadthFirst_t) { traverse<GraphTraversal::BreadthFirst>(root, out); }
//     template <typename T, typename OutputIterator> void traverse(T* root, OutputIterator out) { traverse<GraphTraversal::DepthFirstPreOrder>(root, out); }
// }

// int main(int argc, char** argv)
// {
//     auto gena = mute::Passthrough("gena");
//     auto genb = mute::Passthrough("genb");
//     auto mixab = mute::Passthrough("mixab");
//     mixab.sources = { &gena, &genb };

//     auto genc = mute::Passthrough("genc");
//     auto gainc = mute::Passthrough("gainc");
//     gainc.sources = { &genc };

//     auto mixabc = mute::Passthrough("mixabc");
//     mixabc.sources = { &mixab, &gainc };

//     std::vector<bool> done;
//     std::vector<bool> run;
//     std::vector<int> depcount;
//     std::vector<mute::Processor*> ordered;
//     mute::traverse(&mixabc, std::back_inserter(ordered), mute::DepthFirstPostOrder);

//     done.resize(ordered.size());
//     run.resize(ordered.size());
//     depcount.resize(ordered.size());

//     for (auto i = 0ull; i < ordered.size(); i++)
//     {
//         auto& node = ordered[i];
//         auto depends = node->sources
//             | std::views::transform([](mute::Processor* node) { return node->name; });

//         fmt::println("eval {}: {}, depends on {{{}}}"
//             , i, node->name
//             , fmt::join(depends, ", "));

//         depcount[i] = node->sources.size();
//     }

//     while (!std::ranges::all_of(done, [](auto v) { return v == true; }))
//     {
//         int group = 0;
//         for (auto i = 0ull; i < ordered.size(); i++)
//         {
//             auto& node = ordered[i];
//             if (depcount[i] == 0)
//             {
//                 fmt::println("[{}] processing {}", group, node->name);
//                 done[i] = true; 

//                 for (auto k = 0ull; k < ordered.size(); k++)
//                 {
//                     auto& target = ordered[k];
//                     if (std::ranges::distance(target->sources
//                         | std::views::filter([node](auto ss) { return ss == node; })) > 0)
//                     {
//                         depcount[k]--;
//                     }
//                 }
//             }
//         }
//         group++;
//     }

//     return 0;
// }















// #include <libremidi/libremidi.hpp>
// #include <fmt/printf.h>
// #include <raylib.h>

// #include "audio.h"
// #include "utils.h"
// // #include "midi.h"

// #include "modules/oscillator.h"
// #include "modules/noise.h"
// #include "modules/adsr.h"

// #include <cassert>

// template <size_t N>
// constexpr auto init_array(const auto& value)
// {
//     std::array<std::remove_cvref_t<decltype(value)>, N> result;
//     for (size_t i = 0; i < N; i++)
//         result[i] = value;
//     return result;
// }

// template <size_t N, typename Fn>
// constexpr auto init_array_fn(Fn&& fn)
// {
//     std::array<std::remove_cvref_t<std::invoke_result_t<Fn, size_t>>, N> result;
//     for (size_t i = 0; i < N; i++)
//         result[i] = fn(i);
//     return result;
// }

// template <size_t N>
// struct Wavetable
// {
//     static constexpr uint32_t size = N;
//     static constexpr uint32_t mask = size - 1;

//     std::array<float, N> data;

//     constexpr Wavetable(float (*fn)(float phase))
//         : data(init_array_fn<N>([fn](size_t i) { return fn(2.0f * M_PI * float(i) / float(size)); })) {}

//     float value(float phase) const
//     {
//         float index = float(N) * phase / (2.0f * M_PI);
//         int i1 = uint32_t(index) & mask;
//         int i2 = (i1 + 1) & mask;
//         float frac = index - uint32_t(index);

//         float a = data[i1];
//         float b = data[i2];

//         return mute::lerp(a, b, frac);
//     }
// };

// struct Oscillator
// {
//     Wavetable<1024> wavetable = Wavetable<1024>([](float phase) { return std::sin(phase); });

//     float phase;
//     float increment;
//     float frequency;

//     float process(float sr)
//     {
//         phase = mute::rephase(phase, 2.0f * M_PI * frequency / sr);
//         return wavetable.value(phase);
//     }
// };

// template <int OscillatorCount>
// struct SwarmOscillator
// {
//     std::array<float, OscillatorCount> phase = {};
//     std::array<float, OscillatorCount> increment = {};
//     std::array<float, OscillatorCount> amplitude = init_array<OscillatorCount>(1.0f);

//     float frequency = 100.0f;
//     float spread = 1.0f;

//     void process(float* out, int frames, float sr)
//     {
//         for (int f = 0; f < frames; f++)
//         {
//             float sum = 0.0f;
//             for (int i = 0; i < OscillatorCount; i++)
//             {
//                 auto fq = frequency * (1.0f + i * spread);
//                 increment[i] = 2.0f * M_PI * fq / sr;
//                 amplitude[i] = 1.0f / float(i + 1);
//                 if (fq > sr / 2.0f)
//                     amplitude[i] = 0.0f;

//                 phase[i] += increment[i];
//                 if (phase[i] > 2.0f * M_PI) phase[i] -= 2.0f * M_PI;
//                 if (phase[i] < 0.0f) phase[i] += 2.0f * M_PI;
//                 float output = mute::fast::cos(phase[i]);
//                 sum += output * amplitude[i];
//             }
//             *out++ = sum;
//         }
//     }
// };

// template <int OscillatorCount>
// struct SwarmWaveOscillator
// {
//     Wavetable<1024> wavetable = Wavetable<1024>([](float phase) { return std::sin(phase); });
//     std::array<float, OscillatorCount> phase = {};

//     float frequency = 100.0f;
//     float spread = 1.0f;

//     float process(float sr)
//     {
//         const float twopi = 2.0f * M_PI;
//         const float twopi_sr = twopi / sr;
//         float out = 0.0f;

//         for (int i = 0; i < OscillatorCount; i++)
//         {
//             auto fq = frequency * (1.0f + i * spread);
//             float increment = fq * twopi_sr;
//             float amplitude = 1.0f / float(i + 1);
//             if (fq > sr / 2.0f)
//                 amplitude = 0.0f;

//             phase[i] = mute::rephase(phase[i], increment);
//             out += wavetable.value(phase[i]) * amplitude;
//         }
//         return out;
//     }
// };

// #include <complex>

// template <int OscillatorCount>
// struct SwarmOscillator2
// {
//     std::array<std::complex<float>, OscillatorCount> value = init_array<OscillatorCount>(std::complex<float>(1));
//     std::array<std::complex<float>, OscillatorCount> multiplier = init_array<OscillatorCount>(std::complex<float>(1));
//     std::array<float, OscillatorCount> amplitude = init_array<OscillatorCount>(1.0f);

//     float frequency = 100.0f;
//     float sampleRate = 44100.0f;

//     void setFrequency(float fq)
//     {
//         frequency = fq;
//         for (int i = 0; i < OscillatorCount; i++)
//         {
//             float f = frequency * (1.0f + i);
//             float increment = 2.0f * M_PI * (f / sampleRate);
//             multiplier[i] = { std::cos(increment), std::sin(increment) };
//             amplitude[i] = 1.0f / float(i + 1);
//             if (f > sampleRate / 2.0f)
//                 amplitude[i] = 0.0f;
//         }
//     }

//     void process(float* out, int frames)
//     {
//         for (int f = 0; f < frames; f++)
//         {
//             float sum = 0.0f;
//             for (int i = 0; i < OscillatorCount; i++)
//             {
//                 value[i] *= multiplier[i];
//                 sum += value[i].imag() * amplitude[i];
//             }
//             *out++ = sum;
//         }
//     }
// };

// namespace mute
// {
//     constexpr float scale(float normalized, float min, float max) { return (max - min) * normalized + min; }
//     constexpr float sq(float in) { return in * in; }

//     float random() { return float(rand()) / float(RAND_MAX); }
//     float random(float max) { return scale(random(), 0.0f, max); }
//     float random(float min, float max) { return scale(random(), min, max); }
// }

// int main(int argc, char** argv)
// {
//     auto audio = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2, .deviceName = "Haut-parleurs MacBook Pro" },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });


//     auto source = SwarmWaveOscillator<512> {};
//     auto lfo = Oscillator { .frequency = 0.3f };
//     // source.sampleRate = audio.internalConfiguration.sampleRate;
//     // std::array<float, 512> audioBuffer = {};

//     audio.audioCallback = [&](mute::AudioProcessData data)
//     {
//         // source.process(audioBuffer.data(), data.frames);
//         // source.process(audioBuffer.data(), data.frames, data.sampleRate);

//         for (int f = 0; f < data.frames; f++)
//         {
//             source.spread = 1.0f + lfo.process(data.sampleRate);
//             float out = source.process(data.sampleRate);
//             // float& out = audioBuffer[f];
//             out = std::clamp(out, -1.0f, 1.0f);
//             out *= 0.4f;
//             if (!isfinite(out))
//                 out = 0.0f;

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = out;
//         }
//     };

//     audio.start();
//     InitWindow(800, 600, "mute");

//     while (!WindowShouldClose())
//     {
//         if (IsKeyPressed(KEY_Z) || IsKeyPressedRepeat(KEY_Z))
//         {
//             auto freq = mute::random(20.0f, 1000.0f);
//             // source.setFrequency(freq);
//             source.frequency = freq;
//             fmt::println("FREQ: {:.4f}Hz", freq);
//         }

//         BeginDrawing();
//         EndDrawing();
//     }

//     CloseWindow();
//     audio.stop();
//     return 0;
// }

// namespace mute
// {
//     struct RisingEdgeDetector
//     {
//         float output;
//         float last = 0.f;
//         bool active = false;

//         void process(float in)
//         {
//             if (!active && in > last)
//             {
//                 active = true;
//                 last = in;
//                 output = 1.0f;
//                 return;
//             }

//             if (active && in < last)
//                 active = false;
            
//             last = in;
//             output = 0.0f;
//         }
//     };

//     struct Clock
//     {
//         int counter = 0;
//         int division = 4;
//         std::function<void()> ontick;

//         void reset() { counter = 0; }
//         void tick()
//         {
//             if (counter == 0)
//                 ontick();

//             counter = (counter + 1) % division;
//         }
//     };
// }

// int main(int argc, char** argv)
// {
//     auto context = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2 },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });

//     if (!context.valid())
//         exit(1);

//     mute::MidiIn midi;
//     midi.open("Arturia BeatStepPro");
    
//     mute::Voice voice {
//         .em1.amp = 1.0f,
//         .volume = 1.0f,
//         .cutoff = mute::pitchToFrequency(127)
//     };

//     midi.on.noteon = [&](auto chan, auto note, auto velo)
//     {
//         if (chan == 1)
//         {
//             voice.pitch = mute::pitchToFrequency(note);
//             voice.gate = 1.0f;
//             voice.em2.amp = 0.5f * (velo / 127.0f);
//         }
//     };

//     midi.on.noteoff = [&](auto chan, auto note)
//     {
//         if (chan == 1)
//         {
//             if (voice.pitch == mute::pitchToFrequency(note))
//             {
//                 voice.gate = 0.0f;
//             }
//         }
//     };

//     auto setVolume = [&](float v)
//     {
//         voice.volume = v;
//         fmt::println(" VOL | {:.2f}", voice.volume);
//     };
//     auto setEnveloppe = [&](mute::ADSR& env, float a, float d, float s, float r)
//     {
//         env.a = a;
//         env.d = d;
//         env.s = s;
//         env.r = r;
//         fmt::println("ADSR | {:.4f}ms / {:.4f}ms \\ {:.2f} — {:.4f}ms \\"
//             , 1e3*env.a
//             , 1e3*env.d
//             , env.s
//             , 1e3*env.r);
//     };
//     auto setEnveloppeMod = [&](mute::Voice::EnvMod& mod, float pitch, float wave, float cutoff, float res)
//     {
//         mod.osc.freq = pitch;
//         mod.osc.wave = wave;
//         mod.filter.freq = cutoff;
//         mod.filter.q = res;
//         fmt::println(" MOD | {:.2f} {:.2f} {:.2f}Hz {:.2f}"
//             , mod.osc.freq
//             , mod.osc.wave
//             , mod.filter.freq
//             , mod.filter.q);
//     };
//     auto setWave = [&](float wave)
//     {
//         voice.wave = wave;
//         fmt::println(" OSC | w{:.2f}", wave);
//     };
//     auto setFilter = [&](float cutoff, float resonance)
//     {
//         voice.cutoff = cutoff;
//         voice.resonance = resonance;
//         fmt::println("FILT | {:.2f}Hz {:.2f}", cutoff, resonance);
//     };
//     auto setFilterMode = [&](mute::BiquadFilter::Mode mode)
//     {
//         auto last = voice.filter.mode;
//         if (last != mode)
//         {
//             voice.filter.mode = mode;
//             std::string modestr;
//             switch (mode)
//             {
//                 case mute::BiquadFilter::Mode::LowPass: modestr = "LowPass"; break;
//                 case mute::BiquadFilter::Mode::HighPass: modestr = "HighPass"; break;
//                 case mute::BiquadFilter::Mode::BandPassQ: modestr = "BandPassQ"; break;
//                 case mute::BiquadFilter::Mode::BandPassPeak: modestr = "BandPassPeak"; break;
//                 case mute::BiquadFilter::Mode::Notch: modestr = "Notch"; break;
//                 case mute::BiquadFilter::Mode::AllPass: modestr = "AllPass"; break;
//                 case mute::BiquadFilter::Mode::PeakingEQ: modestr = "PeakingEQ"; break;
//                 case mute::BiquadFilter::Mode::LowShelf: modestr = "LowShelf"; break;
//                 case mute::BiquadFilter::Mode::HighShelf: modestr = "HighShelf"; break;
//                 default:
//                     break;
//             }
//             fmt::println("FILT | mode {}", modestr);
//         }
//     };

//     midi.on.cc = [&](auto chan, auto control, auto value)
//     {
//         // fmt::println("CC {:#x} {}", control, value);
//         float fvalue = value / 127.0f;
//         auto& env1 = voice.env1;
//         auto& env2 = voice.env2;
//         auto& em2 = voice.em2;
//         switch (control)
//         {
//             case 0x72: setVolume(std::pow(fvalue, 2.0f)); break;
//             case 0x12: setWave(fvalue); break;
//             case 0x13: setFilter(mute::pitchToFrequency(value), voice.resonance); break;
//             case 0x10: setFilter(voice.cutoff, fvalue * 20.f); break;

//             case 0x0a: setEnveloppe(env1, std::pow(fvalue, 2.0f), env1.d, env1.s, env1.r); break;
//             case 0x4a: setEnveloppe(env1, env1.a, std::pow(fvalue, 2.0f), env1.s, env1.r); break;
//             case 0x47: setEnveloppe(env1, env1.a, env1.d, fvalue, env1.r); break;
//             case 0x4c: setEnveloppe(env1, env1.a, env1.d, env1.s, std::pow(fvalue, 2.0f)); break;

//             // TODO: find actual CC 
//             case 0x4d: setEnveloppe(env2, std::pow(fvalue, 2.0f), env2.d, env2.s, env2.r); break;
//             case 0x5d: setEnveloppe(env2, env2.a, std::pow(fvalue, 2.0f), env2.s, env2.r); break;
//             case 0x49: setEnveloppe(env2, env2.a, env2.d, fvalue, env2.r); break;
//             case 0x4b: setEnveloppe(env2, env2.a, env2.d, env2.s, std::pow(fvalue, 2.0f)); break;

//             case 0x11: setEnveloppeMod(em2, mute::pitchToFrequency(value), em2.osc.wave, em2.filter.freq, em2.filter.q); break;
//             case 0x5b: setEnveloppeMod(em2, em2.osc.freq, fvalue, em2.filter.freq, em2.filter.q); break;
//             case 0x4f: setEnveloppeMod(em2, em2.osc.freq, em2.osc.wave, mute::pitchToFrequency(value), em2.filter.q); break;
//             case 0x48: setFilterMode(mute::BiquadFilter::Mode(fvalue * 9)); break;

//             default:
//                 break;
//         }
//     };

//     // midi.on.clock = [&]()
//     // {
//     //     clock.tick();
//     // };

//     // midi.on.stop = [&]()
//     // {
//     //     clock.reset();
//     // };

//     // clock.ontick = [&]()
//     // {
        
//     // };

//     context.audioCallback = [&](mute::AudioProcessData data)
//     {
//         auto sr = data.sampleRate;
//         midi.consumeMessages();
        
//         for (int f = 0; f < data.frames; f++)
//         {
//             // voice.pitch = mute::pitchToFrequency(35);
//             // voice.cutoff = mute::pitchToFrequency(100);
//             // voice.gate = 1.0f;
//             voice.process(sr);
//             float output = voice.output;

//             output = std::tanh(output);
//             output = std::clamp(output, -1.f, 1.f);

//             if (!std::isfinite(output))
//                 output = 0.0f;

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = output;            
//         }
//     };

//     context.start();
// #if defined(__APPLE__)
//     CFRunLoopRun();
// #else
//     getchar();
// #endif
//     context.stop();
    
//     return 0;
// }

// int main(int argc, char** argv)
// {
//     auto context = mute::Audio(mute::AudioConfiguration
//     {
//         .playback = { .channels = 2 },
//         .sampleRate = 44100,
//         .bufferSize = 128
//     });

//     if (!context.valid())
//         exit(1);

//     mute::MidiIn midi;
//     midi.open("Arturia BeatStepPro");

//     mute::Sequencerz sequencerz;
//     sequencerz.randomize(mute::Sequencerz::ALL);

//     mute::Oscillator osc { .frequency = 420.0f };
//     mute::ADSR env {
//         .a = 0.0f,
//         .d = 0.0f,
//         .s = 1.0f,
//         .r = 0.0f
//     };
//     mute::Clock clock {};

//     float wfmod = 0.0f;
//     // float wvmod = 0.0f;

//     midi.on.noteon = [&](auto chan, auto note, auto velo)
//     {
//         if (chan == 1)
//         {
//             osc.frequency = mute::pitchToFrequency(note);
//             env.gate = 1.0f;
//             wfmod = velo / 127.0f;
//             // wvmod = velo / 127.0f;
//         }

//         if (chan == 16)
//         {
//             switch (note)
//             {
//                 case 0x2c: sequencerz.randomize(mute::Sequencerz::BASIC); break;
//                 case 0x2d: sequencerz.randomize(mute::Sequencerz::EXT); break;
//                 default: break;
//             }
//         }
//     };

//     midi.on.noteoff = [&](auto chan, auto note)
//     {
//         if (chan == 1)
//         {
//             if (osc.frequency == mute::pitchToFrequency(note))
//                 env.gate = 0.0f;
//         }
//     };

//     float wavefolding = 1.0f;
//     float volume = 1.0f;
//     auto setVolume = [&](float v)
//     {
//         volume = v;
//         fmt::println(" VOL | {:.2f}", volume);
//     };
//     auto setEnveloppe = [&](float a, float d, float s, float r)
//     {
//         env.a = a;
//         env.d = d;
//         env.s = s;
//         env.r = r;
//         fmt::println("ADSR | {:.4f}ms / {:.4f}ms \\ {:.2f} — {:.4f}ms \\"
//             , 1e3*env.a
//             , 1e3*env.d
//             , env.s
//             , 1e3*env.r);
//     };
//     auto setOscParams = [&](float wave, float folding)
//     {
//         osc.wave = wave;
//         wavefolding = folding;
//         fmt::println(" OSC | w{:.2f} f{:.2f}", wave, folding);
//     };

//     midi.on.cc = [&](auto chan, auto control, auto value)
//     {
//         // fmt::println("CC {:#x} {}", control, value);
//         float fvalue = value / 127.0f;
//         switch (control)
//         {
//             case 0x72: setVolume(std::pow(fvalue, 2.0f)); break;
//             case 0x12: setOscParams(fvalue, wavefolding); break;
//             case 0x13: setOscParams(osc.wave, 10.0f * fvalue); break;
//             case 0x10:
//                 break;

//             case 0x0a: setEnveloppe(std::pow(fvalue, 2.0f), env.d, env.s, env.r); break;
//             case 0x4a: setEnveloppe(env.a, std::pow(fvalue, 2.0f), env.s, env.r); break;
//             case 0x47: setEnveloppe(env.a, env.d, fvalue, env.r); break;
//             case 0x4c: setEnveloppe(env.a, env.d, env.s, std::pow(fvalue, 2.0f)); break;

//             default:
//                 break;
//         }
//     };

//     midi.on.clock = [&]()
//     {
//         clock.tick();
//     };

//     midi.on.stop = [&]()
//     {
//         sequencerz.reset();
//         clock.reset();
//     };

//     clock.ontick = [&]()
//     {
//         sequencerz.next();
//     };

//     context.audioCallback = [&](mute::AudioProcessData data)
//     {
//         auto sampleRate = data.sampleRate;
//         midi.consumeMessages();
        
//         for (int f = 0; f < data.frames; f++)
//         {
//             sequencerz.process(sampleRate);
//             // osc.frequency = sequencerz.pitch.output();
//             // osc.wave = sequencerz.wave.output();
//             // env.gate = sequencerz.tg.output;
//             // osc.frequency = mute::pitchToFrequency(midi.notes[15].pitch);
//             // env.gate = midi.notes[15].on ? 1.0f : 0.0f;
//             osc.process(sampleRate);
//             env.process(sampleRate);
//             // osc.output *= mute::mvg(midi.notes[15].velocity);
//             // osc.output *= midi.notes[15].on ? 1.0f : 0.0f;
//             osc.output *= env.output;
//             // osc.output *= float(sequencerz.velocity.output());
//             osc.output = sin((wavefolding * (1.0f + 0.5f * wfmod)) * osc.output);
//             osc.output = mute::saturate(osc.output);
//             osc.output *= volume;
//             osc.output = mute::clamp(osc.output, -1.0f, 1.0f);

//             for (int c = 0; c < data.playback.channels; c++)
//                 data.playback.buffer[f * data.playback.channels + c] = osc.output;            
//         }
//     };

//     context.start();
// #if defined(__APPLE__)
//     CFRunLoopRun();
// #else
//     getchar();
// #endif
//     context.stop();
    
//     return 0;
// }