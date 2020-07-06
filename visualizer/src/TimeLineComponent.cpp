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

int TimeLineComponent::height() const
{
    return !collapsed() ? m_height : ThreadTitleHeight;
}

void TimeLineComponent::mouseMove(QPoint pos)
{
    m_highlightTitle = pos.y() < ThreadTitleHeight;
}

void TimeLineComponent::mouseLeft()
{
    m_highlightTitle = false;
}

void TimeLineComponent::mouseClick(QPoint pos)
{
    if (pos.y() < ThreadTitleHeight)
    {
        collapse(!collapsed());
    }
}

void TimeLineComponent::mouseDoubleClick(QPoint pos)
{
}

void TimeLineComponent::focusLost()
{
}

void TimeLineComponent::render(QPainter& painter, QRect pos)
{
    const auto viewportWidth = pos.width();

    if (collapsed())
    {
        painter.fillRect(QRect(0, pos.y(), pos.width(), ThreadTitleHeight), StatusMessageBackgroundColor);
    }

    if (m_highlightTitle)
    {
        painter.fillRect(QRect(0, pos.y(), pos.width(), ThreadTitleHeight), ComponentHighlightColor);
    }

    QString text;
    painter.setPen(Qt::white);
    painter.drawText(RulerDistReport + std::max(0, pos.x()), pos.y(),
                     viewportWidth, ThreadTitleHeight,
                     Qt::AlignVCenter | Qt::AlignLeft,
                     text.fromStdString(name()));
}

void TimeLineComponent::renderOverlay(QPainter& painter, QRect pos)
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

void TimeLineComponent::setHeight(int height)
{
    m_height = height;
}

} // namespace visualizer
