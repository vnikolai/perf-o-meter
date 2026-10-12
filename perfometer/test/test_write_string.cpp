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
#include <perfometer/format.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <utils/report_reader.h>
#include <utils/timer.h>
#include <array>

namespace {

const std::array<const char*, 16> random_strings = {
    "",
    "!",
    "a@",
    "Q7#",
    "m$2K",
    "v T5!",
    "R8,q?!",
    "u1Cw8%#",
    "t9:lQz4!",
    "!b8Gq4Nfc",
    "x2HkP9sV4!",
    "r6D,q7Wn3 M",
    "u1Cw8Yh5Jk2!",
    "d5Qn7; M2xP9c",
    "s3Fh8Kp2 Lw9R4",
    "z!6Qn7;M2xP9cV3"
};

class report_validator : public perfometer::utils::report_reader
{
public:

    MOCK_METHOD(void, string_handled, ());

private:

    void handle_string(perfometer::string_id id, const std::string& string) override
    {
        // each id should be present exactly once

        if (id >= 2 && id < perfometer::format::invalid_string_id)
        {
            EXPECT_EQ(string, random_strings[string.length()]);
        }

        string_handled();
    }
};

TEST(write_string, not_running_not_initialized)
{
    EXPECT_EQ(perfometer::write_string("test", 4), perfometer::not_running);
}

TEST(write_string, not_running_initialized_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report", false), perfometer::ok);
    EXPECT_EQ(perfometer::write_string("test", 4), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(write_string, not_running_explicitly_paused)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    ASSERT_EQ(perfometer::pause(), perfometer::ok);
    EXPECT_EQ(perfometer::write_string("test", 4), perfometer::not_running);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(write_string, invalid_arguments_invalid_string_id)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);
    EXPECT_EQ(perfometer::write_string(nullptr, 0), perfometer::invalid_arguments);
    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);
}

TEST(write_string, write_and_read_strings)
{
    ASSERT_EQ(perfometer::initialize("test.report"), perfometer::ok);

    for (const auto& str : random_strings)
    {
        const perfometer::string_id string_id = perfometer::write_string(str, strlen(str));
        EXPECT_EQ(string_id, perfometer::format::dynamic_string_id);
    }

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    report_validator validator;
    EXPECT_CALL(validator, string_handled()).Times(random_strings.size() + 3);
    ASSERT_EQ(validator.process("test.report"), perfometer::ok);
}

} // namespace