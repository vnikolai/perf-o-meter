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
#include <ctime>
#include <iostream>
#include <thread>

constexpr int num_threads = 10;
std::thread threads[num_threads];

void wait(unsigned int microsec)
{
    PERFOMETER_LOG_WAIT_FUNCTION();
    PERFOMETER_LOG_EVENT(PERFOMETER_FUNCTION);

    std::this_thread::sleep_for(std::chrono::microseconds(microsec));
}

void sub_task(unsigned int microsec)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    std::time_t result = std::time(nullptr);
    PERFOMETER_LOG_DYNAMIC_EVENT(std::ctime(&result));

    std::this_thread::sleep_for(std::chrono::microseconds(microsec));
}

void task()
{
    PERFOMETER_LOG_WORK_FUNCTION();

    sub_task(2);

    wait(1);

    sub_task(2);
}

void job(int num_tasks)
{
    PERFOMETER_LOG_THREAD_NAME("WORKER");
    std::cout << "job " << num_tasks << " started" << std::endl;

    PERFOMETER_LOG_WORK_FUNCTION();

    for (int i = 0; i < num_tasks; ++i)
    {
        task();
    }

    std::cout << "job " << num_tasks << " done" << std::endl;
}

void start_work_threads()
{
    PERFOMETER_LOG_WORK_FUNCTION();

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i] = std::thread(job, i*1000);

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void wait_work_threads()
{
    PERFOMETER_LOG_WAIT_FUNCTION();

    for (int i = 0; i < num_threads; ++i)
    {
        std::thread& thread = threads[i];
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void test_late_start()
{
    start_work_threads();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    auto result = perfometer::initialize("test_late_start.report");
    std::cout << "perfometer::initialize() returned " << result << std::endl;
    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    wait_work_threads();

    result = perfometer::shutdown();
    std::cout << "perfometer::shutdown() returned " << result << std::endl;
}

void test_early_stop()
{
    auto result = perfometer::initialize("test_early_stop.report");
    std::cout << "perfometer::initialize() returned " << result << std::endl;
    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    start_work_threads();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    result = perfometer::shutdown();
    std::cout << "perfometer::shutdown() returned " << result << std::endl;

    wait_work_threads();
}

void test_start_stop_repeat()
{
    start_work_threads();

    for (int i = 0; i < 100; ++i)
    {
        auto result = perfometer::initialize("test_start_stop_repeat.report");
        std::cout << "perfometer::initialize() returned " << result << std::endl;
        PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        result = perfometer::shutdown();
        std::cout << "perfometer::shutdown() returned " << result << std::endl;
    }

    wait_work_threads();
}

int main(int argc, const char** argv)
{
    test_late_start();
    test_early_stop();
    test_start_stop_repeat();

    return 0;
}
