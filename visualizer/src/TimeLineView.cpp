// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include "TimeLineView.h"
#include <cstring>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

namespace visualizer {

TimeLineView::TimeLineView()
    : QOpenGLWidget(nullptr)
    , m_horizontalBar(Qt::Horizontal, this)
    , m_zoom(DefaultZoom)
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
    const auto this_width = width();
    const auto this_height = height();

    glClear(GL_COLOR_BUFFER_BIT);

    char buff[64];
    snprintf(buff, 64, "%d %d %d", m_mousePosition.x(), m_mousePosition.y(), m_zoom);

    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Helvetica", 16));
    painter.drawText(0, 0, this_width, this_height, Qt::AlignTop | Qt::AlignLeft, buff);

    painter.setPen(Qt::darkRed);
    painter.drawLine(this_width / 2, 0, this_width / 2, this_height);

    painter.setPen(Qt::darkGreen);
    painter.drawLine(m_mousePosition.x(), 0, m_mousePosition.x(), this_height);

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

void TimeLineView::layout()
{
    const auto this_width = width();

    m_horizontalBar.resize(this_width, m_horizontalBar.height());
    m_horizontalBar.move(0, height() - m_horizontalBar.height());

    m_horizontalBar.setVisible(m_zoom > 1);
}

} // namespace visualizer
