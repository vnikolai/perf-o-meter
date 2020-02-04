// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include "TimeLineView.h"
#include <cstring>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStatusBar>

namespace visualizer {

TimeLineView::TimeLineView()
    : QOpenGLWidget(nullptr)
    , m_horizontalBar(Qt::Horizontal, this)
    , m_zoom(DefaultZoom)
    , m_reportStartTime(0)
    , m_reportEndTime(1000)
    , m_offset(0)
{
    setMouseTracking(true);
}

TimeLineView::~TimeLineView()
{
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

    drawStatusMessage(painter);
    drawRuler(painter);

    painter.setPen(Qt::darkGreen);
    painter.drawLine(m_mousePosition.x(), 0, m_mousePosition.x(), thisHeight);

    painter.end();
}

void TimeLineView::mouseMoveEvent(QMouseEvent *event)
{
    super::mouseMoveEvent(event);

    m_mousePosition = event->pos();

    update();
}

void TimeLineView::wheelEvent(QWheelEvent *event)
{
    m_zoom += event->angleDelta().y();

    if (m_zoom < 1)
    {
        m_zoom = 1;
    }

    event->accept();

    update();
    layout();
}

void TimeLineView::resizeEvent(QResizeEvent *event)
{
    super::resizeEvent(event);

    layout();
}

void TimeLineView::drawStatusMessage(QPainter& painter)
{
    const auto thisWidth = width();
    const auto thisHeight = height();

    constexpr size_t bufferSize = 64;
    char text[bufferSize];
    snprintf(text, bufferSize, "%d %d %d", m_mousePosition.x(), m_mousePosition.y(), m_zoom);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Helvetica", 16));
    painter.drawText(thisWidth - 200, thisHeight - 50, text);
}

void TimeLineView::drawRuler(QPainter& painter)
{
    const auto thisWidth = width();
    const auto thisHeight = height();

    //constexpr int default_zoom_hour_px = 128;
    constexpr int RulerHeight = 24; // pixels
    constexpr int RulerStep = 24;
    constexpr int PrimaryStrokeLength = 16;
    constexpr int SecondaryStrokeLength = 12;

    painter.fillRect(0, 0, thisWidth, RulerHeight, Qt::lightGray);

    painter.setPen(Qt::black);
    painter.drawRect(1, 0, thisWidth - 1, RulerHeight);
    painter.drawLine(0, RulerHeight, thisWidth, RulerHeight);

    int zeroX = 0;
    
    double reportTimeLength = m_reportEndTime - m_reportStartTime;
    double timeMin = m_reportStartTime - reportTimeLength * VisibleMargin;
    double timeMax = m_reportEndTime + reportTimeLength * VisibleMargin;
    double timeLen = timeMax - timeMin;

    int rulerCount = thisWidth / RulerStep;
    double stepValue = timeLen / rulerCount;

    std::string prefix;
    double denom = 0;
    if (stepValue < 1e-6)
    {
        // nanos
        prefix = "ns";
        denom = 1000000000;
    }
    else if (stepValue < 1e-3)
    {
        // micros
        prefix = "us";
        denom = 1000000;
    }
    else if (stepValue < 0)
    {
        // millis
        prefix = "ms";
        denom = 1000;
    }
    else if (stepValue < 60)
    {
        // seconds
        prefix = "s";
        denom = 1;
    }
    else if (stepValue < 3600)
    {
        // minutes
        prefix = "m";
        denom = 1/60;
    }
    else
    {
        // hours
        prefix = "h";
        denom = 1/3600;
    }
    

    int pixels_per_second = thisWidth / timeLen;

    constexpr size_t bufferSize = 64;
    char text[bufferSize];

    painter.setFont(QFont("Helvetica", 10));

    //for (int i = -1; i < thisWidth / default_zoom_hour_px; ++i)
    for (int i = 0, x = 0; x < thisWidth; x += RulerStep, ++i)
    {
        if (i % 2)
        {
            painter.drawLine(x, 0, x, SecondaryStrokeLength);
        }
        else
        {
            painter.drawLine(x, 0, x, PrimaryStrokeLength);

            int value = static_cast<int>(i * stepValue * denom);
            std::snprintf(text, bufferSize, "%d%s", value, prefix.c_str());
            painter.drawText(x, 0, 64, RulerHeight, Qt::AlignTop | Qt::AlignLeft, text);
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

    m_horizontalBar.resize(thisWidth, m_horizontalBar.height());
    m_horizontalBar.move(0, height() - m_horizontalBar.height());

    m_horizontalBar.setVisible(m_zoom > 1);
}

} // namespace visualizer
