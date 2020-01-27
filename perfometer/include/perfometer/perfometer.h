// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/config.h>
#include <perfometer/thread.h>
#include <perfometer/time.h>

namespace perfometer
{
	enum result
	{
		ok,
		not_initialized,
		io_error,
		not_running,
		not_implemented
	};

	result initialize(const char file_name[] = "perfometer.report", bool running = true);
	result shutdown();

	result pause();
	result resume();

	result flush();

	result log_thread_name(const char thread_name[], thread_id id);
	result log_thread_name(const char thread_name[]);

	result log_work_start(const char tag_name[], time start_time);
	result log_work_end(time end_time);
	result log_work(const char tag_name[], time start_time, time end_time);
	
} // namespace perfometer
