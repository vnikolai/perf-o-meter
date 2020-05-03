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

#include "PerfometerReport.h"
#include <QPainter>

namespace visualizer
{
    class TimeLineView;

    class TimeLineComponent
    {
    public:
        TimeLineComponent(TimeLineView& view);
        int height() const;

        virtual void mouseMove(QPoint pos);
        virtual void mouseClick(QPoint pos);
        virtual void mouseDoubleClick(QPoint pos);
        virtual void focusLost();

        virtual void render(QPainter& painter, QRect pos);
        virtual void renderOverlay(QPainter& painter, QRect pos);

        const std::string& name() const;
        void setName(const std::string& name);

        bool collapsed() const;
        void collapse(bool flag);

    protected:

        void setHeight(int height);

    protected:
        TimeLineView&   m_view;

    private:
        std::string     m_name;
        bool            m_collapsed;
        int             m_height;
    };
} // namespace visualizer
