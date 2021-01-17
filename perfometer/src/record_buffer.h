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
#include <perfometer/thread.h>
#include <perfometer/time.h>
#include <perfometer/format.h>

namespace perfometer
{
	struct record
	{
		format::record_type	type;
		const char* 		name;
		time 				start;
		time 				end;
		thread_id			tid;
	};

	struct record_buffer
	{
		record_buffer()
		{
			count = 0;
			records = new record[records_cache_size];
		}

		~record_buffer()
		{
			delete[] records;
		}

		record_buffer& operator << (record&& record)
		{
			records[count] = std::move(record);
			++count;

			return *this;
		}

		size_t 		count;
		record*		records;
	};

} // namespace perfometer
