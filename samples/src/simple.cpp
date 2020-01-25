// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <iostream>
#include <thread>
#include <chrono>

void my_func_to_trace()
{
	perfometer::log_work_start(__FUNCTION__, perfometer::get_time());

	std::this_thread::sleep_for(std::chrono::seconds(1));

	perfometer::log_work_end(perfometer::get_time());
}

void my_enclosed_func()
{
	auto start = perfometer::get_time();

	std::this_thread::sleep_for(std::chrono::seconds(2));

	perfometer::log_work(__FUNCTION__, start, perfometer::get_time());
}

void my_another_func()
{
	auto start = perfometer::get_time();

	my_enclosed_func();

	perfometer::log_work(__FUNCTION__, start, perfometer::get_time());
}

int main(int argc, const char** argv)
{
	auto result = perfometer::initialize();
	std::cout << "perfometer_initialize() returned " << result << std::endl;

	my_func_to_trace();

	my_another_func();

	result = perfometer::shutdown();
	std::cout << "perfometer_shutdown() returned " << result << std::endl;

	return 0;
}
