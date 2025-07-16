#include "ui.h"

#include "app.h"

#include <fmt/format.h>

void UI::draw()
{
    drawHeader(cursor);
    drawContent(cursor);
    drawScopes(cursor);
}

void UI::drawHeader(glute::Cursor cursor)
{
    cursor.fillColor = ForegroundColor;

    // draw title
    {
        cursor.font = &font120;
        cursor.fontSize = 120;

        cursor.align = glute::AlignLeft;
        cursor.shadow = true;
        cursor.pos = { 100, 20 };
        cursor.text("MUTE");
    }
    
    cursor.font = &font30;
    cursor.fontSize = 30;

    // draw patch info
    {
        cursor.shadow = false;
        cursor.pos = { 980, 20 };
        cursor.align = glute::AlignRight;
        cursor.text(fmt::format("PATCH\n{}\n{}", PatchName, PatchCreationDate));
    }

    // draw status
    {
        cursor.fillColor = ForegroundColor;
        cursor.pos = { 1000, 0 };
        cursor.rect(400, 170);

        cursor.pos.x += 20;
        cursor.pos.y += 20;
        cursor.fillColor = HintColor;
        cursor.align = glute::AlignLeft;
        auto status = fmt::format("{:<10}{}", "STATUS", app->audio.transport.playing ? "PLAYING" : "PAUSED");
        auto bpm = fmt::format("{:<10}{:.2f}", "BPM", app->audio.transport.bpm);
        const auto& me = app->audio.transport.measure;
        const auto& ti = app->audio.transport.time;
        int minutes = ti.m();
        int seconds = int(ti.s()) % 60;
        int msec = int(ti.ms()) % 1000;
        auto position = fmt::format("{:<10}{}.{}.{}",
            "POSITION"
                , me.bars, me._16th, me._32th);
        auto time = fmt::format("{:<10}{:0>2}m:{:0>2}s:{:0>3}ms",
            "TIME"
                , minutes, seconds, msec);
        cursor.text(fmt::format("{}\n{}\n{}\n{}", status, bpm, position, time));
    }

    cursor.fillColor = ForegroundColor;
    cursor.shadow = false;
    cursor.pos = { 0, 170 };
    cursor.rect(1400, 2);
}

