// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#pragma once

enum perfometer_result
{
	OK,
	NOT_INITIALIZED,
	IO_ERROR
};

perfometer_result perfometer_initialize(const char fileName[] = "profiling.report");
perfometer_result perfometer_shutdown();

perfometer_result perfometer_flush();
