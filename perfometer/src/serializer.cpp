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

#include "serializer.h"

namespace perfometer {

serializer::serializer()
{
}

serializer::~serializer()
{
    close();
}

result serializer::open_file_stream(const char file_name[])
{
    scoped_lock lock(s_file_mutex);

    m_report_file.open(file_name, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);

    if (!m_report_file)
    {
        return result::io_error;
    }

    return status();
}

result serializer::flush()
{
    scoped_lock lock(s_file_mutex);

    m_report_file.flush();

    return status();
}

result serializer::close()
{
    scoped_lock lock(s_file_mutex);

    m_report_file.close();

    return status();
}

result serializer::write(const char* data, size_t size)
{
    scoped_lock lock(s_file_mutex);

    m_report_file.write(data, size);

    return status();
}

} // namespace perfometer
