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

#include <perfometer/perfometer.h>
#include <perfometer/mutex.h>
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
static mutex s_serializer_mutex;

static bool s_logging_enabled = false;

static bool s_logger_thread_running = false;
static std::thread s_logger_thread;

static std::queue<record_buffer*> s_logger_records_queue;
static std::unordered_map<thread_id, record_buffer*> s_records_inprogress;
static mutex s_records_mutex;
static thread_local record_buffer* s_record_cache = nullptr;

std::unordered_map<std::string, string_id> s_strings_map;
mutex s_string_map_mutex;

constexpr string_id invalid_string_id = std::numeric_limits<string_id>::max();

using get_string_result = std::pair<string_id, bool>;

get_string_result get_string_id(const char* string)
{
	scoped_lock lock(s_string_map_mutex);

	auto it = s_strings_map.find(string);
	if (it != s_strings_map.end())
	{
		return get_string_result(it->second, false);
	}

	static string_id s_unique_id = 0;

	if (s_unique_id == invalid_string_id)
	{
		return get_string_result(invalid_string_id, false);
	}

	s_strings_map.emplace(string, s_unique_id);

	return get_string_result(s_unique_id++, true);
}

void logger_thread()
{
	while (s_logger_thread_running)
	{
		record_buffer* buffer = nullptr;
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
			for (size_t i = 0; i < buffer->count; ++i)
			{
				const record& record = buffer->records[i];

				auto string_result = get_string_id(record.name);

				scoped_lock lock(s_serializer_mutex);
				if (string_result.second)
				{
					s_serializer << format::record_type::string
								<< string_result.first
								<< record.name;
				}

				switch (record.type)
				{
					case format::record_type::work:
					case format::record_type::wait:
					{
						s_serializer << record.type
									 << string_result.first
									 << record.start
									 << record.end
									 << record.tid;
						break;
					}
					case format::record_type::event:
					{
						s_serializer << format::record_type::event
									 << string_result.first
									 << record.start
									 << record.tid;
						break;
					}
					default:
					{
						break;
					}
				}
			}

			delete buffer;
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

	// writing "UKNOWN" to string map first to ocupy zero ID
	constexpr char unknown_tag[] = "UNKNOWN";
	auto string_result = get_string_id(unknown_tag);
	if (string_result.second)
	{
		s_serializer << format::record_type::string
					 << string_result.first
					 << unknown_tag;
	}

	s_serializer << format::record_type::string
				 << string_id(invalid_string_id)
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
			record_buffer* buffer = pair.second;
			s_logger_records_queue.push(buffer);
		}

		s_records_inprogress.clear();
	}

	result res = flush();

	s_logger_thread_running = false;

	if (s_logger_thread.joinable())
	{
		s_logger_thread.join();
	}

	scoped_lock lock(s_serializer_mutex);

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
		s_logger_records_queue.push(s_record_cache);
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

	scoped_lock lock(s_serializer_mutex);
	return s_serializer.flush();
}

result ensure_buffer()
{
#if defined(PERFOMETER_LOG_RECORD_SWAP_OVERHEAD)
	time start_time = get_time();
#endif
	if (s_record_cache && s_record_cache->count == records_cache_size)
    {
		flush_thread_cache();
    }

	if (s_record_cache == nullptr)
	{
		s_record_cache = new record_buffer();
		if (!s_record_cache || !s_record_cache->records)
		{
			delete s_record_cache;
			s_record_cache = nullptr;

			return result::no_memory_available;
		}

		auto tid = get_thread_id();
		scoped_lock lock(s_records_mutex);
		s_records_inprogress[tid] = s_record_cache;
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

result log_thread_name(const char thread_name[], thread_id tid)
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	if (thread_name == nullptr)
	{
		return result::invalid_arguments;
	}

	scoped_lock lock(s_serializer_mutex);

	auto string_result = get_string_id(thread_name);
	if (string_result.second)
	{
		s_serializer << format::record_type::string
					 << string_result.first
					 << thread_name;
	}

	s_serializer << format::record_type::thread_name
				 << tid
				 << string_result.first;

	return s_serializer.status();
}

result log_thread_name(const char thread_name[])
{
	return log_thread_name(thread_name, get_thread_id());
}

result log_work(const char tag_name[], time start_time, time end_time)
{
	if (!s_logging_enabled)
	{
		return result::not_running;
	}

	if (tag_name == nullptr)
	{
		return result::invalid_arguments;
	}

	result res = ensure_buffer();
	if (res != result::ok)
	{
		return res;
	}

        *s_record_cache << record{
			format::record_type::work,
			tag_name,
            start_time,
			end_time,
            get_thread_id()};

        return result::ok;
}

result log_wait(const char tag_name[], time start_time, time end_time)
{
	if (!s_logging_enabled)
	{
		return result::not_running;
	}

	if (tag_name == nullptr)
	{
		return result::invalid_arguments;
	}

	result res = ensure_buffer();
	if (res != result::ok)
	{
		return res;
	}

	*s_record_cache << record{
		format::record_type::wait,
		tag_name,
		start_time,
		end_time,
		get_thread_id()};

	return result::ok;
}

result log_event(const char tag_name[], time t)
{
	if (!s_logging_enabled)
	{
		return result::not_running;
	}

	if (tag_name == nullptr)
	{
		return result::invalid_arguments;
	}

	result res = ensure_buffer();
	if (res != result::ok)
	{
		return res;
	}

	*s_record_cache << record{
		format::record_type::event,
		tag_name,
		t,
		time(0),
		get_thread_id()};

	return result::ok;
}

} // namespace perfometer