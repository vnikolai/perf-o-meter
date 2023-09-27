/* Copyright 2020-2023 Volodymyr Nikolaichuk

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

#include <perfometer/config.h>

namespace perfometer
{
    enum result
    {
        ok,
        io_error,
        invalid_arguments,
        no_memory_available,
        not_initialized,
        not_running,
        not_implemented,
        overflow,
        file_not_found,
        wrong_format,
        newer_format
    };

    result initialize(const char file_name[] = "perfometer.report", bool running = true);
    result shutdown();

    result pause();
    result resume();

    result flush_thread_cache();
    result flush();

    string_id register_string(const char* string);
    string_id register_string(const char* string, size_t len);

    result log_thread_name(string_id s_id, thread_id t_id);
    result log_thread_name(string_id s_id);

    result log_work(string_id s_id, time start_time, time end_time);
    result log_wait(string_id s_id, time start_time, time end_time);

    result log_event(string_id s_id, time t);

} // namespace perfometer
