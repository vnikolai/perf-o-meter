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

#pragma once

#include <perfometer/format.h>
#include <perfometer/perfometer.h>

#include <fstream>

namespace perfometer
{
    class serializer
    {
    public:
        serializer();
        ~serializer();

        result open_file_stream(const char fileName[]);
        result flush();
        result close();

        result status() { return m_report_file.fail() ? result::io_error : result::ok; }

        serializer& operator << (const unsigned char byte);
        serializer& operator << (const char* string);
        serializer& operator << (const string_id& id);
        serializer& operator << (const format::record_type type);
        serializer& operator << (const thread_id& id);
        serializer& operator << (const time& time);

    private:
        result write_header();
        result write_clock_config();
        result write_thread_info();

    private:
        std::ofstream m_report_file;
	};

} // namespace perfometer
