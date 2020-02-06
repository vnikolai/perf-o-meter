// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <perfometer/perfometer.h>
#include <perfometer/helpers.h>
#include <iostream>
#include <thread>
#include <chrono>

// Simple starting report and using PERFOMETER_LOG_FUNCTION macro to trace functions performance

void my_func_to_trace()
{
	PERFOMETER_LOG_FUNCTION();

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void my_enclosed_func()
{
	PERFOMETER_LOG_FUNCTION();

	std::this_thread::sleep_for(std::chrono::milliseconds(350));
}

void my_another_func()
{
	PERFOMETER_LOG_FUNCTION();

	my_enclosed_func();
}

int main(int argc, const char** argv)
{
	auto result = perfometer::initialize();
	std::cout << "perfometer::initialize() returned " << result << std::endl;

	perfometer::log_thread_name("MAIN_THREAD");

	my_func_to_trace();

	my_another_func();

	result = perfometer::shutdown();
	std::cout << "perfometer::shutdown() returned " << result << std::endl;

	return 0;
}
