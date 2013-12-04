//********************************************************************
//**    includes
//********************************************************************

#include <iostream>

#include "viewer/SimplePGMWriter.h"
#include "viewer/VTKWriter.h"
#include "inputParser.h"

#if USE_GPU
	#include "solver/navierStokesGPU.h"
#else
	#include "solver/navierStokesCPU.h"
#endif

//********************************************************************
//**    implementation
//********************************************************************

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

		std::cout << "Problem parameter file: " << parameterFileName << std::endl;

		if ( !InputParser::readParameters ( &parameters, parameterFileName ) )
		{
			std::cerr << "Error reading parameter file." << std::endl << "Exiting..." << std:: endl;
			return 1;
		}
	}
	else
	{
		std::cerr << "No parameter file specified. Using default parameters." << std::endl;
	}

	// read obstacle map
	bool** obstacleMap = 0;

	if ( !InputParser::readObstacleMap( &obstacleMap, parameters.nx, parameters.ny, parameters.obstacleFile ) )
	{
		std::cerr << "Error reading obstacle map." << std::endl << "Exiting..." << std::endl;
		return 1;
	}

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	NavierStokesSolver* solver;

	#if USE_GPU
		std::cout << "Simulating on GPU" << std::endl;

		solver = new NavierStokesGPU();
	#else
		std::cout << "Simulating on CPU" << std::endl;

		solver = new NavierStokesCPU();
	#endif

	solver->setParameters ( &parameters );
	if( !solver->setObstacleMap( obstacleMap ) )
	{
		std::cerr << "Obstacle map invalid. Make sure there are no boundary cells between two fluid cells!" << std::endl
				  << "Exiting..." << std::endl;
		return 1;
	}

	Viewer* viewer = new VTKWriter();

	// todo: link gui, solver, viewer

	// todo: start gui in thread


	//-----------------------
	// simulation/visualisation loop
	//-----------------------

	solver->init();

	int n = 0;

	// plot initial state
	viewer->renderFrame(
			solver->getU_CPU(),
			solver->getV_CPU(),
			solver->getP_CPU(),
			parameters.nx,
			parameters.ny,
			n++
		);

	while ( n < 1000 )
	{
		//#if VERBOSE
			std::cout << "simulating frame " << n << std::endl;
		//#endif

		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl

		//#if VERBOSE
		//	std::cout << "visualizing frame " << n << std::endl;
		//#endif

		viewer->renderFrame(
				solver->getU_CPU(),
				solver->getV_CPU(),
				solver->getP_CPU(),
				parameters.nx,
				parameters.ny,
				n
			);

		++n;
	}

	//-----------------------
	// cleanup
	//-----------------------

	SAVE_DELETE( solver );
	SAVE_DELETE( viewer );

    return 0;
}
