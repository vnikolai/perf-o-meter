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

#include <QColor>

namespace visualizer {

    constexpr int       DefaultZoom             = 1000;
    constexpr double    VisibleMargin           = 0.1;  // 10% of report time each size
    constexpr int       RulerHeight             = 24;
    constexpr int       RulerDistReport         = 12;
    constexpr int       ThreadTitleHeight       = 32;
    constexpr int       TitleOffsetSmall        = 2;
    constexpr int       RecordHeight            = 16;
    constexpr int       ScrolBarThickness       = 24;
    constexpr int       MinZoom                 = 10;
    constexpr int       ZoomKeyboardStep        = 250;
    constexpr int       ZoomKeyboardLargeStep   = 5000;
    constexpr int       OffsetKeyboardStep      = 10;
    constexpr int       OffsetKeyboardPageStep  = 240;
    constexpr int       RecordMinTextWidth      = 10;
    constexpr int       PixelsPerSecond         = 128;

    constexpr int       RecordInfoHeight        = 32;
    constexpr int       RecordInfoDist          = 32;
    constexpr int       RecordInfoTextDist      = 12;
    constexpr int       RecordInfoTimeWidth     = 128;

    constexpr int       StatusMessageWidth      = 256;
    constexpr int       StatusMessageTextDist   = 12;
    constexpr int       StatusMessageDist       = 50;

    static QColor       BackgroundColor                 (32, 32, 32, 255);
    static QColor       StatusMessageBackgroundColor    (16, 16, 16, 128);
    static QColor       RulerBackgroundColor            (228, 230, 241, 255);

    constexpr int NumColors = 8;
    static QColor Colors[NumColors] = { Qt::darkRed,
                                        Qt::darkGreen,
                                        Qt::darkCyan,
                                        Qt::darkYellow,
                                        Qt::darkMagenta,
                                        Qt::gray,
                                        Qt::lightGray,
                                        Qt::darkGray };
} // namespace visualizer