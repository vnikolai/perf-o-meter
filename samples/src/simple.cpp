// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <iostream>

int main(int argc, const char** argv)
{
	auto result = perfometer::initialize();
	std::cout << "perfometer::initialize() returned " << result << std::endl;

	result = perfometer::shutdown();
	std::cout << "perfometer::shutdown() returned " << result << std::endl;

	return 0;
}
