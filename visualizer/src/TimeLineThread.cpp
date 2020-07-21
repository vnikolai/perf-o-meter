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
#include "TimeLineView.h"

#include <perfometer/perfometer.h>
#include <perfometer/helpers.h>

#include <utils/time.h>

#include <thread>
#include <chrono>

namespace visualizer {

namespace {

bool hitTestRecord(QPoint pos,
                    double pixelPerSecond,
                    const Record& record,
                    int y,
                    std::shared_ptr<RecordInfo>& result)
{
    int x = static_cast<int>(record.timeStart * pixelPerSecond);
    int w = static_cast<int>((record.timeEnd - record.timeStart) * pixelPerSecond);
    int h = RecordHeight;
    
    if (x > pos.x() || x + w < pos.x())
    {
        return false;
    }

    QRect bounds(x, y, w, h);
    if (bounds.contains(pos))
    {
        RecordInfo info{bounds, record.name, record.timeStart, record.timeEnd };
        result = std::make_shared<RecordInfo>(info);
        return true;
    }

    y += RecordHeight;

    for (auto enclosedRecord : record.enclosed)
    {
        if (hitTestRecord(pos, pixelPerSecond, enclosedRecord, y, result))
        {
            return true;
        }
    }

    return false;
};

} // namespace

TimeLineThread::TimeLineThread(TimeLineView& view, ConstThreadPtr thread)
    : TimeLineComponent(view)
    , m_thread(thread)
    , m_recordsHeight(0)
{
    setName(m_thread->name);

    int thisHeight = calculateThreadHeight(&m_recordsHeight);
    setHeight(thisHeight);
}

void TimeLineThread::mouseMove(QPoint pos)
{
    PERFOMETER_LOG_FUNCTION();

    TimeLineComponent::mouseMove(pos);

    const auto pixelPerSecond = m_view.pixelsPerSecond();

    m_highlightedRecordInfo.reset();

    if (pos.y() > 0 && pos.y() < height())
    {
        for (auto record : m_thread->records)
        {
            if (hitTestRecord(pos,
                              pixelPerSecond,
                              record,
                              ThreadTitleHeight,
                              m_highlightedRecordInfo))
            {
                return;
            }
        }
    }
}

void TimeLineThread::mouseLeft()
{
    TimeLineComponent::mouseLeft();

    m_highlightedRecordInfo.reset();
}

void TimeLineThread::mouseClick(QPoint pos)
{
    TimeLineComponent::mouseClick(pos);

    m_selectedRecordInfo = m_highlightedRecordInfo;
}

void TimeLineThread::mouseDoubleClick(QPoint pos)
{
    TimeLineComponent::mouseDoubleClick(pos);

    if (m_selectedRecordInfo)
    {
        const auto thisWidth = m_view.width();

        const auto duration = m_selectedRecordInfo->endTime - m_selectedRecordInfo->startTime;
        double pixPerSec = thisWidth * (1.0 - VisibleMargin) / duration;

        m_view.zoom(pixPerSec / PixelsPerSecond * DefaultZoom);

        const auto midDuration = m_selectedRecordInfo->startTime + duration / 2;
        m_view.scrollXTo(midDuration * m_view.pixelsPerSecond() - thisWidth / 2);
    }
}

void TimeLineThread::focusLost()
{
    TimeLineComponent::focusLost();

    m_selectedRecordInfo.reset();
}

void TimeLineThread::render(QPainter& painter, QRect pos)
{
    PERFOMETER_LOG_FUNCTION();
    
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    TimeLineComponent::render(painter, pos);

    if (collapsed())
    {
        return;
    }

    painter.setPen(Qt::black);

    QRect childPos(pos);
    childPos.translate(0, ThreadTitleHeight);
    drawRecords(painter, childPos, m_thread->records);

    drawEvents(painter, childPos, m_recordsHeight, m_thread->events);

    if (m_highlightedRecordInfo)
    {
        QRect bounds(m_highlightedRecordInfo->bounds);
        bounds.translate(pos.topLeft());

        painter.setPen(Qt::green);
        painter.drawRect(bounds);
    }
}

void TimeLineThread::renderOverlay(QPainter& painter, QRect pos)
{
    if (!m_selectedRecordInfo)
    {
        return;
    }

    PERFOMETER_LOG_FUNCTION();

    QString text;

    painter.setPen(Qt::black);

    QRect recordInfoBounds(
        RecordInfoDist,
        pos.height() - RecordInfoHeight - RecordInfoDist,
        pos.width() - 2 * RecordInfoDist,
        RecordInfoHeight
    );

    painter.fillRect(recordInfoBounds, RulerBackgroundColor);

    recordInfoBounds.setLeft(recordInfoBounds.left() + RecordInfoTextDist);
    recordInfoBounds.setWidth(recordInfoBounds.width() - 2 * RecordInfoTextDist - RecordInfoTimeWidth);

    text = text.fromStdString(m_selectedRecordInfo->name);
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignLeft, text);

