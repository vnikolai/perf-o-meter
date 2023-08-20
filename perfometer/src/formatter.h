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

#include <cstring>

namespace perfometer
{
    template<typename Buffer>
    class formatter
    {
    public:
        formatter(Buffer& buffer)
            : m_buffer(buffer)
        {            
        }

        formatter& operator << (const unsigned char byte)
        {
            write(reinterpret_cast<const char*>(&byte), 1);
            return *this;
        }

        formatter& operator << (const char* string)
        {
            unsigned char string_length = std::min(255, static_cast<int>(std::strlen(string)));
            *this << string_length;

            write(string, string_length);

            return *this;
        }

        formatter& operator << (const string_id& id)
        {
            write(reinterpret_cast<const char*>(&id), sizeof(string_id));
            return *this;
        }

        formatter& operator << (const format::record_type type)
        {
            write(reinterpret_cast<const char*>(&type), 1);
            return *this;
        }

        formatter& operator << (const thread_id& id)
        {
            write(reinterpret_cast<const char *>(&id), sizeof(thread_id));
            return *this;
        }

        formatter& operator << (const time& time)
        {
            write(reinterpret_cast<const char*>(&time), sizeof(time));
            return *this;
        }

        void write_string(const char* string, size_t len)
        {
            unsigned char string_length = std::min<unsigned int>(255, len);
            *this << string_length;

            write(string, string_length);
        }

        void write(const char* data, size_t size)
        {
            m_buffer.write(data, size);
        }

    private:
        Buffer& m_buffer;
    };

} // namespace perfometer
