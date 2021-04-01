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

#include "TimeLineView.h"
#include "TimeLineThread.h"
#include <utils/time.h>
#include <cstring>
#include <algorithm>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>

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
    , m_statusDisplayMode(StatusDisplayMode::None)
    , m_collapseAll(true)
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
    PERFOMETER_LOG_WORK_FUNCTION();

    QRectF viewport = getViewport();

    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter(this);
    painter.setFont(QFont("Helvetica", 10));

    QPointF pos(-m_offset.x(), RulerHeight + RulerDistReport - m_offset.y());

    m_statistics = Statistics();

    for (auto& component : m_components)
    {
        component->onBeginFrame();
    }

    for (auto& component : m_components)
    {
        coord_t componentHeight = component->height();

        if (pos.ry() + componentHeight <= 0)
        {
            pos.ry() += componentHeight;
            continue;
        }

        component->render(painter, viewport, pos);

        pos.ry() += componentHeight;
        if (pos.ry() >= viewport.height())
        {
            break;
        }
    }

    for (auto& component : m_components)
    {
        component->renderOverlay(painter, viewport, pos);
    }

    for (auto& component : m_components)
    {
        m_statistics += component->getStatistics();
    }

    if (m_statusDisplayMode != StatusDisplayMode::None)
    {
        drawStatusMessage(painter);
    }

    drawRuler(painter, pos);

    painter.setPen(Qt::darkGreen);
    painter.drawLine(m_mousePosition.x(), viewport.top(), m_mousePosition.x(), viewport.bottom());

    QString text;
    painter.setPen(RulerBackgroundColor);
    painter.drawText(m_mousePosition.x() + TitleOffsetSmall, RulerHeight, 200, 50,
                     Qt::AlignTop | Qt::AlignLeft,
                     text.fromStdString(format_time(timeAtPoint(m_mousePosition.x() - viewport.left()))));

    painter.end();
}

QRectF TimeLineView::getViewport() const
{
    auto thisWidth = width();
    auto thisHeight = height();

    if (m_verticalScrollBar.isVisible())
    {
        thisWidth -= m_verticalScrollBar.width();
    }

    if (m_horizontalScrollBar.isVisible())
    {
        thisHeight -= m_horizontalScrollBar.height();
    }

    return QRectF(0, 0, thisWidth, thisHeight);
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
                scrollYTo(std::numeric_limits<coord_t>::max());
            }
            else
            {
                scrollXTo(std::numeric_limits<coord_t>::max());
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
            switch (m_statusDisplayMode)
            {
                case StatusDisplayMode::None:
                    m_statusDisplayMode = StatusDisplayMode::ReportInfo;
                    break;
                case StatusDisplayMode::ReportInfo:
                    m_statusDisplayMode = StatusDisplayMode::Stats;
                    break;
                case StatusDisplayMode::Stats:
                    m_statusDisplayMode = StatusDisplayMode::None;
                    break;
            }

            break;
        }
        case Qt::Key_Plus:
        {
            zoom(m_zoom * (ctrl ? ZoomKeyboardLargeStep : ZoomKeyboardStep),
                 getViewport().width() / 2);
            break;
        }
        case Qt::Key_Minus:
        {
            zoom(m_zoom / (ctrl ? ZoomKeyboardLargeStep : ZoomKeyboardStep),
                 getViewport().width() / 2);
            break;
        }
        case Qt::Key_Asterisk:
        {
            m_zoom = DefaultZoom;
            break;
        }
        case Qt::Key_C:
        {
            if (ctrl)
            {
                for (const auto& component : m_components)
                {
                    component->onCopy(QApplication::clipboard());
                }
            }
            
            break;
        }
        case Qt::Key_V:
        {
            if (ctrl)
            {
                for (const auto& component : m_components)
                {
                    component->onPaste(QApplication::clipboard());
                }
            }
            
            break;
        }
        case Qt::Key_T:
        {
            for (const auto& component : m_components)
            {
                component->collapse(m_collapseAll);
            }

            m_collapseAll = !m_collapseAll;
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

    QPointF hitTestPosition = m_mousePosition - getViewport().topLeft();

    ComponentPtr component = getComponentUnderPoint(hitTestPosition);
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

    QPointF pos;
    QPointF hitTestPosition = m_mousePosition - getViewport().topLeft();

    ComponentPtr component = getComponentUnderPoint(hitTestPosition, &pos);
    if (component)
    {
        QPointF localPosition = hitTestPosition - pos;
        component->mouseClick(localPosition);
    }

    m_componentWithFocus = component;

    update();
    layout();
}

void TimeLineView::mouseMoveEvent(QMouseEvent* event)
{
    super::mouseMoveEvent(event);

    if (m_mouseDragActive)
    {
        QPointF delta = event->pos() - m_mousePosition;
        scrollBy(-delta);
    }

    m_mousePosition = event->pos();

    QPointF pos;
    QPointF hitTestPosition = m_mousePosition - getViewport().topLeft();

    ComponentPtr component = getComponentUnderPoint(hitTestPosition, &pos);

    if (m_componentUnderMouse != component && m_componentUnderMouse)
    {
        m_componentUnderMouse->mouseLeft();
    }

    if (component)
    {
        QPointF localPosition = hitTestPosition - pos;
        component->mouseMove(localPosition);
    }

    m_componentUnderMouse = component;

    update();
    layout();
}