void UI::drawContent(glute::Cursor cursor)
{
    cursor.fillColor = ForegroundColor;
    cursor.font = &font30;
    cursor.fontSize = 30;
    cursor.align = glute::AlignLeft;

    // int patchCount = app->audio.patches.size();
    // const float yoff = 220;
    // const float xoff = 20;
    // const float barh = 1100;
    // const float barw = 30;
    // const float padw = 12;
    // for (int i = 0; i < patchCount; i++)
    // {
    //     const auto& patch = app->audio.patches[i];
    //     cursor.pos = { xoff + i * (barw + padw), yoff };
    //     auto poss = cursor.pos;
    //     cursor.shadow = false;
    //     cursor.strokeColor = ForegroundColor;
    //     cursor.strokeWidth = 2;
    //     for (int i = 0; i < 32; i++)
    //     {
    //         cursor.pos.x = poss.x;
    //         cursor.pos.y += (barh / 32.f);
    //         cursor.lineto(cursor.pos.x + barw, cursor.pos.y);
    //     }
    //     // cursor.fillColor = ForegroundColor;
    //     // cursor.rect(barw, barh);
    //     cursor.pos = poss;

    //     float v = mute::clamp(std::pow(std::abs(patch.output), 1.f), 0.f, 1.f);
    //     cursor.pos.y += barh - (v * barh);
    //     cursor.fillColor = HintColor;
    //     cursor.rect(barw, v * barh);

    //     cursor.strokeColor = HintColor;
    //     cursor.strokeWidth = 8;
    //     cursor.pos.x = poss.x;
    //     cursor.pos.y = poss.y + (1. - patch.volume) * barh;
    //     cursor.lineto(cursor.pos.x + barw, cursor.pos.y);

    //     cursor.pos = { xoff + i * (barw + padw), yoff + barh + 20 };
    //     cursor.fillColor = ForegroundColor;
    //     cursor.shadow = true;
    //     cursor.text(fmt::format("{:0>2}", i));
    // }

    // {
    //     cursor.pos = { 20, 220 };
    //     cursor.pos.y -= 30;
    //     cursor.text("PERC1:");
    //     cursor.pos.y += 30;

    //     auto length = fmt::format("{:->14}: {:0>4.2f}", " length", app->audio.p1.params[app->audio.p1.side].length);
    //     auto increment = fmt::format("{:->14}: {:0>4.2f}", " increment", app->audio.p1.params[app->audio.p1.side].increment);
    //     auto env1_time = fmt::format("{:->14}: {:0>4.2f}", " env1_time", app->audio.p1.params[app->audio.p1.side].env1_time);
    //     auto env1_skew = fmt::format("{:->14}: {:0>4.2f}", " env1_skew", app->audio.p1.params[app->audio.p1.side].env1_skew);
    //     auto env2_time = fmt::format("{:->14}: {:0>4.2f}", " env2_time", app->audio.p1.params[app->audio.p1.side].env2_time);
    //     auto env2_skew = fmt::format("{:->14}: {:0>4.2f}", " env2_skew", app->audio.p1.params[app->audio.p1.side].env2_skew);
    //     auto filterenv = fmt::format("{:->14}: {:0>4.2f}", " filterenv", app->audio.p1.params[app->audio.p1.side].filterenv);
    //     auto pitchmod = fmt::format("{:->14}: {:0>4.2f}", " pitchmod", app->audio.p1.params[app->audio.p1.side].pitchmod);
    //     auto filterfreq = fmt::format("{:->14}: {:0>4.2f}", " filterfreq", app->audio.p1.params[app->audio.p1.side].filterfreq);
    //     auto oscfreq = fmt::format("{:->14}: {:0>4.2f}", " oscfreq", app->audio.p1.params[app->audio.p1.side].oscfreq);
    //     auto noiseamt = fmt::format("{:->14}: {:0>4.2f}", " noiseamt", app->audio.p1.params[app->audio.p1.side].noiseamt);
    //     auto filterq = fmt::format("{:->14}: {:0>4.2f}", " filterq", app->audio.p1.params[app->audio.p1.side].filterq);

    //     auto content = fmt::format("{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n"
    //         , length
    //         , increment
    //         , env1_time
    //         , env1_skew
    //         , env2_time
    //         , env2_skew
    //         , filterenv
    //         , pitchmod
    //         , filterfreq
    //         , oscfreq
    //         , noiseamt
    //         , filterq
    //     );
    //     cursor.text(content);
    // }

    // {
    //     cursor.pos = { 20, 640 };
    //     cursor.pos.y -= 30;
    //     cursor.text("PERC2:");
    //     cursor.pos.y += 30;

    //     auto length = fmt::format("{:->14}: {:0>4.2f}", " length", app->audio.p2.params[app->audio.p2.side].length);
    //     auto increment = fmt::format("{:->14}: {:0>4.2f}", " increment", app->audio.p2.params[app->audio.p2.side].increment);
    //     auto env1_time = fmt::format("{:->14}: {:0>4.2f}", " env1_time", app->audio.p2.params[app->audio.p2.side].env1_time);
    //     auto env1_skew = fmt::format("{:->14}: {:0>4.2f}", " env1_skew", app->audio.p2.params[app->audio.p2.side].env1_skew);
    //     auto env2_time = fmt::format("{:->14}: {:0>4.2f}", " env2_time", app->audio.p2.params[app->audio.p2.side].env2_time);
    //     auto env2_skew = fmt::format("{:->14}: {:0>4.2f}", " env2_skew", app->audio.p2.params[app->audio.p2.side].env2_skew);
    //     auto filterenv = fmt::format("{:->14}: {:0>4.2f}", " filterenv", app->audio.p2.params[app->audio.p2.side].filterenv);
    //     auto pitchmod = fmt::format("{:->14}: {:0>4.2f}", " pitchmod", app->audio.p2.params[app->audio.p2.side].pitchmod);
    //     auto filterfreq = fmt::format("{:->14}: {:0>4.2f}", " filterfreq", app->audio.p2.params[app->audio.p2.side].filterfreq);
    //     auto oscfreq = fmt::format("{:->14}: {:0>4.2f}", " oscfreq", app->audio.p2.params[app->audio.p2.side].oscfreq);
    //     auto noiseamt = fmt::format("{:->14}: {:0>4.2f}", " noiseamt", app->audio.p2.params[app->audio.p2.side].noiseamt);
    //     auto filterq = fmt::format("{:->14}: {:0>4.2f}", " filterq", app->audio.p2.params[app->audio.p2.side].filterq);

    //     auto content = fmt::format("{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n"
    //         , length
    //         , increment
    //         , env1_time
    //         , env1_skew
    //         , env2_time
    //         , env2_skew
    //         , filterenv
    //         , pitchmod
    //         , filterfreq
    //         , oscfreq
    //         , noiseamt
    //         , filterq
    //     );
    //     cursor.text(content);
    // }
}

