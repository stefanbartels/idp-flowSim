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
	QApplication application(argc, argv);
	application.setApplicationName("Interactive Navier Stokes Simulation");

	//-----------------------
	// read parameters
	//-----------------------

	// TODO: make parameter file not compulsory but loadable via gui

	// parse command line arguments and read parameter file
	if ( !InputParser::readParameters ( argc, argv, &parameters ) )
	{
		std::cerr << "Error reading parameter file." << std::endl << "Exiting..." << std:: endl;
		return 1;
	}

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create viewer and gui
	//-----------------------

	window = new MainWindow( &parameters );

	//viewer = window->getViewer();
	viewer = new VTKWriter( &parameters );

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

	QObject::connect(	window, SIGNAL( runSimulation() ),
						simulation, SLOT( simulate() ) );
	QObject::connect(	window, SIGNAL( stopSimulation() ),
						simulation, SLOT( stop() ) );

	window->show();

	//simulation->simulate();

	int application_return_value = application.exec();

	std::cout << "Return value: " << application_return_value << std::endl;

	//-----------------------
	// cleanup
	//-----------------------

	cleanup();

	return application_return_value;
}


void cleanup ( )
{
	SAVE_DELETE( simulation );
	SAVE_DELETE( window );
	SAVE_DELETE( viewer );
}
