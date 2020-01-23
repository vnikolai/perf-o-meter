// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <fstream>

static bool s_initialized = false;
std::ofstream s_report_file;

const char FILE_HEADER[] = "PERFOMETER.1.0.0";

perfometer_result perfometer_initialize(const char fileName[])
{
	if (s_initialized)
	{
		return perfometer_result::OK;
	}

	s_report_file.open(fileName, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);

	if (!s_report_file)
	{
		return perfometer_result::IO_ERROR;
	}

	s_report_file << FILE_HEADER;

	s_initialized = true;

	return perfometer_result::OK;
}

perfometer_result perfometer_shutdown()
{
	if (!s_initialized)
	{
		return perfometer_result::NOT_INITIALIZED;
	}

	perfometer_result result = perfometer_flush();

	s_report_file.close();
	
	s_initialized = false;

	return result;
}

perfometer_result perfometer_flush()
{
	if (!s_initialized)
	{
		return perfometer_result::NOT_INITIALIZED;
	}

	s_report_file.flush();

	return s_report_file.fail() ?
		perfometer_result::IO_ERROR :
		perfometer_result::OK;
}
