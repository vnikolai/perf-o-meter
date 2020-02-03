// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include "TimeLineView.h"
#include "PerfometerReport.h"
#include <cstring>
#include <memory>
#include <perfometer/perfometer.h>


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

		if (report->loadFile(reportFileName))
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
