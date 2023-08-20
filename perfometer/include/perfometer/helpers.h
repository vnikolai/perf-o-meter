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

#include <perfometer/perfometer.h>
#include <cstring>

namespace perfometer
{
    using log_functor = result (*)(string_id, time, time);

    template <log_functor functor>
    class scope_log
    {
    public:
        scope_log(string_id s_id)
            : m_name(s_id)
        {
            m_start_time = get_time();
            m_name = s_id;
        }

        ~scope_log()
        {
            time end_time = get_time();

            if (m_start_time != end_time )
            {
                functor(m_name, m_start_time, end_time);
            }
        }

    private:
        string_id m_name;
        time m_start_time;
    };
}

#if defined(__GNUC__) || defined( __clang__ )
#   define PERFOMETER_FUNCTION     __PRETTY_FUNCTION__
#else
#   define PERFOMETER_FUNCTION     __FUNCTION__
#endif

#define PERFOMETER_CONCAT_WRAPPER(x, y)     x##y
#define PERFOMETER_CONCAT(x, y)             PERFOMETER_CONCAT_WRAPPER(x, y)
#define PERFOMETER_UNIQUE( x )              PERFOMETER_CONCAT( x, __LINE__ )

#define PERFOMETER_REGISTER_STRING(name)                                    \
        static perfometer::string_id PERFOMETER_UNIQUE(s_id) =              \
            perfometer::register_string(name, std::strlen(name))            \

#define PERFOMETER_LOG_WORK_SCOPE(name)                                     \
        PERFOMETER_REGISTER_STRING(name);                                   \
        perfometer::scope_log<perfometer::log_work>                         \
            PERFOMETER_UNIQUE(logger)(PERFOMETER_UNIQUE(s_id))

#define PERFOMETER_LOG_WAIT_SCOPE(name)                                     \
        PERFOMETER_REGISTER_STRING(name);                                   \
        perfometer::scope_log<perfometer::log_wait>                         \
            PERFOMETER_UNIQUE(logger)(PERFOMETER_UNIQUE(s_id))

#define PERFOMETER_LOG_THREAD_NAME(name)                                    \
        PERFOMETER_REGISTER_STRING(name);                                   \
        perfometer::log_thread_name(PERFOMETER_UNIQUE(s_id));

#define PERFOMETER_LOG_EVENT(name)                                          \
        PERFOMETER_REGISTER_STRING(name);                                   \
        perfometer::log_event(PERFOMETER_UNIQUE(s_id), perfometer::get_time())

#define PERFOMETER_LOG_WORK_FUNCTION()      PERFOMETER_LOG_WORK_SCOPE(PERFOMETER_FUNCTION)
#define PERFOMETER_LOG_WAIT_FUNCTION()      PERFOMETER_LOG_WAIT_SCOPE(PERFOMETER_FUNCTION)