void TimeLineView::wheelEvent(QWheelEvent* event)
{
    coord_t delta = event->angleDelta().y();

    if (event->modifiers() & Qt::ControlModifier)
    {
        zoom(delta > 0 ? m_zoom * ZoomWheelCoef : m_zoom / ZoomWheelCoef,
             m_mousePosition.x() - getViewport().left());
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
    QPointF pos;
    QPointF hitTestPosition = m_mousePosition - getViewport().topLeft();

    ComponentPtr component = getComponentUnderPoint(hitTestPosition, &pos);
    if (component)
    {
        QPointF localPosition = hitTestPosition - pos;
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

double TimeLineView::secondsPerPixel() const
{
    return static_cast<double>(DefaultZoom) / (PixelsPerSecond * m_zoom);
}

void TimeLineView::getRulerStep(double& rulerStep, coord_t& timeStep)
{
    constexpr int NumSteps = 31;
    constexpr zoom_t RulerTimeSteps[NumSteps]
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
        100000000,
        200000000,
        500000000,
        1000000000,
        2000000000,
        5000000000,
        10000000000,
        20000000000,
        50000000000,
        100000000000,
    };

    static_assert(
        sizeof(RulerTimeSteps) == NumSteps * sizeof(zoom_t),
        "RulerTimeSteps elements number check failed");

    const auto pixpersec = pixelsPerSecond();
    int idx = 0;

    do
    {
        timeStep = RulerTimeSteps[idx++];
        rulerStep = (timeStep * pixpersec) / 1000000000;

    } while (rulerStep < 32 && idx < NumSteps);
}

void TimeLineView::drawRuler(QPainter& painter, QPointF& pos)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    QRectF viewport = getViewport();

    constexpr coord_t PrimaryStrokeLength = 16;
    constexpr coord_t SecondaryStrokeLength = 12;

    painter.setPen(Qt::black);
    painter.setBrush(RulerBackgroundColor);
    painter.drawRect(viewport.left(), 0, viewport.width() - 1, RulerHeight);
    painter.drawLine(viewport.top(), RulerHeight, viewport.width(), RulerHeight);

    double rulerStep = 0;
    coord_t timeStep = 0;
    getRulerStep(rulerStep, timeStep);

    QString text;

    const auto pixpersec = pixelsPerSecond();
    const long int rulerCount = viewport.width() / rulerStep;

    coord_t rulerOffset = static_cast<coord_t>(pos.x() / (rulerStep * 2));

    for (long int i = 0; i < rulerCount + 2; ++i)
    {
        coord_t x =
            i * rulerStep +
            (pos.x() > 0 ? pos.x() : pos.x() - rulerOffset * 2 * rulerStep) +
            viewport.left();

        if (i % 2)
        {
            painter.drawLine(x, 0, x, SecondaryStrokeLength);
        }
        else
        {
            painter.drawLine(x, 0, x, PrimaryStrokeLength);

            coord_t idx = i + 2 * (pos.x() < 0 ? -rulerOffset : 0);
            double rulerTime = idx * static_cast<double>(timeStep) / 1000000000;
            painter.drawText(x + TitleOffsetSmall, 0, 64, RulerHeight,
                             Qt::AlignVCenter | Qt::AlignLeft,
                             text.fromStdString(format_time(rulerTime)));
        }
    }

    if (pos.x() > 0)
    {
        painter.setPen(Qt::darkRed);
        coord_t zeroPointX = pos.x() + viewport.left();
        painter.drawLine(zeroPointX, viewport.top(), zeroPointX, viewport.bottom());
    }
}

void TimeLineView::drawStatusMessage(QPainter& painter)
{
    PERFOMETER_LOG_WORK_FUNCTION();

    constexpr size_t bufferSize = 256;
    char text[bufferSize];

    switch (m_statusDisplayMode)
    {
        case StatusDisplayMode::ReportInfo:
            snprintf(text, bufferSize,
                     "mouse: %d %d\n"
                     "zoom: %lu\n"
                     "offset: %f %f\n"
                     "report time: [%s] - [%s]\n"
                     "pixel per second: %f\n",
                     m_mousePosition.x(), m_mousePosition.y(), 
                     m_zoom,
                     m_offset.x(), m_offset.y(),
                     format_time(m_report ? m_report->getStartTime() : 0.f).c_str(),
                     format_time(m_report ? m_report->getEndTime() : 0.f).c_str(),
                     pixelsPerSecond());
        break;

        case StatusDisplayMode::Stats:
        {
            snprintf(text, bufferSize,
                     "Frame render time: %f\n"
                     "Hit test time: %f\n"
                     "visible records: %zu",
                     m_statistics.frameRenderTime,
                     m_statistics.hitTestTime,
                     m_statistics.numRecords);

            break;
        }
    }

    QRectF viewport = getViewport();

    const coord_t statusMessageHeight = viewport.height() - 2 * StatusMessageDist;

    const coord_t posX = viewport.right() - StatusMessageWidth - StatusMessageDist;
    const coord_t posY = viewport.top() + StatusMessageDist;

    painter.fillRect(posX, posY, StatusMessageWidth, statusMessageHeight, StatusMessageBackgroundColor);

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
    PERFOMETER_LOG_WORK_FUNCTION();

    const auto thisWidth = width();
    const auto thisHeight = height();
    coord_t reportWidth = 0;

    if (m_report)
    {
        m_reportHeightPx = calculateReportHeight(m_report);

        const auto pixpersec = pixelsPerSecond();
        coord_t reportStartPx = m_report->getStartTime() * pixpersec;
        coord_t reportEndPx = m_report->getEndTime() * pixpersec;
        reportWidth = (reportEndPx - reportStartPx);

        if (reportWidth <= thisWidth)
        {
            m_offset.setX(0);
        }
        else
        {
            coord_t extraWidth = reportWidth - thisWidth;

            reportEndPx = reportStartPx + extraWidth * (1 + VisibleMargin / 2);
            reportStartPx = reportStartPx - extraWidth * VisibleMargin / 2;

            m_horizontalScrollBar.setMinimum(
                std::clamp<coord_t>(reportStartPx,
                                     std::numeric_limits<int>::min(),
                                     std::numeric_limits<int>::max()) );

            m_horizontalScrollBar.setMaximum(
                std::clamp<coord_t>(reportEndPx,
                                     std::numeric_limits<int>::min(),
                                     std::numeric_limits<int>::max()) );
        }

        m_offset.setX(std::max<coord_t>(m_offset.x(), reportStartPx));
        m_offset.setX(std::min<coord_t>(m_offset.x(), reportEndPx));
    }

    coord_t extraHeight = m_reportHeightPx + RecordInfoDist + 2 * RecordInfoHeight
                          - (thisHeight - RulerHeight - RulerDistReport);

    bool vertBarVisible = extraHeight > 0;
    bool horBarVisible = reportWidth > thisWidth;

    if (vertBarVisible)
    {
        m_offset.setY(std::max<qreal>(m_offset.y(), 0));
        m_offset.setY(std::min<qreal>(m_offset.y(), extraHeight));

        m_verticalScrollBar.setMinimum(0);
        m_verticalScrollBar.setMaximum(
            std::clamp<coord_t>(extraHeight,
                                std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max()));
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

coord_t TimeLineView::calculateReportHeight(std::shared_ptr<PerfometerReport> report)
{
    coord_t height = 0;

    for (const auto& component : m_components)
    {
        height += component->height();
    }

    return height;
}

TimeLineView::ComponentPtr TimeLineView::getComponentUnderPoint(QPointF point, QPointF* outPos)
{
    QPointF pos(-m_offset.x(), RulerHeight + RulerDistReport - m_offset.y());

    for (auto& component : m_components)
    {
        coord_t componentHeight = component->height();

        if (pos.ry() + componentHeight <= 0)
        {
            pos.ry() += componentHeight;
            continue;
        }

        QPointF localPosition = point - pos;
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

void TimeLineView::zoom(zoom_t z)
{
    z = std::clamp<zoom_t>(z, MinZoom, MaxZoom);

    if (m_zoom != z)
    {
        m_zoom = z;
        layout();
    }
}

void TimeLineView::zoom(zoom_t z, qreal pivot)
{
    zoom_t prevZoom = m_zoom;
    double mid = m_offset.x() + pivot;

    zoom(z);

    if (m_zoom != prevZoom)
    {
        m_offset.setX((mid / prevZoom) * m_zoom - pivot);
    }
}

void TimeLineView::zoomBy(zoom_t zoomDelta)
{
    zoomBy(zoomDelta, getViewport().width() / 2);
}

void TimeLineView::zoomBy(zoom_t zoomDelta, qreal pivot)
{
    zoom_t prevZoom = m_zoom;
    double mid = m_offset.x() + pivot;

    zoom(m_zoom + zoomDelta);

    if (m_zoom != prevZoom)
    {
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
void TimeLineView::scrollXBy(qreal xDelta)
{
    scrollBy(QPointF(xDelta, 0));
}

void TimeLineView::scrollYBy(qreal yDelta)
{
    scrollBy(QPointF(0, yDelta));
}

void TimeLineView::scrollXTo(qreal x)
{
    scrollTo(QPointF(x, m_offset.y()));
}

void TimeLineView::scrollYTo(qreal y)
{
    scrollTo(QPointF(m_offset.x(), y));
}

double TimeLineView::timeAtPoint(coord_t x)
{
    coord_t rulerX = x + m_offset.x();
    return secondsPerPixel() * rulerX;
}

} // namespace visualizer
