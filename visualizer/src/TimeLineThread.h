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

#include "PerfometerReport.h"
#include "TimeLineComponent.h"
#include <memory>
#include <QPainter>

namespace visualizer
{
    struct RecordInfo
    {
        QRect bounds;
        std::string name;
        double startTime;
        double endTime;
    };

    class TimeLineThread : public TimeLineComponent
    {
    public:
        TimeLineThread(TimeLineView& view, ConstThreadPtr thread);

        void mouseMove(QPoint pos) override;
        void mouseClick(QPoint pos) override;
        void mouseDoubleClick(QPoint pos) override;
        void focusLost() override;

        void render(QPainter& painter, QRect pos) override;
        void renderOverlay(QPainter& painter, QRect pos) override;

    private:

        void drawPerfometerRecord(QPainter& painter, QRect pos, const Record& record);
        void drawPerfometerRecords(QPainter& painter, QRect pos, const std::vector<Record>& records);

        int calculateThreadHeight();
        int calculateRecordHeight(const Record& record);

    private:
        ConstThreadPtr                      m_thread;

        std::shared_ptr<RecordInfo>         m_highlightedRecordInfo;
        std::shared_ptr<RecordInfo>         m_selectedRecordInfo;
    };
} // namespace visualizer
