// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include "serializer.h"

namespace perfometer {

static bool s_initialized = false;
serializer s_report_file;

result initialize(const char file_name[])
{
	if (s_initialized)
	{
		return result::ok;
	}

	result res = s_report_file.open_file_stream(file_name);
	if (res != result::ok)
	{
		s_report_file.close();
		return res;
	}

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

	s_report_file.close();
	
	s_initialized = false;

	return res;
}

result flush()
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	return s_report_file.flush();
}

result log_thread_name(const char thread_name[], thread_id id)
{
	if (!s_initialized)
	{
		return result::not_initialized;
	}

	return result::not_implemented;
}

result log_thread_name(const char thread_name[])
{
	return log_thread_name(thread_name, get_thread_id());
}

result log_work_start(const char tag_name[], time start_time)
{
	return result::not_implemented;
}

result log_work_end(time end_time)
{
	return result::not_implemented;
}

result log_work(const char tag_name[], time start_time, time end_time)
{
	return result::not_implemented;
}

} // namespace perfometer