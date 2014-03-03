#include "GLViewer.h"

#include <iostream>
#include <QApplication>
#include <QDesktopWidget>

//============================================================================
GLViewer::GLViewer
(
	Parameters* parameters,
	QWidget *parent
) :
	QGLWidget( parent ),
	Viewer( parameters )
{
	/*// get initial size for GL viewer
	int screen_width   = QApplication::desktop()->width();
	int screen_height  = QApplication::desktop()->height();
	int initial_width  = _parameters->nx > screen_width  ? screen_width  : _parameters->nx;
	int initial_height = _parameters->ny > screen_height ? screen_height : _parameters->ny;

	// resize OpenGL viewer
	resize( initial_width, initial_height ); */

	doneCurrent();
}

//============================================================================
void GLViewer::renderFrame (
		double** U,
		double** V,
		double** P,
		int it
	)
{
	//QMutexLocker locker( &mutex );

	//makeCurrent();

	std::cout << "rendering " << it << std::endl;

	//updateGL();

	//doneCurrent();
}


void GLViewer::initializeGL ( )
{
	// set background color
	glClearColor( 0.7, 0.7, 0.7, 0.0 );

	// disable depth buffer
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	doneCurrent();
}

void GLViewer::resizeGL ( int width, int height )
{
	glViewport( 0, 0, (GLint)width, (GLint)height);
}

void GLViewer::paintGL ( )
{
	glClear( GL_COLOR_BUFFER_BIT );

	// do visualization here
}
