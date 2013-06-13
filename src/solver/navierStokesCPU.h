#ifndef NAVIERSTOKESCPU_H
#define NAVIERSTOKESCPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesSolver.h"


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
		double	_xlength,	//! domain size in x-direction
				_ylength;	//! domain size in y-direction

		int		_nx,		//! number of interior cells in x-direction
				_ny;		//! number of interior cells in y-direction

		double	_dx,		//! length delta x of on cell in x-direction
				_dy;		//! length delta y of on cell in y-direction

		// time stepping data
		double	_t0,		//! start time
				_t,			//! current time value
				_dt,		//! time step size
				_tau;		//! safety factor for time step size control

		// pressure-iteration data
		int		_it_max;	//! maximal number of pressure iterations per time step
				//_it;		//! SOR iteration counter (-> local variable)

		double	_residual,	//! norm od pressure equation residual
				_epsilon,	//! stopping tolerance eps for pressure iteration
				_omega,		//! relaxation parameter for SOR iteration
				_gamma;		//! upwind differencing factor

		// problem dependent quantities
		double	_re,		//! Reynolds number Re
				_gx,		//! body force gx (e.g. gravity)
				_gy;		//! body force gy (e.g. gravity)

		double	_ui,		//! initial velocity in x-direction
				_vi,		//! initial velocity in y-direction
				_pi;		//! initial pressure

		int		_wN,		//! boundary condition along nothern boundary
				_wS,		//! boundary condition along southern boundary
				_wW,		//! boundary condition along western boundary
				_wE;		//! boundary condition along eastern boundary

							/*
							 * boundary conditions:
							 * 1 = free-slip
							 * 2 = no-slip
							 * 3 = outflow
							 * 4 = periodic
							 */

		char	_problem;	//! flow-specific quantities, depending on problem type

		// arrays
		double	**_U,		//! velocity in x-direction
				**_V,		//! velocity in y-direction
				**_P,		//! pressure
				**_RHS,		//! right-hand side for pressure iteration
				**_F,
				**_G;

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
			//! According to formula 3.50

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

		void		setMatrix ( double** matrix, int xStart, int xStop, int yStart, int yStop, double value );

		void		freeMatrix ( double** matrix );
};

#endif // NAVIERSTOKESCPU_H
