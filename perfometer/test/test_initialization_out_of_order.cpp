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
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <gtest/gtest.h>

namespace {

constexpr int num_threads = 10;
std::thread threads[num_threads];

auto application_start_time = std::chrono::steady_clock::now();

void wait(unsigned int microsec)
{
    PERFOMETER_LOG_WAIT_FUNCTION();
    PERFOMETER_LOG_EVENT(PERFOMETER_FUNCTION);

    std::this_thread::sleep_for(std::chrono::microseconds(microsec));
}

void sub_task(unsigned int microsec)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    auto since_app_start = std::chrono::steady_clock::now() - application_start_time;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(since_app_start);

    std::stringstream ss;
    ss << microseconds.count() << " us since app start";

    PERFOMETER_LOG_DYNAMIC_EVENT(ss.str());

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

    PERFOMETER_LOG_WORK_FUNCTION();

    for (int i = 0; i < num_tasks; ++i)
    {
        task();
    }
}

void start_work_threads()
{
    PERFOMETER_LOG_WORK_FUNCTION();

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i] = std::thread(job, i*10);

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

TEST(initialization, late_start)
{
    start_work_threads();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    ASSERT_EQ(perfometer::initialize("test_late_start.report"), perfometer::result::ok);
    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    wait_work_threads();

    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, early_stop)
{
    ASSERT_EQ(perfometer::initialize("test_early_stop.report"), perfometer::result::ok);
    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    start_work_threads();

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
    wait_work_threads();
}

TEST(initialization, start_stop_repeat)
{
    start_work_threads();

    for (int i = 0; i < 100; ++i)
    {
        ASSERT_EQ(perfometer::initialize("test_start_stop_repeat.report"), perfometer::result::ok);
        PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
    }

    wait_work_threads();
}

} // namespace