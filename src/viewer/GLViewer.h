#ifndef GLVIEWER_H
#define GLVIEWER_H

#include "Viewer.h"
#include <QtOpenGL/QGLWidget>

class GLViewer : public QGLWidget, public Viewer
{
	Q_OBJECT

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
};

#endif // GLVIEWER_H
