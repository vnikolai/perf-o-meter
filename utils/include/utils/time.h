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

#pragma once

#include <perfometer/time.h>
#include <string>
#include <ostream>

namespace perfometer
{
    namespace utils
    {
        std::string time_to_string(double t);
        std::string time_to_string(perfometer::time t, perfometer::time freq);

        enum time_format
        {
            automatic,
            seconds,
            milliseconds,
            microseconds,
            nanoseconds
        };

        struct time_formatter
        {
            time_formatter(double t, time_format fmt)
                : m_time(t)
                , m_tfmt(fmt)
            {                
            }

            template<typename time>
            time_formatter(time t, time f, time_format fmt)
                : m_time(static_cast<double>(t)/f)
                , m_tfmt(fmt) {}
            
            double m_time;
            time_format m_tfmt;

            friend std::ostream& operator << (std::ostream& stream, const time_formatter& formatter)
            {
                switch (formatter.m_tfmt)
                {
                    case time_format::automatic:
                    {
                        stream << perfometer::utils::time_to_string(formatter.m_time);
                        break;
                    }
                    case time_format::seconds:
                    {
                        stream << formatter.m_time << " s";
                        break;
                    }
                    case time_format::milliseconds:
                    {
                        stream << std::fixed << static_cast<uint64_t>(formatter.m_time * 1000) << " ms";
                        break;
                    }
                    case time_format::microseconds:
                    {
                        stream << std::fixed << static_cast<uint64_t>(formatter.m_time * 1000000) << " us";
                        break;
                    }
                    case time_format::nanoseconds:
                    {
                        stream << std::fixed << static_cast<uint64_t>(formatter.m_time * 1000000000) << " ns";
                        break;
                    }
                    default:
                    {
                        stream << "time_formatter: unknown time format";
                        break;
                    }
                }

                return stream;
            }
        };
    } // namespace perfometer
} // namespace perfometer
