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
#include <cstdint>
#include <iostream>

namespace perfometer
{
    const char header[] = "PERFOMETER.";    // PERFOMETER.VER - VER is version composed from
                                            // major minor and patch versions one byte each

    using string_id = uint16_t;

	enum record_type : uint8_t
	{
		clock_configuration = 1,    // 8 bit record type
                                    // 8 bit time size
                                    // time size clock frequency value
                                    // time size initial time

        thread_info = 2,            // 8 bit record type
                                    // 8 bit thread id size
                                    // thread id size initialization thread id

        string = 3,                 // 8 bit record type
                                    // 16 bit string id
                                    // 8 bit string length
                                    // string length size string data

        thread_name = 4,            // 8 bit record type
                                    // thread id size thread id
                                    // 16 bit string id

        work = 5                    // 8 bit record type
                                    // 16 bit name string id
                                    // time size time start
                                    // time size time end
                                    // thread id size thread id
	};

    inline std::ostream& operator << (std::ostream& stream, const record_type& type)
    {
        stream.write(reinterpret_cast<const char *>(&type), 1);
        return stream;
    }

	inline std::istream& operator >> (std::istream& stream, record_type& type)
	{
		stream.read(reinterpret_cast<char *>(&type), 1);
		return stream;
	}

} // namespace perfometer
