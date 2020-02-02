// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <QApplication>
#include <QMainWindow>
#include "TimeLineView.h"
#include <cstring>
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

	QMainWindow window;
	window.resize(1280, 720);
	window.setCentralWidget(new visualizer::TimeLineView());
	window.show();

	return app.exec();
}
