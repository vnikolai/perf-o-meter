// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <map>
#include <string>
#include <vector>

namespace visualizer
{
    using ThreadID = int64_t;

    struct Record
    {
        double timeStart;
        double timeEnd;
        std::string name;
        std::vector<Record> enclosed;
    };

    struct Thread
    {
        Thread(ThreadID _ID)
            : ID(_ID)
        {
        }

        ThreadID ID;
        std::string name;
        std::vector<Record> records;
    };

    using Threads = std::map<ThreadID, Thread>;

    class PerfometerReport
    {
    public:
        PerfometerReport();
        virtual ~PerfometerReport();

        bool loadFile(const std::string& fileName);

        double getStartTime() const { return m_startTime; }
        double getEndTime() const { return m_endTime; }

        const Threads& getThreads() const;
        Thread& getThread(ThreadID id);
        const Thread& getThread(ThreadID id) const;
        
    private:
        double m_startTime;
        double m_endTime;

        Threads m_threads;
    };

} // namespace visualizer
