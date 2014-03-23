#ifndef NAVIERSTOKESCPU_H
#define NAVIERSTOKESCPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesSolver.h"

//====================================================================
/*! \class NavierStokesCpu
	\brief Class for solving the Navier Stokes equations on CPU
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

			//! \param pointer to parameters struct

		NavierStokesCPU ( Parameters* parameters );

		~NavierStokesCPU ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief allocates and initialises simulation memory

		void	initialize ( );

			//! \brief takes the obstacle map and creates geometry information for each cell
			//! true stands for fluid cells and false for boundary cells
			//! Maps must have no obstacle cell between two fluid cells to be valid.
			//! An additional boundary will be applied.
			//! \param obstacle map (domain size)
			//! \returns true if the obstacle map is valid, false otherwise

		bool	setObstacleMap ( bool** map );

			//! @}


		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

			//! \brief simulates the next timestep
			//! \returns number of iterations used to solve the pressure equation

		int		doSimulationStep ( );

			//! @}


		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

			//! \brief gives access to the horizontal velocity component
			//! \returns pointer to horizontal velocity array

		REAL** getU_CPU ( );

			//! \brief gives access to the vertical velocity component
			//! \returns pointer to vertical velocity array

		REAL** getV_CPU ( );

			//! \brief gives access to the pressure
			//! \returns pointer to pressure array

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

		REAL	SORPoisson ( );

			//! \brief calculates new velocities

		void	adaptUV ( );

			//! @}


		// -------------------------------------------------
		//	auxiliary functions for F & G computation
		// -------------------------------------------------
			//! @name auxiliary functions for F and G computations
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