void drawGrid(glute::Cursor cursor, float width, float height, int hdiv, int vdiv)
{
    auto pos = cursor.pos;

    // Vertical lines
    const float xspacing = width / vdiv;
    for (int i = 0; i < vdiv + 1; i++)
    {
        float off = i * xspacing;
        cursor.moveto(pos.x + off, pos.y);
        cursor.lineto(pos.x + off, pos.y + height);
    }

    // Horizontal lines
    const float yspacing = height / hdiv;
    for (int i = 0; i < hdiv + 1; i++)
    {
        float off = i * yspacing;
        cursor.moveto(pos.x, pos.y + off);
        cursor.lineto(pos.x + width, pos.y + off);
    }
}

void UI::drawOscilloscope(glute::Cursor cursor)
{
    auto pos = cursor.pos;

    int points = 2048;
    cursor.strokeColor = HintColor;
    cursor.strokeWidth = 2;
    cursor.moveto(pos.x, pos.y + 200);

    const auto& hist = app->audio.history;
    {
        const float pointSpacing = 1400.f / points;
        const float bx = pos.x;
        const float by = pos.y + 200;
        const float amp = 200;

        const int histPoints = hist.size / 2;

        float ratio = 1.0f;
        int histAdvance = histPoints / points;
        // float ratio = float(histPoints) / float(points);
        // int histIndex = scopeOffset;
        for (int i = 0; i < points; i++)
        {
            // int histIndex = int((scopeOffset + i) * ratio) % (hist.data.size() / 2);

            float offset = i * pointSpacing;
            float left = hist.data[2 * (int(histIndex) % histPoints) + 0];
            float right = hist.data[2 * (int(histIndex) % histPoints) + 1];
            float p = std::clamp(0.5f * (left + right), -1.f, 1.f);
            // float p = std::sin(2.f * M_PI * i / float(points));
            cursor.lineto(bx + offset, by - p * amp);

            histIndex = int(histIndex + histAdvance / ratio) % histPoints;
        }
        // scopeOffset = int(scopeOffset + points * ratio) % (hist.data.size() / 2);
        // scopeOffset = histIndex;
    }

    cursor.strokeColor = ForegroundColor;
    cursor.fillColor = ForegroundColor;
    cursor.font = &font30;
    cursor.fontSize = 30;

    cursor.pos = pos;
    cursor.strokeColor = ForegroundColor;
    cursor.strokeWidth = 2;
    drawGrid(cursor, 1400, 400, 4, 6);

    // float xb = pos.x;
    // float yb = pos.y + 200;
    // // horiz lines
    // {
    //     // +0.5
    //     cursor.moveto(xb, yb-100);
    //     cursor.text("+0.5");
    //     cursor.lineto(xb+1400, yb-100);

    //     // zero
    //     cursor.moveto(xb, yb);
    //     cursor.text("+0.0");
    //     cursor.lineto(xb+1400, yb);

    //     // -0.5
    //     cursor.moveto(xb, yb+100);
    //     cursor.text("-0.5");
    //     cursor.lineto(xb+1400, yb+100);
    // }

    // // vert lines
    // int divs = 6;
    // float timediv = (hist.size / float(app->audio.driver.configuration.sampleRate)) / divs;
    // for (int i = 0; i < divs; i++)
    // {
    //     float off = i * (1400/divs);
    //     cursor.pos = { xb + off + 2, yb-200 };
    //     cursor.text(fmt::format("{:.2f}sec", i * timediv));
    //     cursor.moveto(xb + off, yb-200);
    //     cursor.lineto(xb + off, yb+200);
    // }
}

