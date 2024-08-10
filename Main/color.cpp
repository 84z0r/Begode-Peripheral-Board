#include "color.h"
#include <algorithm>

Color::RGBColor Color::HSVColor::toRGB()
{
    Color::RGBColor result = { uint8_t(0), uint8_t(0), uint8_t(0) };

    int f = (this->hue % 10922) * 255 / 10922;
    int p = (this->value * (255 - this->saturation)) / 255;
    int q = (this->value * (255 - (this->saturation * f) / 255)) / 255;
    int t = (this->value * (255 - (this->saturation * (255 - f)) / 255)) / 255;

    switch (this->hue / 10922)
    {
        case 0: result.red = this->value; result.green = t;           result.blue = p; 	         break;
        case 1: result.red = q;           result.green = this->value; result.blue = p;           break;
        case 2: result.red = p;           result.green = this->value; result.blue = t; 	         break;
        case 3: result.red = p;           result.green = q;           result.blue = this->value; break;
        case 4: result.red = t;           result.green = p;           result.blue = this->value; break;
        case 5: result.red = this->value; result.green = p;           result.blue = q; 	         break;
    }

    return result;
}

Color::HSVColor Color::RGBColor::toHSV()
{
    Color::HSVColor result = { 0, 0, 0 };
    int32_t calc = 0;

    uint16_t max_color = std::max(this->red, std::max(this->green, this->blue));
    uint16_t min_color = std::min(this->red, std::min(this->green, this->blue));

    result.value = (max_color * 255) / 255; // Scaling to [0, 255]

    if (!max_color) result.saturation = 0;
    else result.saturation = ((max_color - min_color) * 255) / max_color;

    if (!result.saturation) calc = 0;
    else
    {
        if (max_color == this->red)
        {
            calc = (10922 * (this->green - this->blue)) / (max_color - min_color);
        }
        else if (max_color == this->green)
        {
            calc = 21845 + (10922 * (this->blue - this->red)) / (max_color - min_color);
        }
        else
        {
            calc = 43690 + (10922 * (this->red - this->green)) / (max_color - min_color);
        }
    }

    if (calc < 0)
    {
        calc += 65535;
    }

    result.hue = calc;

    return result;
}