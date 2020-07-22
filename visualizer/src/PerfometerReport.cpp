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

#include "PerfometerReport.h"
#include <algorithm>
#include <fstream>
#include <cstring>
#include <stack>
#include <unordered_map>
#include <perfometer/perfometer.h>
#include <perfometer/format.h>
#include <perfometer/helpers.h>

namespace visualizer {

using PerfTime = uint64_t;
using PerfStringID = perfometer::string_id;

namespace {
class reader : public std::ifstream
{
public:
	reader(const std::string& fileName)
		: std::ifstream(fileName, std::ios::binary)
	{
	}

	reader& operator >> (unsigned char& byte)
	{
		read(reinterpret_cast<char*>(&byte), 1);
		return *this;
	}

	reader& operator >> (Thread::ID& ID)
	{
		ID = 0;
		read(reinterpret_cast<char*>(&ID), m_threadIDSize);
		return *this;
	}

	reader& operator >> (PerfTime& time)
	{
		time = 0;
		read(reinterpret_cast<char*>(&time), m_timeSize);
		return *this;
	}

	reader& operator >> (PerfStringID& ID)
	{
		read(reinterpret_cast<char*>(&ID), sizeof(PerfStringID));
		return *this;
	}

	reader& readString(char* buffer, size_t buffer_size)
	{
		unsigned char nameLength = 0;
		*this >> nameLength;

		size_t length = std::min(buffer_size - 1, static_cast<size_t>(nameLength));

		read(buffer, length);
		buffer[length] = 0;

		return *this;
	}

	void setThreadIDSize(size_t size) { m_threadIDSize = size; }
	void setTimeSize(size_t size) { m_timeSize = size; }

private:
	size_t m_threadIDSize = 0;
	size_t m_timeSize = 0;
};

} // namespace

PerfometerReport::PerfometerReport()
	: m_startTime(std::numeric_limits<double>::max())
	, m_endTime(std::numeric_limits<double>::min())
	, m_mainThreadID(0)
{
}

PerfometerReport::PerfometerReport(const Traits& traits)
	: PerfometerReport()
{
	m_traits = traits;
}

PerfometerReport::~PerfometerReport()
{
}

bool PerfometerReport::loadFile(const std::string& fileName,
								std::function<void(const std::string&)> logger)
{
	PERFOMETER_LOG_WORK_FUNCTION();

	auto log = [&logger](const std::string& text)
	{
		if (logger)
		{
			logger(text);
		}
	};

    reader report(fileName);
	if (!report)
	{
		log(std::string("ERROR loading report: Cannot open file ") + fileName);
		return false;
	}

	char header[16];
	report.read(header, 11);

	if (report.fail() || std::strncmp(header, perfometer::header, 11))
	{
		log("ERROR loading report: wrong file format");
		return false;
	}

	unsigned char majorVersion = 0;
	unsigned char minorVersion = 0;
	unsigned char patchVersion = 0;

	report >> majorVersion
		   >> minorVersion
		   >> patchVersion;

	constexpr size_t bufferSize = 1024;
	char buffer[bufferSize];

	PerfTime clockFrequency = 0;
	PerfTime initTime = 0;

	std::unordered_map<PerfStringID, std::string>	strings;

	perfometer::record_type record_type;

	while ((report >> record_type) && !report.eof())
	{
		if (report.fail())
		{
			log(std::string("ERROR loading report: cannot read from file ") + fileName);
			return false;
		}

		switch (record_type)
		{
			case perfometer::record_type::clock_configuration:
			{
				unsigned char timeSize = 0;
				report >> timeSize;

				if (timeSize > 8)
				{
					log("ERROR loading report: Time size too large");
					return false;
				}

				report.setTimeSize(timeSize);

				report >> clockFrequency
					   >> initTime;
                				
				break;
			}
			case perfometer::record_type::thread_info:
			{
				unsigned char threadIDSize = 0;
				report >> threadIDSize;

				if (threadIDSize > 8)
				{
					log("ERROR loading report: Thread ID size too large");
					return false;
				}

				report.setThreadIDSize(threadIDSize);

				report >> m_mainThreadID;

				break;
			}
			case perfometer::record_type::string:
			{
				PerfStringID ID = 0;
				report >> ID;

				report.readString(buffer, bufferSize);

				strings[ID] = buffer;

				break;
			}
			case perfometer::record_type::thread_name:
			{
				Thread::ID threadID = 0;
				PerfStringID stringID = 0;

				report >> threadID
					   >> stringID;

                getThread(threadID)->name = strings[stringID];

				break;
			}
			case perfometer::record_type::work:
			case perfometer::record_type::wait:
			{
				PerfStringID stringID = 0;
				Thread::ID threadID = 0;
				PerfTime startTime = 0;
				PerfTime endTime = 0;

                report >> stringID
					   >> startTime
					   >> endTime
					   >> threadID;

                double start = static_cast<double>(startTime - initTime) / clockFrequency;
                double end = static_cast<double>(endTime - initTime) / clockFrequency;

				if (m_traits.SkipRecordsIncorrectTime)
				{
					if (start >= m_traits.RecordTimeMaxLimit || end >= m_traits.RecordTimeMaxLimit)
					{
						continue;
					}
				}

				if (m_traits.SkipEmptyRecords)
				{
					if (end - start < m_traits.EmptyRecordLimit)
					{
						continue;
					}
				}

                ThreadPtr thread = getThread(threadID);
				Record record({start, end, strings[stringID], record_type == perfometer::record_type::wait});

				std::vector<Record>& records = thread->records;

				size_t i = records.size() - 1;
				for (; i < records.size(); i--)
				{
					Record& enclosed = records[i];
					if (enclosed.timeStart < record.timeStart || enclosed.timeEnd > record.timeEnd)
					{
						break;
					}

					record.enclosed.insert(record.enclosed.begin(), records.back());
					records.pop_back();
				}

				records.push_back(record);

				m_startTime = std::min(m_startTime, record.timeStart);
				m_endTime = std::max(m_endTime, record.timeEnd);

				break;
			}
			case perfometer::record_type::event:
			{
				PerfStringID stringID = 0;
				Thread::ID threadID = 0;
				PerfTime time = 0;
				PerfTime endTime = 0;

                report >> stringID
					   >> time
					   >> threadID;

                double event_time = static_cast<double>(time - initTime) / clockFrequency;
				if (m_traits.SkipRecordsIncorrectTime)
				{
					if (event_time >= m_traits.RecordTimeMaxLimit)
					{
						continue;
					}
				}

                ThreadPtr thread = getThread(threadID);

				thread->events.push_back(Event({event_time, strings[stringID]}));

				m_startTime = std::min(m_startTime, event_time);
				m_endTime = std::max(m_endTime, event_time);

				break;
			}
			default:
			{
				log("ERROR loading report: Unknown record type");

				return m_traits.AllowIncompleteReport;
			}
		}
	}

    return true;
}

const Threads& PerfometerReport::getThreads() const
{
    return m_threads;
}

ThreadPtr PerfometerReport::getThread(Thread::ID ID)
{
    auto pair = m_threads.emplace(ID, std::make_shared<Thread>(ID, "UNKNOWN"));
	ThreadPtr& thread = pair.first->second;
    return thread;
}

ConstThreadPtr PerfometerReport::getThread(Thread::ID ID) const
{
    static ThreadPtr emptyThread = std::make_shared<Thread>(-1, "EMPTY");

    auto it = m_threads.find(ID);
    return it != m_threads.end() ? it->second : emptyThread;
}

} // namespace visualizer
