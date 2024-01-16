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

#include <QColor>

namespace visualizer {
    using zoom_t = uint64_t;
    using coord_t = int64_t;

    constexpr int       DefaultZoom             = 1000;
    constexpr double    ZoomWheelCoef           = 1.25;
    constexpr double    VisibleMargin           = 0.1;  // 10% of report time each size
    constexpr int       RulerHeight             = 24;
    constexpr int       RulerDistReport         = 12;
    constexpr int       ThreadTitleHeight       = 32;
    constexpr int       TitleOffsetSmall        = 2;
    constexpr int       TitleOffset             = 5;
    constexpr int       RecordHeight            = 16;
    constexpr int       ScrolBarThickness       = 24;
    constexpr zoom_t    MinZoom                 = 10;
    constexpr zoom_t    MaxZoom                 = 0x8000000000;
    constexpr int       ZoomKeyboardStep        = 2;
    constexpr int       ZoomKeyboardLargeStep   = 5;
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
    constexpr int       StatusMessageDist       = 25;

    const char*         FontFace                = "Helvetica";
    constexpr int       FontSize                = 10;

    static QColor       BackgroundColor                 (32, 32, 32, 255);
    static QColor       StatusMessageBackgroundColor    (16, 16, 16, 128);
    static QColor       ComponentHighlightColor         (48, 48, 48, 128);
    static QColor       RulerBackgroundColor            (228, 230, 241, 255);
    static QColor       SelectionColor                  (128, 230, 128, 32);

    constexpr int NumColors = 8;
    static QColor Colors[NumColors] = { QColor(160, 96, 96, 255), // soft pink
                                        QColor(96, 160, 96, 255), // light green
                                        Qt::darkCyan,
                                        QColor(160, 160, 96, 255), // soft yellow
                                        QColor(160, 96, 160, 255), // soft magenta
                                        Qt::gray,
                                        Qt::lightGray,
                                        Qt::darkGray };
} // namespace visualizer