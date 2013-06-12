#ifndef NAVIERSTOKESCPU_H
#define NAVIERSTOKESCPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "NavierStokesSolver.h"


//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on CPU
*/
//====================================================================

class NavierStokesCPU : public NavierStokesSolver
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		// geometry data
		double	nx,			//! domain size in x-direction
				ny;			//! domain size in y-direction

		int		imax,		//! number of interior cells in x-direction
				jmax;		//! number of interior cells in y-direction

		double	delx,		//! length delta x of on cell in x-direction
				dely;		//! length delta y of on cell in y-direction

		// pressure-iteration data
		int		itermax,	//! maximal number of pressure iterations per time step
				it;			//! SOR iteration counter

		double	residual,	//! norm od pressure equation residual
				epsilon,	//! stopping tolerance eps for pressure iteration
				omega,		//! relaxation parameter for SOR iteration
				gamma;		//! upwind differencing factor

		// problem dependent quantities
		double	re,			//! Reynolds number Re
				gx,			//! body force gx (e.g. gravity)
				gy;			//! body force gy (e.g. gravity)

		double	ui,			//! initial velocity in x-direction
				vi,			//! initial velocity in y-direction
				pi;			//! initial pressure

		int		wN,			//! boundary condition along nothern boundary
				wS,			//! boundary condition along southern boundary
				wW,			//! boundary condition along western boundary
				wE;			//! boundary condition along eastern boundary

							/*
							 * boundary conditions:
							 * 1 = free-slip
							 * 2 = no-slip
							 * 3 = outflow
							 * 4 = periodic
							 */

		char	problem;	//! flow-specific quantities, depending on problem type

		// arrays
		double	**U,		//! velocity in x-direction
				**V,		//! velocity in y-direction
				**P,		//! pressure
				**RHS,		//! right-hand side for pressure iteration
				**F,
				**G;

			//! @}

    public:
		// -------------------------------------------------
		//	constructor/destructor
		// -------------------------------------------------
		//! @name constructor/destructor
			//! @{

		NavierStokesCPU();

			//! @}
		
};

#endif // NAVIERSTOKESCPU_H
