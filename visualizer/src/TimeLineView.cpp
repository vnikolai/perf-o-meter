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

#include "TimeLineView.h"
#include <cstring>
#include <algorithm>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

namespace visualizer {

constexpr int       DefaultZoom             = 1000;
constexpr double    VisibleMargin           = 0.1;  // 10% of report time each size
constexpr int       RulerHeight             = 24;
constexpr int       RulerDistReport         = 12;
constexpr int       ThreadTitleHeight       = 32;
constexpr int       TitleOffsetSmall        = 2;
constexpr int       RecordHeight            = 16;
constexpr int       ScrolBarThickness       = 24;
constexpr int       MinZoom                 = 10;
constexpr int       ZoomKeyboardStep        = 250;
constexpr int       OffsetKeyboardStep      = 10;
constexpr int       OffsetKeyboardPageStep  = 240;
constexpr int       RecordMinTextWidth      = 10;
constexpr int       PixelsPerSecond         = 128;

constexpr int NumColors = 8;
QColor Colors[NumColors] = { Qt::darkRed,
                             Qt::darkGreen,
                             Qt::darkCyan,
                             Qt::darkYellow,
                             Qt::darkMagenta,
                             Qt::gray,
                             Qt::lightGray,                             
                             Qt::darkGray };

TimeLineView::TimeLineView()
    : QOpenGLWidget(nullptr)
    , m_horizontalScrollBar(Qt::Horizontal, this)
    , m_verticalScrollBar(Qt::Vertical, this)
    , m_zoom(DefaultZoom)
    , m_offset(0, 0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    connect(&m_horizontalScrollBar, &QScrollBar::valueChanged,
            this, &TimeLineView::onHorizontalSliderChanged);

    connect(&m_verticalScrollBar, &QScrollBar::valueChanged,
            this, &TimeLineView::onVerticalSliderChanged);
}

TimeLineView::~TimeLineView()
{
}

void TimeLineView::setReport(std::shared_ptr<PerfometerReport> report)
{
    m_report = report;

    m_reportHeightPx = m_report ? getReportHeight(*m_report) : 0;

    layout();
}

void TimeLineView::onHorizontalSliderChanged(int value)
{
    m_offset.setX(value);
    update();
}

void TimeLineView::onVerticalSliderChanged(int value)
{
    m_offset.setY(value);
    update();
}

void TimeLineView::initializeGL()
{
    QOpenGLFunctions::initializeOpenGLFunctions();

    glClearColor(0.117f, 0.117f, 0.117f, 1.0f);

    layout();
}

void TimeLineView::paintGL()
{
    const auto thisWidth = width();
    const auto thisHeight = height();

    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter(this);
    painter.setFont(QFont("Helvetica", 10));

    QPoint pos(-m_offset.x(), RulerHeight + RulerDistReport - m_offset.y());
    if (m_report)
    {
        drawPerfometerReport(painter, pos, *m_report);
    }

    drawStatusMessage(painter);
    drawRuler(painter, pos);

    painter.setPen(Qt::darkGreen);
    painter.drawLine(m_mousePosition.x(), 0, m_mousePosition.x(), thisHeight);

    painter.end();
}

void TimeLineView::mouseMoveEvent(QMouseEvent* event)
{
    super::mouseMoveEvent(event);

    m_mousePosition = event->pos();

    update();
}

void TimeLineView::wheelEvent(QWheelEvent* event)
{
    m_zoom += event->angleDelta().y();
    m_zoom = std::max(m_zoom, MinZoom);

    event->accept();

    update();
    layout();
}

void TimeLineView::resizeEvent(QResizeEvent* event)
{
    super::resizeEvent(event);

    layout();
}

float TimeLineView::pixelsPerSecond() const
{
    return PixelsPerSecond * 1.0f * m_zoom / DefaultZoom;
}

void TimeLineView::drawStatusMessage(QPainter& painter)
{
    const auto thisWidth = width();
    const auto thisHeight = height();

    constexpr size_t bufferSize = 64;
    char text[bufferSize];
    snprintf(text, bufferSize,
             "%d %d %d %d %d",
             m_mousePosition.x(), m_mousePosition.y(), m_zoom, m_offset.x(), m_offset.y());

    painter.setPen(Qt::white);
    painter.drawText(thisWidth - 250, thisHeight - 50, text);
}

int TimeLineView::drawPerfometerRecord(QPainter& painter, QPoint& pos, const Record& record)
{
    const auto thisWidth = width();
    const auto pixpersec = pixelsPerSecond();
    
    int depth = 1;

    int x = pos.x() + static_cast<int>(record.timeStart * pixpersec);
    int y = pos.y();
    int w = static_cast<int>((record.timeEnd - record.timeStart) * pixpersec);
    int h = RecordHeight;
    
    const int colorIndex = rand() % NumColors;

    const bool visible = x + w > 0 && x < thisWidth;
    if (visible)
    {
        if (w > 2)
        {
            painter.fillRect(x, y, w, h, Colors[colorIndex]);
        }

        painter.drawRect(x, y, w, h);
    
        if (w >= RecordMinTextWidth)
        {
            QString text;
            text = text.fromStdString(record.name + " " + formatTime(record.timeEnd - record.timeStart));
            painter.drawText(x + TitleOffsetSmall, y, w, h, Qt::AlignVCenter | Qt::AlignLeft, text);
        }
    }

    pos.ry() += RecordHeight;
    depth += drawPerfometerRecords(painter, pos, record.enclosed);
    pos.ry() -= RecordHeight;

    return depth;
}

int TimeLineView::drawPerfometerRecords(QPainter& painter, QPoint& pos, const std::vector<Record>& records)
{
    int depth = 0;
    for (auto record : records)
    {
        bool inView = true;
        if (inView)
        {
            depth = std::max(drawPerfometerRecord(painter, pos, record), depth);
        }
    }

    return depth;
}

void TimeLineView::drawPerfometerThread(QPainter& painter, QPoint& pos, const Thread& thread)
{
    const auto thisWidth = width();
    int threadHeight = getThreadHeight(thread);

    if (pos.ry() + threadHeight < RulerHeight + RulerDistReport)
    {
        pos.ry() += threadHeight;
        return;
    }

    QString text;
    painter.setPen(Qt::white);
    painter.drawText(RulerDistReport + std::max(0, pos.x()), pos.y(),
                        thisWidth, ThreadTitleHeight,
                        Qt::AlignVCenter | Qt::AlignLeft,
                        text.fromStdString(thread.name));

    pos.ry() += ThreadTitleHeight;

    srand(0);

    painter.setPen(Qt::black);
    int depth = drawPerfometerRecords(painter, pos, thread.records);
    pos.ry() += depth * RecordHeight;
}

void TimeLineView::drawPerfometerReport(QPainter& painter, QPoint& pos, const PerfometerReport& report)
{
    const auto thisHeight = height();

    std::multimap<std::string, const Thread*> threads;
    for (const auto& it : report.getThreads())
    {
        const ThreadID tid = it.first;
        if (tid != report.mainThreadID())
        {
            const Thread& thread = it.second;
            threads.emplace(thread.name, &thread);
        }
    }

    drawPerfometerThread(painter, pos, report.getThread(report.mainThreadID()));
    
    for (const auto& it : threads)
    {
        const Thread& thread = *it.second;

        drawPerfometerThread(painter, pos, thread);

        if (pos.ry() >= thisHeight )
        {
            break;
        }
    }
}

void TimeLineView::drawRuler(QPainter& painter, QPoint& pos)
{
    const auto thisWidth = width();
    const auto thisHeight = height();

    constexpr int RulerStep = 24;
    constexpr int PrimaryStrokeLength = 16;
    constexpr int SecondaryStrokeLength = 12;

    painter.fillRect(0, 0, thisWidth, RulerHeight, QColor(228, 230, 241, 255));

    painter.setPen(Qt::black);
    painter.drawRect(1, 0, thisWidth - 1, RulerHeight);
    painter.drawLine(0, RulerHeight, thisWidth, RulerHeight);

    int zeroX = 0;

    double secondsPerPixel = 1.0f * DefaultZoom / (PixelsPerSecond * m_zoom);

    int rulerCount = thisWidth / RulerStep;
    double stepValue = RulerStep * secondsPerPixel;    

    constexpr size_t bufferSize = 64;
    QString text;

    for (int i = 0, s = 0; s < thisWidth; s += RulerStep, ++i)
    {
        int x = s + std::max(pos.x(), 0);
        if (i % 2)
        {
            painter.drawLine(x, 0, x, SecondaryStrokeLength);
        }
        else
        {
            painter.drawLine(x, 0, x, PrimaryStrokeLength);

            double rulerTime = i * stepValue + (pos.x() < 0 ? -pos.x() * secondsPerPixel : 0);
            painter.drawText(x + TitleOffsetSmall, 0, 64, RulerHeight,
                             Qt::AlignVCenter | Qt::AlignLeft,
                             text.fromStdString(formatTime(rulerTime)));
        }

        if (i == 0)
        {
            zeroX = x;
        }
    }

    painter.setPen(Qt::darkRed);
    painter.drawLine(zeroX, 0, zeroX, thisHeight);
}

void TimeLineView::layout()
{
    const auto thisWidth = width();
    const auto thisHeight = height();
    int reportWidth = 0;

    if (m_report)
    {
        auto pixpersec = pixelsPerSecond();;
        int reportStartPx = m_report->getStartTime() * pixpersec;
        int reportEndPx = m_report->getEndTime() * pixpersec;
        reportWidth = (reportEndPx - reportStartPx);
    
        if (reportWidth <= thisWidth)
        {
            m_offset.setX(0);
        }
        else
        {
            int extraWidth = reportWidth - thisWidth;
            m_horizontalScrollBar.setMinimum(reportStartPx - extraWidth * VisibleMargin / 2);
            m_horizontalScrollBar.setMaximum(reportStartPx + extraWidth * (1 + VisibleMargin / 2));
        }

        m_offset.setX(std::max(m_offset.x(), reportStartPx));
        m_offset.setX(std::min(m_offset.x(), reportEndPx));
    }

    int extraHeight = m_reportHeightPx * (1 + VisibleMargin / 2) - (thisHeight - RulerHeight - RulerDistReport);
    bool vertBarVisible = extraHeight > 0;
    bool horBarVisible = reportWidth > thisWidth;
    
    if (vertBarVisible)
    {
        m_offset.setY(std::max(m_offset.y(), 0));
        m_offset.setY(std::min(m_offset.y(), extraHeight));
        
        m_verticalScrollBar.setMinimum(0);
        m_verticalScrollBar.setMaximum(extraHeight);
    }
    else
    {
        m_offset.setY(0);
    }

    m_horizontalScrollBar.resize(thisWidth - ScrolBarThickness, ScrolBarThickness);
    m_horizontalScrollBar.move(0, height() - m_horizontalScrollBar.height());

    m_verticalScrollBar.resize(ScrolBarThickness, thisHeight - RulerHeight - ScrolBarThickness);
    m_verticalScrollBar.move(thisWidth - ScrolBarThickness, RulerHeight);

    m_horizontalScrollBar.setVisible(horBarVisible);
    m_verticalScrollBar.setVisible(vertBarVisible);
}

std::string TimeLineView::formatTime(double time)
{
    std::string suffix;
    double denom = 0;

    int withFraction = 0;
    bool showPart = false;

    if (time < 1e-6)
    {
        if (abs(time) < std::numeric_limits<double>::epsilon())
        {
            return std::string("0");
        }

        // nanos
        suffix = "ns";
        denom = 1000000000;
    }
    else if (time < 1e-3)
    {
        // micros
        suffix = "us";
        denom = 1000000;
    }
    else if (time < 1)
    {
        // millis
        suffix = "ms";
        denom = 1000;
    }
    else if (time < 60)
    {
        // seconds
        suffix = "s";
        denom = 1;

        withFraction = time < 10 ? 100 : 10;
    }
    else if (time < 3600)
    {
        // minutes
        suffix = "m";
        denom = 1.0/60;

        showPart = true;
    }
    else
    {
        // hours
        suffix = "h";
        denom = 1.0/3600;

        showPart = true;
    }

    std::string result;
    if (withFraction)
    {
        int value = static_cast<int>(time * denom);
        int fraction = static_cast<int>((time * denom - value) * withFraction);
        if (fraction >= 1)
        {
            char text[16];
            std::snprintf(text, 16, "%d.%d", value, fraction);

            return std::string(text) + suffix;
        }
    }
    
    result = std::to_string(static_cast<int>(time * denom)) + suffix;

    if (showPart)
    {
        double fract = time * denom - static_cast<int>(time * denom);
        if (fract >= denom)
        {
            result += " " + formatTime(fract / denom);
        }
    }

    return result;
}

int TimeLineView::getReportHeight(const PerfometerReport& report)
{
    int height = 0;
    for (const auto& it : report.getThreads())
    {
        const Thread& thread = it.second;

        height += getThreadHeight(thread);
    }

    return height;
}

int TimeLineView::getThreadHeight(const Thread& thread)
{
    int height = ThreadTitleHeight;

    int recordsHeight = 0;
    for (auto record : thread.records)
    {
        recordsHeight = std::max(recordsHeight, getRecordHeight(record));
    }

    height += recordsHeight * RecordHeight;

    return height;
}

int TimeLineView::getRecordHeight(const Record& record)
{
    int recordsHeight = 0;
    for (auto record : record.enclosed)
    {
        recordsHeight = std::max(recordsHeight, getRecordHeight(record));
    }

    return recordsHeight + 1;
}

} // namespace visualizer
