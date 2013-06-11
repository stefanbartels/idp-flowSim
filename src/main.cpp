//********************************************************************
//**    includes
//********************************************************************

#include <iostream>
#include <time.h>

#include "solver/NavierStokesCPU.h"

//********************************************************************
//**    implementation
//********************************************************************

using namespace std;

int main()
{
	// declare variables

	double	t0,		//! start time
			t;		//! elapsed time

	int		n,		//! iteration number
			it,		//! SOR iteration number
			it_max;	//! max. number of SOR iterations


	// read parameters

	// create gui, solver and viewer objects and pass parameters
	NavierStokesSolver* solver = new NavierStokesCPU();

	// link gui, solver, viewer

	// simulation/visualisation loop
	while ( true ) // TODO: add exit condition
	{
		// do simulation step
			// get delta_t
			// set boundary values for u and v
			// compute F(n) and G(n)
			// compute right hand side of pressure equation
			// do SOR loop
			for ( it = 0; it < it_max; ++it ) // TODO: complete exit condition
			{
				// do SOR step
				// compute residual
			}
			// compute U(n+1) and V(n+1)

		// update visualisation


		++n;
	}

	// cleanup

    return 0;
}
