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
	auto result = perfometer::initialize("perfometer.report", false);
	
	// perfometer started in paused mode, my_func_to_trace will not be logged
	my_func_to_trace();

	// thread name will be logged even while work logging paused
	perfometer::log_thread_name("MAIN_THREAD");

	perfometer::resume();

	// my_enclosed_func will be logged
	my_enclosed_func();

	perfometer::pause();

	// perfometer paused, my_another_func not logged
	my_another_func();

	result = perfometer::shutdown();

	return 0;
}
