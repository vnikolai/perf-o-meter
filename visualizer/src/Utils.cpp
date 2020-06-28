/* Copyright 2020 Volodymyr Nikolaichuk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include "Utils.h"

namespace visualizer {

std::string formatTime(double time)
{
    if (time < 0)
    {
        return std::string("-") + formatTime(-time);
    }

    constexpr double precision = 1.0 / 10000000; // 0.1 nanosec precision    
    std::string suffix;
    double denom = 0;

    int withFraction = 0;
    bool showPart = false;

    if (time < 1e-6)
    {
        if (abs(time) < precision)
        {
            return std::string("0");
        }

        // nanos
        suffix = "ns";
        denom = 1000000000;
    }
    else if (time < 1e-3 - precision)
    {
        // micros
        suffix = "us";
        denom = 1000000;
    }
    else if (time < 1 - precision)
    {
        // millis
        suffix = "ms";
        denom = 1000;
    }
    else if (time < 60 - precision)
    {
        // seconds
        suffix = "s";
        denom = 1;

        withFraction = time < 10 ? 100 : 10;
    }
    else if (time < 3600 - precision)
    {
        // minutes
        suffix = "m";
        denom = 1.0/60;

        showPart = true;
    }
    else
    {
        // hours
        suffix = "h";
        denom = 1.0/3600.0;

        showPart = true;
    }

    std::string result;
    if (withFraction)
    {
        int value = static_cast<int>(time * denom + precision);
        int fraction = static_cast<int>((time * denom - value) * withFraction);
        if (fraction >= 1)
        {
            char text[16];
            std::snprintf(text, 16, "%d.%d", value, fraction);

            return std::string(text) + suffix;
        }
    }
    
    result = std::to_string(static_cast<int>(time * denom + precision)) + suffix;

    if (showPart)
    {
        double fract = time * denom - static_cast<int>(time * denom + precision);
        if (fract + precision >= denom)
        {
            result += " " + formatTime(fract / denom);
        }
    }

    return result;
}

} // namespace visualizer
