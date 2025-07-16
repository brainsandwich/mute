#pragma once

#include "glute.h"
#include "constants.h"
#include "mute/fft.h"

#include <raylib.h>
#include <vector>
#include <span>

struct MuteApp;

// struct Scope
// {
//     std::array<float, 2048> data;
//     int offset = 0;
// };

struct SpectrumAnalyzer
{
    // mute::FFT fft;
    std::vector<float> output;
    std::vector<float> past;
    Image spectrographImage;
    int spectroOffset = 0;

    void compute(std::span<const float> in);
    void draw(glute::Cursor cursor);
};

struct UI
{
    glute::Cursor cursor;

    MuteApp* app = nullptr;
    Font font120;
    Font font30;

    int histIndex = 0;
    SpectrumAnalyzer spectrumAnalyzer;
    Texture2D spectrographTexture;
    std::vector<float> spectrumInput = std::vector<float>((1<<11)*2);

    void draw();
    void drawHeader(glute::Cursor cursor);
    void drawContent(glute::Cursor cursor);

    void drawOscilloscope(glute::Cursor cursor);
    void drawSpectrum(glute::Cursor cursor);
    void drawSpectrograph(glute::Cursor cursor);
    void drawScopes(glute::Cursor cursor);
};