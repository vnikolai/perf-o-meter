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

#include <perfometer/perfometer.h>

#include "record_buffer.h"
#include "serializer.h"

#include <atomic>
#include <cstring>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

namespace perfometer {

struct record_block_start {
    record_block_start(format::record_type t, string_id sid, time st)
        : type(t), str_id(sid), start_time(st)
    {
    }

    format::record_type type;
    string_id str_id;
    time start_time;
};

static bool s_initialized = false;
serializer s_serializer;

static bool s_logging_enabled = false;

static bool s_logger_thread_running = false;
static std::thread s_logger_thread;

static std::queue<std::shared_ptr<record_buffer>> s_logger_records_queue;
static std::unordered_map<thread_id, std::shared_ptr<record_buffer>> s_records_inprogress;
static mutex s_records_mutex;
static thread_local std::shared_ptr<record_buffer> s_record_cache = nullptr;
static thread_local std::stack<record_block_start> s_started_blocks;

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
            formatter<serializer> output(s_serializer);
            
            output << format::record_type::page
                   << uint16_t(buffer->used_size());
            
            output.write(buffer->data(), buffer->used_size());

            output << format::record_type::page_end;
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

    uint8_t time_size = sizeof(time);
    auto start_time = get_time();
    auto clock_frequency = get_clock_frequency();

    output << format::record_type::clock_configuration
           << time_size
           << clock_frequency
           << start_time;

    uint8_t thread_id_size = sizeof(thread_id);
    output << format::record_type::thread_info
           << thread_id_size
           << get_thread_id();

    output << format::record_type::string
           << format::unknown_string_id
           << "UNKNOWN";

    output << format::record_type::string
           << format::dynamic_string_id
           << "Dynamic string";

    output << format::record_type::string
           << format::invalid_string_id
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
        result res = flush_thread_cache();
        if (res != result::ok)
        {
            return res;
        }
    }

    if (s_record_cache == nullptr)
    {
        s_record_cache = std::make_shared<record_buffer>();
        if (!s_record_cache || !s_record_cache->data())
        {
            s_record_cache = nullptr;

            return result::no_memory_available;
        }

        thread_id t_id = get_thread_id();
        formatter<record_buffer>(*s_record_cache) << t_id;

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

result log_thread_name(string_id str_id, thread_id t_id)
{
    if (!s_initialized)
    {
        return result::not_initialized;
    }

    if (str_id == format::invalid_string_id)
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
           << str_id;

    return result::ok;
}

result log_thread_name(string_id str_id)
{
    return log_thread_name(str_id, get_thread_id());
}

result log_work(string_id str_id, time start_time, time end_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (str_id == format::invalid_string_id)
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
           << str_id
           << start_time
           << end_time
           << get_thread_id();

    return result::ok;
}

result log_work_start(string_id str_id, time start_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (str_id == format::invalid_string_id)
    {
        return result::invalid_arguments;
    }

    s_started_blocks.emplace(format::record_type::work, str_id, start_time);

    return result::ok;
}

result log_block_end(time end_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (s_started_blocks.size() == 0)
    {
        return result::invalid_arguments;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    const record_block_start& block_start = s_started_blocks.top();

    formatter<record_buffer> output(*s_record_cache);

    output << block_start.type
           << block_start.str_id
           << block_start.start_time
           << end_time
           << get_thread_id();

    s_started_blocks.pop();

    return result::ok;
}

result log_work_end(time end_time)
{
    return log_block_end(end_time);
}

result log_wait(string_id str_id, time start_time, time end_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (str_id == format::invalid_string_id)
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
           << str_id
           << start_time
           << end_time
           << get_thread_id();

    return result::ok;
}

result log_wait_start(string_id str_id, time start_time)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (str_id == format::invalid_string_id)
    {
        return result::invalid_arguments;
    }

    s_started_blocks.emplace(format::record_type::wait, str_id, start_time);

    return result::ok;
}

result log_wait_end(time end_time)
{
    return log_block_end(end_time);
}

result log_event(string_id str_id, time t)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    if (str_id == format::invalid_string_id)
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
           << str_id
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
    // reserved ids
    // 0 - "UNKNOWN"
    // 1 - dynamic string marker
    // string_id::max - invalid id
    static std::atomic<uint32_t> s_unique_id(2);

    uint32_t str_id32 = s_unique_id.fetch_add(1, std::memory_order_relaxed);
    if (str_id32 >= format::invalid_string_id)
    {
        s_unique_id.store(format::invalid_string_id, std::memory_order_relaxed);
        return format::invalid_string_id;
    }

    string_id str_id = static_cast<string_id>(str_id32);

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);
    
    output << format::record_type::string
           << str_id;
    output.write_string(string, len);

    return str_id;
}

string_id write_string(const char* string, size_t len)
{
    if (!s_logging_enabled)
    {
        return result::not_running;
    }

    result res = ensure_buffer();
    if (res != result::ok)
    {
        return res;
    }

    formatter<record_buffer> output(*s_record_cache);
    
    output << format::record_type::string
           << format::dynamic_string_id;
    output.write_string(string, len);

    return format::dynamic_string_id;
}

} // namespace perfometer