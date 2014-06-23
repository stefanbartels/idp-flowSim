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
	_viewer     = viewer;

	_clManager  = 0;

	_running    = false;

	_iterations = 0;
	_time       = 0.0;

	_elapsedSimulationTime = 0;
	_elapsedTotalTime      = 0;


	// TODO: stop using hardcoded flag
	if( parameters->useGPU )
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
	// let the viewer prepare something for visualization, if applicable
	_viewer->initialze();

	emit simulationStarted();

	// start performance measurement
	_totalTimer.start();

	while( _running && ( !_parameters->VTKWriteFiles || _time < _parameters->VTKTimeLimit ) )
	{
		#if VERBOSE
			std::cout << "Simulating iteration " << _iterations << " at time " << _time << std::endl;
		#endif

		_simulationTimer.start();

		// do simulation step
		int numPressureIterations = _solver->doSimulationStep( );

		// update simulation measurement
		_elapsedSimulationTime += _simulationTimer.elapsed();

		// update simulated time
		_time += _parameters->dt;

		// update visualisation
		// todo: use flow field class instead of parameters
		_viewer->renderFrame(
				_solver->getU_CPU(),
				_solver->getV_CPU(),
				_solver->getP_CPU(),
				_time,
				_iterations
			);

		emit simulatedFrame( numPressureIterations );

		++_iterations;
	}

	// update total time measurement
	_elapsedTotalTime += _totalTimer.elapsed();

	emit simulationStopped();
}

void Simulation::simulationTrigger()
{
	if( _running )
	{
		_running = false;
	}
	else
	{
		_running = true;
		start();
	}
}

//============================================================================
void Simulation::stopSimulation ( )
{
	_running = false;
}

//============================================================================
void Simulation::drawObstacles
	(
		int x0,
		int y0,
		int x1,
		int y1,
		bool delete_flag
	)
{
	_solver->drawObstacles( x0, y0, x1, y1, delete_flag );
}

// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
REAL** Simulation::getU_CPU ( )
{
	return _solver->getU_CPU();
}

//============================================================================
REAL** Simulation::getV_CPU ( )
{
	return _solver->getV_CPU();
}

//============================================================================
REAL** Simulation::getP_CPU ( )
{
	return _solver->getP_CPU();
}

//============================================================================
void Simulation::printPerformanceMeasurements ( )
{
	std::cout << "=======================\n"
			  << ( _parameters->useGPU ? "GPU\n" : "CPU\n" )
			  << "Iterations:                 " << _iterations << "\n"
			  << "Simulated time:             " << _time << "\n"
			  << "Elapsed time:               " << ((double)_elapsedTotalTime      / 1000) << " s\n"
			  << "    Simulation only:        " << ((double)_elapsedSimulationTime / 1000) << " s\n"
			  << "Avg. time per iteration:    " << ((double)_elapsedTotalTime      / _iterations) << " ms\n"
			  << "    Simulation only (avg):  " << ((double)_elapsedSimulationTime / _iterations) << " ms\n"
			  << "Iterations per second:      " << ((double)(_iterations * 1000)   / _elapsedTotalTime) << "\n"
			  << "=======================" << std::endl;
}
