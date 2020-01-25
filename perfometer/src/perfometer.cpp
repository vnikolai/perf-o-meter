// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include "serializer.h"

namespace perfometer {

static bool s_initialized = false;
serializer s_report_file;

result initialize(const char fileName[])
{
	if (s_initialized)
	{
		return result::ok;
	}

	result res = s_report_file.open_file_stream(fileName);
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

result log_work_start(const char tagName[], time startTime)
{
	return result::ok;
}

result log_work_end(time endTime)
{
	return result::ok;
}

result log_work(const char tagName[], time startTime, time endTime)
{
	return result::ok;
}

} // namespace perfometer