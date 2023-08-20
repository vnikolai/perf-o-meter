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

#include <perfometer/config.h>
#include <perfometer/format.h>

#include "formatter.h"

namespace perfometer
{
    class record_buffer
    {
    public:
        record_buffer()
            : m_curr_pos(m_data)
        {
        }

        ~record_buffer()
        {
        }

        size_t used_size() const
        {
            return m_curr_pos - m_data;
        }

        size_t free_size() const
        {
            size_t used = used_size();
            return records_cache_size > used ? records_cache_size - used : 0;
        }

        const char* data() const { return m_data; }

        void write(const char *data, size_t size)
        {
            if (data && size <= free_size())
            {
                memcpy(m_curr_pos, data, size);
                m_curr_pos += size;
            }
        }

    private:
        char        m_data[records_cache_size];
        char*       m_curr_pos;
    };

} // namespace perfometer