    recordInfoBounds.setRight(pos.width() - RecordInfoDist - RecordInfoTextDist);

    const auto duration = m_selectedRecordInfo->endTime - m_selectedRecordInfo->startTime;
    text = text.fromStdString(format_time(duration));
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignRight, text);
}

void TimeLineThread::drawRecord(QPainter& painter, QRect pos, const Record& record)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();
    
    int x = pos.x() + static_cast<int>(record.timeStart * pixpersec);
    int y = pos.y();
    int w = static_cast<int>((record.timeEnd - record.timeStart) * pixpersec);
    int h = RecordHeight;

    clampWidth(x, w, viewportWidth);
    
    bool selected = false;
    if (w > 2)
    {
        int id = static_cast<int>(record.timeStart * PixelsPerSecond) + record.name.length();
        const int colorIndex = id % NumColors;

        QRect bounds(x, y, w, h);
        painter.fillRect(bounds, Colors[colorIndex]);
    }

    painter.drawRect(x, y, w, h);

    if (w >= RecordMinTextWidth)
    {
        QString text;
        text = text.fromStdString(record.name + " " + format_time(record.timeEnd - record.timeStart));
        painter.drawText(x + TitleOffsetSmall, y, w - 2 * TitleOffsetSmall, h, Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    pos.translate(0, RecordHeight);
    drawRecords(painter, pos, record.enclosed);
}

void TimeLineThread::drawRecords(QPainter& painter, QRect pos, const std::vector<Record>& records)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    for (const auto& record : records)
    {
        int x = pos.x() + static_cast<int>(record.timeStart * pixpersec);
        int w = static_cast<int>((record.timeEnd - record.timeStart) * pixpersec);

        if (x + w < 0)
        {
            continue;
        }

        if (x > viewportWidth)
        {
            break;
        }

        drawRecord(painter, pos, record);
    }
}

void TimeLineThread::drawEvents(QPainter& painter, QRect pos, int recordsHeight, const std::vector<Event>& events)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    QString text;

    for (const auto& event : m_thread->events)
    {
        int x = pos.x() + static_cast<int>(event.time * pixpersec);
        if (x < 0)
        {
            continue;
        }

        if (x > viewportWidth)
        {
            break;
        }

        painter.setPen(Qt::cyan);
        painter.drawLine(x,
                         pos.y() - ThreadTitleHeight / 4,
                         x,
                         pos.y() + height() - ThreadTitleHeight);
        
        painter.setPen(Qt::darkCyan);
        painter.drawText(x  + TitleOffsetSmall, pos.y() + recordsHeight,
                         viewportWidth, RecordHeight,
                         Qt::AlignVCenter | Qt::AlignLeft,
                         text.fromStdString(event.name));
    }
}

void TimeLineThread::clampWidth(int& x, int& w, int width)
{
    if (x < 0)
    {
        w -= -x;
        x = 0;
    }

    if (x + w > width)
    {
        w -= x + w - width;
    }
}

int TimeLineThread::calculateThreadHeight(int* oRecordsHeight)
{
    PERFOMETER_LOG_FUNCTION();

    int height = ThreadTitleHeight;

    int recordsHeight = 0;
    for (const auto& record : m_thread->records)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }

    height += recordsHeight * RecordHeight;

    if (oRecordsHeight)
    {
        *oRecordsHeight = recordsHeight * RecordHeight;
    }

    if (m_thread->events.size() > 0)
    {
        height += RecordHeight;
    }

    return height;
}

int TimeLineThread::calculateRecordHeight(const Record& record)
{
    int recordsHeight = 0;
    for (const auto& enclosedRecord : record.enclosed)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(enclosedRecord));
    }

    return recordsHeight + 1;
}

} // namespace visualizer
