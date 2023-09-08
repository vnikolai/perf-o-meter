/* Copyright 2020-2021 Volodymyr Nikolaichuk

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

#include <perfometer/perfometer.h>
#include "record_buffer.h"
#include "serializer.h"
#include <string>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <queue>
#include <thread>
#include <atomic>
#include <limits>

namespace perfometer {

static bool s_initialized = false;
serializer s_serializer;

static bool s_logging_enabled = false;

static bool s_logger_thread_running = false;
static std::thread s_logger_thread;

static std::queue<std::shared_ptr<record_buffer>> s_logger_records_queue;
static std::unordered_map<thread_id, std::shared_ptr<record_buffer>> s_records_inprogress;
static mutex s_records_mutex;
static thread_local std::shared_ptr<record_buffer> s_record_cache = nullptr;

constexpr string_id invalid_string_id = std::numeric_limits<string_id>::max();

void logger_thread()
{
    while (s_logger_thread_running)
    {
        std::shared_ptr<record_buffer> buffer = nullptr;
        {
            scoped_lock lock(s_records_mutex);

            if (!s_logger_records_queue.empty())
            {
                buffer = s_logger_records_queue.front();
                s_logger_records_queue.pop();
            }
        }

        if (buffer)
        {
            s_serializer.write(buffer->data(), buffer->used_size());
        }
        else
        {
            std::this_thread::yield();
        }
    }
}

result initialize(const char file_name[], bool running)
{
    if (s_initialized)
    {
        return result::ok;
    }

    if (file_name == nullptr)
    {
        return result::invalid_arguments;
    }

    result res = s_serializer.open_file_stream(file_name);
    if (res != result::ok)
    {
        s_serializer.close();
        return res;
    }

    formatter<serializer> output(s_serializer);

    output.write(format::header, sizeof(format::header) - 1);
    output << format::major_version
           << format::minor_version
           << format::patch_version;

    unsigned char time_size = sizeof(time);
    auto start_time = get_time();
    auto clock_frequency = get_clock_frequency();

    output << format::record_type::clock_configuration
           << time_size
           << clock_frequency
           << start_time;

    unsigned char thread_id_size = sizeof(thread_id);
    output << format::record_type::thread_info
           << thread_id_size
           << get_thread_id();

    // writing "UKNOWN" to string map first to ocupy zero ID
    constexpr char unknown_tag[] = "UNKNOWN";
    string_id s_id = register_string(unknown_tag);

    output << format::record_type::string
           << s_id
           << unknown_tag;

    output << format::record_type::string
           << invalid_string_id
           << "String limit overflow";

    s_logger_thread_running = true;
    s_logger_thread = std::thread(logger_thread);

    s_logging_enabled = running;

    s_initialized = true;

    return result::ok;
}

result shutdown()
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    s_logging_enabled = false;

    {
        scoped_lock lock(s_records_mutex);

        for (auto pair : s_records_inprogress)
        {
            std::shared_ptr<record_buffer> buffer = pair.second;
            std::shared_ptr<record_buffer> copy(new record_buffer(*buffer));
            s_logger_records_queue.push(std::move(copy));
        }

        s_records_inprogress.clear();
        s_record_cache.reset();
    }

    result res = flush();

    s_logger_thread_running = false;

    if (s_logger_thread.joinable())
    {
        s_logger_thread.join();
    }

    s_serializer.flush();
    s_serializer.close();

    s_initialized = false;

    return res;
}

result pause()
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    s_logging_enabled = false;

    return result::ok;
}

result resume()
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    s_logging_enabled = true;

    return result::ok;
}

result flush_thread_cache()
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    if (s_record_cache)
    {
        scoped_lock lock(s_records_mutex);
        s_logger_records_queue.push(std::move(s_record_cache));
    }

    s_record_cache = nullptr;

    return result::ok;
}

result flush()
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    bool logger_done = false;
    while (!logger_done)
    {
        {
            scoped_lock lock(s_records_mutex);
            logger_done = s_logger_records_queue.empty();
        }

        if (!logger_done)
        {
            std::this_thread::yield();
        }
    }

    return s_serializer.flush();
}

result ensure_buffer()
{
#if defined(PERFOMETER_LOG_RECORD_SWAP_OVERHEAD)
    time start_time = get_time();
#endif
    if (s_record_cache && s_record_cache->free_size() < 256)
    {
        flush_thread_cache();
    }

    if (s_record_cache == nullptr)
    {
        s_record_cache = std::make_shared<record_buffer>();
        if (!s_record_cache || !s_record_cache->data())
        {
            s_record_cache = nullptr;

            return result::no_memory_available;
        }

        auto t_id = get_thread_id();
        scoped_lock lock(s_records_mutex);
        s_records_inprogress[t_id] = s_record_cache;
    }

#if defined(PERFOMETER_LOG_RECORD_SWAP_OVERHEAD)
    time end_time = get_time();
    if (end_time - start_time > 1000)
    {
        log_work(__FUNCTION__, start_time, end_time);
    }
#endif

    return result::ok;
}

result log_thread_name(string_id s_id, thread_id t_id)
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    if (s_id == invalid_string_id)
    {
        return result::invalid_arguments;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);

    output << format::record_type::thread_name
           << t_id
           << s_id;

    return result::ok;
}

result log_thread_name(string_id s_id)
{
    return log_thread_name(s_id, get_thread_id());
}

result log_work(string_id s_id, time start_time, time end_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (s_id == invalid_string_id)
    {
        return result::invalid_arguments;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);

    output << format::record_type::work
           << s_id
           << start_time
           << end_time
           << get_thread_id();

    return result::ok;
}

result log_wait(string_id s_id, time start_time, time end_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (s_id == invalid_string_id)
    {
        return result::invalid_arguments;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);

    output << format::record_type::wait
           << s_id
           << start_time
           << end_time
           << get_thread_id();

    return result::ok;
}

result log_event(string_id s_id, time t)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (s_id == invalid_string_id)
    {
        return result::invalid_arguments;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);

    output << format::record_type::event
           << s_id
           << t
           << get_thread_id();

    return result::ok;
}

string_id register_string(const char* string)
{
    return register_string(string, std::strlen(string));
}

string_id register_string(const char* string, size_t len)
{
    static std::atomic<string_id> s_unique_id(0);

    string_id s_id = s_unique_id;
    if (s_unique_id == invalid_string_id)
    {
        return invalid_string_id;
    }

    s_unique_id++;

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);
    
    output << format::record_type::string
           << s_id;
    output.write_string(string, len);

    return s_id;
}

} // namespace perfometer