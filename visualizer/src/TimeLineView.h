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
        void drawStatusMessage(QPainter& painter);
        void drawRuler(QPainter& painter);

        void layout();

    private:
        using super = QOpenGLWidget;

        static constexpr int DefaultZoom = 1000;
        static constexpr double VisibleMargin = 0.1; //10% of report time each size

        QScrollBar                          m_horizontalBar;
        QPoint                              m_mousePosition;
        int                                 m_zoom;

        double                              m_reportStartTime;
        double                              m_reportEndTime;
        double                              m_offset;

        std::shared_ptr<PerfometerReport>   m_report;
    };
} // namespace visualizer
