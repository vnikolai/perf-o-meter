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

#include <QDebug>

namespace visualizer {

using PerfTime = uint64_t;
using PerfStringID = perfometer::string_id;

static const std::string UNKNOWN = "UNKNOWN";

namespace {
class reader : public std::ifstream
{
public:
	reader(const std::string& fileName,
		   std::ios::openmode openMode = std::ios::binary)
		: std::ifstream(fileName, openMode)
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

bool PerfometerReport::loadFile(const std::string& fileName)
{
	PERFOMETER_LOG_WORK_FUNCTION();

	qDebug() << "Loading report" << fileName.c_str();

    reader report(fileName, std::ios::binary | std::ios::ate);
	if (!report)
	{
		qCritical() << "Loading report: Cannot open file" << fileName.c_str();
		return false;
	}

	const size_t reportSize = report.tellg();
	report.seekg(0);
	size_t progress = 0;

	char header[16];
	report.read(header, 11);

	if (report.fail() || std::strncmp(header, perfometer::format::header, 11))
	{
		qCritical() << "Loading report: wrong file format";
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

	m_strings.clear();
	std::unordered_map<Thread::ID, PerfStringID> thread_name_ids;

	perfometer::format::record_type record_type;

	while ((report >> record_type) && !report.eof())
	{
		if (report.fail())
		{
			qCritical() << "Loading report: cannot read from file" << fileName.c_str();
			return false;
		}

		size_t currentProgress = report.tellg() * 100 / reportSize;
		if (currentProgress > progress)
		{
			progress = currentProgress;
			qDebug() << "Report loading progress " << progress << "%";
		}

		switch (record_type)
		{
			case perfometer::format::record_type::clock_configuration:
			{
				unsigned char timeSize = 0;
				report >> timeSize;

				if (timeSize > 8)
				{
					qCritical() << "Loading report: Time size too large";
					return false;
				}

				report.setTimeSize(timeSize);

				report >> clockFrequency
					   >> initTime;

				break;
			}
			case perfometer::format::record_type::thread_info:
			{
				unsigned char threadIDSize = 0;
				report >> threadIDSize;

				if (threadIDSize > 8)
				{
					qCritical() << "Loading report: Thread ID size too large";
					return false;
				}

				report.setThreadIDSize(threadIDSize);

				report >> m_mainThreadID;

				break;
			}
			case perfometer::format::record_type::string:
			{
				PerfStringID ID = 0;
				report >> ID;

				report.readString(buffer, bufferSize);

				m_strings[ID] = buffer;

				break;
			}
			case perfometer::format::record_type::thread_name:
			{
				Thread::ID threadID = 0;
				PerfStringID stringID = 0;

				report >> threadID
					   >> stringID;

				thread_name_ids[threadID] = stringID;

				break;
			}
			case perfometer::format::record_type::work:
			case perfometer::format::record_type::wait:
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
				Record record{start, end, m_strings[stringID], record_type == perfometer::format::record_type::wait};

				std::vector<Record>& records = thread->records;

				// take records with enclosed time from thread into self 
				size_t count = -1;
				size_t i = records.size() - 1;
				for (; i < records.size(); i--)
				{
					Record& enclosed = records[i];
					if (enclosed.timeStart < record.timeStart || enclosed.timeEnd > record.timeEnd)
					{
						break;
					}

					count = i;
				}

				if (count < records.size())
				{
					auto it = records.begin() + count;
					std::move(it, records.end(), std::back_inserter(record.enclosed));
				
					// records.resize(count);
					// shrinking requires move assignment for some reason, so here's workaround:
					while (records.size() > count)
					{
						records.pop_back();
					}
				}

				records.push_back(record);

				m_startTime = std::min(m_startTime, record.timeStart);
				m_endTime = std::max(m_endTime, record.timeEnd);

				break;
			}
			case perfometer::format::record_type::event:
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

				thread->events.push_back(Event{event_time, m_strings[stringID]});

				m_startTime = std::min(m_startTime, event_time);
				m_endTime = std::max(m_endTime, event_time);

				break;
			}
			default:
			{
				qCritical() << "Loading report: Unknown record type";

				return m_traits.AllowIncompleteReport;
			}
		}
	}

	for (auto&& pair : thread_name_ids)
	{
		m_thread_names[pair.first] = m_strings[pair.second];
	}

	qDebug() << "Report loading done";

    return true;
}

const Threads& PerfometerReport::getThreads() const
{
    return m_threads;
}

ThreadPtr PerfometerReport::getThread(Thread::ID ID)
{
	auto name_pair = m_thread_names.emplace(ID, UNKNOWN);
    auto thread_pair = m_threads.emplace(ID, std::make_shared<Thread>(ID, name_pair.first->second));
	return thread_pair.first->second;
}

ConstThreadPtr PerfometerReport::getThread(Thread::ID ID) const
{
	static ThreadPtr emptyThread = std::make_shared<Thread>(-1, UNKNOWN);

    auto it = m_threads.find(ID);
    return it != m_threads.end() ? it->second : emptyThread;
}

} // namespace visualizer
