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
	_doResize = false;
	_width = _height = 0;

	// deactivate automatic updates of GL context by main thread
	setAutoBufferSwap( false );

	/*// get initial size for GL viewer
	int screen_width   = QApplication::desktop()->width();
	int screen_height  = QApplication::desktop()->height();
	int initial_width  = _parameters->nx > screen_width  ? screen_width  : _parameters->nx;
	int initial_height = _parameters->ny > screen_height ? screen_height : _parameters->ny;

	// resize OpenGL viewer
	resize( initial_width, initial_height ); */
}

//============================================================================
void GLViewer::initialze ( )
{
	makeCurrent();

	// set background color
	glClearColor( 0.0, 0.0, 0.0, 1.0 );

	// disable depth buffer
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	glClear( GL_COLOR_BUFFER_BIT );

	swapBuffers();
}

//============================================================================
void GLViewer::renderFrame (
        REAL** U,
        REAL** V,
        REAL** P,
		int it
	)
{
	if( _doResize )
	{
		glViewport( 0, 0, _width, _height );
		_doResize = false;
	}

	// do visualization here


	swapBuffers();
}

//============================================================================
void GLViewer::resizeEvent ( QResizeEvent *event )
{
	// prevent Qt from calling resizeGL, so no makeCurrent() is called

	_width  = event->size().width();
	_height = event->size().height();
	_doResize = true;
}

//============================================================================
void GLViewer::paintEvent ( QPaintEvent* event )
{
	// prevent main thread from updating the GL context.
	// rendering is done in the method renderFrame,
	// called from the simulation thread
}

