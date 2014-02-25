//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include <iostream>

#include "Simulation.h"
#include "viewer/SimplePGMWriter.h"
#include "viewer/VTKWriter.h"
#include "inputParser.h"


//********************************************************************
//**    implementation
//********************************************************************

int main ( int argc, char* argv[] )
{
	Parameters parameters;
	Simulation* simulation;

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
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	try
	{
		// TODO: move check for valid obstacle map to inputParser
		//       and remove this try/catch

		simulation = new Simulation( &parameters );
	}
	catch( const char* error_message )
	{
		std::cerr << "Error during simulation setup:\n " << error_message << "\nExiting..." << std::endl;
		return 1;
	}





	Viewer* viewer = new VTKWriter();

	// todo: link gui, solver, viewer

	// todo: start gui in thread


	//-----------------------
	// simulation/visualisation loop
	//-----------------------

	// plot initial state
	viewer->renderFrame(
			simulation->getU_CPU(),
			simulation->getV_CPU(),
			simulation->getP_CPU(),
			parameters.nx,
			parameters.ny,
			0
		);

	simulation->simulate( viewer );

	//-----------------------
	// cleanup
	//-----------------------

	SAVE_DELETE( simulation );
	SAVE_DELETE( viewer );

    return 0;
}
