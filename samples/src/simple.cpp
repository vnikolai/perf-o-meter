// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <iostream>

int main(int argc, const char** argv)
{
	auto result = perfometer_initialize();
	std::cout << "perfometer_initialize() returned " << result << std::endl;

	result = perfometer_shutdown();
	std::cout << "perfometer_shutdown() returned " << result << std::endl;

	return 0;
}