void SpectrumAnalyzer::compute(std::span<const float> in)
{
    // if (fft.size == 0)
    // {
    //     fft.init(in.size());
    //     output.resize(in.size());
    //     past.resize(in.size());

    //     spectrographImage = GenImageColor(512, 2048, BLACK);
    // }
    // fft.compute(in, past);
    // static constexpr float damp = 0.1;
    // for (size_t i = 0; i < past.size(); i++)
    // {
    //     if (isfinite(past[i]))
    //         // output[i] = past[i];
    //         output[i] = damp * output[i] + (1 - damp) * past[i];
    // }
    // for (int i = 0; i < spectrographImage.height; i++)
    // {
    //     float ip = float(i) / float(spectrographImage.height);
    //     ip = std::pow(ip, 3.f);
    //     float fi = ((past.size() - 1) / 2) * ip;
    //     int i1 = fi;
    //     int i2 = i1 + 1;
    //     float v1 = past[i1] / (fft.size / 2);
    //     float v2 = past[i2] / (fft.size / 2);
    //     float t = fi - float(i1);
    //     float vf = mute::lerp(v1, v2, t);
    //     float va = mute::clamp(std::pow(vf, 0.25f), 0.f, 1.f);
    //     float vb = mute::clamp(std::pow(vf * 2.f, 1.f), 0.f, 1.f);
    //     float vc = mute::clamp(std::pow(vf * 5.f, 5.f), 0.f, 1.f);
    //     vf = mute::clamp(std::pow(vf, 0.25f), 0.f, 1.f);

    //     static constexpr Color A = { .r = 20, .g = 10, .b = 100, .a = 255 };
    //     static constexpr Color B = { .r = 255, .g = 125, .b = 0, .a = 255 };
    //     static constexpr Color C = { .r = 125, .g = 125, .b = 255, .a = 255 };
    //     Color current = {
    //         .r = (unsigned char) (A.r*va + B.r*vb + C.r*vc),
    //         .g = (unsigned char) (A.g*va + B.g*vb + C.g*vc),
    //         .b = (unsigned char) (A.b*va + B.b*vb + C.b*vc),
    //         .a = 255
    //     };

    //     // static constexpr Color Low = { .r = 20, .g = 10, .b = 75, .a = 255 };
    //     // static constexpr Color High = { .r = 255, .g = 125, .b = 0, .a = 255 };
    //     // Color current = {
    //     //     .r = (unsigned char) (vf * High.r + (1 - vf) * Low.r),
    //     //     .g = (unsigned char) (vf * High.g + (1 - vf) * Low.g),
    //     //     .b = (unsigned char) (vf * High.b + (1 - vf) * Low.b),
    //     //     .a = 255
    //     // };
        
    //     ImageDrawPixel(&spectrographImage, spectroOffset, spectrographImage.height - 1 - i, current);
    // }
    // spectroOffset = (spectroOffset + 1) % spectrographImage.width;
}

void SpectrumAnalyzer::draw(glute::Cursor cursor)
{
    // auto pos = cursor.pos;

    // const int fftpoints = fft.size / 2;
    // const int points = std::min(4096, fftpoints);
    // cursor.strokeColor = HintColor;
    // cursor.strokeWidth = 2;
    // cursor.moveto(pos.x, pos.y + 200);

    // // const float pointSpacing = 1400.f / points;
    // const float bx = pos.x;
    // const float by = pos.y + 200;
    // const float amp = 200;

    // const int fftadvance = float(fftpoints) / float(points);
    // const float min = log10(fftadvance);
    // const float max = log10(fftpoints);
    // const float range = max - min;

    // const auto M = std::ranges::max(output);
    // const auto m = std::ranges::min(output);

    // (void) M;
    // (void) m;

    // for (int i = 0; i < points - 1; i++)
    // {
    //     float freq = (i + 1) * fftadvance;
    //     float lf = log10(freq);
    //     float offset = ((lf - min) / range + 0) * 1400.f;
    //     // float offset = i * pointSpacing;
    //     int index = freq;
    //     float v = output[index] / fftpoints;
    //     v = v*v;
    //     v = mute::clamp(std::pow(v, 0.1f), 0.f, 1.f);
        
    //     if (i == 0)
    //         cursor.moveto(bx + offset, by - amp * v);
    //     else
    //         cursor.lineto(bx + offset, by - amp * v);
    // }

    // cursor.pos = pos;
    // cursor.strokeColor = ForegroundColor;
    // cursor.strokeWidth = 2;
    // drawGrid(cursor, 1400, 200, 1, 1);
}

