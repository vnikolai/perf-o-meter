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
#include <utils/report_reader.h>
#include <utils/timer.h>

#include <array>
#include <thread>
#include <vector>

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

std::vector<int> num_ids(perfometer::format::invalid_string_id + 2);
const int num_invalid_tries = 17;

const int num_threads = 12;
const int num_registers_per_thread = (perfometer::format::invalid_string_id - 1 - 2) / num_threads;

class report_validator : public perfometer::utils::report_reader
{
private:
    void handle_string(perfometer::string_id id, const std::string& string) override
    {
        // each id should be present exactly once
        EXPECT_EQ(num_ids[id], 1);
        num_ids[id]--;

        if (id >= 2 && id < perfometer::format::invalid_string_id)
        {
            EXPECT_EQ(string, random_strings[string.length()]);
        }
    }
};

TEST(register_string, ignore_nullptr)
{
    EXPECT_EQ(perfometer::register_string(nullptr), perfometer::format::invalid_string_id);
    EXPECT_EQ(perfometer::register_string(nullptr, 0), perfometer::format::invalid_string_id);
}

TEST(register_string_test, until_max_and_then_some)
{
    ASSERT_EQ(perfometer::initialize("test_register_string.report"), perfometer::result::ok);

    // 0 and 1 are reserved, first registered string id is 2
    EXPECT_EQ(perfometer::register_string(""), 2);
    num_ids[2]++;

    std::vector<std::thread> threads;

    int counter = 0;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.push_back(std::thread(
            [&counter]()
            {
                for (int j = 0; j < num_registers_per_thread; ++j)
                {
                    perfometer::string_id id = perfometer::register_string(
                        random_strings[counter++ % random_strings.size()]);
                    num_ids[id]++;
                }
            }
        ));
    }

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }

    // should return invalid id after max strings registered
    for (int i = 0; i < num_invalid_tries; ++i)
    {
        perfometer::string_id id = perfometer::register_string("");
        EXPECT_EQ(id, perfometer::format::invalid_string_id);

        num_ids[id]++;
    }

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    // no registration for reserved id 0 and 1
    EXPECT_EQ(num_ids[0], 0);
    EXPECT_EQ(num_ids[1], 0);

    // each id returned exactly once
    for (int i = 3; i < perfometer::format::invalid_string_id; ++i)
    {
        EXPECT_EQ(num_ids[i], 1);
    }

    // number of invalid registrations should match number of tries
    EXPECT_EQ(num_ids[perfometer::format::invalid_string_id], num_invalid_tries);
    // no registration for id after invalid_string_id
    EXPECT_EQ(num_ids[perfometer::format::invalid_string_id + 1], 0);

    // expecting reserved and invalid id to be present in report once
    num_ids[0] = 1;
    num_ids[1] = 1;
    num_ids[perfometer::format::invalid_string_id] = 1; 

    // open actual report file and check that it contains expected number of string records
    report_validator validator;
    validator.process("test_register_string.report");

    // each id substracted exactly once by report validator
    for (int i = 0; i < perfometer::format::invalid_string_id; ++i)
    {
        EXPECT_EQ(num_ids[i], 0);
    }
}

} // namespace
