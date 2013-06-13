//********************************************************************
//**    includes
//********************************************************************

#include <iostream>
#include <time.h>

#include "solver/navierStokesCPU.h"

//********************************************************************
//**    implementation
//********************************************************************

using namespace std;

int main()
{
	// declare variables

	int		n;		//! iteration number

	// read parameters

	// create gui, solver and viewer objects and pass parameters
	NavierStokesSolver* solver = new NavierStokesCPU();

	// link gui, solver, viewer

	// simulation/visualisation loop
	while ( true ) // TODO: add exit condition
	{
		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl

		++n;
	}

	// cleanup

    return 0;
}
