// Copyright 2019 Volodymyr Nikolaichuk <nikolaychuk.volodymyr@gmail.com>

#include <QApplication>
#include <QMainWindow>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <cstring>
#include <perfometer/perfometer.h>

class OpenGLRenderer : public QOpenGLWidget, QOpenGLFunctions
{
public:
	OpenGLRenderer() :
		QOpenGLWidget(nullptr)
	{
	}

protected:
	void initializeGL() override
	{
		QOpenGLFunctions::initializeOpenGLFunctions();

		glClearColor(0.117f, 0.117f, 0.117f, 1.0f);
	}

	void paintGL() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
};

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
	window.setCentralWidget(new OpenGLRenderer());
	window.show();

	return app.exec();
}
