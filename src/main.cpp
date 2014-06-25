//********************************************************************
//**    includes
//********************************************************************

#include "Simulation.h"
#include "inputParser.h"
#include "ui/MainWindow.h"

//#include "viewer/SimplePGMWriter.h"
#include "viewer/VTKWriter.h"


#include <stdlib.h>
#include <iostream>
#include <QApplication>

//********************************************************************
//**    forward declarations
//********************************************************************

void cleanup ( );

//********************************************************************
//**    global variables
//********************************************************************

Parameters  parameters;
Simulation* simulation = 0;
Viewer*	    viewer     = 0;
MainWindow* window     = 0;

//********************************************************************
//**    implementation
//********************************************************************

/* TODO:
 *
 * - program started with fixed domain size and flow parameters,
 *   specified in config file
 *    => allow start without config file and with dynamic initialization
 */

int main ( int argc, char* argv[] )
{
	//-----------------------
	// read parameters
	//-----------------------

	// TODO: make parameter file not compulsory but loadable via gui

	// parse command line arguments and read parameter file
	if ( !InputParser::readParameters ( argc, argv, &parameters ) )
	{
		return 1;
	}

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create Qt application
	//-----------------------

	if( !parameters.VTKWriteFiles )
	{
		// support opengl in thread
		QCoreApplication::setAttribute( Qt::AA_X11InitThreads );
	}

	QApplication application( argc, argv, !parameters.VTKWriteFiles ); // third parameter: false => start without gui, so no x server is required
	application.setApplicationName( "Interactive Navier Stokes Simulation" );


	//-----------------------
	// create viewer and gui
	//-----------------------

	if( parameters.VTKWriteFiles )
	{
		viewer = new VTKWriter( &parameters );
	}
	else
	{
		// support opengl in thread
		QCoreApplication::setAttribute( Qt::AA_X11InitThreads );

		window = new MainWindow( &parameters );
		viewer = window->getViewer();
	}


	//-----------------------
	// create simulation
	//-----------------------

	try
	{
		// TODO: move check for valid obstacle map to inputParser
		//       and remove this try/catch

		simulation = new Simulation( &parameters, viewer );
	}
	catch( const char* error_message )
	{
		std::cerr << "Error during simulation setup:\n " << error_message << "\nExiting..." << std::endl;
		cleanup();
		return 1;
	}


	//-----------------------
	// start application
	//-----------------------

	if( parameters.VTKWriteFiles )
	{
		simulation->simulationTrigger();

		QObject::connect(	simulation, SIGNAL( simulationStopped() ),
							QApplication::instance(), SLOT( quit() ) );
	}
	else
	{
		// TODO: move connectins between gui elements and simulation somewhere else
		QObject::connect(	window, SIGNAL( simulationTrigger() ),
							simulation, SLOT( simulationTrigger() ) );

		QObject::connect(	simulation, SIGNAL( simulationStarted() ),
							window, SLOT( simulationStartedSlot() ) );
		QObject::connect(	simulation, SIGNAL( simulationStopped() ),
							window, SLOT( simulationStoppedSlot() ) );
		QObject::connect(	simulation, SIGNAL( simulatedFrame( int ) ),
							window, SLOT( simulatedFrame( int ) ) );

		// signal to stop simulation thread if application is stopped
		QObject::connect(	&application, SIGNAL( aboutToQuit() ),
							simulation, SLOT( stopSimulation() ) );

		// connect viewer and simulation for interactivity
		QObject::connect(	window->getViewer(), SIGNAL( drawObstacles( int, int, int, int, bool ) ),
							simulation, SLOT( drawObstacles( int, int, int, int, bool ) ) );

		// open window
		window->show();
	}


	//-----------------------
	// cleanup
	//-----------------------

	// catch return value, but wait for threads to finish
	int application_return_value = application.exec();


	#if VERBOSE
		std::cout << "Waiting for threads..." << std::endl;
	#endif

	// wait for simulation thread to finish properly
	simulation->wait();

	simulation->printPerformanceMeasurements();

	cleanup();

	return application_return_value;
}


void cleanup ( )
{
	SAFE_DELETE( simulation );
	SAFE_DELETE( window );
	if( parameters.VTKWriteFiles )
	{
		SAFE_DELETE( viewer ); // Qt takes care of that already
	}
}
