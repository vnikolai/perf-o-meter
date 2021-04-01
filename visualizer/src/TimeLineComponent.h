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

#pragma once

#include "PerfometerReport.h"
#include "TimeLineConfig.h"
#include <QPainter>
#include <QClipboard>

namespace visualizer
{
    struct Statistics
    {
        size_t numRecords = 0;
        double frameRenderTime = 0.0;
        double hitTestTime = 0.0;

        Statistics& operator+=(const Statistics& other)
        {
            numRecords += other.numRecords;
            frameRenderTime += other.frameRenderTime;
            hitTestTime += other.hitTestTime;
            return *this;
        }
    };

    class TimeLineView;

    class TimeLineComponent
    {
    public:
        TimeLineComponent(TimeLineView& view);
        coord_t height() const;

        virtual void mouseMove(QPointF pos);
        virtual void mouseLeft();
        virtual void mouseClick(QPointF pos);
        virtual void mouseDoubleClick(QPointF pos);
        virtual void focusLost();

        virtual void onCopy(QClipboard* clipboard);
        virtual void onPaste(QClipboard* clipboard);

        virtual void onBeginFrame();
        virtual void render(QPainter& painter, QRectF viewport, QPointF offset);
        virtual void renderOverlay(QPainter& painter, QRectF viewport, QPointF offset);

        const std::string& name() const;
        void setName(const std::string& name);

        bool collapsed() const;
        void collapse(bool flag);

        const Statistics& getStatistics() const;

    protected:

        void setHeight(coord_t height);

    protected:
        TimeLineView&       m_view;
        Statistics           m_statistics;

    private:
        std::string         m_name;
        bool                m_collapsed;
        coord_t             m_height;
        bool                m_highlightTitle;
    };
} // namespace visualizer
