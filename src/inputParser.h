#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <string>
#include <stdlib.h>
#include "Definitions.h"

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

	bool**		obstacleMap;	//! map defining the obstacle positions




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





//====================================================================
/*! \class InputParser
	\brief Class for input file parsing
*/
//====================================================================

class InputParser
{
	public:

		static void setDefaultParameters (
				Parameters*	parameters
			);

		static bool readParameters (
				int			argc,
				char*		argv[],
				Parameters*	parameters
			);

		// TODO: move check for valid map here!
		static bool readObstacleMap (
				bool***		obstacleMap,
				int			width,
				int			height,
				std::string	fileName
			);

		static void printParameters (
				Parameters*	parameters
			);
};

#endif // INPUTPARSER_H
