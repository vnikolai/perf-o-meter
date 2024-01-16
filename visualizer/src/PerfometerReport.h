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

#pragma once

#include <utils/report_reader.h>

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace visualizer
{
    using StringID = perfometer::string_id;

    // string& in Record, Event & Thread are here for optimization
    // and correctly displaying strings that loaded later that used
    struct Record
    {
        double timeStart;
        double timeEnd;
        const std::string& name;
        bool wait;
        std::vector<Record> enclosed;
    };

    struct Event
    {
        double time;
        const std::string& name;
    };

    struct Thread
    {
        using ID = perfometer::utils::perf_thread_id;

        Thread(ID _id, const std::string& n)
            : id(_id)
            , name(n)
        {
        }

        ID id;
        const std::string& name;
        std::vector<Record> records;
        std::vector<Event> events;
    };

    using ThreadPtr = std::shared_ptr<Thread>;
    using ConstThreadPtr = std::shared_ptr<const Thread>;

    using Threads = std::map<Thread::ID, ThreadPtr>;

    class PerfometerReport : public perfometer::utils::report_reader
    {
    public:
        struct Traits
        {
            bool AllowIncompleteReport = true;
            bool SkipEmptyRecords = true;
            bool SkipRecordsIncorrectTime = true;
            double EmptyRecordLimit = 0.000000001;
            double RecordTimeMaxLimit = 24*60*60;
        };

    public:
        PerfometerReport();
        PerfometerReport(const Traits&);

        virtual ~PerfometerReport();

        bool loadFile(const std::string& fileName);

        double getStartTime() const { return m_startTime; }
        double getEndTime() const { return m_endTime; }

        Thread::ID mainThreadID() const { return m_mainThreadID; }

        const Threads& getThreads() const;
        ThreadPtr getThread(Thread::ID id);
        ConstThreadPtr getThread(Thread::ID id) const;

    private:

        void log(const std::string& message) override;
        void log_error(const std::string& message) override;
        void handle_loading_progress(size_t percentage) override;
        void handle_clock_configuration(uint8_t time_size, perfometer::utils::perf_time clock_frequency, perfometer::utils::perf_time init_time) override;
        void handle_thread_info(uint8_t thread_id_size, perfometer::utils::perf_thread_id main_thread_id) override;
        void handle_string(perfometer::string_id id, const std::string& string) override;
        void handle_thread_name(perfometer::utils::perf_thread_id thread_id, const std::string& name) override;
        void handle_work(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end) override;
        void handle_wait(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end) override;
        void handle_event(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time) override;

        void process_record(perfometer::string_id string_id, perfometer::utils::perf_thread_id thread_id, double time_start, double time_end, bool wait);
        uint64_t check_for_dynamic_string(perfometer::string_id string_id);
        const std::string& stringByID(uint64_t string_id);

    private:
        double m_startTime;
        double m_endTime;

        Traits  m_traits;
        Threads m_threads;

        Thread::ID m_mainThreadID;

        uint64_t m_dynamic_string_id;
        std::unordered_map<uint64_t, std::string> m_dynamic_strings;
    };

} // namespace visualizer
