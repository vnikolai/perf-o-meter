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
#include "TimeLineConfig.h"
#include "TimeLineThread.h"
#include "Utils.h"
#include <cstring>
#include <algorithm>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <perfometer/perfometer.h>
#include <perfometer/helpers.h>

namespace visualizer {

TimeLineView::TimeLineView()
    : QOpenGLWidget(nullptr)
    , m_horizontalScrollBar(Qt::Horizontal, this)
    , m_verticalScrollBar(Qt::Vertical, this)
    , m_mousePosition(0, 0)
    , m_mouseDragActive(false)
    , m_zoom(DefaultZoom)
    , m_reportHeightPx(0)
    , m_statusTextVisible(false)
    , m_offset(0.0f, 0.0f)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    connect(&m_horizontalScrollBar, &QScrollBar::valueChanged,
            this, &TimeLineView::onHorizontalScrollBarValueChanged);

    connect(&m_verticalScrollBar, &QScrollBar::valueChanged,
            this, &TimeLineView::onVerticalScrollBarValueChanged);
}

TimeLineView::~TimeLineView()
{
}

void TimeLineView::setReport(std::shared_ptr<PerfometerReport> report)
{
    m_report = report;

    m_components.emplace_back(std::make_shared<TimeLineThread>(
        *this, m_report->getThread(m_report->mainThreadID())
    ));

    std::multimap<std::string, std::shared_ptr<TimeLineThread>> threads;
    for (const auto& it : m_report->getThreads())
    {
        const Thread::ID tid = it.first;
        if (tid != m_report->mainThreadID())
        {
            auto thread = it.second;
            threads.emplace(thread->name, std::make_shared<TimeLineThread>(*this, thread));
        }
    }

    for (const auto& it : threads)
    {
        auto thread = it.second;
        m_components.emplace_back(thread);
    }

    m_reportHeightPx = 0;

    for (const auto& component : m_components)
    {
        m_reportHeightPx += component->height();
    }

    layout();
}

void TimeLineView::onHorizontalScrollBarValueChanged(int value)
{
    m_offset.setX(value);
    update();
}

void TimeLineView::onVerticalScrollBarValueChanged(int value)
{
    m_offset.setY(value);
    update();
}

void TimeLineView::initializeGL()
{
    QOpenGLFunctions::initializeOpenGLFunctions();

    glClearColor(BackgroundColor.redF(),
                 BackgroundColor.greenF(),
                 BackgroundColor.blueF(),
                 1.0f);

    layout();
}

void TimeLineView::paintGL()
{
    PERFOMETER_LOG_FUNCTION();

    const auto thisWidth = width();
    const auto thisHeight = height();

    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter(this);
    painter.setFont(QFont("Helvetica", 10));

    QPoint pos(-m_offset.x(), RulerHeight + RulerDistReport - m_offset.y());

    for (auto& component : m_components)
    {
        int componentHeight = component->height();

        if (pos.ry() + componentHeight < RulerHeight + RulerDistReport)
        {
            pos.ry() += componentHeight;
            continue;
        }

        component->render(painter, QRect(pos, QSize(width(), height())));

        pos.ry() += componentHeight;
        if (pos.ry() >= thisHeight)
        {
            break;
        }
    }

    const int offset = RulerHeight + RulerDistReport;

    for (auto& component : m_components)
    {
        component->renderOverlay(painter, QRect(0, offset, thisWidth, thisHeight - offset));
    }

    if (m_statusTextVisible)
    {
        drawStatusMessage(painter);
    }
    
    drawRuler(painter, pos);

    painter.setPen(Qt::darkGreen);
    painter.drawLine(m_mousePosition.x(), 0, m_mousePosition.x(), thisHeight);

    painter.end();
}

