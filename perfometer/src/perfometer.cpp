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

static mutex s_strings_mutex;
static std::vector<std::string> s_strings;
constexpr string_id invalid_string_id = std::numeric_limits<string_id>::max();

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
				scoped_lock lock(s_serializer_mutex);

				switch (record.type)
				{
					case format::record_type::string:
					{
						s_strings_mutex.lock();
						s_serializer << format::record_type::string
									 << record.s_id
									 << s_strings[record.s_id].c_str();
						s_strings_mutex.unlock();
						break;
					}
					case format::record_type::thread_name:
					{
						s_serializer << format::record_type::thread_name
				 					 << record.t_id
				 					 << record.s_id;
						break;
					}
					case format::record_type::work:
					case format::record_type::wait:
					{
						s_serializer << record.type
									 << record.s_id
									 << record.start
									 << record.end
									 << record.t_id;
						break;
					}
					case format::record_type::event:
					{
						s_serializer << format::record_type::event
									 << record.s_id
									 << record.start
									 << record.t_id;
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
	string_id s_id = register_string(unknown_tag);

	s_serializer << format::record_type::string
				 << s_id
				 << unknown_tag;

	s_serializer << format::record_type::string
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
	s_record_cache = nullptr;

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

	*s_record_cache << record{
		format::record_type::thread_name,
		s_id,
		0,
		0,
		t_id};

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

	*s_record_cache << record{
		format::record_type::work,
		s_id,
		start_time,
		end_time,
		get_thread_id()};

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

	*s_record_cache << record{
		format::record_type::wait,
		s_id,
		start_time,
		end_time,
		get_thread_id()};

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

	*s_record_cache << record{
		format::record_type::event,
		s_id,
		t,
		time(0),
		get_thread_id()};

	return result::ok;
}

string_id register_string(const char name[])
{
	return register_string(std::string(name));
}

string_id register_string(std::string&& string)
{
	static std::atomic<string_id> s_unique_id(0);
	s_strings.reserve(1024);

	string_id s_id = s_unique_id;
	if (s_unique_id == invalid_string_id)
	{
		return invalid_string_id;
	}

	{
		s_strings_mutex.lock();
		s_strings.push_back(string);
		s_strings_mutex.unlock();
	}

	s_unique_id++;

	result res = ensure_buffer();
	if (res != result::ok)
	{
		return res;
	}
	
	*s_record_cache << record{
		format::record_type::string,
		s_id,
		0,
		0,
		thread_id()};

	return s_id;
}

} // namespace perfometer