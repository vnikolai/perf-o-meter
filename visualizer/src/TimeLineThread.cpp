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

#include "TimeLineThread.h"
#include "TimeLineConfig.h"
#include "Utils.h"

namespace visualizer {

TimeLineThread::TimeLineThread(ConstThreadPtr thread)
    : m_thread(thread)
{
    setHeight(calculateThreadHeight());
}

void TimeLineThread::render(QPainter& painter, QRect pos, double pixelPerSecond)
{
    
}

int TimeLineThread::calculateThreadHeight()
{
    int height = ThreadTitleHeight;

    int recordsHeight = 0;
    for (auto record : m_thread->records)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }

    height += recordsHeight * RecordHeight;

    return height;
}

int TimeLineThread::calculateRecordHeight(const Record& record)
{
    int recordsHeight = 0;
    for (auto record : record.enclosed)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }

    return recordsHeight + 1;
}

} // namespace visualizer
