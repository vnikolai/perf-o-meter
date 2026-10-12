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

class report_validator : public perfometer::utils::report_reader
{
public:

    MOCK_METHOD(void, clock_configuration_handled, ());
    MOCK_METHOD(void, thread_info_handled, ());

    void handle_clock_configuration(uint8_t time_size, perfometer::utils::perf_time clock_frequency, perfometer::utils::perf_time init_time) override
    {
        EXPECT_EQ(time_size, sizeof(perfometer::time));
        EXPECT_EQ(clock_frequency, perfometer::get_clock_frequency());
        EXPECT_GE(perfometer::get_time(), init_time);

        clock_configuration_handled();
    }

    void handle_thread_info(uint8_t thread_id_size, perfometer::utils::perf_thread_id main_thread_id) override
    {
        EXPECT_EQ(thread_id_size, sizeof(perfometer::thread_id));
        EXPECT_EQ(main_thread_id, bit_cast<perfometer::utils::perf_thread_id>(perfometer::get_thread_id()));

        thread_info_handled();
    }
};

TEST(initialization, invalid_arguments_nullptr)
{
    EXPECT_EQ(perfometer::initialize(nullptr), perfometer::result::invalid_arguments);
}

TEST(initialization, cannot_open_file)
{
    EXPECT_EQ(perfometer::initialize(""), perfometer::result::io_error);
}

TEST(initialization, ok)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, paused_ok)
{
    EXPECT_EQ(perfometer::initialize("test.report", false), perfometer::result::ok);
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, repeated_initialization_ok)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);

    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, repeated_init_shutdown_ok)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);

    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
    }
}

TEST(initialization, shutdown_without_initialization)
{
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::not_initialized);
}

TEST(initialization, pause)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::pause(), perfometer::result::ok);
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, pause_not_initialized)
{
    EXPECT_EQ(perfometer::pause(), perfometer::result::not_initialized);
}

TEST(initialization, resume)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::pause(), perfometer::result::ok);
    EXPECT_EQ(perfometer::resume(), perfometer::result::ok);
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);
}

TEST(initialization, resume_not_initialized)
{
    EXPECT_EQ(perfometer::resume(), perfometer::result::not_initialized);
}

TEST(initialization, flush_not_initialized)
{
    EXPECT_EQ(perfometer::flush(), perfometer::result::not_initialized);
}

TEST(initialization, flush_thread_cache_not_initialized)
{
    EXPECT_EQ(perfometer::flush_thread_cache(), perfometer::result::not_initialized);
}

TEST(initialization, header_clock_and_thread_info)
{
    EXPECT_EQ(perfometer::initialize("test.report"), perfometer::result::ok);
    EXPECT_EQ(perfometer::shutdown(), perfometer::result::ok);

    report_validator validator;
    EXPECT_CALL(validator, clock_configuration_handled());
    EXPECT_CALL(validator, thread_info_handled());

    EXPECT_EQ(validator.process("test.report"), perfometer::result::ok);
}

} // namespace