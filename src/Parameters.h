#ifndef PARAMETERS_H
#define PARAMETERS_H

//********************************************************************
//**    includes
//********************************************************************

#include "Definitions.h"
#include <string>
#include <stdlib.h>

/*
 * boundary conditions:
 * 1 = free-slip
 * 2 = no-slip
 * 3 = outflow
 * 4 = periodic
 */

//====================================================================
/*! \struct VTKWriter
	\brief Structure containing all parameters required for the
	simulation
*/
//====================================================================

struct Parameters
{
	// -------------------------------------------------
	//	simulation parameters
	// -------------------------------------------------
		//! @name simulation parameters
		//! @{

	// geometry data
	REAL		xlength,		//! domain size in x-direction
				ylength;		//! domain size in y-direction

	int			nx,				//! number of interior cells in x-direction
				ny;				//! number of interior cells in y-direction

	REAL		dx,				//! width of cells
				dy;				//! height of cells

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

	// TODO: use better names
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

		//! @}

	// -------------------------------------------------
	//	constructor / destructor
	// -------------------------------------------------
		//! @name constructor / destructor
		//! @{

	Parameters ( )
	{
		obstacleMap = 0;
	}

		//! \brief required to free obstacle memory properly

	~Parameters ( )
	{
		if( obstacleMap != 0)
		{
			free( obstacleMap[0] );
			free( obstacleMap );
		}
	}
		//! @}
};

#endif // PARAMETERS_H
