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
#include "TimeLineView.h"

#include <perfometer/perfometer.h>
#include <perfometer/helpers.h>

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
{
    setHeight(calculateThreadHeight());
}

void TimeLineThread::mouseMove(QPoint pos)
{
    PERFOMETER_LOG_FUNCTION();

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

void TimeLineThread::mouseClick(QPoint pos)
{
    m_selectedRecordInfo = m_highlightedRecordInfo;
}

void TimeLineThread::mouseDoubleClick(QPoint pos)
{
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
    m_selectedRecordInfo.reset();
}

void TimeLineThread::render(QPainter& painter, QRect pos)
{
    PERFOMETER_LOG_FUNCTION();

    //painter.setPen(Qt::green);
    //painter.fillRect(QRect(0, pos.y(), pos.width(), height()), QColor(37, 37, 37, 255));
    
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    QString text;
    painter.setPen(Qt::white);
    painter.drawText(RulerDistReport + std::max(0, pos.x()), pos.y(),
                     viewportWidth, ThreadTitleHeight,
                     Qt::AlignVCenter | Qt::AlignLeft,
                     text.fromStdString(m_thread->name));

    painter.setPen(Qt::black);

    QRect childPos(pos);
    childPos.translate(0, ThreadTitleHeight);
    drawPerfometerRecords(painter, childPos, m_thread->records);

    painter.setPen(Qt::cyan);

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
        
        painter.drawLine(x,
                         childPos.y() - ThreadTitleHeight / 4,
                         x,
                         pos.y() + height() + ThreadTitleHeight / 4);
    }

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
    text = text.fromStdString(formatTime(duration));
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignRight, text);
}

void TimeLineThread::drawPerfometerRecord(QPainter& painter, QRect pos, const Record& record)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();
    
    int x = pos.x() + static_cast<int>(record.timeStart * pixpersec);
    int y = pos.y();
    int w = static_cast<int>((record.timeEnd - record.timeStart) * pixpersec);
    int h = RecordHeight;
    
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
        text = text.fromStdString(record.name + " " + formatTime(record.timeEnd - record.timeStart));
        painter.drawText(x + TitleOffsetSmall, y, w - 2 * TitleOffsetSmall, h, Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    pos.translate(0, RecordHeight);
    drawPerfometerRecords(painter, pos, record.enclosed);
}

void TimeLineThread::drawPerfometerRecords(QPainter& painter, QRect pos, const std::vector<Record>& records)
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

        drawPerfometerRecord(painter, pos, record);
    }
}

int TimeLineThread::calculateThreadHeight()
{
    PERFOMETER_LOG_FUNCTION();

    int height = ThreadTitleHeight;

    int recordsHeight = 0;
    for (const auto& record : m_thread->records)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }

    height += recordsHeight * RecordHeight;

    if (m_thread->records.size() == 0 && m_thread->events.size() > 0)
    {
        height += RecordHeight;
    }

    return height;
}

int TimeLineThread::calculateRecordHeight(const Record& record)
{
    int recordsHeight = 0;
    for (const auto& record : record.enclosed)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }

    return recordsHeight + 1;
}

} // namespace visualizer
