#ifndef GLVIEWER_H
#define GLVIEWER_H

#include "Viewer.h"
#include <QtOpenGL/QGLWidget>
#include <QMutex>

class GLViewer : public QGLWidget, public Viewer
{
	Q_OBJECT

	protected:

		QMutex mutex;

	public:
		GLViewer (
				Parameters* parameters,
				QWidget* parent = 0
			);

		void renderFrame (
				double** U,
				double** V,
				double** P,
				int it
			);

	protected:
		void initializeGL ( );
		void resizeGL ( int width, int height );
		void paintGL ( );
};

#endif // GLVIEWER_H
