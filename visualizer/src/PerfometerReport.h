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

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace visualizer
{
    using StringID = size_t;

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
        using ID = int64_t;

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

    class PerfometerReport
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

        bool loadFile(const std::string& fileName,
                      std::function<void(const std::string&)> logger = nullptr);

        double getStartTime() const { return m_startTime; }
        double getEndTime() const { return m_endTime; }

        Thread::ID mainThreadID() const { return m_mainThreadID; }

        const Threads& getThreads() const;
        ThreadPtr getThread(Thread::ID id);
        ConstThreadPtr getThread(Thread::ID id) const;

    private:
        double m_startTime;
        double m_endTime;

        Traits  m_traits;
        Threads m_threads;

        std::map<StringID, std::string> m_strings;
        std::map<Thread::ID, std::string> m_thread_names;

        Thread::ID m_mainThreadID;
    };

} // namespace visualizer
