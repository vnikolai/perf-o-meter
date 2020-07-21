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

// Sample creating 10 threads with varying payload and names

constexpr int num_threads = 10;
std::thread threads[num_threads];
const char* thread_names[num_threads] = { "job01", "job02", "job03", "job04", "job05",
										  "job06", "job07", "job08", "job09", "job10" };

void wait(unsigned int millisec)
{
	PERFOMETER_LOG_WAIT_FUNCTION();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
}

void sub_task()
{
	PERFOMETER_LOG_WORK_FUNCTION();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void task()
{
	PERFOMETER_LOG_WORK_FUNCTION();

	sub_task();

	wait(50);

	sub_task();
}

void job(int num_tasks)
{
	perfometer::log_thread_name(thread_names[num_tasks]);
	
	PERFOMETER_LOG_WORK_FUNCTION();

	for (int i = 0; i < num_tasks; ++i)
	{
		task();
	}
}

void start_threads()
{
	for (int i = 0; i < num_threads; ++i)
	{
		threads[i] = std::thread(job, i);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void waith_threads()
{
	for (int i = 0; i < num_threads; ++i)
	{
		std::thread& thread = threads[i];
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

int main(int argc, const char** argv)
{
	auto result = perfometer::initialize();
	std::cout << "perfometer::initialize() returned " << result << std::endl;

	perfometer::log_thread_name("MAIN_THREAD");

	start_threads();	

	waith_threads();

	result = perfometer::shutdown();
	std::cout << "perfometer::shutdown() returned " << result << std::endl;

	return 0;
}
