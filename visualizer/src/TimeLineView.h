// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>
#pragma once

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScrollBar>
#include "PerfometerReport.h"
#include <memory>

namespace visualizer
{
    class TimeLineView : public QOpenGLWidget,
                                QOpenGLFunctions
    {
        static constexpr int DefaultZoom = 1000;
    public:
        TimeLineView();
        virtual ~TimeLineView();

        void setReport(std::shared_ptr<PerfometerReport> report) { m_report = report; };

    protected:
        void initializeGL() override;
        void paintGL() override;

        virtual void mouseMoveEvent(QMouseEvent *event) override;
        virtual void wheelEvent(QWheelEvent *event) override;
        virtual void resizeEvent(QResizeEvent *event) override;

    private:
        void layout();

    private:
        using super = QOpenGLWidget;

        QScrollBar                          m_horizontalBar;
        QPoint                              m_mousePosition;
        int                                 m_zoom;

        std::shared_ptr<PerfometerReport>   m_report;
    };
} // namespace visualizer
