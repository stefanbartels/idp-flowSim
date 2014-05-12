#ifndef NAVIERSTOKESGPU_H
#define NAVIERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesSolver.h"
#include "../CLManager.h"

//====================================================================
/*! \class NavierStokesGpu
	\brief Class for solving the Navier Stokes equations on GPU
	\todo optimize device memory, for example u,v and flag as constant memory
*/
//====================================================================

class NavierStokesGPU : public NavierStokesSolver
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		// GPU arrays
		cl::Buffer	_U_g,		//! velocity in x-direction
					_V_g,		//! velocity in y-direction
					_P_g,		//! pressure
					_RHS_g,		//! right-hand side for pressure iteration
					_F_g,
					_G_g,
					_FLAG_g;	//! obstacle map


		int			_pitch;		//! pitch for GPU memory

		// host arrays for data exchange
		REAL	**_U_host,		//! pointer to host memory for horizontal velocity
				**_V_host,		//! pointer to host memory for vertical velocity
				**_P_host;		//! pointer to host memory for pressure


		// OpenCL data
		CLManager*			_clManager;			//! pointer to the CL Manager
		cl::NDRange			_clRange;			//! range to use for kernels, size of the domain incl. boundaries
		int					_clWorkgroupSize;	//! maximum size of a work group

			// context and queue allow use of cl functions without extra methods in the manager
		cl::Context*		_clContext;			//! pointer to CL context
		cl::CommandQueue*	_clQueue;			//! pointer to CL queue

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct
			//! \param pointer to cl manager object

		NavierStokesGPU
			(
				Parameters* parameters,
				CLManager*  clManager
			);

		~NavierStokesGPU ( );

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
			//! The velocity is copied from device to host memory before returned.
			//! \returns pointer to horizontal velocity array

		REAL** getU_CPU ( );

			//! \brief gives access to the vertical velocity component
			//! The velocity is copied from device to host memory before returned.
			//! \returns pointer to vertical velocity array

		REAL** getV_CPU ( );

			//! \brief gives access to the pressure
			//! The pressure is copied from device to host memory before returned.
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
		//	auxiliary functions
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

			//! \brief loads and compiles all required kernels

		void	loadKernels ( );


			//! \brief sets kernel arguments for frequently called kernels

		void	setKernelArguments ( );

			//! @}
};

#endif // NAVIERSTOKESGPU_H
