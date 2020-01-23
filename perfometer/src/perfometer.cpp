// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>

static bool s_initialized = false;

perfometer_result perfometer_initialize()
{
	s_initialized = true;

	return perfometer_result::OK;
}

perfometer_result perfometer_shutdown()
{
	if (!s_initialized)
	{
		return perfometer_result::NOT_INITIALIZED;
	}

	s_initialized = false;

	return perfometer_result::OK;
}
