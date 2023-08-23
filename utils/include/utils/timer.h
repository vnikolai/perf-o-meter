/* Copyright 2023 Volodymyr Nikolaichuk

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
#include <iostream>

namespace perfometer
{
    namespace utils
    {
        using timer_callback = void (*)(time);

        template <timer_callback callback>
        struct timer
        {
            timer()
            {
                m_start_time = get_time();
            }

            ~timer()
            {
                callback(get_time() - m_start_time);
            }

        private:
            time m_start_time;
        };

        struct logging_timer
        {
            static void log(perfometer::time duration)
            {
                std::cout << perfometer::utils::time_to_string(duration * 1.0 / perfometer::get_clock_frequency()) << std::endl;
            }

            private:
                timer<logging_timer::log> m_timer;
        };
    }
} // namespace perfometer
