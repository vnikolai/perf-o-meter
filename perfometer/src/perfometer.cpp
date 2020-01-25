// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <fstream>

namespace perfometer {

static bool s_initialized = false;
std::ofstream s_report_file;

const char FILE_HEADER[] = "PERFOMETER.1.0.0";

result initialize(const char fileName[])
{
	if (s_initialized)
	{
		return result::ok;
	}

	s_report_file.open(fileName, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);

	if (!s_report_file)
	{
		return result::io_error;
	}

	s_report_file << FILE_HEADER;

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

	s_report_file.flush();

	return s_report_file.fail() ? result::io_error : result::ok;
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