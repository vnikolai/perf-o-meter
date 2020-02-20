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

	reader& operator >> (ThreadID& ID)
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

PerfometerReport::~PerfometerReport()
{
}

bool PerfometerReport::loadFile(const std::string& fileName)
{
    reader report(fileName);
	if (!report)
	{
		return false;
	}

	char header[16];
	report.read(header, 11);

	if (report.fail() || std::strncmp(header, perfometer::header, 11))
	{
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

	perfometer::record_type record;

	while ((report >> record) && !report.eof())
	{
		if (report.fail())
		{
			return false;
		}

		switch (record)
		{
			case perfometer::record_type::clock_configuration:
			{
				unsigned char timeSize = 0;
				report >> timeSize;

				if (timeSize > 8)
				{
					// ERROR: Time size too large
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
					// ERROR: Thread ID size too large
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
				ThreadID threadID = 0;
				PerfStringID stringID = 0;

				report >> threadID
					   >> stringID;

                getThread(threadID).name = strings[stringID];

				break;
			}
			case perfometer::record_type::work:
			{
				PerfStringID stringID = 0;
				ThreadID threadID = 0;
				PerfTime startTime = 0;
				PerfTime endTime = 0;

                report >> stringID
					   >> startTime
					   >> endTime
					   >> threadID;

                double start = static_cast<double>(startTime - initTime) / clockFrequency;
                double end = static_cast<double>(endTime - initTime) / clockFrequency;

                Thread& thread = getThread(threadID);
				Record record({start, end, strings[stringID]});

				size_t i = thread.records.size() - 1;
				for (; i < thread.records.size(); i--)
				{
					Record& enclosed = thread.records[i];
					if (enclosed.timeStart < record.timeStart || enclosed.timeEnd > record.timeEnd)
					{
						break;
					}

					record.enclosed.insert(record.enclosed.begin(), thread.records.back());
					thread.records.pop_back();
				}

				thread.records.push_back(record);

				m_startTime = std::min(m_startTime, record.timeStart);
				m_endTime = std::max(m_endTime, record.timeEnd);

				break;
			}
			default:
			{
				// ERROR: Unknown record type
				return false;
			}
		}
	}

	for (auto& it : m_threads)
    {
        Thread& thread = it.second;
		if (!thread.name.length())
		{
			thread.name = "UNKNOWN";
		}
	}

    return true;
}

const Threads& PerfometerReport::getThreads() const
{
    return m_threads;
}

Thread& PerfometerReport::getThread(ThreadID ID)
{
    auto pair = m_threads.emplace(ID, Thread(ID));
	Thread& thread = pair.first->second;
    return thread;
}

const Thread& PerfometerReport::getThread(ThreadID ID) const
{
    static Thread emptyThread(-1);

    auto it = m_threads.find(ID);
    return it != m_threads.end() ? it->second : emptyThread;
}

} // namespace visualizer
