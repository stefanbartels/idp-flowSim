//********************************************************************
//**    includes
//********************************************************************

#include <iostream>
#include <time.h>
#include <string>

#include "solver/navierStokesCPU.h"
#include "inputParser.h"

//********************************************************************
//**    implementation
//********************************************************************

using namespace std;

int main ( int argc, char* argv[] )
{
	//-----------------------
	// read parameters
	//-----------------------

	// set default problem parameters
	ProblemParameters parameters;
	InputParser::setStandardParameters ( &parameters );

	char* parameterFileName;

	// parse command line arguments
	// only one until now: parameter file name

	// parameter file
	if ( argc > 1 )
	{
		parameterFileName = argv[1];

		cout << "\nProblem parameter file: " << parameterFileName;

		if ( !InputParser::readParameters ( &parameters, parameterFileName ) )
		{
			cerr << "\nError reading parameter file. Exiting...";
			return 1;
		}
	}
	else
	{
		cerr << "\nNo parameter file specified. Using default parameters.";
	}

	// todo: read obstacle map

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	NavierStokesSolver* solver = new NavierStokesCPU ( );
	solver->setParameters ( &parameters );

	// link gui, solver, viewer

	// start gui in thread


	//-----------------------
	// simulation/visualisation loop
	//-----------------------

	solver->init();

	int n = 0;

	while ( n < 10 )
	{
		cout << "\ndoing frame " << n;
		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl

		cout << "\ndone with frame " << n;
		++n;
	}

	//-----------------------
	// cleanup
	//-----------------------

	// todo

    return 0;
}
