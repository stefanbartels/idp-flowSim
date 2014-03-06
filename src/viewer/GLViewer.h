#ifndef GLVIEWER_H
#define GLVIEWER_H

#include "Viewer.h"
#include <QtOpenGL/QGLWidget>
#include <QWidget>
#include <QResizeEvent>

class GLViewer : public QGLWidget, public Viewer
{
	Q_OBJECT

	protected:

		bool _doResize;

		int _width;
		int _height;

	public:
		GLViewer (
				Parameters* parameters,
				QWidget* parent = 0
			);

		void initialze ( );

		void renderFrame (
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			);

	protected:
		void resizeEvent ( QResizeEvent* event );
		void paintEvent  ( QPaintEvent*  event );

};

#endif // GLVIEWER_H
