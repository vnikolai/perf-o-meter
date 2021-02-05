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

template<typename T>
    void clampWidth(T& x, T& w, T width)
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

bool hitTestRecord(QPointF pos,
                    double pixelPerSecond,
                    const Record& record,
                    coord_t y,
                    std::shared_ptr<RecordInfo>& result)
{
    qreal x = record.timeStart * pixelPerSecond;
    qreal w = (record.timeEnd - record.timeStart) * pixelPerSecond;
    coord_t h = RecordHeight;

    if (x > pos.x() || x + w < pos.x())
    {
        return false;
    }

    QRectF bounds(x, y, w, h);
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

    auto thisHeight = calculateThreadHeight(&m_recordsHeight);
    setHeight(thisHeight);
}

void TimeLineThread::mouseMove(QPoint pos)
{
    PERFOMETER_LOG_WORK_FUNCTION();

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

void TimeLineThread::render(QPainter& painter, QRectF pos)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    TimeLineComponent::render(painter, pos);

    if (collapsed())
    {
        return;
    }

    painter.setPen(Qt::black);

    QRectF childPos(pos);
    childPos.translate(0, ThreadTitleHeight);
    drawRecords(painter, childPos, m_thread->records);

    drawEvents(painter, childPos, m_recordsHeight, m_thread->events);

    if (m_highlightedRecordInfo)
    {
        QRectF bounds(m_highlightedRecordInfo->bounds);
        bounds.translate(pos.topLeft());

        painter.setPen(Qt::green);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(bounds);
    }
}

void TimeLineThread::renderOverlay(QPainter& painter, QRectF pos)
{
    if (!m_selectedRecordInfo)
    {
        return;
    }

    PERFOMETER_LOG_WORK_FUNCTION();

    QString text;

    QRectF recordInfoBounds(
        RecordInfoDist,
        pos.height() - RecordInfoHeight - RecordInfoDist,
        pos.width() - 2 * RecordInfoDist,
        RecordInfoHeight
    );

    painter.setPen(Qt::black);
    painter.setBrush(RulerBackgroundColor);
    painter.drawRect(recordInfoBounds);

    recordInfoBounds.setLeft(recordInfoBounds.left() + RecordInfoTextDist);
    recordInfoBounds.setWidth(recordInfoBounds.width() - 2 * RecordInfoTextDist - RecordInfoTimeWidth);

    text = text.fromStdString(m_selectedRecordInfo->name);
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignLeft, text);

    recordInfoBounds.setRight(pos.width() - RecordInfoDist - RecordInfoTextDist);

    const auto duration = m_selectedRecordInfo->endTime - m_selectedRecordInfo->startTime;
    text = text.fromStdString(format_time(duration));
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignRight, text);
}

void TimeLineThread::drawRecord(QPainter& painter, QRectF pos, const Record& record)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    coord_t x = pos.x() + record.timeStart * pixpersec;
    coord_t y = pos.y();
    coord_t w = (record.timeEnd - record.timeStart) * pixpersec;
    coord_t h = RecordHeight;

    clampWidth<coord_t>(x, w, viewportWidth);

    bool selected = false;
    if (w > 2)
    {
        int id = static_cast<int>(record.timeStart * PixelsPerSecond) + record.name.length();
        const int colorIndex = id % NumColors;

        QColor color = Colors[colorIndex];
        if (record.wait)
        {
            color.setAlpha(128);
        }

        painter.setBrush(color);
        painter.drawRect(x, y, w, h);
    }
    else
    {
        painter.drawLine(x, y, x, y + h);
    }

    if (w >= RecordMinTextWidth)
    {
        QString text;
        text = text.fromStdString(format_time(record.timeEnd - record.timeStart));
        painter.drawText(x + TitleOffsetSmall, y, w - 2 * TitleOffsetSmall, h, Qt::AlignVCenter | Qt::AlignRight, text);
        
        auto time_text_width = painter.fontMetrics().boundingRect(text).width();
        auto label_width = w - 2 * TitleOffsetSmall - time_text_width - TitleOffset;
        if (label_width > 0)
        {
            text = text.fromStdString(record.name);
            painter.drawText(x + TitleOffsetSmall, y, label_width, h, Qt::AlignVCenter | Qt::AlignLeft, text);
        }
    }

    pos.translate(0, RecordHeight);
    drawRecords(painter, pos, record.enclosed);
}

void TimeLineThread::drawRecords(QPainter& painter, QRectF pos, const std::vector<Record>& records)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    for (const auto& record : records)
    {
        coord_t x = pos.x() + record.timeStart * pixpersec;
        coord_t w = (record.timeEnd - record.timeStart) * pixpersec;

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

void TimeLineThread::drawEvents(QPainter& painter, QRectF pos, coord_t textYOffset, const std::vector<Event>& events)
{
    const auto viewportWidth = pos.width();
    const auto pixpersec = m_view.pixelsPerSecond();

    QString text;

    for (const auto& event : m_thread->events)
    {
        coord_t x = pos.x() + event.time * pixpersec;
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
        painter.drawText(x  + TitleOffsetSmall, pos.y() + textYOffset,
                         viewportWidth, RecordHeight,
                         Qt::AlignVCenter | Qt::AlignLeft,
                         text.fromStdString(event.name));
    }
}

coord_t TimeLineThread::calculateThreadHeight(coord_t* oRecordsHeight)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    coord_t height = ThreadTitleHeight;

    coord_t recordsHeight = calculateRecordsHeight(m_thread->records) * RecordHeight;

    height += recordsHeight;

    if (oRecordsHeight)
    {
        *oRecordsHeight = recordsHeight;
    }

    if (m_thread->events.size() > 0)
    {
        height += RecordHeight;
    }

    return height;
}

coord_t TimeLineThread::calculateRecordsHeight(const std::vector<Record>& records)
{
    coord_t recordsHeight = 0;
    for (const auto& record : records)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(record));
    }
    return recordsHeight;
}

coord_t TimeLineThread::calculateRecordHeight(const Record& record)
{
    coord_t recordsHeight = 0;
    for (const auto& enclosedRecord : record.enclosed)
    {
        recordsHeight = std::max(recordsHeight, calculateRecordHeight(enclosedRecord));
    }

    return recordsHeight + 1;
}

} // namespace visualizer
