//********************************************************************
//**    includes
//********************************************************************

#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <stdio.h>

#include "solver/navierStokesCPU.h"
#include "inputParser.h"

//********************************************************************
//**    implementation
//********************************************************************

using namespace std;

int main()
{
	cout << "starting programm\n";

	//-----------------------
	// read parameters
	//-----------------------

	ProblemParameters parameters;
	if ( !InputParser::readParameters ( &parameters, "inputFile.dat" ) )
	{
		fprintf( stderr, "Error while reading parameter file.\n" );
		return 1;
	}

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
		cout << "doing frame " << n << "\n";
		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl

		cout << "done with frame " << n << "\n";
		++n;
	}

	//-----------------------
	// cleanup
	//-----------------------

    return 0;
}
