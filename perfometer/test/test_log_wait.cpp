/* Copyright 2020-2025 Volodymyr Nikolaichuk

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
#include <perfometer/format.h>
#include <utils/report_reader.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_utils.hpp"

namespace {

const std::string wait_block_name = "wait block";

class wait_record_validator : public perfometer::utils::report_reader
{
public:
    perfometer::utils::perf_time expected_start = 0;
    perfometer::utils::perf_time expected_end = 0;

    MOCK_METHOD(void, wait_handled, ());

    void handle_wait(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end) override
    {
        EXPECT_EQ(wait_block_name, string_by_id(string_id));
        EXPECT_EQ(thread_id, bit_cast<perfometer::utils::perf_thread_id>(perfometer::get_thread_id()));
        
        EXPECT_DOUBLE_EQ(time_start, convert_time(expected_start));
        EXPECT_DOUBLE_EQ(time_end, convert_time(expected_end));

        wait_handled();
    }
};

TEST(log_wait, not_running_not_initialized)
{
    EXPECT_EQ(perfometer::log_wait(0, 0, 0), perfometer::not_running);
}

TEST(log_wait, not_running_initialized_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait(0, 0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait(10, 10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait, not_running_explicitly_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    ASSERT_EQ(perfometer::pause(), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait(0, 0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait(10, 10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait_start, not_running_not_initialized)
{
    EXPECT_EQ(perfometer::log_wait_start(0, 0), perfometer::not_running);
}

TEST(log_wait_start, not_running_initialized_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_start(0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait_start(10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait_start, not_running_explicitly_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    ASSERT_EQ(perfometer::pause(), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_start(0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait_start(10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait_end, not_running_not_initialized)
{
    EXPECT_EQ(perfometer::log_wait_end(0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait_end(10), perfometer::not_running);
}

TEST(log_wait_end, not_running_initialized_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_end(0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait_end(10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait_end, not_running_explicitly_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    ASSERT_EQ(perfometer::pause(), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_end(0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_wait_end(10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait_end, invalid_arguments_wait_not_started)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_end(perfometer::format::invalid_string_id), perfometer::invalid_arguments);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_wait, records_completed_block)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    const perfometer::string_id string_id = perfometer::register_string(wait_block_name.c_str());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time start_time = 1000;
    const perfometer::time end_time = 2500;
    EXPECT_EQ(perfometer::log_wait(string_id, start_time, end_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    wait_record_validator validator;
    validator.expected_start = start_time;
    validator.expected_end = end_time;

    EXPECT_CALL(validator, wait_handled());

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

TEST(log_wait_start_end, records_started_block)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    const perfometer::string_id string_id = perfometer::register_string(wait_block_name.c_str());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time start_time = 4000;
    const perfometer::time end_time = 6000;

    EXPECT_EQ(perfometer::log_wait_start(string_id, start_time), perfometer::ok);
    EXPECT_EQ(perfometer::log_wait_end(end_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    wait_record_validator validator;
    validator.expected_start = start_time;
    validator.expected_end = end_time;

    EXPECT_CALL(validator, wait_handled());

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

} // namespace