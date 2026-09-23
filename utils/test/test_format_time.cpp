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

#include <utils/time.h>
#include <iostream>

#include <gtest/gtest.h>

TEST(test_format_time, test)
{
    using namespace perfometer::utils;

    EXPECT_EQ(time_to_string(3600), "1h");
    EXPECT_EQ(time_to_string(5400), "1h 30m");
    EXPECT_EQ(time_to_string(7200), "2h");

    EXPECT_EQ(time_to_string(10000), "2h 46m 40s");
    EXPECT_EQ(time_to_string(1000), "16m 40s");
    EXPECT_EQ(time_to_string(100), "1m 40s");
    EXPECT_EQ(time_to_string(10), "10s");
    EXPECT_EQ(time_to_string(1), "1s");
    EXPECT_EQ(time_to_string(0.1), "100ms");
    EXPECT_EQ(time_to_string(0.5), "500ms");
    EXPECT_EQ(time_to_string(0.01), "10ms");
    EXPECT_EQ(time_to_string(0.001), "1ms");
    EXPECT_EQ(time_to_string(0.0001), "100us");
    EXPECT_EQ(time_to_string(0.00001), "10us");
    EXPECT_EQ(time_to_string(0.000001), "1us");
    EXPECT_EQ(time_to_string(0.0000001), "100ns");
    EXPECT_EQ(time_to_string(0.00000001), "10ns");
    EXPECT_EQ(time_to_string(0.000000001), "1ns");
    EXPECT_EQ(time_to_string(0.0000000001), "0ns");

    EXPECT_EQ(time_to_string(1.4), "1.4s");
    EXPECT_EQ(time_to_string(11.4), "11.4s");
    EXPECT_EQ(time_to_string(11.47), "11.47s");
    EXPECT_EQ(time_to_string(111.47), "1m 51.47s");
    EXPECT_EQ(time_to_string(10.4), "10.4s");
    EXPECT_EQ(time_to_string(1.04), "1.04s");
    EXPECT_EQ(time_to_string(10.04), "10.04s");
    EXPECT_EQ(time_to_string(60), "1m");
    EXPECT_EQ(time_to_string(72), "1m 12s");
    EXPECT_EQ(time_to_string(3659), "1h 59s");
    EXPECT_EQ(time_to_string(3660), "1h 1m");
    EXPECT_EQ(time_to_string(3661), "1h 1m 1s");
    EXPECT_EQ(time_to_string(3601), "1h 1s");
}