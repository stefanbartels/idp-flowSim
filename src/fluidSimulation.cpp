//********************************************************************
//**    includes
//********************************************************************

#include "fluidSimulation.h"
#include <iostream>

#include "solver/navierStokesCPU.h"
//#include "viewer/SimplePGMWriter.h"
#include "viewer/SimpleQTViewer.h"
#include "inputParser.h"


//********************************************************************
//**    implementation
//********************************************************************

using namespace std;

fluidSimulation::fluidSimulation()
{
}
fluidSimulation::fluidSimulation(SimpleQTViewer* view)
{
	viewer = view;
	//-----------------------
	// read parameters
	//-----------------------

	// set default problem parameters
	InputParser::setStandardParameters ( &parameters );


	char* parameterFileName= "karman_vortex.txt";

	cout << "\nProblem parameter file: " << parameterFileName;

	if ( !InputParser::readParameters ( &parameters, parameterFileName ) )
	{
		cerr << "\nError reading parameter file. Exiting...";
		return;
	}

	// read obstacle map

	bool** obstacleMap = 0;

	if ( !InputParser::readObstacleMap( &obstacleMap, parameters.nx, parameters.ny, parameters.obstacleFile ) )
	{
		cerr << "\nError reading obstacle map. Exiting...";
		return;
	}

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	solver = new NavierStokesCPU();
	solver->setParameters ( &parameters );
	solver->setObstacleMap( obstacleMap );

	//Viewer* viewer = new SimplePGMWriter();
	viewer->initializeParameters(&parameters);


}


void fluidSimulation::runSimulation()
{
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

    return;


}
