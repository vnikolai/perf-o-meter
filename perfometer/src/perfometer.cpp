// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include "serializer.h"
#include <string>
#include <unordered_map>
#include <stack>

namespace perfometer {

static bool s_initialized = false;
serializer s_serializer;

static bool s_logging_running = false;

std::unordered_map<std::string, string_id> s_strings_map;

string_id get_string_id(const char* string)
{
	auto it = s_strings_map.find(string);
	if (it != s_strings_map.end())
	{
		return it->second;
	}

	static string_id s_unique_id = invalid_string_id + 1;

	if (s_unique_id == invalid_string_id)
	{
		// loop after overflow 
		return invalid_string_id;
	}

	s_strings_map.emplace(string, s_unique_id);

	return s_unique_id++;
}

result initialize(const char file_name[], bool running)
{
	if (s_initialized)
	{
		return result::ok;
	}

	result res = s_serializer.open_file_stream(file_name);
	if (res != result::ok)
	{
		s_serializer.close();
		return res;
	}

	s_serializer << record_type::string
				 << string_id(invalid_string_id)
				 << "String limit overflow";

	s_logging_running = running;
	s_initialized = true;
	
	return result::ok;
}

result shutdown()
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	result res = flush();

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

	s_logging_running = false;

	return result::ok;
}

result resume()
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	s_logging_running = true;

	return result::ok;
}

result flush()
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	return s_serializer.flush();
}

result log_thread_name(const char thread_name[], thread_id id)
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	// TODO asynchornous
	
	string_id name_id = get_string_id(thread_name);
	if (name_id != invalid_string_id)
	{
		s_serializer << record_type::string
			  		 << name_id
		  			 << thread_name;
	}

	s_serializer << record_type::thread_name
		  		 << id
		  		 << name_id;

	return s_serializer.status();
}

result log_thread_name(const char thread_name[])
{
	return log_thread_name(thread_name, get_thread_id());
}

result log_work(const char tag_name[], time start_time, time end_time)
{
	if (!s_logging_running)
	{
		return result::not_running;
	}

	// TODO asynchornous
	string_id name_id = get_string_id(tag_name);

	if (name_id != invalid_string_id)
	{
		s_serializer << record_type::string
			  		 << name_id
			  		 << tag_name;
	}

	s_serializer << record_type::work
				 << name_id
				 << start_time
				 << end_time
				 << get_thread_id();

	return s_serializer.status();
}

} // namespace perfometer