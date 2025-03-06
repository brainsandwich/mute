#include "mute/driver.h"

#include <raylib.h>

static constexpr int DEFAULT_SAMPLERATE = 44100;
static constexpr int DEFAULT_BUFFERSIZE = 256;

int main(int argc, char** argv)
{
    InitWindow(800, 600, "mute");
    SetTargetFPS(60);

    auto driver = mute::AudioDriver({
        .playback = {
            .channels = 2
        },
        .sampleRate = DEFAULT_SAMPLERATE,
        .bufferSize = DEFAULT_BUFFERSIZE,
        .callback = [&](mute::AudioProcessData data) {  }
    });

    driver.start();
    while (!WindowShouldClose())
    {
        PollInputEvents();
        // ...
        SwapScreenBuffer();
    }
    driver.stop();
    CloseWindow();

    return 0;
}