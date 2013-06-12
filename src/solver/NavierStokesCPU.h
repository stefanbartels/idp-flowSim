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

		// time stepping data
		double	t0,			//! start time
				t,			//! current time value
				delt,		//! time step size
				tau;		//! safety factor for time step size control

		// pressure-iteration data
		int		it_max,	//! maximal number of pressure iterations per time step
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
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

		NavierStokesCPU();

			//! @}

		// -------------------------------------------------
		//	initialisation
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief initialises the arrays U, V and P

		void	init ( );

			//! @}

		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

		void doSimulationStep ( );

			//! @}

	protected:
		// -------------------------------------------------
		//	boundaries
		// -------------------------------------------------
			//! @name boundaries
			//! @{

			//!  \brief sets the boundary values for U and V depending on wN, wS, wW and wE

		void	setBoundaryConditions ( );

			//! \brief  TODO

		void	setSpecificBoundaryConditions ( );

		// -------------------------------------------------
		//	simulation
		// -------------------------------------------------
			//! @name simulation
			//! @{

			//! \brief calculates the stepsize for next time step

		void	computeDeltaT ( );

			//! \brief computes F and G

		void	computeFG ( );

			//! \brief computes the right-hand side of the pressure equation

		void	computeRightHandSide ( );

			//! \brief SOR iteration for pressure Poisson equation
			//! stores the residual in member variable residual
			//! \returns number of SOR iterations

		int		SORPoisson ( );

			//! \brief calculates new velocities

		void	adaptUV ( );

			//! @}


		// -------------------------------------------------
		//	helper functions
		// -------------------------------------------------
			//! @name helper functions
			//! @{

		//! \todo doublecheck for correctness
		double**	allocMatrix ( int width, int height );

		void		initMatrix ( double** matrix, int width, int height, double value );

		void		freeMatrix ( double** matrix );
};

#endif // NAVIERSTOKESCPU_H
