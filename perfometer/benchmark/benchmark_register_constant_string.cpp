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
#include <iostream>
#include <chrono>
#include <utils/time.h>
#include <utils/timer.h>

#define SIMPLE_REGISTER(name) perfometer::register_string(name);
#define LENGTH_REGISTER(name) perfometer::register_string(name, std::strlen(name));

size_t strlen_wrapper(const char* str)
{
    return std::strlen(str);
}

void benchmark_simple_registration()
{
    std::cout << "benchmark_simple_registration" << std::endl;
    perfometer::utils::logging_timer timer;

    for (size_t i = 0; i < 10000; ++i)
    {
        SIMPLE_REGISTER("lets_say_it's_some_pretty_long_string_to_look_like_function_declaration");
    }
}

void benchmark_length_in_registration()
{
    std::cout << "benchmark_length_in_registration" << std::endl;
    perfometer::utils::logging_timer timer;

    for (size_t i = 0; i < 10000; ++i)
    {
        LENGTH_REGISTER("lets_say_it's_some_pretty_long_string_to_look_like_function_declaration");
    }
}

void benchmark_strlen()
{
    std::cout << "benchmark_strlen" << std::endl;
    perfometer::utils::logging_timer timer;

    size_t cnt = 0;
    for (size_t i = 0; i < 10000; ++i)
    {
        const char* bla = "lets_say_it's_some_pretty_long_string_to_look_like_function_declaration";
        cnt += strlen_wrapper(bla);
    }
}


int main(int argc, const char** argv)
{
    perfometer::initialize();

    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    //benchmark_simple_registration();
    benchmark_length_in_registration();
    //benchmark_length_in_registration();
    //benchmark_simple_registration();

    benchmark_strlen();
    
    perfometer::shutdown();

    return 0;
}
