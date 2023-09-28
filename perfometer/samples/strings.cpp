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
#include <string>
#include <sstream>
#include <iostream>

// Showing automatic usage both of char* and std::string

void func_with_c_string()
{
    const char* c_string = "This is char* string";
    PERFOMETER_LOG_WORK_SCOPE(c_string);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void func_with_cpp_string()
{
    const std::string cpp_string = "This is std::string string";
    PERFOMETER_LOG_WORK_SCOPE(cpp_string);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void func_with_dynamic_string()
{
    std::stringstream s;
    s << "Have some random number " << rand();
    PERFOMETER_LOG_DYNAMIC_EVENT(s.str());

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    s.seekp(0);
    s << "Have some random number " << rand();
    PERFOMETER_LOG_DYNAMIC_EVENT(s.str());
}

int main(int argc, const char** argv)
{
    auto result = perfometer::initialize();
    std::cout << "perfometer::initialize() returned " << result << std::endl;

    PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    func_with_c_string();

    func_with_cpp_string();

    func_with_dynamic_string();

    result = perfometer::shutdown();
    std::cout << "perfometer::shutdown() returned " << result << std::endl;

    return 0;
}
