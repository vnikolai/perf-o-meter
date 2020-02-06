// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

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

void sub_task()
{
	PERFOMETER_LOG_FUNCTION();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void task()
{
	PERFOMETER_LOG_FUNCTION();

	sub_task();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	sub_task();
}

void job(int num_tasks)
{
	perfometer::log_thread_name(thread_names[num_tasks]);
	
	PERFOMETER_LOG_FUNCTION();

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