void TimeLineView::keyPressEvent(QKeyEvent* event)
{
    const bool ctrl = event->modifiers() & Qt::ControlModifier;
    const bool shift = event->modifiers() & Qt::ShiftModifier;
    const bool alt = event->modifiers() & Qt::AltModifier;
    const bool modifier = ctrl || shift || alt;

    switch (event->key())
    {
        case Qt::Key_Left:
        {
            scrollXBy(-(ctrl ? OffsetKeyboardPageStep : OffsetKeyboardStep));
            break;
        }
        case Qt::Key_Right:
        {
            scrollXBy(ctrl ? OffsetKeyboardPageStep : OffsetKeyboardStep);
            break;
        }
        case Qt::Key_Home:
        {
            if (ctrl)
            {
                scrollYTo(0);
            }
            else
            {
                scrollXTo(0);
            }
            break;
        }
        case Qt::Key_End:
        {
            if (ctrl)
            {
                scrollYTo(std::numeric_limits<int>::max());
            }
            else
            {
                scrollXTo(std::numeric_limits<int>::max());
            }
            break;
        }
        case Qt::Key_Up:
        {
            scrollYBy(-(ctrl ? OffsetKeyboardPageStep : OffsetKeyboardStep));
            break;
        }
        case Qt::Key_Down:
        {
            scrollYBy(ctrl ? OffsetKeyboardPageStep : OffsetKeyboardStep);
            break;
        }
        case Qt::Key_PageUp:
        {
            scrollYBy(-OffsetKeyboardPageStep);
            break;
        }
        case Qt::Key_PageDown:
        {
            scrollYBy(OffsetKeyboardPageStep);
            break;
        }
        case Qt::Key_QuoteLeft:
        {
            m_statusTextVisible = !m_statusTextVisible;
            break;
        }
        case Qt::Key_Plus:
        {
            zoomBy(ctrl ? ZoomKeyboardLargeStep : ZoomKeyboardStep);
            break;
        }
        case Qt::Key_Minus:
        {
            zoomBy(-(ctrl ? ZoomKeyboardLargeStep : ZoomKeyboardStep));
            break;
        }
        case Qt::Key_Asterisk:
        {
            m_zoom = DefaultZoom;
            break;
        }
        default:
            break;
    }

    event->accept();

    layout();
    update();
}

void TimeLineView::keyReleaseEvent(QKeyEvent* event)
{
    super::keyReleaseEvent(event);
}

void TimeLineView::mousePressEvent(QMouseEvent* event)
{
    super::mousePressEvent(event);

    const bool ctrl = event->modifiers() & Qt::ControlModifier;
    const bool shift = event->modifiers() & Qt::ShiftModifier;
    const bool alt = event->modifiers() & Qt::AltModifier;
    const bool modifier = ctrl || shift || alt;

    if (!modifier && event->button() == Qt::LeftButton)
    {
        m_mouseDragActive = true;
    }

    ComponentPtr component = getComponentUnderPoint(m_mousePosition);
    if (m_componentWithFocus && m_componentWithFocus != component)
    {
        if (m_componentWithFocus)
        {
            m_componentWithFocus->focusLost();
            m_componentWithFocus.reset();
        }
    }

    update();
}

void TimeLineView::mouseReleaseEvent(QMouseEvent* event)
{
    super::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        m_mouseDragActive = false;
    }

    QPoint pos;
    ComponentPtr component = getComponentUnderPoint(m_mousePosition, &pos);
    if (component)
    {
        QPoint localPosition = m_mousePosition - pos;
        component->mouseClick(localPosition);
    }

    m_componentWithFocus = component;

    update();
}

void TimeLineView::mouseMoveEvent(QMouseEvent* event)
{
    super::mouseMoveEvent(event);

    if (m_mouseDragActive)
    {
        QPoint delta = event->pos() - m_mousePosition;
        scrollBy(-delta);
    }

    m_mousePosition = event->pos();

    QPoint pos;
    ComponentPtr component = getComponentUnderPoint(m_mousePosition, &pos);
    if (component)
    {
        QPoint localPosition = m_mousePosition - pos;
        component->mouseMove(localPosition);
    }

    update();
    layout();
}

void TimeLineView::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();

    if (event->modifiers() & Qt::ControlModifier)
    {
        zoomBy(delta, m_mousePosition.x());
    }
    else
    {
        scrollYBy(-delta);
    }

    event->accept();

    update();
    layout();
}

