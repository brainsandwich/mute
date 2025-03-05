#pragma once

#include "mute/modules/sequencer.h"
#include "mute/modules/sequencer.h"

struct DrumPatch
{
    float volume = 1;

    mute::EuclideanSequencer seq { .length = 8, .increment = 3 };
    mute::Sequencer pitch { .values = { 100, 30, 30, 30 } };
    mute::Sequencer filfreq; // = mute::Sequencer::init(4, 0, 0.8);
    
    mute::Perc perc;

    int rando = 0;
    int rsee = 0;

    void rnd()
    {
        // mute::RAND_ENGINE.seed((rsee + 9) * 223);
        // rsee++;
        // perc.noiseamt = mute::random(0, 0.3);
        // perc.env1.skew = mute::random(0.4, 0.9);
        // perc.env2.skew = mute::random(0.4, 0.9);
        // perc.env1.time = mute::random(0.4, 1.0);
        // perc.env2.time = mute::random(0.4, 1.0);
        // seq.length = mute::random(6, 9);
        // seq.increment = mute::random(2, 5);
        // pitch.randomize(20, 100);
        // filfreq.randomize(0, 0.3);

        fmt::println("EUCLID {}:{}", seq.length, seq.increment);
        fmt::println("ENV1   {:.2f}:{:.2f}", perc.env1.time, perc.env1.skew);
        fmt::println("ENV2   {:.2f}:{:.2f}", perc.env2.time, perc.env2.skew);
        fmt::println("PITCH  {:.2f}", fmt::join(pitch.values, ", "));
        fmt::println("FILTER {:.2f}", fmt::join(filfreq.values, ", "));
    }

    DrumPatch() { rnd(); }

    float process(float sr, mute::Transport transport)
    {
        if (transport.measure.ticks.bar)
            rando++;

        if (transport.measure.ticks._16th)
        {
            seq.next();
            pitch.next();
            filfreq.next();
        }

        seq.process(sr);
        pitch.process(sr);
        filfreq.process(sr);

        perc.oscfreq = pitch.output;
        perc.gate = seq.output;
        perc.filterenv = filfreq.output;
        perc.process(sr);

        float out = perc.output;
        out = mute::saturate(out, 4.f);
        return out;
    }

    void process(mute::AudioProcessData data, mute::Transport transport)
    {
        for (int f = 0; f < data.frames; f++)
        {
            transport.update(1. / data.sampleRate);

            float out = mute::clamp(volume * process(data.sampleRate, transport));
            for (int c = 0; c < data.playback.channels; c++)
                data.playback.buffer[f * data.playback.channels + c] = out;
        }
    }
};

struct OscSequencePatch
{
    float volume = 1;

    mute::SineOscillator osc1;
    mute::SineOscillator osc2;
    mute::AHDSR env = { .a = 0.001, .d = 0.3, .s = 0.0, .r = 0.1 };
    mute::AHDSR fmenv = { .a = 0.001, .d = 0.2, .s = 0.0, .r = 0.1 };

    mute::Sequencer metaseq_selector;
    mute::Sequencer metaseq_length;
    
    struct Seqz
    {
        std::array<mute::Sequencer, 32> cv;
        std::array<mute::EuclideanSequencer, 8> gate;

        void reset()
        {
            for (auto& c: cv)
                c.reset();
            for (auto& g: gate)
                g.reset();
        }
        void next()
        {
            for (auto& c: cv)
                c.next();
            for (auto& g: gate)
                g.next();
        }
        void randomize()
        {
            // for (auto& c: cv)
            //     c.randomize(8, 0, 1);
            // for (auto& g: gate)
            // {
            //     g.length = mute::random(3, 30);
            //     g.increment = mute::random(3, 30);
            // }
        }
        void process(float sr)
        {
            for (auto& c: cv)
                c.process(sr);
            for (auto& g: gate)
                g.process(sr);
        }
    };

    std::array<Seqz, 32> seqz;

    int seqindex = 0;
    int indexResetCounter = 0;
    Seqz& currentSeqz() { return seqz[seqindex % seqz.size()]; }

