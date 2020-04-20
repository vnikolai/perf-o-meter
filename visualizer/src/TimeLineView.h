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
#include "PerfometerReport.h"
#include <memory>

namespace visualizer
{
    class TimeLineView : public QOpenGLWidget,
                                QOpenGLFunctions
    {
        struct RecordInfo
        {
            QRect bounds;
            std::string name;
            double startTime;
            double endTime;
        };

    public:
        TimeLineView();
        virtual ~TimeLineView();

        void setReport(std::shared_ptr<PerfometerReport> report);

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
        double pixelsPerSecond() const;
        
        int drawPerfometerRecord(QPainter& painter, QPoint& pos, const Record& record);
        int drawPerfometerRecords(QPainter& painter, QPoint& pos, const std::vector<Record>& records);
        void drawPerfometerThread(QPainter& painter, QPoint& pos, const Thread& thread);
        void drawPerfometerReport(QPainter& painter, QPoint& pos, const PerfometerReport& report);

        void getRulerStep(int& rulerStep, int& timeStep);
        void drawRuler(QPainter& painter, QPoint& pos);

        void drawRecordInfo(QPainter& painter, const RecordInfo& info);
        void drawStatusMessage(QPainter& painter);

        void layout();

        int getReportHeight(const PerfometerReport& report);
        int getThreadHeight(const Thread& report);
        int getRecordHeight(const Record& record);

        void zoom(int zoomDelta);
        void zoom(int zoomDelta, int pivot);
        void scrollBy(QPointF delta);
        void scrollTo(QPointF pos);
        void scrollXBy(int xDelta);
        void scrollYBy(int yDelta);
        void scrollXTo(int x);
        void scrollYTo(int y);

    private:
        using super = QOpenGLWidget;

        QScrollBar                          m_horizontalScrollBar;
        QScrollBar                          m_verticalScrollBar;

        QPoint                              m_mousePosition;
        bool                                m_mouseDragActive;

        int                                 m_zoom;
        int                                 m_reportHeightPx;

        bool                                m_statusTextVisible;

        QPointF                             m_offset;

        std::shared_ptr<RecordInfo>         m_highlightedRecordInfo;
        std::shared_ptr<RecordInfo>         m_selectedRecordInfo;

        std::shared_ptr<PerfometerReport>   m_report;
    };
} // namespace visualizer
