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

#include "../src/formatter.h"
#include <vector>
#include <iostream>

template<typename T1, typename T2>
void print_error(const T1& a, const T2& b, const char* desc_a, const char* desc_b)
{
    std::cout << "check failed " << desc_a << " != " << desc_b << std::endl;
    std::cout << "Expected: " << b << ", actual: " << a << std::endl;
}

#define CHECK(a, b) if (a != b) { print_error(a, b, #a, #b); result = -1; }

class stream_stub
{
public:
    size_t size()
    {
        return m_data.size();
    }

    const uint8_t* data()
    {
        return &m_data[0];
    }

    uint8_t& operator[] (size_t index)
    {
        return m_data[index];
    }

    void write(const void* data, size_t size)
    {
        auto end = m_data.size();
        m_data.resize(m_data.size() + size);
        memcpy(&m_data[end], data, size);
    }

private:
    std::vector<uint8_t> m_data;
};

int result = 0;

template <typename T> void check_formatting_size(const T& value)
{
    stream_stub s;
    perfometer::formatter<stream_stub> fmt(s);
    fmt << value;

    CHECK(s.size(), sizeof(value));
}

template <typename T> void check_formatting_value(const T& value)
{
    stream_stub s;
    perfometer::formatter<stream_stub> fmt(s);
    fmt << value;

    CHECK(s.size(), sizeof(value));
    CHECK(*reinterpret_cast<const T*>(s.data()), value);
}

template <typename T> void check_formatting(const T& value)
{
    check_formatting_size(value);
    check_formatting_value(value);
}

void check_formatting_string()
{
    const char* dummy_string = "dummy string";
    stream_stub s;
    perfometer::formatter<stream_stub> fmt(s);
    fmt << dummy_string;

    CHECK(s.size(), 1 + strlen(dummy_string));
    CHECK(s[0], strlen(dummy_string));
    CHECK(std::memcmp(s.data() + 1, dummy_string, strlen(dummy_string)), 0);
}

void check_formatting_string2()
{
    const char* dummy_string = "dummy string";
    stream_stub s;
    perfometer::formatter<stream_stub> fmt(s);
    fmt.write_string(dummy_string, strlen(dummy_string));

    CHECK(s.size(), 1 + strlen(dummy_string));
    CHECK(s[0], strlen(dummy_string));
    CHECK(std::memcmp(s.data() + 1, dummy_string, strlen(dummy_string)), 0);
}

int main(int argc, const char** argv)
{
    check_formatting(perfometer::format::record_type::clock_configuration);
    check_formatting(perfometer::string_id(1557));
    check_formatting(perfometer::time(178976));
    check_formatting_size(perfometer::thread_id());
    check_formatting_string();

    return result;
}