    // void boogie_swang()
    // {
    //     mute::RAND_ENGINE.seed(23);
    //     metaseq_selector.randomize(4, 0, 31);
    //     metaseq_length.values = { 4, 4, 4, 4 };
    //     for (auto& s: seqz)
    //         s.randomize();
    // }

    // void net_radio()
    // {
    //     mute::RAND_ENGINE.seed(28);
    //     metaseq_selector.randomize(4, 0, 31);
    //     metaseq_length.values = { 4, 4, 4, 4 };
    //     for (auto& s: seqz)
    //         s.randomize();
    // }

    // void mainframe_error()
    // {
    //     mute::RAND_ENGINE.seed(28);
    //     metaseq_selector.randomize(2, 0, 31);
    //     metaseq_length.values = { 5, 4, 2, 4 };
    //     for (auto& s: seqz)
    //         s.randomize();
    // }

    // void rnd()
    // {
    //     mute::RAND_ENGINE.seed(mute::random(2, 22222));
    //     metaseq_selector.randomize(mute::random(2, 6), 0, 31);
    //     metaseq_length.randomize(mute::random(2, 6), 3, 6);
    //     for (auto& s: seqz)
    //         s.randomize();
    // }

    OscSequencePatch()
    {
        // mute::RAND_ENGINE.seed(93399);
        // metaseq_selector.randomize(4, 0, 31);
        // metaseq_length.values = { 4, 4, 3, 1 };
        // for (auto& s: seqz)
        //     s.randomize();
    }

    int se = 0;

    float process(float sr, mute::Transport transport)
    {
        bool next = false;
        metaseq_length.process(sr);
        metaseq_selector.process(sr);

        // if (transport.measure.ticks.bar)
        // {
        //     int seb = se;
        //     se = (se + 1) % (3*4);
        //     if (seb / 4 != se / 4)
        //     {
        //         fmt::println(">> LEAD CHANGE !!");
        //         if (se / 4 == 0) { fmt::println("boogie_swang"); boogie_swang(); }
        //         if (se / 4 == 1) { fmt::println("net_radio"); net_radio(); }
        //         if (se / 4 == 2) { fmt::println("mainframe_error"); mainframe_error(); }
        //     }
        // }

        if (transport.measure.ticks._16th)
        {
            indexResetCounter++;
            if (indexResetCounter >= metaseq_length.output)
            {
                next = true;
                indexResetCounter = 0;
                metaseq_selector.next();
                metaseq_length.next();
            }
        }

        seqindex = metaseq_selector.output;

        auto& seq = currentSeqz();
        if (next)
            seq.reset();
        seq.gate[0].gatelen = seq.cv[8].output * 0.1;
        seq.gate[1].gatelen = seq.cv[9].output * 0.1;
        if (transport.measure.ticks._16th)
        {
            // const auto& m = transport.measure;
            // fmt::println("{}.{}", m.bars, m._16th);

            seq.next();
        }
        
        seq.process(sr);

        fmenv.gate = seq.gate[0].output;
        fmenv.a = seq.cv[0].output * 0.3 + 0.001;
        fmenv.d = seq.cv[1].output * 0.2 + 0.5;
        fmenv.process(sr);

        float o1pitch = seq.cv[2].output * 100;
        osc1.frequency = o1pitch + seq.cv[3].output * 0.2 * fmenv.output * sr;
        osc1.process(sr);

        float o2pitch = seq.cv[4].output * 80 + 20;
        osc2.frequency = o2pitch + 0.04 * sr * osc1.output * osc1.output * (seq.cv[5].output + 0.1 * fmenv.output);
        osc2.process(sr);

        env.gate = seq.gate[1].output;
        env.a = seq.cv[6].output * 0.01 + 0.001;
        env.d = seq.cv[7].output * 0.3 + 0.8;
        env.process(sr);

        float out = osc2.output * env.output;
        out = mute::saturate(out, 1.8f);
        return out;
    }

    void process(mute::AudioProcessData data, mute::Transport transport)
    {
        for (int f = 0; f < data.frames; f++)
        {
            transport.update(1. / data.sampleRate);

            float out = mute::clamp(volume * process(data.sampleRate, transport), -1.f, 1.f);
            for (int c = 0; c < data.playback.channels; c++)
                data.playback.buffer[f * data.playback.channels + c] = out;
        }
    }
};