void TimeLineView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPoint pos;
    ComponentPtr component = getComponentUnderPoint(m_mousePosition, &pos);
    if (component)
    {
        QPoint localPosition = m_mousePosition - pos;
        component->mouseDoubleClick(localPosition);
    }

    update();
}

void TimeLineView::resizeEvent(QResizeEvent* event)
{
    super::resizeEvent(event);

    layout();
}

double TimeLineView::pixelsPerSecond() const
{
    return static_cast<double>(PixelsPerSecond) * m_zoom / DefaultZoom;
}

void TimeLineView::getRulerStep(int& rulerStep, int& timeStep)
{
    constexpr int NumSteps = 22;
    constexpr int RulerTimeSteps[NumSteps]
    {
        10,
        20,
        50,
        100,
        200,
        500,
        1000,
        2000,
        5000,
        10000,
        20000,
        50000,
        100000,
        200000,
        500000,
        1000000,
        2000000,
        5000000,
        10000000,
        20000000,
        50000000,
        10000000,
    };

    static_assert(
        sizeof(RulerTimeSteps) == NumSteps * sizeof(int),
        "RulerTimeSteps elements number check failed");

    const auto pixpersec = pixelsPerSecond();
    int idx = 0;

    do
    {
        timeStep = RulerTimeSteps[idx++];
        rulerStep = timeStep * (pixpersec / 1000000);
        
    } while (rulerStep < 32 && idx < NumSteps);
}

void TimeLineView::drawRuler(QPainter& painter, QPoint& pos)
{
    PERFOMETER_LOG_FUNCTION();

    const auto thisWidth = width();
    const auto thisHeight = height();

    constexpr int PrimaryStrokeLength = 16;
    constexpr int SecondaryStrokeLength = 12;

    painter.fillRect(0, 0, thisWidth, RulerHeight, RulerBackgroundColor);

    painter.setPen(Qt::black);
    painter.drawRect(1, 0, thisWidth - 1, RulerHeight);
    painter.drawLine(0, RulerHeight, thisWidth, RulerHeight);

    int rulerStep = 0;
    int timeStep = 0;
    getRulerStep(rulerStep, timeStep);

    QString text;

    const auto pixpersec = pixelsPerSecond();
    const double secondsPerPixel = 1.0f * DefaultZoom / (PixelsPerSecond * m_zoom);
    const int rulerCount = thisWidth / rulerStep;

    for (int i = 0; i < rulerCount + 2; ++i)
    {
        int x = i * rulerStep + (pos.x() > 0 ? pos.x() : pos.x() % (2 * rulerStep));

        if (i % 2)
        {
            painter.drawLine(x, 0, x, SecondaryStrokeLength);
        }
        else
        {
            painter.drawLine(x, 0, x, PrimaryStrokeLength);

            int idx = i + 2 * (pos.x() < 0 ? -pos.x() / (2 * rulerStep) : 0);
            double rulerTime = idx * static_cast<double>(timeStep) / 1000000;
            painter.drawText(x + TitleOffsetSmall, 0, 64, RulerHeight,
                             Qt::AlignVCenter | Qt::AlignLeft,
                             text.fromStdString(formatTime(rulerTime)));
        }
    }

    if (pos.x() > 0)
    {
        painter.setPen(Qt::darkRed);
        painter.drawLine(pos.x(), 0, pos.x(), thisHeight);
    }
}

void TimeLineView::drawStatusMessage(QPainter& painter)
{
    PERFOMETER_LOG_FUNCTION();

    const auto thisWidth = width();
    const auto thisHeight = height();

    const int statusMessageHeight = thisHeight - 2 * StatusMessageDist;

    const int posX = thisWidth - StatusMessageWidth - StatusMessageDist;
    const int posY = StatusMessageDist;

    painter.fillRect(posX, posY, StatusMessageWidth, statusMessageHeight, StatusMessageBackgroundColor);

    constexpr size_t bufferSize = 256;
    char text[bufferSize];
    snprintf(text, bufferSize,
             "mouse: %d %d\n"
             "zoom: %d\n"
             "offset: %f %f\n"
             "report time: [%s] - [%s]\n"
             "pixel per second: %f",
             m_mousePosition.x(), m_mousePosition.y(), 
             m_zoom,
             m_offset.x(), m_offset.y(),
             formatTime(m_report ? m_report->getStartTime() : 0.f).c_str(),
                formatTime(m_report ? m_report->getEndTime() : 0.f).c_str(),
             pixelsPerSecond());

    painter.setPen(Qt::white);
    painter.drawText(posX + StatusMessageTextDist,
                     posY + StatusMessageTextDist,
                     StatusMessageWidth - 2 * StatusMessageTextDist,
                     statusMessageHeight - 2 * StatusMessageTextDist,
                     Qt::AlignTop | Qt::AlignLeft,
                     text);
}

