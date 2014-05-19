#include "GLViewer.h"

#include <iostream>
#include <QApplication>
#include <QDesktopWidget>
#include <cmath>

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

	_isInitialized = 0;

	_texture = new GLubyte[ _parameters->nx * _parameters->ny * 3 ]; // 3: RGB

	// deactivate automatic updates of GL context by main thread
	setAutoBufferSwap( false );
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

	//rescaleColors ( P );
	rescaleColors ( U, V );

	unsigned int idx;
	GLubyte color;


	for( int y = 0; y < _parameters->ny; ++y )
	for( int x = 0; x < _parameters->nx; ++x )
	{
		idx = (y * _parameters->nx + x) * 3;

		if( !_parameters->obstacleMap[y+1][x+1] )
		{
			_texture[idx]     = 0;
			_texture[idx + 1] = 0;
			_texture[idx + 2] = 255;
		}
		else
		{

			color = (GLubyte)(
					  (
						  sqrt(
							  U[y+1][x+1] * U[y+1][x+1]	// +1 for boundaries
							+ V[y+1][x+1] * V[y+1][x+1]
						  )
						- _minValue )
					* _factor );
			// color = (GLubyte)( (P[y+1][x+1] - _minValue ) * _factor ); // pressure

			_texture[idx]     = color;
			_texture[idx + 1] = color;
			_texture[idx + 2] = color;
		}
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
	_minValue = 0.0;

	for ( int i = 0; i < size; ++i )
	{
		if ( (*P)[i] > max )
			max = (*P)[i];
		if ( (*P)[i] < _minValue )
			_minValue = (*P)[i];
	}

	if ( max - _minValue != 0.0 )
		_factor = 255.0 / ( max - _minValue );
}

//============================================================================
void GLViewer::rescaleColors ( REAL** U, REAL** V )
{
	// calculate factors to scale pressure values to 0-255
	int size = (_parameters->nx + 2) * (_parameters->ny + 2);

	REAL max = -INFINITY;
	_minValue = 0.0;

	REAL value;

	for ( int i = 0; i < size; ++i )
	{
		value = (*U)[i] * (*U)[i] + (*V)[i] * (*V)[i];
		if ( value > max )
			max = value;
		if ( value < _minValue )
			_minValue = value;
	}

	max = sqrt( max );
	_minValue = sqrt( _minValue );

	if ( max - _minValue != 0.0 )
		_factor = 255.0 / ( max - _minValue );
}

//============================================================================
QSize GLViewer::sizeHint ( ) const
{
	// get native size for GL viewer
	//int screen_width   = QApplication::desktop()->width();
	//int screen_height  = QApplication::desktop()->height();
	//int width  = _parameters->nx > screen_width  ? screen_width  : _parameters->nx;
	//int height = _parameters->ny > screen_height ? screen_height : _parameters->ny;

	return QSize( _parameters->nx, _parameters->ny );
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
void GLViewer::paintEvent ( QPaintEvent* )
{
	// prevent main thread from updating the GL context.
	// rendering is done in the method renderFrame,
	// called from the simulation thread

	if( !_isInitialized )
	{
		// display black area at startup :)
		_isInitialized = true;
		makeCurrent();
		glClearColor( 0.0, 0.0, 0.0, 1.0 );
		swapBuffers();
		doneCurrent();
	}
}

//============================================================================
void GLViewer::mouseMoveEvent ( QMouseEvent *event )
{
	// rescale positions
	// invert y position: texture is rendered head first

	int x = (event->x() * _parameters->nx) / _width;
	int y =  _parameters->ny - ((event->y() * _parameters->ny) / _height) - 1;


	// guards
	if(    x < 0 || x > _parameters->nx - 1
		|| y < 0 || y > _parameters->ny - 1)
	{
		return;
	}

	// test for modifier keys
	if( event->buttons() == Qt::RightButton ) //event->modifiers() & Qt::ShiftModifier
	{
		// tear down walls if right mouse button is used
		//_parameters->obstacleMap[y+1][x+1] = true;
		emit drawObstacle( x, y, true );
	}
	else
	{
		// paint wall on normal click
		//_parameters->obstacleMap[y+1][x+1] = false;
		emit drawObstacle( x, y, false );
	}
}