// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

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