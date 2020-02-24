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

        bool loadFile(const std::string& fileName,
                      std::function<void(const std::string&)> logger = nullptr);

        double getStartTime() const { return m_startTime; }
        double getEndTime() const { return m_endTime; }

        ThreadID mainThreadID() const { return m_mainThreadID; }

        const Threads& getThreads() const;
        Thread& getThread(ThreadID id);
        const Thread& getThread(ThreadID id) const;
        
    private:
        double m_startTime;
        double m_endTime;

        Threads m_threads;

        ThreadID m_mainThreadID;
    };

} // namespace visualizer
