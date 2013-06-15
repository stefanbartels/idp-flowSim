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

		// CPU arrays
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

		NavierStokesCPU ( );

		~NavierStokesCPU ( );

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

		void	doSimulationStep ( );

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
			//! \todo take obstacles into account

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
		//	F & G helper functions
		// -------------------------------------------------
			//! @name helper functions
			//! @{

		inline double d2m_dx2 ( double** M, int x, int y );
		inline double d2m_dy2 ( double** M, int x, int y );

		inline double du2_dx  ( int x, int y, double alpha );
		inline double dv2_dy  ( int x, int y, double alpha );

		inline double duv_dx  ( int x, int y, double alpha );
		inline double duv_dy  ( int x, int y, double alpha );

			//! @}








		// -------------------------------------------------
		//	helper functions
		// -------------------------------------------------
			//! @name helper functions
			//! @{

		//! \todo doublecheck for correctness
		double**	allocMatrix (
				int width,
				int height
			);

		void		setMatrix (
				double** matrix,
				int xStart,
				int xStop,
				int yStart,
				int yStop,
				double value
			);

		void		freeMatrix (
				double** matrix
			);
};

#endif // NAVIERSTOKESCPU_H
