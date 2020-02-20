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
