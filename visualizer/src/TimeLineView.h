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

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScrollBar>
#include "TimeLineConfig.h"
#include "PerfometerReport.h"
#include "TimeLineComponent.h"
#include <memory>

namespace visualizer
{
    class TimeLineView : public QOpenGLWidget,
                                QOpenGLFunctions
    {
        using ComponentPtr = std::shared_ptr<TimeLineComponent>;

    public:
        TimeLineView();
        virtual ~TimeLineView();

        void setReport(std::shared_ptr<PerfometerReport> report);

        double pixelsPerSecond() const;
        double secondsPerPixel() const;

        void zoom(zoom_t zoom);
        void zoom(zoom_t zoom, qreal pivot);
        void zoomBy(zoom_t zoomDelta);
        void zoomBy(zoom_t zoomDelta, qreal pivot);
        void scrollBy(QPointF delta);
        void scrollTo(QPointF pos);
        void scrollXBy(qreal xDelta);
        void scrollYBy(qreal yDelta);
        void scrollXTo(qreal x);
        void scrollYTo(qreal y);

    public slots:
        void onHorizontalScrollBarValueChanged(int value);
        void onVerticalScrollBarValueChanged(int value);

    protected:
        void initializeGL() override;
        void paintGL() override;

        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void keyReleaseEvent(QKeyEvent* event) override;

        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void wheelEvent(QWheelEvent* event) override;
        virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

        virtual void resizeEvent(QResizeEvent* event) override;

    private:
        void getRulerStep(double& rulerStep, coord_t& timeStep);
        void drawRuler(QPainter& painter, QPointF& pos);

        void drawStatusMessage(QPainter& painter);

        void layout();

        coord_t calculateReportHeight(std::shared_ptr<PerfometerReport> report);

        ComponentPtr getComponentUnderPoint(QPoint point, QPoint* outPos = nullptr);

        double timeAtPoint(coord_t x);

    private:
        using super = QOpenGLWidget;

        QScrollBar                          m_horizontalScrollBar;
        QScrollBar                          m_verticalScrollBar;

        QPoint                              m_mousePosition;
        bool                                m_mouseDragActive;

        zoom_t                              m_zoom;
        coord_t                             m_reportHeightPx;

        bool                                m_statusTextVisible;
        bool                                m_collapseAll;

        QPointF                             m_offset;

        std::shared_ptr<PerfometerReport>   m_report;

        std::vector<ComponentPtr>           m_components;

        ComponentPtr                        m_componentUnderMouse;
        ComponentPtr                        m_componentWithFocus;
    };
} // namespace visualizer