void UI::drawSpectrum(glute::Cursor cursor)
{
    // auto pos = cursor.pos;
    spectrumAnalyzer.draw(cursor);
}

void UI::drawSpectrograph(glute::Cursor cursor)
{
    // auto pos = cursor.pos;
    if (spectrographTexture.id == 0) {
        spectrographTexture = LoadTextureFromImage(spectrumAnalyzer.spectrographImage);
        SetTextureWrap(spectrographTexture, TEXTURE_WRAP_REPEAT);
    } else
        UpdateTexture(spectrographTexture, spectrumAnalyzer.spectrographImage.data);

    DrawTexturePro(
        spectrographTexture, {
            0.0f, 0.f,
            float(spectrographTexture.width), float(spectrographTexture.height)
        }, {
            cursor.pos.x/cursor.dpi, cursor.pos.y/cursor.dpi,
            1400.f / cursor.dpi, 800.f / cursor.dpi
        }, {
            0.0f, 0.0f
        }, 0.0f, WHITE);
}

void UI::drawScopes(glute::Cursor cursor)
{
    app->audio.history.read(spectrumInput);
    for (size_t i = 0; i < spectrumInput.size(); i+=2)
        spectrumInput[i/2] = (spectrumInput[i+0] + spectrumInput[i+1]) * 0.5; // (app->audio.history.data[i] + app->audio.history.data[i + 1]) * 0.5;
    spectrumAnalyzer.compute(std::span { spectrumInput.begin(), spectrumInput.size() / 2 });

    cursor.pos = { 1400, 0 };
    drawOscilloscope(cursor);

    // Draw current spectrum
    cursor.pos = {1400, 400};
    drawSpectrum(cursor);

    // Draw spectrograph
    cursor.pos = {1400, 600};
    drawSpectrograph(cursor);

    // // Draw crossfader
    // cursor.pos = { 1400 + 20, 500 };
    // cursor.fillColor = ForegroundColor;
    // cursor.font = &font30;
    // cursor.fontSize = 30;
    // cursor.text(fmt::format("FADER: {:.2f}\nSEED: {}", app->audio.common_t, app->audio.seedbase));

    // // Draw smiley
    // auto centerpos = Vector2 { xb + 700, yb + 700 };
    // cursor.shadow = true;
    // cursor.fillColor = ForegroundColor;
    // cursor.strokeWidth = 4.f;
    // cursor.pos = centerpos;
    // cursor.circle(200);

    // auto cross1Pos = Vector2 { centerpos.x - 140, centerpos.y - 100 };
    // cursor.strokeColor = HintColor;
    // cursor.strokeWidth = 8;
    // cursor.moveto(cross1Pos.x, cross1Pos.y);
    // cursor.lineto(cross1Pos.x + 50, cross1Pos.y + 50);
    // cursor.moveto(cross1Pos.x + 50, cross1Pos.y);
    // cursor.lineto(cross1Pos.x, cross1Pos.y + 50);

    // auto cross2Pos = Vector2 { centerpos.x + 80, centerpos.y - 100 };
    // cursor.strokeColor = HintColor;
    // cursor.strokeWidth = 8;
    // cursor.moveto(cross2Pos.x, cross2Pos.y);
    // cursor.lineto(cross2Pos.x + 50, cross2Pos.y + 50);
    // cursor.moveto(cross2Pos.x + 50, cross2Pos.y);
    // cursor.lineto(cross2Pos.x, cross2Pos.y + 50);

    // auto mouthPos = Vector2 { centerpos.x, centerpos.y };
    // cursor.moveto(mouthPos.x - 170, mouthPos.y);
    // cursor.lineto(mouthPos.x + 170, mouthPos.y);
}