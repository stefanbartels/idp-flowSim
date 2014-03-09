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

		GLuint _textureID;

		GLubyte* _texture;



		REAL _pressureMin;
		REAL _pressureFactor;

	public:
		GLViewer (
				Parameters* parameters,
				QWidget* parent = 0
			);

		~GLViewer ( );

		void initialze ( );

		void renderFrame (
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			);

		void rescaleColors ( REAL** P );

	protected:
		void resizeEvent ( QResizeEvent* event );
		void paintEvent  ( QPaintEvent* );

};

#endif // GLVIEWER_H
