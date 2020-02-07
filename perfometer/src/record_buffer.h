#pragma once

#include <perfometer/config.h>
#include <perfometer/thread.h>
#include <perfometer/time.h>

namespace perfometer {

struct work_record
{
	const char* name;
	time 		start;
	time 		end;
	thread_id	tid;
};

struct record_buffer
{
	record_buffer()
	{
		count = 0;
		records = new work_record[records_cache_size];
	}

	~record_buffer()
	{
		delete[] records;
	}

	void add_record(work_record&& record)
	{
		records[count] = std::move(record);
		++count;
	}

	size_t 			count;
	work_record*	records;
};

} // namespace perfometer
