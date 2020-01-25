// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <perfometer/config.h>

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
	
} // namespace perfometer
