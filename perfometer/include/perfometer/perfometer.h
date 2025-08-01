/* Copyright 2020-2025 Volodymyr Nikolaichuk

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

    // initializes perfometer engine
    result initialize(const char file_name[] = "perfometer.report", bool running = true);
    // shuts down perfometer engine
    result shutdown();

    // pauses writing log data
    result pause();
    // resumes writing log data
    result resume();

    // flushes records cached in current thread page into file writing queue
    result flush_thread_cache();
    // waits untill file writing queue is written to file
    result flush();

    // register static reusable string, returns assigned string id, up until string_id::max
    string_id register_string(const char* string);
    string_id register_string(const char* string, size_t len);

    // writes string id without registration, to be used once, returns format::dynamic_string_id
    string_id write_string(const char* string, size_t len);

    // log record_type::thread_name block with string id, thread id
    result log_thread_name(string_id str_id, thread_id t_id);
    // log record_type::thread_name block with string id and current thread id
    result log_thread_name(string_id str_id);

    // log record_type::work block with string id, start time, end time
    result log_work(string_id str_id, time start_time, time end_time);
    // begin record_type::work block with string id, start time
    result log_work_start(string_id str_id, time start_time);
    // log last block in current thread with end time
    result log_work_end(time end_time);

    // log record_type::wait block with string id, start time, end time
    result log_wait(string_id str_id, time start_time, time end_time);
    // begin record_type::wait block with string id, start time
    result log_wait_start(string_id str_id, time start_time);
    // log last block in current thread with end time
    result log_wait_end(time end_time);

    // log record_type::event block with string id, time
    result log_event(string_id str_id, time t);

} // namespace perfometer
