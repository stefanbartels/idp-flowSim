#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//********************************************************************
//**    includes
//********************************************************************

#include "../Parameters.h"
#include "../viewer/GLViewer.h"
#include "../viewer/Viewer.h"
#include <QMainWindow>
#include <QTime>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

//====================================================================
/*! \class MainWindow
	\brief The MainWindow is the main user interface class, providing
	the possibility to interact with the simulation
*/
//====================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		Parameters*  _parameters;	//! pointer to the set of simulation parameters
		GLViewer*    _viewer;		//! pointer to the OpenGL viewer

		// UI

		QWidget*     _ui;			//! main widget containing the GUI elements
		QVBoxLayout* _layout;		//! main layout

		QPushButton* _button_run;	//! start / pause button for simulation
		QLabel*      _label_info;	//! label displaying information like time/frame, fps, number of iterations, ...


		// frames / second
		QTime        _time;			//! time since the last FPS update
		int          _frames;		//! number of frames since the last FPS update

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct

		MainWindow ( Parameters* parameters );

		~MainWindow ( );

			//! @}

		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

			//! \brief gives acces to the viewer object
			//! \returns pointer to the OpenGL viewer

		GLViewer* getViewer ( );

			//! @}

	protected:
		void createUI ( );

	public slots:
		// -------------------------------------------------
		//	slots
		// -------------------------------------------------
			//! @name slots
			//! @{

			//! \brief resets the FPS counter and emits the simulationTrigger signal

		void simulationTriggerSlot ( );

			//! \brief should be called when the simulation stops

		void simulationStoppedSlot ( );

			//! \brief should be called when the simulation starts

		void simulationStartedSlot ( );

			//! \brief updates the UI after a timestep is simulated
			//! \param number of iterations used to solve the pressure equation

		void simulatedFrame ( int numPressureIterations );

			//! @}

	signals:
		// -------------------------------------------------
		//	signals
		// -------------------------------------------------
			//! @name signals
			//! @{

			//! \brief emitted when simulation should start or stop

		void simulationTrigger ( );

			//! @}
};

#endif // MAINWINDOW_H
