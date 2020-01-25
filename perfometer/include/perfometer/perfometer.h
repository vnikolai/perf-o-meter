// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/config.h>
#include <perfometer/time.h>

namespace perfometer
{
	enum result
	{
		ok,
		not_initialized,
		io_error
	};

	result initialize(const char fileName[] = "profiling.report");
	result shutdown();

	result flush();

	result log_work_start(const char tagName[], time startTime);
	result log_work_end(time endTime);
	result log_work(const char tagName[], time startTime, time endTime);
	
} // namespace perfometer
