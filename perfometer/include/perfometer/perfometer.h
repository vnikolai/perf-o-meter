// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#pragma once

enum perfometer_result
{
	OK,
	NOT_INITIALIZED
};

perfometer_result perfometer_initialize();
perfometer_result perfometer_shutdown();
