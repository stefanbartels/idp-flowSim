#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "Definitions.h"
#include <string>
#include <stdlib.h>

struct Parameters
{
	// geometry data
	REAL		xlength,		//! domain size in x-direction
				ylength;		//! domain size in y-direction

	int			nx,				//! number of interior cells in x-direction
				ny;				//! number of interior cells in y-direction

	// time stepping data
	REAL		dt,				//! time step size
				tau;			//! safety factor for time step size control

	// pressure-iteration data
	int			it_max;			//! maximal number of pressure iterations per time step

	REAL		epsilon,		//! stopping tolerance eps for pressure iteration
				omega,			//! relaxation parameter for SOR iteration
				gamma;			//! upwind differencing factor

	// problem dependent quantities
	REAL		re,				//! Reynolds number Re
				gx,				//! body force gx (e.g. gravity)
				gy;				//! body force gy (e.g. gravity)

	REAL		ui,				//! initial velocity in x-direction
				vi,				//! initial velocity in y-direction
				pi;				//! initial pressure

	int			wN,				//! boundary condition along northern boundary
				wS,				//! boundary condition along southern boundary
				wW,				//! boundary condition along western boundary
				wE;				//! boundary condition along eastern boundary

	std::string	problem;		//! problem type

	std::string	obstacleFile;	//! obstacle map file name

	bool**		obstacleMap;	//! map defining the obstacle positions.




	// Constructor and destructor to free memory properly
	Parameters ( )
	{
		obstacleMap = 0;
	}

	~Parameters ( )
	{
		if( obstacleMap != 0)
		{
			free( obstacleMap[0] );
			free( obstacleMap );
		}
	}
};

#endif // PARAMETERS_H
