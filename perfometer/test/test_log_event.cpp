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

const std::string event_name = "test event";

class event_record_validator : public perfometer::utils::report_reader
{
public:
    perfometer::utils::perf_time expected_time = 0;
    std::string expected_name = event_name;

    MOCK_METHOD(void, event_handled, ());

    void handle_event(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time) override
    {
        EXPECT_EQ(expected_name, string_by_id(string_id));
        EXPECT_EQ(thread_id, bit_cast<perfometer::utils::perf_thread_id>(perfometer::get_thread_id()));
        EXPECT_DOUBLE_EQ(time, convert_time(expected_time));

        event_handled();
    }
};

TEST(log_event, not_running_not_initialized)
{
    EXPECT_EQ(perfometer::log_event(0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_event(10, 10), perfometer::not_running);
}

TEST(log_event, not_running_initialized_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_event(10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_event, not_running_explicitly_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    ASSERT_EQ(perfometer::pause(), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(0, 0), perfometer::not_running);
    EXPECT_EQ(perfometer::log_event(10, 10), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(log_event, record_event)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    const perfometer::string_id string_id = perfometer::register_string(event_name.c_str());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time event_time = 2000;
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    event_record_validator validator;
    validator.expected_time = event_time;

    EXPECT_CALL(validator, event_handled());

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

TEST(log_event, record_event_repeatedly)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    const perfometer::string_id string_id = perfometer::register_string(event_name.c_str());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time event_time = 2000;
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    event_record_validator validator;
    validator.expected_time = event_time;

    EXPECT_CALL(validator, event_handled()).Times(3);

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

TEST(log_event, record_dynamic_event)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    std::string dynamic_event_name = event_name + " dynamic";

    const perfometer::string_id string_id = perfometer::write_string(dynamic_event_name.c_str(), dynamic_event_name.length());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time event_time = 2500;
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    event_record_validator validator;
    validator.expected_time = event_time;
    validator.expected_name = dynamic_event_name;

    EXPECT_CALL(validator, event_handled());

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

TEST(log_event, record_dynamic_event_repeatedly)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    std::string dynamic_event_name = event_name + " dynamic";

    const perfometer::string_id string_id = perfometer::write_string(dynamic_event_name.c_str(), dynamic_event_name.length());
    ASSERT_NE(string_id, perfometer::format::invalid_string_id);

    const perfometer::time event_time = 2500;
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);
    EXPECT_EQ(perfometer::log_event(string_id, event_time), perfometer::ok);

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    event_record_validator validator;
    validator.expected_time = event_time;
    validator.expected_name = dynamic_event_name;

    EXPECT_CALL(validator, event_handled()).Times(3);

    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

} // namespace