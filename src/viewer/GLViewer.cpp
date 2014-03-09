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

	_texture = new GLubyte[ _parameters->nx * _parameters->ny * 3 ]; // 3: RGB

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
GLViewer::~GLViewer()
{
	if( _texture != 0 )
	{
		delete[] _texture;
	}
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
	glShadeModel(GL_FLAT);

	// texture initialization
	glGenTextures( 1, &_textureID );
	glBindTexture( GL_TEXTURE_2D, _textureID );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D(
				GL_TEXTURE_2D,
				0,					// base level (no mipmapping)
				GL_RGB,
				_parameters->nx,	// texture width
				_parameters->ny,	// texture height
				0,					// border width
				GL_RGB,
				GL_UNSIGNED_BYTE,
				_texture
			);

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

	rescaleColors ( P );

	unsigned int idx = 0;

	for( unsigned int y = 0; y < _parameters->ny; ++y )
	for( unsigned int x = 0; x < _parameters->nx; ++x )
	{
		// +1 for boundaries
		GLubyte color = (GLubyte)( (P[y+1][x+1] - _pressureMin ) * _pressureFactor );

		idx = (y * _parameters->nx + x) * 3;

		_texture[idx]     = color;
		_texture[idx + 1] = color;
		_texture[idx + 2] = color;
	}


	// copy texture to gpu memory
	// TODO: is there a faster way?
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, _parameters->nx, _parameters->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, _texture );

	glClear( GL_COLOR_BUFFER_BIT );

	// render texture to quad
	glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, _textureID );

		glBegin( GL_QUADS );
			glTexCoord2f( 0.0, 0.0 );
			glVertex3f( -1.0, -1.0, 0.0 );

			glTexCoord2f( 0.0, 1.0 );
			glVertex3f( -1.0,  1.0, 0.0 );

			glTexCoord2f( 1.0, 1.0 );
			glVertex3f( 1.0,  1.0, 0.0 );

			glTexCoord2f( 1.0, 0.0 );
			glVertex3f( 1.0, -1.0, 0.0 );
		glEnd();
	glDisable( GL_TEXTURE_2D );

	swapBuffers();
}

//============================================================================
void GLViewer::rescaleColors ( REAL** P )
{
	// calculate factors to scale pressure values to 0-255
	int size = (_parameters->nx + 2) * (_parameters->ny + 2);
	REAL max = 0.0;
	_pressureMin = 0.0;

	for ( int i = 0; i < size; ++i )
	{
		if ( (*P)[i] > max )
			max = (*P)[i];
		if ( (*P)[i] < _pressureMin )
			_pressureMin = (*P)[i];
	}

	if ( max - _pressureMin != 0.0 )
		_pressureFactor = 255.0 / ( max - _pressureMin );
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

