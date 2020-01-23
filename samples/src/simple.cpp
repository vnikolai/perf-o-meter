// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>

int main(int argc, const char** argv)
{
	if (perfometer_initialize() == perfometer_result::OK)
	{
		perfometer_shutdown();
	}

	return 0;
}
