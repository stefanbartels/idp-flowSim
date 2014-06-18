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
	//	program parameters
	// -------------------------------------------------
		//! @name program parameters
		//! @{

	bool         VTKWriteFiles;		//! indicates if vtk files should be written
	unsigned int VTKInterval;		//! interval of vtk outputs
	unsigned int VTKIterations;		//! number of iterations to simulate if vtk files are written

		//! @}

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
		// program parameters
		VTKWriteFiles = false;
		VTKInterval   = 100;
		VTKIterations = 5000;

		// simulation parameters
		xlength       = 1.0;
		ylength       = 1.0;
		nx            = 128;
		ny            = 128;
		dx            = 1.0 / 128.0;
		dy            = 1.0 / 128.0;
		dt            = 0.02;
		tau			  = 0.5;
		it_max        = 100;
		epsilon       = 0.001;
		omega         = 1.7;
		gamma         = 0.9;
		re            = 1000;
		gx            = 0.0;
		gy            = 0.0;
		ui            = 0.0;
		vi            = 0.0;
		pi            = 0.0;
		wN            = 2;
		wS            = 2;
		wW            = 2;
		wE            = 2;
		problem       = "moving_lid";
		obstacleFile  = "";
		obstacleMap   = 0;
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
