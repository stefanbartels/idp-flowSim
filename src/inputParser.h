#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <string>


struct ProblemParameters
{
	// geometry data
	double		xlength,	//! domain size in x-direction
				ylength;	//! domain size in y-direction

	int			nx,			//! number of interior cells in x-direction
				ny;			//! number of interior cells in y-direction

	// time stepping data
	double		dt,			//! time step size
				tau;		//! safety factor for time step size control

	// pressure-iteration data
	int			it_max;		//! maximal number of pressure iterations per time step

	double		epsilon,	//! stopping tolerance eps for pressure iteration
				omega,		//! relaxation parameter for SOR iteration
				gamma;		//! upwind differencing factor

	// problem dependent quantities
	double		re,			//! Reynolds number Re
				gx,			//! body force gx (e.g. gravity)
				gy;			//! body force gy (e.g. gravity)

	double		ui,			//! initial velocity in x-direction
				vi,			//! initial velocity in y-direction
				pi;			//! initial pressure

	int			wN,			//! boundary condition along northern boundary
				wS,			//! boundary condition along southern boundary
				wW,			//! boundary condition along western boundary
				wE;			//! boundary condition along eastern boundary

	std::string	problem;	//! problem type
};





//====================================================================
/*! \class InputParser
	\brief Class for input file parsing
*/
//====================================================================

class InputParser
{
	public:

		static bool readParameters (
				ProblemParameters*	parameters,
				std::string			fileName
			);

		static bool printParameters (
				ProblemParameters*	parameters
			);
};

#endif // INPUTPARSER_H
