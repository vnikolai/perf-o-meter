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

template<typename T1, typename T2>
void print_error(const T1& a, const T2& b, const char* desc_a, const char* desc_b)
{
    std::cout << "check failed " << desc_a << " != " << desc_b << std::endl;
    std::cout << "Expected: " << b << ", actual: " << a << std::endl;
}

#define CHECK(a, b) if (a != b) { print_error(a, b, #a, #b); result = -1; }

int main(int argc, const char** argv)
{
    using namespace perfometer::utils;

    int result = 0;

    CHECK(time_to_string(3600), "1h");
    CHECK(time_to_string(5400), "1h 30m");
    CHECK(time_to_string(7200), "2h");

    CHECK(time_to_string(10000), "2h 46m 40s");
    CHECK(time_to_string(1000), "16m 40s");
    CHECK(time_to_string(100), "1m 40s");
    CHECK(time_to_string(10), "10s");
    CHECK(time_to_string(1), "1s");
    CHECK(time_to_string(0.1), "100ms");
    CHECK(time_to_string(0.5), "500ms");
    CHECK(time_to_string(0.01), "10ms");
    CHECK(time_to_string(0.001), "1ms");
    CHECK(time_to_string(0.0001), "100us");
    CHECK(time_to_string(0.00001), "10us");
    CHECK(time_to_string(0.000001), "1us");
    CHECK(time_to_string(0.0000001), "100ns");
    CHECK(time_to_string(0.00000001), "10ns");
    CHECK(time_to_string(0.000000001), "1ns");
    CHECK(time_to_string(0.0000000001), "0ns");

    CHECK(time_to_string(1.2), "1.20s");
    CHECK(time_to_string(1.4), "1.40s");
    CHECK(time_to_string(1.5), "1.50s");
    CHECK(time_to_string(11.5), "11.5s");
    CHECK(time_to_string(60), "1m");
    CHECK(time_to_string(72), "1m 12s");
    CHECK(time_to_string(3659), "1h 59s");
    CHECK(time_to_string(3660), "1h 1m");
    CHECK(time_to_string(3661), "1h 1m 1s");
    CHECK(time_to_string(3601), "1h 1s");

    return result;
}