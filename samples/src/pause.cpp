/* Copyright 2020 Volodymyr Nikolaichuk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <perfometer/perfometer.h>
#include <iostream>
#include <thread>
#include <chrono>

// Using log_work functions directoy in contrast with PERFOMETER_LOG_WORK_FUNCTION helper macro
// Starting reporting paused, and using perfometer::pause()/perfometer::resume() to control logging

void my_func_to_trace()
{
	auto start = perfometer::get_time();

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	perfometer::log_work(__FUNCTION__, start, perfometer::get_time());
}

void my_enclosed_func()
{
	auto start = perfometer::get_time();

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

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
	std::cout << "perfometer::initialize() returned " << result << std::endl;
	
	// perfometer started in paused mode, my_func_to_trace will not be logged
	my_func_to_trace();

	// thread name will be logged even while work logging paused
	perfometer::log_thread_name("MAIN_THREAD");

	result = perfometer::resume();
	std::cout << "perfometer::resume() returned " << result << std::endl;

	// my_another_func will be logged
	my_another_func();

	result = perfometer::pause();
	std::cout << "perfometer::pause() returned " << result << std::endl;

	// perfometer paused, my_enclosed_func not logged
	my_enclosed_func();

	result = perfometer::shutdown();
	std::cout << "perfometer::shutdown() returned " << result << std::endl;

	return 0;
}
