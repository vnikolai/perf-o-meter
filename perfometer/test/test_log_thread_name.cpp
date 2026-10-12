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

const std::string thread_name = "thread name";

class thread_name_validator : public perfometer::utils::report_reader
{
public:
    perfometer::utils::perf_thread_id expected_thread_id = 0;

    MOCK_METHOD(void, thread_name_handled, ());

    void handle_thread_name(perfometer::utils::perf_thread_id thread_id, const std::string& name) override
    {
        EXPECT_EQ(thread_id, bit_cast<perfometer::utils::perf_thread_id>(perfometer::get_thread_id()));

        EXPECT_EQ(thread_name, name);

        thread_name_handled();
    }
};

TEST(log_thread_name, not_initialized)
{
    EXPECT_EQ(perfometer::log_thread_name(0), perfometer::not_initialized);
    EXPECT_EQ(perfometer::log_thread_name(10), perfometer::not_initialized);

    EXPECT_EQ(perfometer::log_thread_name(0, std::this_thread::get_id()), perfometer::not_initialized);
    EXPECT_EQ(perfometer::log_thread_name(10, std::this_thread::get_id()), perfometer::not_initialized);
}

TEST(log_thread_name, invalid_arguments_invalid_string_id)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    EXPECT_EQ(perfometer::log_thread_name(perfometer::format::invalid_string_id, std::this_thread::get_id()),
              perfometer::invalid_arguments);
    EXPECT_EQ(perfometer::log_thread_name(perfometer::format::invalid_string_id), perfometer::invalid_arguments);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_thread_name, records_current_thread_name_even_when_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);

    const perfometer::string_id string_id = perfometer::register_string(thread_name.c_str());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    EXPECT_EQ(perfometer::log_thread_name(string_id), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    thread_name_validator validator;
    EXPECT_CALL(validator, thread_name_handled());

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

} // namespace