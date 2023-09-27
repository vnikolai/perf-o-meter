/* Copyright 2020-2023 Volodymyr Nikolaichuk

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

#include "TimeLineComponent.h"
#include "TimeLineConfig.h"

namespace visualizer
{
TimeLineComponent::TimeLineComponent(TimeLineView& view)
    : m_view(view)
    , m_collapsed(false)
    , m_height(0)
    , m_highlightTitle(false)
{   
}

coord_t TimeLineComponent::height() const
{
    return !collapsed() ? m_height : ThreadTitleHeight;
}

void TimeLineComponent::mouseMove(QPointF pos)
{
    m_highlightTitle = pos.y() < ThreadTitleHeight;
}

void TimeLineComponent::mouseLeft()
{
    m_highlightTitle = false;
    m_statistics.hitTestTime = 0.0;
}

void TimeLineComponent::mouseClick(QPointF pos)
{
    if (pos.y() < ThreadTitleHeight)
    {
        collapse(!collapsed());
    }
}

void TimeLineComponent::mouseDoubleClick(QPointF pos)
{
}

void TimeLineComponent::focusLost()
{
}

void TimeLineComponent::onCopy(QClipboard* clipboard)
{
}

void TimeLineComponent::onPaste(QClipboard* clipboard)
{
}

void TimeLineComponent::onBeginFrame()
{
    m_statistics.numRecords = 0;
    m_statistics.frameRenderTime = 0.0;
}

void TimeLineComponent::render(QPainter& painter, const RenderContext& context, const Parameters& parameters)
{
    const auto viewportWidth = context.viewport.width();

    if (context.offset.y() < -ThreadTitleHeight)
    {
        return;
    }
    
    QRectF titleRect(context.viewport);
    titleRect.translate(0, context.offset.y());
    titleRect.setHeight(ThreadTitleHeight);

    if (collapsed())
    {
        painter.fillRect(titleRect, StatusMessageBackgroundColor);
    }

    if (m_highlightTitle)
    {
        painter.fillRect(titleRect, ComponentHighlightColor);
    }

    QString text;
    painter.setPen(Qt::white);

    painter.drawText(RulerDistReport + std::max<qreal>(context.viewport.left(), context.offset.x()),
                     context.viewport.top() + context.offset.y(),
                     context.viewport.width(), ThreadTitleHeight,
                     Qt::AlignVCenter | Qt::AlignLeft,
                     text.fromStdString(name()));
}

void TimeLineComponent::renderOverlay(QPainter& painter, const RenderContext& context, const Parameters& parameters)
{
}

const std::string& TimeLineComponent::name() const
{
    return m_name;
}

void TimeLineComponent::setName(const std::string& name)
{
    m_name = name;
}

bool TimeLineComponent::collapsed() const
{
    return m_collapsed;
}

void TimeLineComponent::collapse(bool flag)
{
    m_collapsed = flag;
}

void TimeLineComponent::setHeight(coord_t height)
{
    m_height = height;
}

const Statistics& TimeLineComponent::getStatistics() const
{
    return m_statistics;
}

} // namespace visualizer
