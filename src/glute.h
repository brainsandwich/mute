#pragma once

#include <raylib.h>

#include <string_view>
#include <string>

namespace glute
{
    enum Align
    {
        AlignLeft,
        AlignCenter,
        AlignRight
    };

    struct Cursor
    {
        // Parameters
        Vector2 pos = {};
        const Font* font = nullptr;
        float fontSize = 0;
        Color fillColor = Color { 0, 0, 0, 0 };
        Color strokeColor = Color { 0, 0, 0, 0 };
        float strokeWidth = 1.0f;
        bool shadow = false;
        Align align = AlignLeft;

        // Config
        float dpi = 1;

        Vector2 measure(std::string_view content)
        {
            return MeasureTextEx(*font, content.data(), fontSize/dpi, 0.0f);
        }

        void text(std::string_view content)
        {
            static std::string textBuffer;
            if (content.empty())
                return;

            std::string_view firstLine = content;
            std::string_view remainder = {};
            auto lfpos = content.find("\n");
            if (lfpos != std::string_view::npos)
            {
                firstLine = content.substr(0, lfpos);
                remainder = content.substr(lfpos + 1);
            }

            Vector2 offset = pos;
            textBuffer = firstLine;
            auto [sx, sy] = measure(textBuffer.data());
            switch (align)
            {
                case AlignCenter: {
                    offset.x -= 0.5 * (sx * dpi);
                }
                    break;
                case AlignRight: {
                    offset.x -= sx * dpi;
                }
                    break;
                default:
                    break;
            }

            if (shadow)
                DrawTextEx(*font, textBuffer.data(), { (offset.x+2)/dpi, (offset.y+2)/dpi }, fontSize/dpi, 0.0f, BLACK);
            DrawTextEx(*font, textBuffer.data(), { offset.x/dpi, offset.y/dpi }, fontSize/dpi, 0.0f, fillColor);

            Vector2 posbefore = pos;
            if (!remainder.empty())
            {
                pos.y += dpi*sy;
                text(remainder);
            }
            pos = posbefore;
        }

        void rect(float width, float height)
        {
            DrawRectangleLinesEx({ pos.x/dpi, pos.y/dpi, width/dpi, height/dpi }, strokeWidth/dpi, strokeColor);
            DrawRectangleV({ pos.x/dpi, pos.y/dpi }, { width/dpi, height/dpi }, fillColor);
        }

        void moveto(float x, float y) { pos = { x, y }; }
        void lineto(float x, float y)
        {
            if (shadow)
                DrawLineEx({ (pos.x+2)/dpi, (pos.y+2)/dpi }, { (x+2)/dpi, (y+2)/dpi }, strokeWidth/dpi, BLACK);
            DrawLineEx({ pos.x/dpi, pos.y/dpi }, { x/dpi, y/dpi }, strokeWidth/dpi, strokeColor);
            pos = { x, y };
        }

        void circle(float radius)
        {
            if (shadow)
            {
                // DrawRing({ pos.x/dpi, pos.y/dpi }, (radius-strokeWidth/2)/dpi, (radius+strokeWidth/2)/dpi, 0, 2.f * M_PI, 32, strokeColor);
                // DrawCircleLinesV({ (pos.x+2)/dpi, (pos.y+2)/dpi }, radius/dpi, strokeColor);
                DrawCircleV({ (pos.x+2)/dpi, (pos.y+2)/dpi }, radius/dpi, BLACK);
            }
            // DrawRing({ pos.x/dpi, pos.y/dpi }, (radius-strokeWidth/2)/dpi, (radius+strokeWidth/2)/dpi, 0, 2.f * M_PI, 32, strokeColor);
            // DrawCircleLinesV({ pos.x/dpi, pos.y/dpi }, radius/dpi, strokeColor);
            DrawCircleV({ pos.x/dpi, pos.y/dpi }, radius/dpi, fillColor);
        }
    };
}