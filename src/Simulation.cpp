//********************************************************************
//**    includes
//********************************************************************

#include "Simulation.h"
#include "solver/navierStokesCPU.h"
#include "solver/navierStokesGPU.h"
#include <iostream>

//********************************************************************
//**    implementation
//********************************************************************

//============================================================================
Simulation::Simulation ( Parameters* parameters, Viewer* viewer )
{
	_parameters = parameters;
	_viewer = viewer;

	_clManager = 0;

	_running = false;

	_iterations = 0;


	// TODO: stop using hardcoded flag
	if( USE_GPU )
	{
		std::cout << "Simulating on GPU" << std::endl;

		_clManager = new CLManager( parameters );

		_solver = new NavierStokesGPU( parameters, _clManager );
	}
	else
	{
		std::cout << "Simulating on CPU" << std::endl;

		_solver = new NavierStokesCPU( parameters );
	}

	// TODO: move check for valid obstacle map to inputParser
	if( !_solver->setObstacleMap( parameters->obstacleMap ) )
	{
		throw "Obstacle map invalid. Make sure there are no boundary cells between two fluid cells!";
	}

	_solver->initialize();
}

//============================================================================
Simulation::~Simulation ( )
{
	SAFE_DELETE( _solver );
	SAFE_DELETE( _clManager );
}

//============================================================================
void Simulation::run ( )
{
	_viewer->initialze();

	std::cout << "Simulate!" << std::endl;

	while( _running )
	{
		#if VERBOSE
			std::cout << "Simulating iteration " << _iterations << std::endl;
		#endif

		// do simulation step
		_solver->doSimulationStep( );

		// update visualisation
		// todo: use flow field class instead of parameters
		_viewer->renderFrame(
				_solver->getU_CPU(),
				_solver->getV_CPU(),
				_solver->getP_CPU(),
				_iterations
			);

		emit simulatedFrame();

		++_iterations;
	}
}

void Simulation::simulate()
{
	_running = true;
	start();
}

//============================================================================
void Simulation::stop ( )
{
	_running = false;
}

// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
REAL** Simulation::getU_CPU()
{
	return _solver->getU_CPU();
}

//============================================================================
REAL** Simulation::getV_CPU()
{
	return _solver->getV_CPU();
}

//============================================================================
REAL** Simulation::getP_CPU()
{
	return _solver->getP_CPU();
}
