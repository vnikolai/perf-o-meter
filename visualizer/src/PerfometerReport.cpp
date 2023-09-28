/* Copyright 2020-2023 Volodymyr Nikolaichuk

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
#include <perfometer/format.h>
#include <perfometer/helpers.h>
#include <cstring>
#include <QDebug>

namespace visualizer {

static const std::string UNKNOWN = "UNKNOWN";

PerfometerReport::PerfometerReport()
    : m_startTime(std::numeric_limits<double>::max())
    , m_endTime(std::numeric_limits<double>::min())
    , m_mainThreadID(0)
    , m_dynamic_string_id(perfometer::format::invalid_string_id + 1)
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

    process(fileName.c_str());

    qDebug() << "Report loading done";

    return true;
}

void PerfometerReport::log(const std::string& message)
{
    qDebug() << message.c_str();
}

void PerfometerReport::log_error(const std::string& message)
{
    qCritical() << message.c_str();
}

void PerfometerReport::handle_loading_progress(size_t percentage)
{
    qDebug() << "Report loading progress " << percentage << "%";
}

void PerfometerReport::handle_clock_configuration(char time_size, perfometer::utils::perf_time clock_frequency, perfometer::utils::perf_time init_time)
{
}

void PerfometerReport::handle_thread_info(char thread_id_size, perfometer::utils::perf_thread_id main_thread_id)
{
    m_mainThreadID = main_thread_id;
}

void PerfometerReport::handle_string(perfometer::string_id id, const std::string& string)
{
}

void PerfometerReport::handle_thread_name(perfometer::utils::perf_thread_id thread_id, const std::string& name)
{
}

void PerfometerReport::handle_work(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end)
{
    process_record(string_id, thread_id, time_start, time_end, false);
}

void PerfometerReport::handle_wait(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end)
{
    process_record(string_id, thread_id, time_start, time_end, true);
}

void PerfometerReport::process_record(perfometer::string_id string_id,
                                      perfometer::utils::perf_thread_id thread_id,
                                      double time_start,
                                      double time_end,
                                      bool wait)
{
    if (m_traits.SkipRecordsIncorrectTime)
    {
        if (time_start >= m_traits.RecordTimeMaxLimit || time_end >= m_traits.RecordTimeMaxLimit)
        {
            return;
        }
    }

    if (m_traits.SkipEmptyRecords)
    {
        if (time_end - time_start < m_traits.EmptyRecordLimit)
        {
            return;
        }
    }

    ThreadPtr thread = getThread(thread_id);
    Record record{time_start, time_end, stringByID(check_for_dynamic_string(string_id)), wait};

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
}

void PerfometerReport::handle_event(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double event_time)
{
    if (m_traits.SkipRecordsIncorrectTime)
    {
        if (event_time >= m_traits.RecordTimeMaxLimit)
        {
            return;
        }
    }

    ThreadPtr thread = getThread(thread_id);

    thread->events.push_back(Event{event_time, stringByID(check_for_dynamic_string(string_id))});

    m_startTime = std::min(m_startTime, event_time);
    m_endTime = std::max(m_endTime, event_time);
}

uint64_t PerfometerReport::check_for_dynamic_string(perfometer::string_id string_id)
{
    if (string_id != perfometer::format::dynamic_string_id)
    {
        return string_id;
    }

    m_dynamic_strings[++m_dynamic_string_id] = string_by_id(string_id);
    return m_dynamic_string_id;
}

const std::string& PerfometerReport::stringByID(uint64_t string_id)
{
    return string_id <= perfometer::format::invalid_string_id ?
            string_by_id(string_id) :
            m_dynamic_strings[string_id];
}

const Threads& PerfometerReport::getThreads() const
{
    return m_threads;
}

ThreadPtr PerfometerReport::getThread(Thread::ID ID)
{
    auto thread_pair = m_threads.emplace(ID, std::make_shared<Thread>(ID, thread_name_by_id(ID)));
    return thread_pair.first->second;
}

ConstThreadPtr PerfometerReport::getThread(Thread::ID ID) const
{
    static ThreadPtr emptyThread = std::make_shared<Thread>(-1, UNKNOWN);

    auto it = m_threads.find(ID);
    return it != m_threads.end() ? it->second : emptyThread;
}

} // namespace visualizer
