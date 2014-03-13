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
		REAL	**_U,		//! velocity in x-direction
				**_V,		//! velocity in y-direction
				**_P,		//! pressure
				**_RHS,		//! right-hand side for pressure iteration
				**_F,
				**_G;

		unsigned char **_FLAG;	//! obstacle map

			//! @}

    public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

		NavierStokesCPU ( Parameters* parameters );

		~NavierStokesCPU ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief initialises the arrays U, V and P

		void	initialize ( );

			//! \brief takes the obstacle map and creates geometry information for each cell
			//! \param obstacle map (domain size)
			//! an additional boundary will be applied

		bool	setObstacleMap ( bool** map );

			//! @}

		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

		void	doSimulationStep ( );

			//! @}


		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

		REAL** getU_CPU ( );

		REAL** getV_CPU ( );

		REAL** getP_CPU ( );

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

			//! @}

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

			//! \brief SOR iteration step for pressure Poisson equation
			//! \returns residual

		REAL	SORPoisson( );

			//! \brief calculates new velocities

		void	adaptUV ( );

			//! @}









		// -------------------------------------------------
		//	auxiliary functions for F & G
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

		inline REAL d2m_dx2 ( REAL** M, int x, int y );
		inline REAL d2m_dy2 ( REAL** M, int x, int y );

		inline REAL du2_dx  ( int x, int y, REAL alpha );
		inline REAL dv2_dy  ( int x, int y, REAL alpha );

		inline REAL duv_dx  ( int x, int y, REAL alpha );
		inline REAL duv_dy  ( int x, int y, REAL alpha );

			//! @}
};

#endif // NAVIERSTOKESCPU_H
