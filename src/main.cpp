//********************************************************************
//**    includes
//********************************************************************

#include <iostream>

#include "solver/navierStokesCPU.h"
#include "viewer/SimplePGMWriter.h"
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

	// read obstacle map

	bool** obstacleMap = 0;

	if ( !InputParser::readObstacleMap( &obstacleMap, parameters.nx, parameters.ny, parameters.obstacleFile ) )
	{
		cerr << "\nError reading obstacle map. Exiting...";
		return 1;
	}

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	NavierStokesSolver* solver = new NavierStokesCPU();
	solver->setParameters ( &parameters );
	solver->setObstacleMap( obstacleMap );

	Viewer* viewer = new SimplePGMWriter();

	// todo: link gui, solver, viewer

	// todo: start gui in thread


	//-----------------------
	// simulation/visualisation loop
	//-----------------------

	solver->init();

	int n = 0;

	while ( n < 1000 )
	{
		cout << "\ndoing frame " << n;
		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl

		viewer->renderFrame(
				solver->getU_CPU(),
				solver->getV_CPU(),
				solver->getP_CPU(),
				parameters.nx,
				parameters.ny,
				n
			);

		cout << "\ndone with frame " << n;
		++n;
	}

	//-----------------------
	// cleanup
	//-----------------------

	// todo

	delete solver;
	delete viewer;

    return 0;
}
