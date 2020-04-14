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

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include "TimeLineView.h"
#include "PerfometerReport.h"
#include <cstring>
#include <memory>
#include <fstream>
#include <perfometer/perfometer.h>

static void log(const std::string& text)
{
	static std::ofstream logFile("perfometer.log", std::ios::trunc);
	
	logFile << text << std::endl;
	logFile.flush();
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	char title[64];
	std::snprintf(title, 64,
				  "Perf-o-meter Visualizer %d.%d.%d",
				  int(perfometer::major_version),
				  int(perfometer::minor_version),
				  int(perfometer::patch_version));

	QApplication::setApplicationName(title);

	visualizer::TimeLineView* timeLineView = new visualizer::TimeLineView();

	QMainWindow window;
	window.resize(1280, 720);
	window.setCentralWidget(timeLineView);
	window.show();

	if (argc > 1)
	{
		std::shared_ptr<visualizer::PerfometerReport> report = std::make_shared<visualizer::PerfometerReport>();

		const std::string reportFileName(argv[1]);

		if (report->loadFile(reportFileName, log))
		{
			timeLineView->setReport(report);

			char titleWithFile[1024];
			std::snprintf(titleWithFile, 1024, "%s - %s", title, reportFileName.c_str());

			window.setWindowTitle(titleWithFile);
		}
		else
		{
			QString text;
			text = text.fromStdString(reportFileName);
			text.prepend("Cannot open report file ");

			QMessageBox messageBox;
			messageBox.setText(text);
			messageBox.exec();
		}
	}

	return app.exec();
}