void TimeLineView::layout()
{
    PERFOMETER_LOG_FUNCTION();
    
    const auto thisWidth = width();
    const auto thisHeight = height();
    int reportWidth = 0;

    if (m_report)
    {
        const auto pixpersec = pixelsPerSecond();
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
            
            reportEndPx = reportStartPx + extraWidth * (1 + VisibleMargin / 2);
            reportStartPx = reportStartPx - extraWidth * VisibleMargin / 2;

            m_horizontalScrollBar.setMinimum(reportStartPx);
            m_horizontalScrollBar.setMaximum(reportEndPx);
        }

        m_offset.setX(std::max<qreal>(m_offset.x(), reportStartPx));
        m_offset.setX(std::min<qreal>(m_offset.x(), reportEndPx));
    }

    int extraHeight = m_reportHeightPx + RecordInfoDist + 2 * RecordInfoHeight
                      - (thisHeight - RulerHeight - RulerDistReport);

    bool vertBarVisible = extraHeight > 0;
    bool horBarVisible = reportWidth > thisWidth;
    
    if (vertBarVisible)
    {
        m_offset.setY(std::max<qreal>(m_offset.y(), 0));
        m_offset.setY(std::min<qreal>(m_offset.y(), extraHeight));
        
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

TimeLineView::ComponentPtr TimeLineView::getComponentUnderPoint(QPoint point, QPoint* outPos)
{
    QPoint pos(-m_offset.x(), RulerHeight + RulerDistReport - m_offset.y());

    for (auto& component : m_components)
    {
        int componentHeight = component->height();

        if (pos.ry() + componentHeight < RulerHeight + RulerDistReport)
        {
            pos.ry() += componentHeight;
            continue;
        }

        QPoint localPosition = point - pos;
        if (localPosition.y() < 0 || localPosition.y() > componentHeight)
        {
            pos.ry() += componentHeight;
            continue;
        }

        if (outPos)
        {
            *outPos = pos;
        }

        return component;
    }

    return ComponentPtr();
}

void TimeLineView::zoom(int z)
{
    z = std::max(z, MinZoom);

    if (m_zoom != z)
    {
        m_zoom = z;
        layout();
    }
}

void TimeLineView::zoomBy(int zoomDelta)
{
    zoomBy(zoomDelta, width() / 2);
}

void TimeLineView::zoomBy(int zoomDelta, int pivot)
{
    int prevZoom = m_zoom;
    zoom(m_zoom + zoomDelta);

    if (m_zoom != prevZoom)
    {
        double mid = m_offset.x() + pivot;
        m_offset.setX((mid / prevZoom) * m_zoom - pivot);
    }
}

void TimeLineView::scrollBy(QPointF delta)
{
    scrollTo(m_offset + delta);
}

void TimeLineView::scrollTo(QPointF pos)
{
    m_offset = pos;

    m_horizontalScrollBar.setValue(m_offset.x());
    m_verticalScrollBar.setValue(m_offset.y());
}
void TimeLineView::scrollXBy(int xDelta)
{
    scrollBy(QPointF(xDelta, 0));
}

void TimeLineView::scrollYBy(int yDelta)
{
    scrollBy(QPointF(0, yDelta));
}

void TimeLineView::scrollXTo(int x)
{
    scrollTo(QPoint(x, m_offset.y()));
}

void TimeLineView::scrollYTo(int y)
{
    scrollTo(QPoint(m_offset.x(), y));
}

} // namespace visualizer
