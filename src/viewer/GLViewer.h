#ifndef GLVIEWER_H
#define GLVIEWER_H

//********************************************************************
//**    includes
//********************************************************************

#include "Viewer.h"
#include <QtOpenGL/QGLWidget>
#include <QWidget>
#include <QResizeEvent>

#include <QMutex>

//====================================================================
/*! \class GLViewer
	\brief Class for visualizing the simulation results in a
	OpenGL Widget
*/
//====================================================================

class GLViewer : public QGLWidget, public Viewer
{
	Q_OBJECT

	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		bool     _doResize;			//! flag indicating if the window size has changed since the last frame
		int      _width;			//! width of the GL window, allowing resizing the window inside the thread
		int      _height;			//! height of the GL window

		GLubyte* _texture;			//! OpenGL texture that is created out of the results
		GLuint   _textureID;		//! id of the OpenGL texture

		REAL     _minValue;			//! minimum value in the data to visualize. Required for rescaling the colors
		REAL     _factor;			//! rescaling factor

		bool     _isInitialized;	//! flag for rendering initial screen color

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct
			//! \param pointer to parent widget

		GLViewer (
				Parameters* parameters,
				QWidget*    parent = 0
			);

		~GLViewer ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief initializes OpenGLm and clears the screen

		void initialze ( );

			//! @}

		// -------------------------------------------------
		//	visualization
		// -------------------------------------------------
			//! @name visualization
			//! @{

			//! \brief visualizes the result of a timestep
			//! Renders the result in a OpenGL window
			//! \param pointer to horizontal velocity components (host memory)
			//! \param pointer to vertical velocity components (host memory)
			//! \param pointer to pressure components (host memory)
			//! \param number of the current iteration

		void renderFrame
			(
                REAL** U,
                REAL** V,
                REAL** P,
				double time,
				unsigned int iteration
			);

			//! @}

		// -------------------------------------------------
		//	inherited Qt methods
		// -------------------------------------------------
			//! @name inherited Qt methods
			//! @{

			//! \brief determines the optimum window size for this widget
			//! \returns optimum size of the widget. This is the domain size of the simulation

		QSize sizeHint ( ) const;

			//! @}


	signals:
		// -------------------------------------------------
		//	signals
		// -------------------------------------------------
			//! @name signals
			//! @{

			//! \brief emitted if drawing of obstacles is requested
			//! \param x offset of the obstacle to draw
			//! \param y offset of the obstacle to draw
			//! \param drawing mode, true if a wall ist to be teared down instead of created

		void drawObstacle
			(
				int  x,
				int  y,
				bool delete_flag
			);

			//! @}


	protected:
		// -------------------------------------------------
		//	interaction methods
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

			//! \brief handling mouse events for obstacle painting
			//! \param Qt's mouse event

			void mouseMoveEvent ( QMouseEvent* event );

			//! @}


		// -------------------------------------------------
		//	auxiliary functions
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

			//! \brief recalculates the minimum value and the scaling factor
			//! for the color scaling of the pressure values
			//! \param pointer to pressure array

		void rescaleColors ( REAL** P );

			//! \brief recalculates the minimum value and the scaling factor
			//! for the color scaling of the velocity values
			//! \param pointer to horizontal velocity array
			//! \param pointer to vertical velocity array

		void rescaleColors ( REAL** U, REAL** V );

				//! @}

		// -------------------------------------------------
		//	OpenGL functions
		// -------------------------------------------------
			//! @name OpenGL functions
			//! @{

			//! \brief catches resize events.
			//! Prevents OpenGL from resizing automatically, but stores the
			//! size information to do the resizing in the simulation/rendering thread
			//! \param Qt's resize event

		void resizeEvent ( QResizeEvent* event );

			//! \brief catches paint events.
			//! Prevents OpenGL from updating automatically. This is done in the
			//! simulation and rendering thread
			//! \param Qt's paint event

		void paintEvent  ( QPaintEvent* );



			//! @}
};

#endif // GLVIEWER_H
