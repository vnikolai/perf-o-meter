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
#include <thread>
#include <vector>

std::vector<int> num_ids(perfometer::format::invalid_string_id + 2);
const int num_invalid_tries = 17;

const int num_threads = 12;
const int num_registers_per_thread = (perfometer::format::invalid_string_id - 1 - 2) / num_threads;

TEST(register_string_test, until_max_and_then_some)
{
    ASSERT_EQ(perfometer::initialize("test_register_string.report"), perfometer::ok);

    EXPECT_EQ(perfometer::register_string(""), 2);

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.push_back(std::thread(
            []()
            {
                std::cout << num_registers_per_thread << std::endl;
                for (int i = 0; i < num_registers_per_thread; ++i)
                {
                    perfometer::string_id id = perfometer::register_string("");
                    num_ids[id]++;
                }
            }
        ));
    }

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }



    // for (int i = 2; i < perfometer::format::invalid_string_id; ++i)
    // {
    //     perfometer::string_id id = perfometer::register_string("");
    //     EXPECT_EQ(id, i);

    //     num_ids[id]++;
    // }

    // repeated invalid id after max strings
    for (int i = 0; i < num_invalid_tries; ++i)
    {
        perfometer::string_id id = perfometer::register_string("");
        EXPECT_EQ(id, perfometer::format::invalid_string_id);

        num_ids[id]++;
    }

    ASSERT_EQ(perfometer::shutdown(), perfometer::ok);

    EXPECT_EQ(num_ids[0], 0);
    EXPECT_EQ(num_ids[1], 0);

    int counter = 0;

    for (int i = 3; i < perfometer::format::invalid_string_id; ++i)
    {
        EXPECT_EQ(num_ids[i], 1);

        if (num_ids[i] != 1)
        {
            std::cout << "this is the shit " << i << std::endl;

            ++counter;

            if (counter >= 10)
            {
                ASSERT_EQ(0, 1);                
            }
        }
    }

    EXPECT_EQ(num_ids[perfometer::format::invalid_string_id], num_invalid_tries);
    EXPECT_EQ(num_ids[perfometer::format::invalid_string_id + 1], 0);
}
