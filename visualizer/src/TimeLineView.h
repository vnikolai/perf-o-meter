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

        void setReport(std::shared_ptr<PerfometerReport> report);

    public slots:
        void onHorizontalSliderChanged(int value);
        void onVerticalSliderChanged(int value);

    protected:
        void initializeGL() override;
        void paintGL() override;

        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void wheelEvent(QWheelEvent* event) override;
        virtual void resizeEvent(QResizeEvent* event) override;
    
    private:
        float pixelsPerSecond() const;
        void drawStatusMessage(QPainter& painter);
        int drawPerfometerRecord(QPainter& painter, QPoint& pos, const Record& record);
        int drawPerfometerRecords(QPainter& painter, QPoint& pos, const std::vector<Record>& records);
        void drawPerfometerReport(QPainter& painter, QPoint& pos, const PerfometerReport& report);
        void drawRuler(QPainter& painter, QPoint& pos);

        void layout();

        std::string formatTime(double time);

        int getReportHeight(const PerfometerReport& report);
        int getThreadHeight(const Thread& report);
        int getRecordHeight(const Record& record);

    private:
        using super = QOpenGLWidget;

        QScrollBar                          m_horizontalScrollBar;
        QScrollBar                          m_verticalScrollBar;
        QPoint                              m_mousePosition;
        int                                 m_zoom;
        int                                 m_reportHeightPx;

        QPoint                              m_offset;

        std::shared_ptr<PerfometerReport>   m_report;
    };
} // namespace visualizer
