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

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include "TimeLineView.h"
#include "PerfometerReport.h"
#include <cstring>
#include <memory>
#include <fstream>
#include <perfometer/perfometer.h>
#include <iostream>

struct options
{
	std::string reportFileName;
	bool		profile = false;
};

void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	std::string message;

  	switch (type)
	{
		case QtDebugMsg: break;
		case QtInfoMsg:  break;
		case QtWarningMsg: message = "WARNING: "; break;
		case QtCriticalMsg: message = "CRITICAL: "; break;
		case QtFatalMsg: 	message = "FATAL: "; break;
	}

	message += msg.toStdString();

	std::cout << message << std::endl;

  	static std::ofstream logFile("perfometer.log", std::ios::trunc);
	logFile << message << std::endl << std::flush;
}

void parseCommandline(options& opts, int argc, char* argv[])
{
	if (argc > 1)
	{
		opts.reportFileName = argv[1];

		if (argc > 2)
		{
			for (int i = 2; i < argc; ++i)
			{
				if (strcmp(argv[i], "--profile") == 0)
				{
					opts.profile = true;
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	qInstallMessageHandler(MessageHandler);

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

	options opts;
	parseCommandline(opts, argc, argv);

	if (opts.profile)
	{
		perfometer::initialize("visualizer.report");
	}

	if (opts.reportFileName.length())
	{
		auto report = std::make_shared<visualizer::PerfometerReport>();

		if (report->loadFile(opts.reportFileName))
		{
			timeLineView->setReport(report);

			char titleWithFile[1024];
			std::snprintf(titleWithFile, 1024, "%s - %s", title, opts.reportFileName.c_str());

			window.setWindowTitle(titleWithFile);
		}
		else
		{
			QString text;
			text = text.fromStdString(opts.reportFileName);
			text.prepend("Cannot open report file ");

			QMessageBox messageBox;
			messageBox.setText(text);
			messageBox.exec();
		}
	}

	int retval = app.exec();

	if (opts.profile)
	{
		perfometer::shutdown();
	}

	return retval;
}
