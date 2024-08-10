#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace Color
{
    class RGBColor;
    class HSVColor;

    class RGBColor
    {
    public:
        RGBColor(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {};
        uint8_t red, green, blue;
        HSVColor toHSV();
    };

    class HSVColor
    {
    public:
        HSVColor(uint16_t h, uint8_t s, uint8_t v) : hue(h), saturation(s), value(v) {};
        uint16_t hue;
        uint8_t saturation, value;
        RGBColor toRGB();
    };
}