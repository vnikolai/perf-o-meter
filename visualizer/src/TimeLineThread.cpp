/* Copyright 2020-2021 Volodymyr Nikolaichuk

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

#include <QTime>

#include <thread>
#include <chrono>

namespace visualizer {

namespace {

template<typename T>
    void clampWidth(T& x, T& w, T left, T right)
{
    if (x < left)
    {
        w -= left - x;
        x = left;
    }

    if (x + w > right)
    {
        w -= x + w - right;
    }
}

enum HitTestResult
{
    NoHit,
    Hit,
    Abort
};

HitTestResult hitTestRecord(QPointF pos,
                    double pixelPerSecond,
                    const Record& record,
                    coord_t y,
                    std::shared_ptr<RecordInfo>& result)
{
    qreal x = record.timeStart * pixelPerSecond;
    qreal w = (record.timeEnd - record.timeStart) * pixelPerSecond;
    coord_t h = RecordHeight;

    if (pos.x() < x)
    {
        return HitTestResult::Abort;
    }

    if (pos.x() > x + w || w < 1 || pos.y() < y)
    {
        return HitTestResult::NoHit;
    }

    QRectF bounds(x, y, w, h);
    if (bounds.contains(pos))
    {
        RecordInfo info{bounds, record.name, record.timeStart, record.timeEnd };
        result = std::make_shared<RecordInfo>(info);
        return HitTestResult::Hit;
    }

    y += RecordHeight;

    for (auto enclosedRecord : record.enclosed)
    {
        HitTestResult hitTestResult = hitTestRecord(pos, pixelPerSecond, enclosedRecord, y, result);
        if (hitTestResult != HitTestResult::NoHit)
        {
            return hitTestResult;
        }
    }

    return HitTestResult::NoHit;
};

} // namespace

TimeLineThread::TimeLineThread(TimeLineView& view, ConstThreadPtr thread)
    : TimeLineComponent(view)
    , m_thread(thread)
    , m_recordsHeight(0)
{
    setName(std::to_string(m_thread->id) + " - " + m_thread->name);

    auto thisHeight = calculateThreadHeight(&m_recordsHeight);
    setHeight(thisHeight);
}

void TimeLineThread::mouseMove(QPointF pos)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    TimeLineComponent::mouseMove(pos);

    const auto pixelPerSecond = m_view.pixelsPerSecond();

    m_highlightedRecordInfo.reset();

    if (pos.y() <= 0 || pos.y() >= height())
    {
        return;
    }

    QTime hitTestTime;
    hitTestTime.start();
    
    for (const auto& record : m_thread->records)
    {
        HitTestResult result = hitTestRecord(pos,
                                             pixelPerSecond,
                                             record,
                                             ThreadTitleHeight,
                                             m_highlightedRecordInfo);
        
        if (result == HitTestResult::Abort)
        {
            break;
        }
    }

    m_statistics.hitTestTime = hitTestTime.elapsed() / 1000.0;
    
}

void TimeLineThread::mouseLeft()
{
    TimeLineComponent::mouseLeft();

    m_highlightedRecordInfo.reset();
}

void TimeLineThread::mouseClick(QPointF pos)
{
    TimeLineComponent::mouseClick(pos);

    m_selectedRecordInfo = m_highlightedRecordInfo;
}

void TimeLineThread::mouseDoubleClick(QPointF pos)
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

void TimeLineThread::render(QPainter& painter, QRectF viewport, QPointF offset)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    QTime frameTime;
    frameTime.start();

    const auto pixpersec = m_view.pixelsPerSecond();

    TimeLineComponent::render(painter, viewport, offset);

    if (collapsed())
    {
        return;
    }

    painter.setPen(Qt::black);

    QPointF childPos(offset);
    childPos.ry() += ThreadTitleHeight;
    drawRecords(painter, viewport, childPos, m_thread->records);

    drawEvents(painter, viewport, childPos, m_recordsHeight, m_thread->events);

    if (m_selectedRecordInfo)
    {
        QRectF bounds(m_selectedRecordInfo->bounds);
        bounds.translate(viewport.topLeft() + offset);

        painter.setPen(Qt::white);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(bounds.intersected(viewport));
    }

    if (m_highlightedRecordInfo)
    {
        QRectF bounds(m_highlightedRecordInfo->bounds);
        bounds.translate(viewport.topLeft() + offset);

        painter.setPen(Qt::green);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(bounds.intersected(viewport));
    }

    m_statistics.frameRenderTime += frameTime.elapsed() / 1000.0;
}

void TimeLineThread::renderOverlay(QPainter& painter, QRectF viewport, QPointF offset)
{
    if (!m_selectedRecordInfo)
    {
        return;
    }

    PERFOMETER_LOG_WORK_FUNCTION();

    QString text;

    QRectF recordInfoBounds(
        viewport.left() + RecordInfoDist,
        viewport.bottom() - RecordInfoHeight - RecordInfoDist,
        viewport.width() - 2 * RecordInfoDist,
        RecordInfoHeight
    );

    painter.setPen(Qt::black);
    painter.setBrush(RulerBackgroundColor);
    painter.drawRect(recordInfoBounds);

    recordInfoBounds.setLeft(recordInfoBounds.left() + RecordInfoTextDist);
    recordInfoBounds.setWidth(recordInfoBounds.width() - 2 * RecordInfoTextDist - RecordInfoTimeWidth);

    text = text.fromStdString(m_selectedRecordInfo->name);
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignLeft, text);

    recordInfoBounds.setRight(viewport.right() - RecordInfoDist - RecordInfoTextDist);

    const auto duration = m_selectedRecordInfo->endTime - m_selectedRecordInfo->startTime;
    text = text.fromStdString(format_time(duration));
    painter.drawText(recordInfoBounds, Qt::AlignVCenter | Qt::AlignRight, text);
}

void TimeLineThread::drawRecord(QPainter& painter, QRectF viewport, QPointF offset, const Record& record)
{
    const auto pixpersec = m_view.pixelsPerSecond();

    coord_t x = viewport.left() + offset.x() + record.timeStart * pixpersec;
    coord_t y = viewport.top() + offset.y();
    coord_t w = (record.timeEnd - record.timeStart) * pixpersec;
    coord_t h = RecordHeight;

    if (offset.y() > -RecordHeight && offset.y() < viewport.height())
    {
        clampWidth<coord_t>(x, w, viewport.left(), viewport.right());

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

            offset.ry() += RecordHeight;
            drawRecords(painter, viewport, offset, record.enclosed);
        }
        else
        {
            painter.drawLine(x, y, x, y + h);
        }
    }

    m_statistics.numRecords++;
}

void TimeLineThread::drawRecords(QPainter& painter, QRectF viewport, QPointF offset, const std::vector<Record>& records)
{
    const auto pixpersec = m_view.pixelsPerSecond();

    for (const auto& record : records)
    {
        coord_t x = offset.x() + record.timeStart * pixpersec;
        coord_t w = (record.timeEnd - record.timeStart) * pixpersec;

        if (x + w < 0)
        {
            continue;
        }

        if (x > viewport.width())
        {
            break;
        }

        drawRecord(painter, viewport, offset, record);
    }
}

void TimeLineThread::drawEvents(QPainter& painter, QRectF viewport, QPointF offset, coord_t textYOffset, const std::vector<Event>& events)
{
    const auto pixpersec = m_view.pixelsPerSecond();

    QString text;

    for (const auto& event : m_thread->events)
    {
        coord_t x = offset.x() + event.time * pixpersec;
        coord_t y = offset.y();
        if (x < 0)
        {
            continue;
        }

        if (x > viewport.width())
        {
            break;
        }

        x += viewport.x();
        y += viewport.y();

        coord_t y_start = std::max<coord_t>(y - ThreadTitleHeight / 4, viewport.top());
        coord_t y_end = std::min<coord_t>(y + height() - ThreadTitleHeight, viewport.bottom());

        if (y_end > y_start)
        {
            painter.setPen(Qt::cyan);
            painter.drawLine(x, y_start, x, y_end);

            QRectF titleRect(x  + TitleOffsetSmall, y + textYOffset, viewport.width(), RecordHeight);
            if (titleRect.intersects(viewport))
            {
                painter.setPen(Qt::darkCyan);
                painter.drawText(titleRect,
                                Qt::AlignVCenter | Qt::AlignLeft,
                                text.fromStdString(event.name));
            }
        }
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
