#include <algorithm>

#include <variant>
#include <string>
#include <array>
#include <vector>

#include <fmt/printf.h>
#include <fmt/std.h>

#include "raylib.h"

#include "mute/driver.h"
#include "patch/modal_drummer.h"

static constexpr Color BackgroundColor = Color { 102, 102, 102, 255 };
static constexpr Color ShadowColor = Color { 0, 0, 0, 255 };
static constexpr Color ForegroundColor = Color { 245, 76, 0, 255 };
static constexpr Color HintColor = Color { 255, 255, 255, 255 };

struct MIDIKeyboard
{
    //  1 3   6 8 10
    // 0 2 4 5 7 9 11 12
    // 
    //  z e   t y u
    // q s d f g h j k
    //
    //  w x -> octave shifts

    int octave = 4;
    std::array<bool, 13> status = {};
    std::vector<int> playedNotes = std::vector<int>(13);

    std::string_view keyNoteName(int key)
    {
        return std::array {
            "C", "C#",
            "D", "D#",
            "E",
            "F", "F#",
            "G", "G#",
            "A", "A#",
            "B",
            "C"
        }[key];
    }

    void process()
    {
        int index = 0;
        #define k(key) status[index++] = IsKeyPressed(KEY_ ## key)
        k(A); k(W); k(S); k(E); k(D); k(F); k(T); k(G); k(Y); k(H); k(U); k(J); k(K);
        #undef k

        if (IsKeyPressed(KEY_Z)) { octave--; fmt::println("-- octave down ({})", octave); }
        if (IsKeyPressed(KEY_X)) { octave++; fmt::println("-- octave up ({})", octave); }
        octave = std::clamp(octave, 0, 20);

        int pressedKeys = 0;
        for (int i = 0; i < (int) status.size(); i++)
        {
            if (status[i])
            {
                playedNotes.push_back(i + octave * 12);
                pressedKeys++;
            }
        }

        playedNotes.resize(pressedKeys);
    }
};

int main(int argc, char** argv)
{
    fmt::println("---- MUTE patch {} // {}\n{}----"
        , patch::PatchName
        , patch::PatchCreationDate
        , patch::PatchDesc);

    auto patch = patch::ModalDrummer {};

    auto driver = mute::Driver {
        .configuration = {
            .playback = mute::Device { .channels = 2 },
            .bufferSize = 64,
            .sampleRate = 48000
        },
        .audioCallback = [&](mute::ProcessContext pc)
        {
            float sr = pc.sampleRate;

            for (int f = 0; f < pc.frames; f++)
            {
                patch.process(sr);
                pc.playback.buffer[2 * f + 0] = mute::clamp(patch.output.left, -1, 1);
                pc.playback.buffer[2 * f + 1] = mute::clamp(patch.output.right, -1, 1);
            }
        }
    };
    driver.init();
    driver.start();
    
    getchar();

    driver.stop();
    driver.uninit();
    return 0;
}